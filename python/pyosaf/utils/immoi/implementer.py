############################################################################
#
# (C) Copyright 2015 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Ericsson
#
############################################################################
'''
    Class representing an IMM Object Implementer.
'''

import select

from pyosaf import saImm, saImmOm, saImmOi, saAis

from pyosaf.saAis import eSaAisErrorT

from pyosaf.saImm import eSaImmValueTypeT, eSaImmAttrModificationTypeT, \
    eSaImmClassCategoryT

from pyosaf.utils import immom, immoi, SafException

from pyosaf.utils.immom.object import ImmObject

from pyosaf.utils.immom.ccb import marshal_c_array

implementer_instance = None

# Cache CCBs
completed_ccbs = {}
ccbs           = {}

def _collect_full_transaction(ccb_id):
    ''' Goes through a completed CCB and summarizes the full transaction as
        seen by the OI.

        Returns:

        {'instances_after' : instances,
         'created'         : created,
         'deleted'         : deleted,
         'updated'         : updated}
    '''

    # Collect current state
    all_objects_now = []

    created = []
    deleted = []
    updated = []

    # Go through current instances
    for class_name in implementer_instance.class_names:
        dns = immoi.get_object_names_for_class(class_name)
        for dn in dns:
            obj = immoi.get_object_no_runtime(dn, class_name)

            all_objects_now.append(obj)

    # Collect proposed state by applying changes on current state
    for operation in ccbs[ccb_id]:

        operation_type = operation['type']

        # Handle creates
        if operation_type == 'CREATE':
            parent     = operation['parent']
            class_name = operation['className']
            attributes = operation['attributes']

            instance = immoi.create_non_existing_imm_object(class_name, 
                                                            parent, attributes)

            created.append(instance)

            deleted = filter(lambda i: i != dn, deleted)

        # Handle deletes
        elif operation_type == 'DELETE':
            dn = operation['dn']

            deleted.append(dn)

            created = filter(lambda i: i.dn != dn, created)
            updated = filter(lambda i: i.dn != dn, updated)

        # Handle modify operations
        elif operation_type == 'MODIFY':
            dn            = operation['dn']
            modifications = operation['modification']

            for attr_modification in modifications: 

                mod_type  = attr_modification['modification']
                attribute = attr_modification['attribute']
                values    = attr_modification['values']

                # Find affected object
                affected_instance = None

                affected_instances = filter(lambda i: i.dn == dn, all_objects_now)

                if len(affected_instances) == 0:
                    print 'ERROR: Failed to find object %s affected by modify operation' % dn
                else:
                    affected_instance = affected_instances[0]

                    if not affected_instance in updated:
                        updated.append(affected_instance)

                if mod_type == eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_ADD:

                    curr_value = affected_instance.__getattr__(attribute)

                    curr_value.append(values)

                    affected_instance.__setattr__(attribute, curr_value)
                
                elif mod_type == eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_DELETE:
                    for value in values:

                        curr_value = affected_instance.__getattr__(attribute)

                        curr_value.remove(value)

                        affected_instance.__setattr__(attribute, curr_value)
                    
                elif mod_type == eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_REPLACE:
                    affected_instance.__setattr__(attribute, values)

    # Return the summary
    instances_after = all_objects_now + created
    instances_after = filter(lambda i: not i.dn in deleted, instances_after)

    out = {'instances_after' : instances_after,
           'created'         : created,
           'deleted'         : deleted,
           'updated'         : updated}

    return out

# Set up callbacks
def admin_operation(oi_handle, c_invocation_id, c_name, c_operation_id, c_params):
    ''' Callback for administrative operations '''

    # Unmarshal parameters
    invocation_id = c_invocation_id
    name          = saImm.unmarshalSaImmValue(c_name, 
                                              eSaImmValueTypeT.SA_IMM_ATTR_SANAMET)
    operation_id  = c_operation_id

    params = []

    for p in saAis.unmarshalNullArray(c_params):
        paramName   = p.paramName
        paramType   = p.paramType
        paramBuffer = p.paramBuffer

        value = saImm.unmarshalSaImmValue(paramBuffer, paramType)

        params.append(value)

    # Invoke the operation
    result = implementer_instance.admin_operation(operation_id, name, params)

    # Report the result
    try:
        immoi.report_admin_operation_result(invocation_id, result)
    except SafException as err:
        print "ERROR: Failed to report that %s::%s returned %s" % \
            (name, invocation_id, result)

