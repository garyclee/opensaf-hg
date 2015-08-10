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
# Author(s): Oracle
#
############################################################################

import time

from pyosaf.saAis import eSaAisErrorT

TRY_AGAIN_COUNT = 60

class SafException(Exception):
    ''' SAF Exception that can be printed '''

    def __init__(self, value, msg=None):
        Exception.__init__(self)
        self.value = value
        self.msg   = msg

    def __str__(self):
        return eSaAisErrorT.whatis(self.value)

def raise_saf_exception(function, error):
    ''' Raises an exception for a given SAF function, based on
        the given error code 
    '''

    error_string = "%s: %s" % (function.__name__, eSaAisErrorT.whatis(error))

    raise SafException(error, error_string)

def decorate(function):
    ''' Decorates the given SAF function so that it retries a fixed number of
        times if needed and raises an exception if it encounters any fault
        other than SA_AIS_ERR_TRY_AGAIN
    '''

    def inner(*args):
        ''' Calls "function" in the lexical scope in a retry loop and raises
            an exception if it encounters any other faults
        '''
        one_sec_sleeps = 0

        error = function(*args)

        while error == eSaAisErrorT.SA_AIS_ERR_TRY_AGAIN:
            if one_sec_sleeps == TRY_AGAIN_COUNT:
                break

            time.sleep(1)
            one_sec_sleeps += 1

            error = function(*args)

        if error != eSaAisErrorT.SA_AIS_OK:
            raise_saf_exception(function, error)

        return error

    return inner
