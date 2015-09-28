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
    IMM OI common utilitites
'''

import time

from pyosaf.utils import immom

from pyosaf import saAis, saImm, saImmOi

from pyosaf.saAis import SaVersionT, SaNameT, SaSelectionObjectT, \
    unmarshalSaStringTArray, eSaDispatchFlagsT

from pyosaf.saImm import unmarshalSaImmValue, SaImmAttrNameT,      \
    SaImmAttrValuesT_2, SaImmClassNameT, SaImmSearchParametersT_2, \
    eSaImmValueTypeT, SaImmAttrDefinitionT_2, SaImmClassCategoryT, \
    SaImmAttrModificationT_2, eSaImmAttrModificationTypeT

from pyosaf.saImmOi import SaImmOiHandleT, SaImmOiImplementerNameT

from pyosaf.utils.immom.object import ImmObject
from pyosaf.utils.immom.ccb import marshal_c_array
from pyosaf.utils.immom.iterator import SearchIterator

from pyosaf.utils import decorate

from ctypes import c_char_p, c_void_p, cast, pointer

SELECTION_OBJECT = SaSelectionObjectT()
HANDLE           = SaImmOiHandleT()

TRYAGAIN_CNT = 60

OPENSAF_IMM_OBJECT = "opensafImm=opensafImm,safApp=safImmService"

# Decorate the raw saImmOi* functions with retry and raising exceptions
saImmOiInitialize_2       = decorate(saImmOi.saImmOiInitialize_2)
saImmOiSelectionObjectGet = decorate(saImmOi.saImmOiSelectionObjectGet)
saImmOiDispatch           = decorate(saImmOi.saImmOiDispatch)
saImmOiFinalize           = decorate(saImmOi.saImmOiFinalize)
saImmOiImplementerSet     = decorate(saImmOi.saImmOiImplementerSet)
saImmOiImplementerClear   = decorate(saImmOi.saImmOiImplementerClear)
saImmOiClassImplementerSet = decorate(saImmOi.saImmOiClassImplementerSet)
saImmOiClassImplementerRelease = decorate(saImmOi.saImmOiClassImplementerRelease)
saImmOiObjectImplementerSet = decorate(saImmOi.saImmOiObjectImplementerSet)
saImmOiObjectImplementerRelease = decorate(saImmOi.saImmOiObjectImplementerRelease)
saImmOiRtObjectCreate_2   = decorate(saImmOi.saImmOiRtObjectCreate_2)
saImmOiRtObjectDelete     = decorate(saImmOi.saImmOiRtObjectDelete)
saImmOiRtObjectUpdate_2   = decorate(saImmOi.saImmOiRtObjectUpdate_2)
saImmOiAdminOperationResult = decorate(saImmOi.saImmOiAdminOperationResult)
saImmOiAdminOperationResult_o2 = decorate(saImmOi.saImmOiAdminOperationResult_o2)
saImmOiAugmentCcbInitialize = decorate(saImmOi.saImmOiAugmentCcbInitialize)
saImmOiCcbSetErrorString  = decorate(saImmOi.saImmOiCcbSetErrorString)


def initialize(callbacks=None):
    ''' Initializes IMM OI '''

    version = SaVersionT('A', 2, 15)

    saImmOiInitialize_2(HANDLE, callbacks, version)


def register_applier(name):
    ''' Registers an an applier '''

    applier_name = "@" + name

    register_implementer(applier_name)

def register_implementer(name):
    ''' Registers as an implementer '''

    implementer_name = SaImmOiImplementerNameT(name)

    saImmOiImplementerSet(HANDLE, implementer_name)


def get_selection_object():
    ''' Retrieves the the selection object '''

    saImmOiSelectionObjectGet(HANDLE, SELECTION_OBJECT)


def implement_class(class_name):
    ''' Registers the implementer as an OI for the given class '''

    c_class_name = SaImmClassNameT(class_name)

    saImmOiClassImplementerSet(HANDLE, c_class_name)


def dispatch(mode=eSaDispatchFlagsT.SA_DISPATCH_ALL):
    ''' Dispatches all queued callbacks.
    '''

    saImmOiDispatch(HANDLE, mode)


def create_rt_object(class_name, parent_name, obj):
    ''' Creates a runtime object '''

    # Marshall parameters
    c_class_name = SaImmClassNameT(class_name)

    if parent_name:
        c_parent_name = SaNameT(parent_name)
    else:
        c_parent_name = None

    c_attr_values = []

    for name, (c_attr_type, values) in obj.attrs.iteritems():

        if values == None:
            values = []
        elif values == [None]:
            values = []

        # Make sure all values are in lists
        if not isinstance(values, list):
            values = [values]

        # Create the values struct
        c_attr = SaImmAttrValuesT_2()

        c_attr.attrName         = SaImmAttrNameT(name)
        c_attr.attrValueType    = c_attr_type
        c_attr.attrValuesNumber = len(values)

        if len(values) == 0:
            c_attr.attrValues = None
        else:
            c_attr.attrValues = marshal_c_array(c_attr_type, values)

        c_attr_values.append(c_attr)

    # Call the function
    saImmOiRtObjectCreate_2(HANDLE, c_class_name, c_parent_name,
                                    c_attr_values)


def delete_rt_object(dn):
    ''' Deletes a runtime object '''

    # Marshall the parameter
    c_dn = SaNameT(dn)

    saImmOiRtObjectDelete(HANDLE, c_dn)


def update_rt_object(dn, attributes):
    ''' Updates the given object with the given attribute modifications
    '''

    # Get the class name for the object
    class_name = get_class_name_for_dn(dn)

    # Create and marshall attribute modifications
    attr_mods = []

    for name, values in attributes.iteritems():

        if values is None:
            print "WARNING: Received no values for %s in %s" % (name, dn)
            continue

        if not isinstance(values, list):
            values = [values]

        attr_type = get_attribute_type(name, class_name)

        c_attr_mod = SaImmAttrModificationT_2()
        c_attr_mod.modType = eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_REPLACE
        c_attr_mod.modAttr                  = SaImmAttrValuesT_2()
        c_attr_mod.modAttr.attrName         = SaImmAttrNameT(name)
        c_attr_mod.modAttr.attrValueType    = attr_type
        c_attr_mod.modAttr.attrValuesNumber = len(values)
        c_attr_mod.modAttr.attrValues       = marshal_c_array(attr_type, values)
        attr_mods.append(c_attr_mod)

    # Call the function
    saImmOiRtObjectUpdate_2(HANDLE, SaNameT(dn), attr_mods)


def report_admin_operation_result(invocation_id, result):
    ''' Reports the result of an administrative operation '''

    saImmOiAdminOperationResult(HANDLE, invocation_id, result)


def set_error_string(ccb_id, error_string):
    ''' Sets the error string. This can only be called from within OI
        callbacks from a real implementer'''

    c_error_string = saAis.SaStringT(error_string)

    saImmOiCcbSetErrorString(HANDLE, ccb_id, c_error_string)


def get_class_category(class_name):
    ''' Returns the category of the given class '''

    c_attr_defs  = pointer(pointer(SaImmAttrDefinitionT_2()))
    c_category   = SaImmClassCategoryT()
    c_class_name = SaImmClassNameT(class_name)

    immom.saImmOmClassDescriptionGet_2(immom.HANDLE, c_class_name, c_category,
                                       c_attr_defs)

    return c_category.value


def get_parent_name_for_dn(dn):
    ''' returns the dn of the parent of the instance of the given dn '''

    if ',' in dn:
        return dn.split(',', 1)[1]
    else:
        return None


def get_object_names_for_class(class_name):
    ''' Returns the instances of the given class, optionally under the given
        root dn

        will not read runtime attributes and is safe to call from OI callbacks
    '''

    # Set up and marshall the search parameter
    c_class_name = c_char_p(class_name)

    c_search_param = SaImmSearchParametersT_2()
    c_search_param.searchOneAttr.attrName = "SaImmAttrClassName"
    c_search_param.searchOneAttr.attrValueType = eSaImmValueTypeT.SA_IMM_ATTR_SASTRINGT
    c_search_param.searchOneAttr.attrValue = cast(pointer(c_class_name), c_void_p)

    # Create the search iterator
    sit = SearchIterator(_search_param=c_search_param,
                         attribute_names=['SaImmAttrClassName'])

    # Return the results
    return [s.dn for s in sit]

def get_class_name_for_dn(dn):
    ''' returns the class name for an instance with the given dn '''

    obj = immom.get(dn, ["SaImmAttrClassName"])

    if obj:
        return obj.SaImmAttrClassName
    else:
        return None

def get_object_no_runtime(dn):
    ''' returns the IMM object with the given DN

        this is safe to call from OI callbacks as only the config attributes
        will be read

        the function needs to query the class to find config attributes. If
        the class name is known by the caller it can be passed in. Otherwise
        it will be looked up.
    '''

    return immom.get(dn, ['SA_IMM_SEARCH_GET_CONFIG_ATTR'])

def get_attribute_type(attribute, class_name):
    ''' Returns the type of the attribute in the given class

        This is safe to use from OI callbacks
    '''

    class_desc = immom.class_description_get(class_name)

    attr_desc = [ad for ad in class_desc if ad.attrName == attribute][0]

    return attr_desc.attrValueType

def get_rdn_attribute_for_class(class_name):
    ''' Returns the RDN attribute for the given class

        This is safe to call from OI callbacks
    '''

    desc = immom.class_description_get(class_name)

    for attr_desc in desc:
        if attr_desc.attrFlags & saImm.saImm.SA_IMM_ATTR_RDN:
            return attr_desc.attrName

    return None

def unmarshall_len_array(c_array, length, value_type):
    ''' Convert c array with a known length to a Python list. '''

    if not c_array:
        return []
    ctype = c_array[0].__class__
    if ctype is str:
        return unmarshalSaStringTArray(c_array)
    val_list = []
    i = 0
    for ptr in c_array:
        if i == length:
            break
        if not ptr:
            break

        val = unmarshalSaImmValue(ptr, value_type)

        val_list.append(val)

        i = i + 1

    return val_list

def get_available_classes_in_imm():
    ''' Returns a list of all available classes in IMM

        Safe to call from OI callbacks
    '''

    opensaf_imm = immom.get(OPENSAF_IMM_OBJECT)

    return opensaf_imm.opensafImmClassNames

def create_non_existing_imm_object(class_name, parent_name, attributes):
    ''' Creates an ImmObject instance for an object that does not yet
        exist in IMM.
    '''

    rdn_attribute = get_rdn_attribute_for_class(class_name)
    rdn_value = attributes[rdn_attribute][0]

    if parent_name:
        dn = '%s,%s' % (rdn_value, parent_name)
    else:
        dn = rdn_value

    obj = ImmObject(class_name=class_name, dn=dn)

    for name, values in attributes.iteritems():
        obj.__setattr__(name, values)

    obj.__setattr__('SaImmAttrClassName', class_name)
    obj.__setattr__('dn', dn)

    return obj
