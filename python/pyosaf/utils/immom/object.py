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
    IMM Object representation.
'''

from pyosaf.saImm import saImm
from pyosaf import saImmOm
import pyosaf.utils.immom


class ImmObject(object):
    ''' Instances of this class represents an IMM object.
        Created by iterators and accessor reading from IMM or when
        creating objects in IMM.
    '''

    ''' cache of class descriptions for the purpose to know if an attribute
    is multivalue and also its type
    '''
    class_desc = {}

    def __init__(self, dn=None, attributes=None, class_name=None):
        '''
        Constructor for IMM object.
        attributes is a map where attribute name is key, value is a tuple
        of (valueType, values), values is a list.
        class_name and attributes are mutually exclusive.
        '''
        self.__dict__["dn"] = dn

        # mutually exclusive for read vs create
        if class_name is not None:
            assert attributes is None
            self.__dict__["attrs"] = {}
            self.__dict__["class_name"] = class_name
            self.class_desc[class_name] = \
                pyosaf.utils.immom.class_description_get(class_name)
        elif attributes is not None:
            assert class_name is None
            self.__dict__["attrs"] = attributes
            class_name = attributes["SaImmAttrClassName"][1][0]
            self.__dict__["class_name"] = class_name
            if not class_name in self.class_desc:
                self.class_desc[class_name] = \
                    pyosaf.utils.immom.class_description_get(class_name)
        else:
            raise

    def get_value_type(self, attrname):
        ''' returns IMM value type of the named attribute '''
        for attrdef in self.class_desc[self.class_name]:
            if attrdef.attrName == attrname:
                return attrdef.attrValueType

    def __attr_is_multivalue(self, attrname):
        ''' returns True if attribute is multivalue (internal) '''
        for attrdef in self.class_desc[self.class_name]:
            if attrdef.attrName == attrname:
                return attrdef.attrFlags & saImm.SA_IMM_ATTR_MULTI_VALUE

    def __getattr__(self, name):
        if not name in self.attrs:
            raise KeyError("%s is not a valid attribute name" % name)

        if self.__attr_is_multivalue(name):
            return self.attrs[name][1]  # returns a list of values
        else:
            if len(self.attrs[name][1]) > 0:
                return self.attrs[name][1][0]  # returns single value
            else:
                return None

    def __getitem__(self, item):
        return self.__getattr__(item)

    def __contains__(self, attr_name):
        if attr_name in self.attrs:
            if len(self.attrs[attr_name][1]) > 0:
                return True
            else:
                return False
        else:
            return False

    def __setattr__(self, key, value):
        value_type = self.get_value_type(key)
        if type(value) is list:
            attr_value = (value_type, value)
        else:
            attr_value = (value_type, [value])

        self.attrs[key] = attr_value