def abort_ccb(oi_handle, ccb_id):
    ''' Callback for aborted CCBs. 

        Removes the given CCB from the cache.
    '''

    del ccbs[ccb_id]

def apply_ccb(oi_handle, ccb_id):
    ''' Callback for apply of CCBs '''

    all_instances = []

    for class_name in implementer_instance.class_names:
        dns = immoi.get_object_names_for_class(class_name)

        for dn in dns:
            obj = immoi.get_object_no_runtime(dn, class_name)

            all_instances.append(obj)

    updated = completed_ccbs[ccb_id]['updated']
    added   = completed_ccbs[ccb_id]['added']
    removed = completed_ccbs[ccb_id]['removed']

    # Remove the CCB from the caches
    del ccbs[ccb_id]
    del completed_ccbs[ccb_id]

    # Tell the implementer to apply the changes
    return implementer_instance.on_apply(all_instances, updated, added, removed)

def attr_update(oi_handle, c_name, c_attr_names):
    ''' Callback for attribute update calls from IMM '''

    # Unmarshall parameters
    name       = saImm.unmarshalSaImmValue(c_name, 
                                     eSaImmValueTypeT.SA_IMM_ATTR_SANAMET)
    attr_names = saAis.unmarshalNullArray(c_attr_names)

    # Get the class of the object
    class_name = immoi.get_class_name_for_dn(name)

    # Get the values from the user and report back
    attributes = {}

    for attr_name in attr_names:
        values = implementer_instance.on_runtime_values_get(name, class_name, 
                                                            attr_name)

        if values is None:
            return eSaAisErrorT.SA_AIS_ERR_UNAVAILABLE

        if not isinstance(values, list):
            values = [values]

        attributes[attr_name] = values

    # Report the updated values for the attributes
    try:
        immoi.update_rt_object(name, attributes)
        return eSaAisErrorT.SA_AIS_OK
    except SafException as err:
        return eSaAisErrorT.SA_AIS_ERR_FAILED_OPERATION

def delete_added(oi_handle, ccb_id, c_name):
    ''' Callback for object delete '''

    # Unmarshall the parameters
    name = saImm.unmarshalSaImmValue(c_name, 
                                     eSaImmValueTypeT.SA_IMM_ATTR_SANAMET)

    # Create a new CCB in the cache if needed
    if not ccb_id in ccbs.keys():
        ccbs[ccb_id] = []

    # Cache the operation
    ccbs[ccb_id].append({'type' : 'DELETE', 
                         'dn'   : name})

    # Tell the implementer about the operation
    return implementer_instance.on_delete_added(name)

def modify_added(oi_handle, c_ccb_id, c_name, c_attr_modification):
    ''' Callback for object modify '''

    # Unmarshal the parameters
    name   = saImm.unmarshalSaImmValue(c_name, 
                                       eSaImmValueTypeT.SA_IMM_ATTR_SANAMET)
    ccb_id = c_ccb_id

    attribute_modifications = []

    implementer_objection = None

    for a in saAis.unmarshalNullArray(c_attr_modification):
        attr_name  = a.modAttr.attrName
        attr_type  = a.modAttr.attrValueType
        mod_type   = a.modType
        nr_values  = a.modAttr.attrValuesNumber
        attrValues = immoi.unmarshalLenArray(a.modAttr.attrValues, 
                                             a.modAttr.attrValuesNumber,
                                             a.modAttr.attrValueType)

        attribute_modifications.append({'attribute'    : attr_name, 
                                        'type'         : attr_type, 
                                        'modification' : mod_type, 
                                        'values'       : attrValues})

        # Tell the implementer about the modification
        result = implementer_instance.on_modify_added(attr_name, mod_type, 
                                                      attrValues)

        if result != eSaAisErrorT.SA_AIS_OK:
            implementer_objected = result

    # Create a new CCB in the cache if needed
    if not ccb_id in ccbs.keys():
        ccbs[ccb_id] = []

    # Store the modifications in the cache
    ccbs[ccb_id].append({'type'         : 'MODIFY', 
                         'dn'           : name, 
                         'modification' : attribute_modifications})

    # Respond and say if this is accepted by the implementer
    if implementer_objection:
        return implementer_objection
    else:
        return eSaAisErrorT.SA_AIS_OK


