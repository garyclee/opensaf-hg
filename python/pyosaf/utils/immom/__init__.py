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

from pyosaf.utils.immom.common import SafException
from pyosaf.utils.immom.object import ImmObject

HANDLE = saImmOm.SaImmHandleT()
ACCESSOR_HANDLE = SaImmAccessorHandleT()

TRYAGAIN_CNT = 60


def _initialize():
    ''' saImmOmInitialize with TRYAGAIN handling '''
    version = SaVersionT('A', 2, 1)
    one_sec_sleeps = 0
    err = saImmOm.saImmOmInitialize(HANDLE, None, version)
    while err == eSaAisErrorT.SA_AIS_ERR_TRY_AGAIN:
        if one_sec_sleeps == TRYAGAIN_CNT:
            break
        time.sleep(1)
        one_sec_sleeps += 1
        err = saImmOm.saImmOmInitialize(HANDLE, None, version)

    if err != eSaAisErrorT.SA_AIS_OK:
        raise SafException(err,
                           "saImmOmInitialize: %s" % eSaAisErrorT.whatis(err))

    # TODO TRYAGAIN handling? Is it needed?
    err = saImmOm.saImmOmAccessorInitialize(HANDLE, ACCESSOR_HANDLE)
    if err != eSaAisErrorT.SA_AIS_OK:
        raise SafException(err,
                           "saImmOmAccessorInitialize: %s" %
                           eSaAisErrorT.whatis(err))


def get(object_name, attr_name_list=None):
    ''' obtain values of some attributes of the specified object '''

    attrib_names = [SaImmAttrNameT(a) for a in attr_name_list]\
        if attr_name_list else None

    attributes = pointer(pointer(SaImmAttrValuesT_2()))

    one_sec_sleeps = 0
    err = saImmOm.saImmOmAccessorGet_2(ACCESSOR_HANDLE,
                                       SaNameT(object_name),
                                       attrib_names, attributes)
    while err == eSaAisErrorT.SA_AIS_ERR_TRY_AGAIN:
        if one_sec_sleeps == TRYAGAIN_CNT:
            break
        time.sleep(1)
        one_sec_sleeps += 1
        err = saImmOm.saImmOmAccessorGet_2(ACCESSOR_HANDLE,
                                           SaNameT(object_name),
                                           attrib_names, attributes)

    if err == eSaAisErrorT.SA_AIS_ERR_NOT_EXIST:
        return None

    if err != eSaAisErrorT.SA_AIS_OK:
        raise SafException(err,
                           "saImmOmInitialize: %s" % eSaAisErrorT.whatis(err))

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

    return ImmObject(object_name, attribs)

# initialize handles needed when module is loaded
_initialize()


def class_description_get(class_name):
    ''' get class description as a python list '''

    attr_defs = pointer(pointer(saImm.SaImmAttrDefinitionT_2()))
    category = saImm.SaImmClassCategoryT()
    err = saImmOm.saImmOmClassDescriptionGet_2(HANDLE,
                                               class_name,
                                               category,
                                               attr_defs)
    if err != eSaAisErrorT.SA_AIS_OK:
        raise SafException(err, "saImmOmClassDescriptionGet_2(%s)" %
                           class_name)

    return saAis.unmarshalNullArray(attr_defs)


def admin_op_invoke(dn, op_id, params=None):
    ''' invokes admin op for dn '''
    owner_handle = saImmOm.SaImmAdminOwnerHandleT()
    owner_name = saImmOm.SaImmAdminOwnerNameT(os.getlogin())
    err = saImmOm.saImmOmAdminOwnerInitialize(HANDLE,
                                              owner_name,
                                              saAis.eSaBoolT.SA_TRUE,
                                              owner_handle)

    if err != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOwnerInitialize: %s" % eSaAisErrorT.whatis(err)
        raise SafException(err)

    idx = dn.rfind(",")
    parent_name = SaNameT(dn[idx+1:])
    object_names = [parent_name]
    err = saImmOm.saImmOmAdminOwnerSet(owner_handle, object_names,
                                       eSaImmScopeT.SA_IMM_SUBTREE)
    if err != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOwnerInitialize: %s" % eSaAisErrorT.whatis(err)
        raise SafException(err)

    if params is None:
        params = []

    object_dn = SaNameT(dn)
    retval = saAis.SaAisErrorT()

    err = saImmOm.saImmOmAdminOperationInvoke_2(
        owner_handle,
        object_dn,
        0,
        op_id,
        params,
        retval,
        saAis.saAis.SA_TIME_ONE_SECOND * 10)

    if err != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOperationInvoke_2: %s" % eSaAisErrorT.whatis(err)
        raise SafException(err)

    one_sec_sleeps = 0
    while retval.value == eSaAisErrorT.SA_AIS_ERR_TRY_AGAIN:
        if one_sec_sleeps == TRYAGAIN_CNT:
            break
        time.sleep(0.1)
        one_sec_sleeps += 1
        err = saImmOm.saImmOmAdminOperationInvoke_2(
            owner_handle,
            object_dn,
            0,
            op_id,
            params,
            retval,
            saAis.saAis.SA_TIME_ONE_SECOND * 10)

    if err != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOperationInvoke_2: %s" % eSaAisErrorT.whatis(err)
        raise SafException(err)

    if retval.value != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOperationInvoke_2: %s" % \
            eSaAisErrorT.whatis(retval.value)
        raise SafException(retval.value)

    error = saImmOm.saImmOmAdminOwnerFinalize(owner_handle)
    if error != eSaAisErrorT.SA_AIS_OK:
        print "saImmOmAdminOwnerFinalize: %s" % eSaAisErrorT.whatis(error)
        raise SafException(error)
