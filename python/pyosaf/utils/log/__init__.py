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
    LOG common utilitites
'''
from pyosaf import saLog, saAis

from pyosaf.utils import decorate

LOG_VERSION = saAis.SaVersionT('A', 2, 1)

# Decorate LOG functions to add retry loops and error handling
saLogInitialize         = decorate(saLog.saLogInitialize)
saLogSelectionObjectGet = decorate(saLog.saLogSelectionObjectGet)
saLogDispatch           = decorate(saLog.saLogDispatch)
saLogFinalize           = decorate(saLog.saLogFinalize)
saLogStreamOpen_2       = decorate(saLog.saLogStreamOpen_2)
saLogStreamOpenAsync_2  = decorate(saLog.saLogStreamOpenAsync_2)
saLogWriteLog           = decorate(saLog.saLogWriteLog)
saLogWriteLogAsync      = decorate(saLog.saLogWriteLogAsync)
saLogStreamClose        = decorate(saLog.saLogStreamClose)
saLogLimitGet           = decorate(saLog.saLogLimitGet)