def create_added(oi_handle, c_ccb_id, c_class_name, c_parent, c_attr_values):
    ''' Callback for object create '''

    # Unmarshal parameters
    parent     = saImm.unmarshalSaImmValue(c_parent, 
                                           eSaImmValueTypeT.SA_IMM_ATTR_SANAMET)
    class_name = c_class_name
    ccb_id     = c_ccb_id

    attributes = {}

    for a in saAis.unmarshalNullArray(c_attr_values):
        attr_name  = a.attrName
        attr_type  = a.attrValueType
        nr_values  = a.attrValuesNumber

        attr_values = immoi.unmarshalLenArray(a.attrValues, nr_values, 
                                              attr_type)

        if len(attr_values) > 0:
            attributes[attr_name] = attr_values
        else:
            attributes[attr_name] = None

    # Fill in any missing attributes
    description = immom.class_description_get(class_name)

    for attribute in description:

        if not attribute.attrName in attributes:
            attributes[attribute.attrName] = None

    # Create a new CCB in the cache if needed
    if not ccb_id in ccbs.keys():
        ccbs[ccb_id] = []

    # Cache the create operation
    ccbs[ccb_id].append({'type'       : 'CREATE', 
                         'parent'     : parent, 
                         'className'  : class_name, 
                         'attributes' : attributes})

    # Tell the implementer about the operation
    obj = immoi.create_non_existing_imm_object(class_name, parent, attributes)

    return implementer_instance.on_create_added(class_name, parent, obj)


def completed_ccb(oi_handle, ccb_id):
    ''' Callback for CCB completed

        Validates any configured containments and calls the configured 
        on_validate function 
    '''

    # Get a summary of the changes in the CCB
    summary = _collect_full_transaction(ccb_id)

    instances = summary['instances_after']

    created = summary['created']
    deleted = summary['deleted']
    updated = summary['updated']

    # Store added, removed, updated for apply
    completed_ccbs[ccb_id] = {'added'   : created,
                              'removed' : deleted,
                              'updated' : updated}

    # Perform validation on the full transaction
    return implementer_instance._validate(ccb_id, instances, updated, 
                                          created, deleted)


# OI callbacks
callbacks = saImmOi.SaImmOiCallbacksT_2()

callbacks.saImmOiCcbAbortCallback        = \
    saImmOi.SaImmOiCcbAbortCallbackT(abort_ccb)
callbacks.saImmOiCcbApplyCallback        = \
    saImmOi.SaImmOiCcbApplyCallbackT(apply_ccb)
callbacks.saImmOiCcbCompletedCallback    = \
    saImmOi.SaImmOiCcbCompletedCallbackT(completed_ccb)
callbacks.saImmOiCcbObjectCreateCallback = \
    saImmOi.SaImmOiCcbObjectCreateCallbackT_2(create_added)
callbacks.saImmOiCcbObjectDeleteCallback = \
    saImmOi.SaImmOiCcbObjectDeleteCallbackT(delete_added)
callbacks.saImmOiCcbObjectModifyCallback = \
    saImmOi.SaImmOiCcbObjectModifyCallbackT_2(modify_added)
callbacks.saImmOiRtAttrUpdateCallback    = \
    saImmOi.SaImmOiRtAttrUpdateCallbackT(attr_update)
callbacks.saImmOiAdminOperationCallback  = \
    saImmOi.SaImmOiAdminOperationCallbackT_2(admin_operation)


def AdminOperation(class_name, op_id):
    ''' Admin operation decorator, marks and returns the function with the
        specified operation id and returns the function
    '''

    def inner_admin_op_decorator(func):
        ''' Inner decorator which actually sets the admin_op id '''
        
        setattr(func, 'AdminOperationOperationId', op_id)
        setattr(func, 'AdminOperationClassName',   class_name)
        return func

    return inner_admin_op_decorator

class AdminOperationFunction:
    ''' Encapsulation of an admin operation and its id '''

    def __init__(self, class_name, operation_id, func):
        ''' Creates a pair of an operation id and a function '''

        self.operation_id = operation_id
        self.class_name   = class_name
        self.func         = func

    def execute(self, name, parameters):
        ''' Executes the admin operation '''

        return self.func(name, parameters)

    def matches(self, class_name, operation_id):
        ''' Returns true if this admin operation pair matches the given
            admin operation id
        '''

        return operation_id == self.operation_id and \
               class_name == self.class_name


