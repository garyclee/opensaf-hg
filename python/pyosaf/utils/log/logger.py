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

import ctypes

from pyosaf import saLog, saAis
from pyosaf.utils import log

# Logger class to encapsulate LOG handling
class SafLogger(object):

	@staticmethod
	def writeLogCallback(invocation, error):
		pass

	def __init__(self, exec_name=""):
		self.invocation   = 0
		self.logHandle    = saLog.SaLogHandleT()
		self.fd           = saAis.SaSelectionObjectT()
		self.callbacks    = saLog.SaLogCallbacksT()
		self.streamHandle = saLog.SaLogStreamHandleT()
		self.record       = saLog.SaLogRecordT()

		self.callbacks.saLogWriteLogCallback = \
			saLog.SaLogWriteLogCallbackT(self.writeLogCallback)

                log.saLogInitialize(self.logHandle, self.callbacks, log.LOG_VERSION)
                log.saLogSelectionObjectGet(self.logHandle, self.fd)

		streamName = saAis.SaNameT(saLog.saLog.SA_LOG_STREAM_SYSTEM)
                log.saLogStreamOpen_2(self.logHandle, streamName, None, 0,
                                      saAis.saAis.SA_TIME_ONE_SECOND, self.streamHandle)

		userName   = ctypes.pointer(saAis.SaNameT(exec_name))
		headerType = saLog.eSaLogHeaderTypeT.SA_LOG_GENERIC_HEADER
		self.record.logTimeStamp = saAis.saAis.SA_TIME_UNKNOWN
		self.record.logHdrType   =  headerType
		self.record.logHeader.genericHdr.logSvcUsrName = userName

	def finalize(self):
		log.saLogStreamClose(self.streamHandle)
		log.saLogFinalize(self.logHandle)

	def get_invocation(self):
		self.invocation += 1
		return self.invocation

	def __del__(self):
            self.finalize()

	def log(self, logstr):
		log_buffer = ctypes.pointer(saLog.SaLogBufferT(logstr))
		notice     = saLog.saLog.SA_LOG_SEV_NOTICE
		self.record.logBuffer = log_buffer
		self.record.logHeader.genericHdr.logSeverity = notice
                log.saLogWriteLogAsync(self.streamHandle,
                                       self.get_invocation(),
                                       saLog.saLog.SA_LOG_RECORD_WRITE_ACK, self.record)

	def dispatch(self, readfds):
		if self.fd.value in readfds:
                        log.saLogDispatch(self.logHandle, saAis.eSaDispatchFlagsT.SA_DISPATCH_ALL)
