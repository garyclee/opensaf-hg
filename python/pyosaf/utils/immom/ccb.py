############################################################################
#
# (C) Copyright 2014 The OpenSAF Foundation
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
Class representing an IMM CCB.
'''

from ctypes import c_void_p, pointer, cast

from pyosaf.saAis import eSaAisErrorT, SaNameT, SaStringT, \
    SaDoubleT, SaTimeT, SaUint64T, SaInt64T, SaUint32T, SaInt32T, SaFloatT

from pyosaf.saImm import eSaImmScopeT, eSaImmValueTypeT, SaImmAttrValuesT_2

from pyosaf import saImm
from pyosaf import saImmOm
from pyosaf import saAis

from pyosaf.utils import immom
from pyosaf.utils import SafException

def _value_to_ctype_ptr(value_type, value):
    ''' convert a value to a ctypes value ptr '''
    if value_type is eSaImmValueTypeT.SA_IMM_ATTR_SAINT32T:
        ctypeptr = cast(pointer(SaInt32T(value)), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SAUINT32T:
        ctypeptr = cast(pointer(SaUint32T(long(value))), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SAINT64T:
        ctypeptr = cast(pointer(SaInt64T(long(value))), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SAUINT64T:
        ctypeptr = cast(pointer(SaUint64T(long(value))), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SATIMET:
        ctypeptr = cast(pointer(SaTimeT(long(value))), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SANAMET:
        ctypeptr = cast(pointer(SaNameT(value)), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SAFLOATT:
        ctypeptr = cast(pointer(SaFloatT(value)), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SADOUBLET:
        ctypeptr = cast(pointer(SaDoubleT(value)), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SASTRINGT:
        ctypeptr = cast(pointer(SaStringT(value)), c_void_p)
    elif value_type is eSaImmValueTypeT.SA_IMM_ATTR_SAANYT:
        assert 0
    else:
        assert 0

    return ctypeptr

def marshal_c_array(value_type, value_list):
    ''' creates a C array initialized with values from value_list '''
    c_array_type = saImm.SaImmAttrValueT * len(value_list)
    c_array = c_array_type()
    for i in range(len(value_list)):
        c_array[i] = _value_to_ctype_ptr(value_type, value_list[i])
    return c_array

class Ccb(object):
    ''' Represents an ongoing CCB '''

    def __init__(self, flags=[saImm.saImm.SA_IMM_CCB_REGISTERED_OI]):
        self.owner_handle = saImmOm.SaImmAdminOwnerHandleT()

        owner_name = saImmOm.SaImmAdminOwnerNameT("DummyName")

        immom.saImmOmAdminOwnerInitialize(immom.HANDLE, owner_name,
                saAis.eSaBoolT.SA_TRUE, self.owner_handle)

        self.ccb_handle = saImmOm.SaImmCcbHandleT()

        if flags:
            ccb_flags = saImmOm.SaImmCcbFlagsT(reduce(lambda a, b: a|b, flags))
        else:
            ccb_flags = saImmOm.SaImmCcbFlagsT(0)

        immom.saImmOmCcbInitialize(self.owner_handle, ccb_flags,
                                   self.ccb_handle)

    def __enter__(self):
        ''' Called when Ccb is used in a with statement:

            with Ccb() as ccb:
                ...

            The call is invoked before any code within is run
        '''
        return self

    def __exit__(self, exit_type, value, traceback):
        ''' Called when Ccb is used in a with statement,
            just before it exits

            exit_type, value and traceback are only set if the with
            statement was left via an exception
        '''
        if exit_type or value or traceback:
            self.__del__()
        else:
            self.apply()
            self.__del__()

    def __del__(self):
        immom.saImmOmAdminOwnerFinalize(self.owner_handle)

    def create(self, obj, _parent_name=None):
        ''' add to the CCB the object 'obj' '''
        if _parent_name is not None:
            parent_name = SaNameT(_parent_name)
            object_names = [parent_name]
            immom.saImmOmAdminOwnerSet(self.owner_handle, object_names,
                eSaImmScopeT.SA_IMM_SUBTREE)
        else:
            parent_name = None

        attr_values = []
        for attr_name, type_values in obj.attrs.iteritems():
            values = type_values[1]
            attr = SaImmAttrValuesT_2()
            attr.attrName = attr_name
            attr.attrValueType = type_values[0]
            attr.attrValuesNumber = len(values)
            attr.attrValues = marshal_c_array(attr.attrValueType, values)
            attr_values.append(attr)

        immom.saImmOmCcbObjectCreate_2(self.ccb_handle,
                                       obj.class_name,
                                       parent_name,
                                       attr_values)

    def delete(self, _object_name):
        ''' Adds a delete operation of the object with the given DN to the
            CCB'''

        if _object_name is None:
            raise SafException(eSaAisErrorT.SA_AIS_ERR_NOT_EXIST)

        object_name = SaNameT(_object_name)
        object_names = [object_name]

        immom.saImmOmAdminOwnerSet(self.owner_handle, object_names,
                                   eSaImmScopeT.SA_IMM_SUBTREE)

        immom.saImmOmCcbObjectDelete(self.ccb_handle, object_name)

    def modify_value_add(self, object_name, attr_name, values):
        ''' add to the CCB an ADD modification of an existing object '''

        assert object_name

        # Make sure the values field is a list
        if not isinstance(values, list):
            values = [values]

        # first get class name to read class description to get value type...
        try:
            obj = immom.get(object_name)
        except SafException as err:
            print "failed: %s" % err
            return

        object_names = [SaNameT(object_name)]
        class_name = obj.SaImmAttrClassName
        value_type = None
        attr_def_list = immom.class_description_get(class_name)
        for attr_def in attr_def_list:
            if attr_def.attrName == attr_name:
                value_type = attr_def.attrValueType
                break

        if value_type is None:
            # means attribute name is invalid
            raise SafException(eSaAisErrorT.SA_AIS_ERR_NOT_EXIST,
                "attribute '%s' does not exist" % attr_name)

        err = immom.saImmOmAdminOwnerSet(self.owner_handle, object_names,
            eSaImmScopeT.SA_IMM_ONE)

        attr_mods = []

        attr_mod = saImmOm.SaImmAttrModificationT_2()
        attr_mod.modType = \
            saImm.eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_ADD
        attr_mod.modAttr = SaImmAttrValuesT_2()
        attr_mod.modAttr.attrName = attr_name
        attr_mod.modAttr.attrValueType = value_type
        attr_mod.modAttr.attrValuesNumber = len(values)
        attr_mod.modAttr.attrValues = marshal_c_array(value_type, values)

        attr_mods.append(attr_mod)

        err = immom.saImmOmCcbObjectModify_2(self.ccb_handle,
            object_names[0], attr_mods)

    def modify_value_replace(self, object_name, attr_name, values):
        ''' add to the CCB an REPLACE modification of an existing object '''

        assert object_name

        # Make sure the values field is a list
        if not isinstance(values, list):
            values = [values]

        # first get class name to read class description to get value type...
        try:
            obj = immom.get(object_name)
        except SafException as err:
            print "failed: %s" % err
            return

        object_names = [SaNameT(object_name)]
        class_name = obj.SaImmAttrClassName
        value_type = None
        attr_def_list = immom.class_description_get(class_name)
        for attr_def in attr_def_list:
            if attr_def.attrName == attr_name:
                value_type = attr_def.attrValueType
                break

        if value_type is None:
            # means attribute name is invalid
            raise SafException(eSaAisErrorT.SA_AIS_ERR_NOT_EXIST,
                "attribute '%s' does not exist" % attr_name)

        err = immom.saImmOmAdminOwnerSet(self.owner_handle, object_names,
            eSaImmScopeT.SA_IMM_ONE)

        attr_mods = []
        attr_mod = saImmOm.SaImmAttrModificationT_2()
        attr_mod.modType = \
            saImm.eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_REPLACE
        attr_mod.modAttr = SaImmAttrValuesT_2()
        attr_mod.modAttr.attrName = attr_name
        attr_mod.modAttr.attrValueType = value_type
        attr_mod.modAttr.attrValuesNumber = len(values)
        attr_mod.modAttr.attrValues = marshal_c_array(value_type, values)

        attr_mods.append(attr_mod)

        err = immom.saImmOmCcbObjectModify_2(self.ccb_handle,
            object_names[0], attr_mods)

    def modify_value_delete(self, object_name, attr_name, values):
        ''' add to the CCB an DELETE modification of an existing object '''

        assert object_name

        # Make sure the values field is a list
        if not isinstance(values, list):
            values = [values]

        # first get class name to read class description to get value type...
        try:
            obj = immom.get(object_name)
        except SafException as err:
            print "failed: %s" % err
            return

        object_names = [SaNameT(object_name)]
        class_name = obj.SaImmAttrClassName
        value_type = None
        attr_def_list = immom.class_description_get(class_name)
        for attr_def in attr_def_list:
            if attr_def.attrName == attr_name:
                value_type = attr_def.attrValueType
                break

        if value_type is None:
            # means attribute name is invalid
            raise SafException(eSaAisErrorT.SA_AIS_ERR_NOT_EXIST,
                "attribute '%s' does not exist" % attr_name)

        err = immom.saImmOmAdminOwnerSet(self.owner_handle, object_names,
            eSaImmScopeT.SA_IMM_ONE)

        attr_mods = []
        attr_mod = saImmOm.SaImmAttrModificationT_2()
        attr_mod.modType = \
            saImm.eSaImmAttrModificationTypeT.SA_IMM_ATTR_VALUES_DELETE
        attr_mod.modAttr = SaImmAttrValuesT_2()
        attr_mod.modAttr.attrName = attr_name
        attr_mod.modAttr.attrValueType = value_type
        attr_mod.modAttr.attrValuesNumber = len(values)
        attr_mod.modAttr.attrValues = marshal_c_array(value_type, values)

        attr_mods.append(attr_mod)

        immom.saImmOmCcbObjectModify_2(self.ccb_handle,
                                       object_names[0], attr_mods)

    def apply(self):
        ''' Apply the CCB '''

        immom.saImmOmCcbApply(self.ccb_handle)


def test():
    ''' Small test case to show how the Ccb class can be used'''

    ccb = Ccb()
    ccb.modify_value_replace("safAmfCluster=myAmfCluster",
                             "saAmfClusterStartupTimeout",
                             [10000000000])
    ccb.apply()
    del ccb

if __name__ == '__main__':
    test()