class _ContainmentConstraint:
    ''' Defines a containment constraint '''

    def __init__(self, parent_class, child_class, lower, upper):
        ''' 
            Creates a containment constraint with optional lower and upper 
            cardinality.
        '''
        self.parent_class = parent_class
        self.child_class  = child_class
        self.lower        = lower
        self.upper        = upper


class Constraints:
    ''' Defines constraints for changes to the instances implemented by the OI '''

    def __init__(self):
        ''' Creates an empty Constraints instance '''
        self.containments = {}
        self.cardinality  = {}

    def add_allowed_containment(self, parent_class, child_class, lower=None, upper=None):
        ''' Adds a constraint on which type of classes can be created 
            under the parent 
        '''

        # Store the allowed parent-child relationship
        if not parent_class in self.containments:
            self.containments[parent_class] = []

        self.containments[parent_class] = [child_class]

        # Store the cardinality
        pair = (parent_class, child_class)

        self.cardinality[pair] = [lower, upper]

    def validate(self, all_instances, updated, created, deleted):
        ''' Validates the constraints in this Constraints instance '''

        # Validate containments
        for mo in created:

            parent_name = immoi.get_parent_name_for_dn(mo.dn)

            # Handle the case where there is no parent
            if not parent_name:

                for parent_class, child_classes in self.containments.iteritems():
                    if mo.SaImmAttrClassName in child_classes:
                        error_string = "ERROR: Cannot create %s, %s must have a parent" % \
                                       (mo.dn, mo.SaImmAttrClassName)
                        raise SafException(eSaAisErrorT.SA_AIS_ERR_INVALID_PARAM, 
                                           error_string)

            else:

                parent_class = immoi.get_class_name_for_dn(parent_name)

                child_classes = self.containments[parent_class]

                if not mo.SaImmAttrClassName in child_classes:
                    error_string = "ERROR: Cannot create %s as a child under %s" % \
                                   (mo.dn, parent_class)
                    raise SafException(eSaAisErrorT.SA_AIS_ERR_INVALID_PARAM,
                                       error_string)

        # Validate cardinality of containments
        count_cardinality = {}

        for mo in all_instances:

            parent_name = immoi.get_parent_name_for_dn(mo.dn)

            if parent_name:

                parent_class = immoi.get_class_name_for_dn(parent_name)

                # Count children of each parent-child relationship
                pair = (parent_class, mo.SaImmAttrClassName)

                if not pair in count_cardinality:
                    count_cardinality[pair] = 0

                count_cardinality[pair] += 1

        # Verify the cardinality
        for counted_pair, number in count_cardinality.iteritems():
            parent_class = counted_pair[0]
            child_class  = counted_pair[1]

            # Ignore pairs that have no specified cardinality
            if not counted_pair in self.cardinality:
                continue

            lower = self.cardinality[counted_pair][0]
            upper = self.cardinality[counted_pair][1]

            if lower and number < lower:
                error_string = "ERROR: Must have at least %s instances of %s under %s" % \
                               (lower, child_class, parent_class)
                raise SafException(eSaAisErrorT.SA_AIS_ERR_INVALID_PARAM,
                                   error_string)
            elif upper and number > upper:
                error_string = "ERROR: Must have at most %s instances of %s under %s" % \
                               (upper, child_class, parent_class)
                raise SafException(eSaAisErrorT.SA_AIS_ERR_INVALID_PARAM,
                                   error_string)


