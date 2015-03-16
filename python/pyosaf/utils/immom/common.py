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

from pyosaf.saAis import eSaAisErrorT

class SafException(Exception):
    ''' SAF Exception that can be printed '''
    def __init__(self, value, msg=None):
        Exception.__init__(self)
        self.value = value
        self.msg = msg
    def __str__(self):
        return eSaAisErrorT.whatis(self.value)

