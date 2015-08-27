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
    IMM OM common utilitites
'''

import os
import time
from ctypes import pointer

from pyosaf import saAis
from pyosaf.saAis import eSaAisErrorT, SaVersionT, SaNameT, unmarshalNullArray
from pyosaf import saImmOm, saImm
from pyosaf.saImm import eSaImmScopeT, unmarshalSaImmValue, SaImmAttrNameT, \
    SaImmAttrValuesT_2
from pyosaf.saImmOm import SaImmHandleT, SaImmAccessorHandleT,\
    saImmOmAdminOwnerInitialize

from pyosaf.utils import decorate

from pyosaf.utils import SafException
from pyosaf.utils.immom.object import ImmObject

HANDLE = saImmOm.SaImmHandleT()
ACCESSOR_HANDLE = SaImmAccessorHandleT()

# Decorate IMM functions to add retry loops and error handling
saImmOmInitialize         = decorate(saImmOm.saImmOmInitialize)
saImmOmSelectionObjectGet = decorate(saImmOm.saImmOmSelectionObjectGet)
saImmOmDispatch           = decorate(saImmOm.saImmOmDispatch)
saImmOmFinalize           = decorate(saImmOm.saImmOmFinalize)
saImmOmClassCreate_2      = decorate(saImmOm.saImmOmClassCreate_2)
saImmOmClassDescriptionGet_2 = decorate(saImmOm.saImmOmClassDescriptionGet_2)
saImmOmClassDescriptionMemoryFree_2 = decorate(saImmOm.saImmOmClassDescriptionMemoryFree_2)
saImmOmClassDelete        = decorate(saImmOm.saImmOmClassDelete)
saImmOmSearchInitialize_2 = decorate(saImmOm.saImmOmSearchInitialize_2)
saImmOmSearchNext_2       = decorate(saImmOm.saImmOmSearchNext_2)
saImmOmSearchFinalize     = decorate(saImmOm.saImmOmSearchFinalize)
saImmOmAccessorInitialize = decorate(saImmOm.saImmOmAccessorInitialize)
saImmOmAccessorGet_2      = decorate(saImmOm.saImmOmAccessorGet_2)
saImmOmAccessorFinalize   = decorate(saImmOm.saImmOmAccessorFinalize)
saImmOmAdminOwnerInitialize = decorate(saImmOm.saImmOmAdminOwnerInitialize)
saImmOmAdminOwnerSet      = decorate(saImmOm.saImmOmAdminOwnerSet)
saImmOmAdminOwnerRelease  = decorate(saImmOm.saImmOmAdminOwnerRelease)
saImmOmAdminOwnerFinalize = decorate(saImmOm.saImmOmAdminOwnerFinalize)
saImmOmAdminOwnerClear    = decorate(saImmOm.saImmOmAdminOwnerClear)
saImmOmCcbInitialize      = decorate(saImmOm.saImmOmCcbInitialize)
saImmOmCcbObjectCreate_2  = decorate(saImmOm.saImmOmCcbObjectCreate_2)
saImmOmCcbObjectDelete    = decorate(saImmOm.saImmOmCcbObjectDelete)
saImmOmCcbObjectModify_2  = decorate(saImmOm.saImmOmCcbObjectModify_2)
saImmOmCcbApply           = decorate(saImmOm.saImmOmCcbApply)
saImmOmCcbFinalize        = decorate(saImmOm.saImmOmCcbFinalize)
saImmOmAdminOperationInvoke_2 = decorate(saImmOm.saImmOmAdminOperationInvoke_2)
saImmOmAdminOperationInvokeAsync_2 = decorate(saImmOm.saImmOmAdminOperationInvokeAsync_2)
saImmOmAdminOperationContinue = decorate(saImmOm.saImmOmAdminOperationContinue)
saImmOmAdminOperationContinueAsync = decorate(saImmOm.saImmOmAdminOperationContinueAsync)
saImmOmAdminOperationContinuationClear = decorate(saImmOm.saImmOmAdminOperationContinuationClear)

def _initialize():
    ''' saImmOmInitialize with TRYAGAIN handling '''
    version = SaVersionT('A', 2, 15)

    err = saImmOmInitialize(HANDLE, None, version)

    err = saImmOmAccessorInitialize(HANDLE, ACCESSOR_HANDLE)


def get(object_name, attr_name_list=None, class_name=None):
    ''' obtain values of some attributes of the specified object '''

    # Always request the SaImmAttrClassName attribute if needed
    if attr_name_list and not class_name and \
       not 'SaImmAttrClassName' in attr_name_list:
        attr_name_list.append('SaImmAttrClassName')

    attrib_names = [SaImmAttrNameT(a) for a in attr_name_list]\
        if attr_name_list else None

    attributes = pointer(pointer(SaImmAttrValuesT_2()))

    try:
        err = saImmOmAccessorGet_2(ACCESSOR_HANDLE,
                                   SaNameT(object_name),
                                   attrib_names, attributes)
    except SafException as err:
        if err.value == eSaAisErrorT.SA_AIS_ERR_NOT_EXIST:
            return None
        else:
            raise err

    attribs = {}
    attr_list = unmarshalNullArray(attributes)
    for attr in attr_list:
        attr_range = range(attr.attrValuesNumber)
        attribs[attr.attrName] = [
            attr.attrValueType,
            [unmarshalSaImmValue(
                attr.attrValues[val],
                attr.attrValueType) for val in attr_range]
            ]

    if not 'SaImmAttrClassName' in attribs and class_name:
        attribs['SaImmAttrClassName'] = class_name

    return ImmObject(object_name, attribs)

# initialize handles needed when module is loaded
_initialize()


def class_description_get(class_name):
    ''' get class description as a python list '''

    attr_defs = pointer(pointer(saImm.SaImmAttrDefinitionT_2()))
    category = saImm.SaImmClassCategoryT()
    err = saImmOmClassDescriptionGet_2(HANDLE,
                                       class_name,
                                       category,
                                       attr_defs)

    return saAis.unmarshalNullArray(attr_defs)


def admin_op_invoke(dn, op_id, params=None):
    ''' invokes admin op for dn '''
    owner_handle = saImmOm.SaImmAdminOwnerHandleT()
    owner_name = saImmOm.SaImmAdminOwnerNameT(os.getlogin())
    err = saImmOmAdminOwnerInitialize(HANDLE,
                                      owner_name,
                                      saAis.eSaBoolT.SA_TRUE,
                                      owner_handle)

    idx = dn.rfind(",")
    parent_name = SaNameT(dn[idx+1:])
    object_names = [parent_name]
    err = saImmOmAdminOwnerSet(owner_handle, object_names,
                               eSaImmScopeT.SA_IMM_SUBTREE)

    if params is None:
        params = []

    object_dn = SaNameT(dn)
    retval = saAis.SaAisErrorT()

    err = saImmOmAdminOperationInvoke_2(
        owner_handle,
        object_dn,
        0,
        op_id,
        params,
        retval,
        saAis.saAis.SA_TIME_ONE_SECOND * 10)

    if retval.value != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOperationInvoke_2: %s" % \
            eSaAisErrorT.whatis(retval.value)
        raise SafException(retval.value)

    error = saImmOmAdminOwnerFinalize(owner_handle)