class Implementer:

    def __init__(self, class_names=[], name="wrapper", on_create=None, 
                 on_delete=None, on_modify=None, on_validate=None, 
                 on_apply=None, on_runtime_values_get=None, 
                 admin_operations=None, constraints=None):
        ''' Creates an Implementer instance '''

        self.class_names              = class_names
        self.name                     = name
        self.on_create_cb             = on_create
        self.on_delete_cb             = on_delete
        self.on_modify_cb             = on_modify
        self.on_validate_cb           = on_validate
        self.on_apply_cb              = on_apply
        self.on_runtime_values_get_cb = on_runtime_values_get
        self.admin_operations         = admin_operations
        self.constraints              = constraints

        global implementer_instance

        implementer_instance = self

        # Initialize OI API and register as implementer for the classes
        immoi.cache_class_descriptions(self.class_names)

        self._register()

    def get_implemented_classes(self):
        ''' Returns a list of the classes this implementer implements '''
        return self.class_names

    def implement_class(self, class_name):
        ''' Adds the the given class_name to the list of classes this
            implementer implements '''
        immoi.implement_class(class_name)

        self.class_names.append(class_name)

    def set_constraints(self, constraints):
        ''' Sets constraints to be verified by the OI '''
        self.constraints = constraints

    def set_admin_operations(self, admin_operations):
        ''' Sets the admin operations to be executed by the OI '''
        self.admin_operations = admin_operations

    def on_runtime_values_get(self, name, class_name, attribute_name):
        ''' Retrieves values for the requested attribute in the given 
            instance
        '''

        if self.on_runtime_values_get_cb:
            try:
                return self.on_runtime_values_get_cb(name, class_name, attribute_name)
            except SafException as err:
                return err.value
        else:
            return None

    def on_modify_added(self, attribute_name, modification_type, values):
        ''' Called when an object modify operation has been added to an
            ongoing CCB.

            Will call the on_modify parameter if it has been set. This method
            can also be overridden by a subclass that wants to handle 
            on_modify_added'''

        if self.on_modify_cb:
            try:
                self.on_modify_cb(attribute_name, modification_type, values)
            except SafException as err:
                return err.value

        return eSaAisErrorT.SA_AIS_OK

    def on_delete_added(self, dn):
        ''' Called when an object delete operation has been added to the
            active CCB.

            Will call the on_delete parameter if it has been set. This method 
            can also be overridden by a subclass that wants to handle 
            on_delete_added
        '''

        if self.on_delete_cb:
            try:
                self.on_delete_cb(dn)
            except SafException as err:
                return err.value

        return eSaAisErrorT.SA_AIS_OK

    def on_create_added(self, class_name, parent_name, obj):
        ''' Called when an object create operation has been performed.

            Will call the on_create parameter if it has been set. This
            method can also be overridden by a subclass that wants to 
            handle on_create_added
        '''

        if self.on_create_cb:
            try:
                self.on_create_cb(class_name, parent_name, obj)
            except SafException as err:
                return err.value

        return eSaAisErrorT.SA_AIS_OK


    def on_apply(self, instances, updated, created, deleted):
        ''' Calls the registered on_apply callback function if it exists.

            Another way to perform apply is to override this method'''
        if self.on_apply_cb:
            try:
                self.on_apply_cb(instances, updated, created, deleted)
            except SafException as err:
                return err.value

        return eSaAisErrorT.SA_AIS_OK

    def on_validate(self, instances, updated, created, deleted):
        ''' Calls the registered on_validate callback function if it exists.

            Another way to perform validation is to override this method.
         '''

        if self.on_validate_cb:
            self.on_validate_cb(instances, updated, created, deleted)

    def _validate(self, ccb_id, instances, updated, created, deleted):
        ''' Performs any configured validation code '''

        try:

            # Validate constraints on the containments (if configured)
            self.__validate_constraints(instances, updated, created, 
                                                 deleted)

            # Let the user code validate the CCB (if configured)
            self.on_validate(instances, updated, created, deleted)
        except SafException as err:
            immoi.set_error_string(ccb_id, err.msg)
            return err.value
        except:
            return eSaAisErrorT.SA_AIS_ERR_FAILED_OPERATION

        return eSaAisErrorT.SA_AIS_OK


    def enter_dispatch_loop(self):
        ''' Activates the OI and starts an infinite dispatch loop
        '''
        self.__start_dispatch_loop()

    def admin_operation(self, operation_id, object_name, parameters):
        ''' Executes the specified admin operation and returns the result

            Returns eSaAisErrorT.SA_AIS_ERR_NOT_SUPPORTED if no corresponding
            operation is registered
        '''

        # Get the class name
        class_name = immoi.get_class_name_for_dn(object_name)

        # Find and execute a matching admin operation
        if self.admin_operations:
            for admin_operation in self.admin_operations:
                if admin_operation.matches(class_name, operation_id):
                    try:
                        admin_operation.execute(object_name, parameters)
                        return eSaAisErrorT.SA_AIS_OK
                    except SafException as err:
                        print "ERROR: Admin operation %s caused exception %s" %\
                            (operation_id, err)
                        return err.value

        # Scan for AdminOperation-decorated functions in subclasses
        for member_name in dir(self):

            func = getattr(self, member_name)

            tmp_id         = getattr(func, 'AdminOperationOperationId', None)
            tmp_class_name = getattr(func, 'AdminOperationClassName', None)

            if tmp_id == operation_id and tmp_class_name == class_name:
                try:
                    func(object_name, parameters)
                    return eSaAisErrorT.SA_AIS_OK
                except SafException as err:
                    print "ERROR: Admin operation %s caused exception %s" % \
                        (operation_id, err)
                    return err.value

        # Report that the operation is not supported
        return eSaAisErrorT.SA_AIS_ERR_NOT_SUPPORTED

    def _register(self):
        ''' Initializes IMM OI and registers as an OI for the configured 
            classes 
        '''

        # Initialize the OI API
        immoi._initialize(callbacks)

        # Ensure that all classes are configuration classes
        runtime_classes = filter(lambda c: immoi.get_class_category(c) == \
                                 eSaImmClassCategoryT.SA_IMM_CLASS_RUNTIME, self.class_names)
        if  runtime_classes:
            raise Exception("ERROR: Can't be an applier for runtime "
                            "classes %s" % runtime_classes)

        # Become an implementer
        immoi.register_implementer(self.name)

        # Get the selection objects
        immoi.get_selection_object()

        available_classes = immoi.get_available_classes_in_imm()

        for class_name in self.class_names:

            if class_name in available_classes:
                immoi.implement_class(class_name)

            else:
                print "WARNING: %s is missing in IMM. Not becoming implementer." % class_name

    def get_selection_object(self):
        ''' Returns the selection object '''
        return immoi.SELECTION_OBJECT.value

    def update_runtime_attributes(self, dn, attributes):
        ''' Updates the given runtime attributes for the specified DN '''

        # Report the updates
        try:
            immoi.update_rt_object(dn, attributes)
        except SafException as err:
            print "ERROR: Failed to update runtime attributes of %s: %s" % \
                (dn, err)

    def create(self, obj):

        # Get the parent name for the object
        parent_name = immoi.get_parent_name_for_dn(obj.dn)
        class_name  = obj.class_name

        # Create the object
        immoi.create_rt_object(class_name, parent_name, obj)

    def delete(self, dn):
        ''' Deletes a runtime object with the given DN '''

        immoi.delete_rt_object(dn)

    def __start_dispatch_loop(self):
        ''' Starts an infinite dispatch loop '''

        inputs  = [immoi.SELECTION_OBJECT.value]
        outputs = []

        # Handle updates
        while inputs:

            readable, writable, exceptional = select.select(inputs, outputs, 
                                                            inputs)

            immoi.dispatch()

    def __validate_constraints(self, all_instances, updated, created, 
                              deleted):
        ''' Validates configured constraints '''
        if self.constraints:
            return self.constraints.validate(all_instances, updated, 
                                             created, deleted)
        else:
            return eSaAisErrorT.SA_AIS_OK


