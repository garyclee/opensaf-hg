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
IMM Search iterators
'''

import sys
import time
import collections
from ctypes import pointer, c_char_p, cast, c_void_p

from pyosaf.saAis import eSaAisErrorT, SaNameT, unmarshalNullArray

from pyosaf.saImm import saImm, eSaImmScopeT, eSaImmValueTypeT, \
    unmarshalSaImmValue, SaImmAttrNameT, SaImmSearchParametersT_2, \
    SaImmAttrValuesT_2

from pyosaf import saImmOm
import pyosaf.utils.immom
from pyosaf.utils.immom.common import SafException
from pyosaf.utils.immom.object import ImmObject

TRYAGAIN_CNT = 60


class SearchIterator(collections.Iterator):
    ''' general search iterator '''
    def __init__(self, root_name=None, scope=eSaImmScopeT.SA_IMM_SUBTREE,
            attribute_names=None, _search_param=None):

        self.search_handle = saImmOm.SaImmSearchHandleT()
        if root_name is not None:
            self.root_name = SaNameT(root_name)
        else:
            self.root_name = None
        self.scope = scope
        self.attribute_names = \
            [SaImmAttrNameT(n) for n in attribute_names] \
                if attribute_names else None

        search_options = saImm.SA_IMM_SEARCH_ONE_ATTR
        if attribute_names is None:
            search_options |= saImm.SA_IMM_SEARCH_GET_ALL_ATTR
        else:
            search_options |= saImm.SA_IMM_SEARCH_GET_SOME_ATTR

        if _search_param is None:
            search_param = SaImmSearchParametersT_2()
        else:
            search_param = _search_param

        err = saImmOm.saImmOmSearchInitialize_2(pyosaf.utils.immom.HANDLE, self.root_name,
            self.scope, search_options, search_param, self.attribute_names,
            self.search_handle)
        one_sec_sleeps = 0
        while err == eSaAisErrorT.SA_AIS_ERR_TRY_AGAIN:
            if one_sec_sleeps == TRYAGAIN_CNT:
                break
            time.sleep(1)
            one_sec_sleeps += 1
            err = saImmOm.saImmOmSearchInitialize_2(pyosaf.utils.immom.HANDLE,
                self.root_name, self.scope, search_options, search_param,
                self.attribute_names, self.search_handle)

        if err != eSaAisErrorT.SA_AIS_OK:
            if err == eSaAisErrorT.SA_AIS_ERR_NOT_EXIST:
                self.search_handle = None
                raise SafException(err, "%s does not exist" % root_name)
            else:
                raise SafException(err)

    def __del__(self):
        if self.search_handle is not None:
            error = saImmOm.saImmOmSearchFinalize(self.search_handle)
            if error != eSaAisErrorT.SA_AIS_OK:
                raise SafException(error)

    def __iter__(self):
        return self

    def next(self):
        name = SaNameT()
        attributes = pointer(pointer(SaImmAttrValuesT_2()))
        error = saImmOm.saImmOmSearchNext_2(self.search_handle, name,
                                            attributes)
        if error == eSaAisErrorT.SA_AIS_ERR_NOT_EXIST:
            raise StopIteration
        elif error != eSaAisErrorT.SA_AIS_OK:
            raise SafException(error)

        attribs = {}
        attr_list = unmarshalNullArray(attributes)
        for attr in attr_list:
            attr_range = range(attr.attrValuesNumber)
            attribs[attr.attrName] = [attr.attrValueType,
                [unmarshalSaImmValue(
                    attr.attrValues[val],
                    attr.attrValueType) for val in attr_range]
                ]

        return ImmObject(name.value, attribs)


class InstanceIterator(SearchIterator):
    ''' iterator over instances of a class '''
    def __init__(self, class_name, root_name=None):
        name = c_char_p(class_name)
        search_param = SaImmSearchParametersT_2()
        search_param.searchOneAttr.attrName = "SaImmAttrClassName"
        search_param.searchOneAttr.attrValueType = \
            eSaImmValueTypeT.SA_IMM_ATTR_SASTRINGT
        search_param.searchOneAttr.attrValue = cast(pointer(name), c_void_p)
        SearchIterator.__init__(self, root_name, _search_param=search_param)


def test():
    ''' test function '''
    it = InstanceIterator(sys.argv[1])
    for dn, attr in it:
        print dn.split("=")[1]
        print attr, "\n"

if __name__ == '__main__':
    test()