class Applier(Implementer):
    ''' Class representing an applier '''

    def __init__(self, class_names, name="wrapper", on_create=None, 
                 on_delete=None, on_modify=None, on_apply=None):
        ''' Creates an Applier instance and runs the Implementer superclass'
            __init__ method
        '''

        # Initialize the base class
        Implementer.__init__(self, class_names=class_names, name=name, 
                             on_create=on_create, on_delete=on_delete,
                             on_modify=on_modify, on_apply=on_apply)
        
    def _validate(self, instances, updated, created, deleted):
        ''' Empty validate handler as appliers cannot validate '''
        return eSaAisErrorT.SA_AIS_OK

    def _register(self):
        ''' Initializes IMM OI and registers as an applier for the configured 
            classes 
        '''

        # Initialize the OI API
        immoi._initialize(callbacks)

        # Ensure that all classes are configuration classes
        runtime_classes = filter(lambda c: 
                                 immoi.get_class_category(c) == \
                                 eSaImmClassCategoryT.SA_IMM_CLASS_RUNTIME, self.class_names)
        if runtime_classes:
            raise Exception("ERROR: Can't be an applier for runtime "
                            "classes %s" % runtime_classes)

        # Become an applier
        immoi.register_applier(self.name)

        # Get the selection objects
        immoi.get_selection_object()

        # Register as applier for each class
        available_classes = immoi.get_available_classes_in_imm()

        for class_name in self.class_names:

            if class_name in available_classes:
                immoi.implement_class(class_name)

            else:
                print "WARNING: %s is missing in IMM. Not becoming applier." % \
                    class_name
