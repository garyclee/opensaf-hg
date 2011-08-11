############################################################################
#
# (C) Copyright 2011 The OpenSAF Foundation
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
# Author(s): Wind River Systems, Inc.
#
############################################################################

from saImm import *

oidll = CDLL('libSaImmOi.so.0')

SaImmOiHandleT = SaUint64T

SaImmOiImplementerNameT = SaStringT

SaImmOiCcbIdT = SaUint64T

SaImmOiRtAttrUpdateCallbackT = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, POINTER(SaNameT), POINTER(SaImmAttrNameT))
#ifdef IMM_A_01_01
SaImmOiCcbObjectCreateCallbackT = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, SaImmOiCcbIdT, SaImmClassNameT, POINTER(SaNameT),
	POINTER(POINTER(SaImmAttrValuesT)))
#endif

SaImmOiCcbObjectCreateCallbackT_2 = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, SaImmOiCcbIdT, SaImmClassNameT, POINTER(SaNameT),
	POINTER(POINTER(SaImmAttrValuesT_2)))

SaImmOiCcbObjectDeleteCallbackT = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, SaImmOiCcbIdT, POINTER(SaNameT))
#ifdef IMM_A_01_01
SaImmOiCcbObjectModifyCallbackT = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, SaImmOiCcbIdT, POINTER(SaNameT),
	POINTER(POINTER(SaImmAttrModificationT)))
#endif

SaImmOiCcbObjectModifyCallbackT_2 = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, SaImmOiCcbIdT, POINTER(SaNameT),
	POINTER(POINTER(SaImmAttrModificationT_2)))

SaImmOiCcbCompletedCallbackT = CFUNCTYPE(SaAisErrorT,
	SaImmOiHandleT, SaImmOiCcbIdT)

SaImmOiCcbApplyCallbackT = CFUNCTYPE(None,
	SaImmOiHandleT, SaImmOiCcbIdT)

SaImmOiCcbAbortCallbackT = CFUNCTYPE(None,
	SaImmOiHandleT, SaImmOiCcbIdT)

#ifdef IMM_A_01_01
SaImmOiAdminOperationCallbackT = CFUNCTYPE(None,
	SaImmOiHandleT, SaInvocationT, POINTER(SaNameT),
	SaImmAdminOperationIdT, POINTER(POINTER(SaImmAdminOperationParamsT)))
#endif

SaImmOiAdminOperationCallbackT_2 = CFUNCTYPE(None,
	SaImmOiHandleT, SaInvocationT, POINTER(SaNameT),
	SaImmAdminOperationIdT, POINTER(POINTER(SaImmAdminOperationParamsT_2)))

#ifdef IMM_A_01_01
class SaImmOiCallbacksT(Structure):
	"""Contain various callbacks IMM may invoke on a registrant.
	"""
	_fields_ = [('saImmOiRtAttrUpdateCallback',
			SaImmOiRtAttrUpdateCallbackT),
		('saImmOiCcbObjectCreateCallback',
			SaImmOiCcbObjectCreateCallbackT),
		('saImmOiCcbObjectDeleteCallback',
			SaImmOiCcbObjectDeleteCallbackT),
		('saImmOiCcbObjectModifyCallback',
			SaImmOiCcbObjectModifyCallbackT),
		('saImmOiCcbCompletedCallback',
			SaImmOiCcbCompletedCallbackT),
		('saImmOiCcbApplyCallback',
			SaImmOiCcbApplyCallbackT),
		('saImmOiCcbAbortCallback',
			SaImmOiCcbAbortCallbackT),
		('saImmOiAdminOperationCallback',
			SaImmOiAdminOperationCallbackT)]
#endif

class SaImmOiCallbacksT_2(Structure):
	"""Contain various callbacks IMM may invoke on a registrant.
	"""
#	_fields_ = [('SaImmOiAdminOperationCallbackT', ),
	#The downloaded includefile contained the above line which is a bug!! */
	_fields_ = [('saImmOiAdminOperationCallback',
			SaImmOiAdminOperationCallbackT_2),
		('saImmOiCcbAbortCallback',
			SaImmOiCcbAbortCallbackT),
		('saImmOiCcbApplyCallback',
			SaImmOiCcbApplyCallbackT),
		('saImmOiCcbCompletedCallback',
			SaImmOiCcbCompletedCallbackT),
		('saImmOiCcbObjectCreateCallback', 
			SaImmOiCcbObjectCreateCallbackT_2),
		('saImmOiCcbObjectDeleteCallback',
			SaImmOiCcbObjectDeleteCallbackT),
		('saImmOiCcbObjectModifyCallback',
			SaImmOiCcbObjectModifyCallbackT_2),
		('saImmOiRtAttrUpdateCallback',
			SaImmOiRtAttrUpdateCallbackT)]

#ifdef IMM_A_01_01
def saImmOiInitialize(immOiHandle, immOiCallbacks, version):
	"""
	Register invoking process with IMM.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmOiCallbacksT immOiCallbacks
		SaVersionT version

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiInitialize(BYREF(immOiHandle),
			BYREF(immOiCallbacks),
			BYREF(version))
#endif

def saImmOiInitialize_2(immOiHandle, immOiCallbacks, version):
	"""
	Register invoking process with IMM.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmOiCallbacksT_2 immOiCallbacks
		SaVersionT version

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiInitialize_2(BYREF(immOiHandle),
			BYREF(immOiCallbacks),
			BYREF(version))

def saImmOiSelectionObjectGet(immOiHandle, selectionObject):
	"""Return operating system handle associated with IMM handle to detect
	pending callbacks.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaSelectionObjectT selectionObject

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiSelectionObjectGet(immOiHandle,
			BYREF(selectionObject))

def saImmOiDispatch(immOiHandle, dispatchFlags):
	"""Invoke callbacks pending for the IMM handle.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaDispatchFlagsT dispatchFlags

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiDispatch(immOiHandle, dispatchFlags)

def saImmOiFinalize(immOiHandle):
	"""Close association between IMM and the handle.

	type arguments:
		SaImmOiHandleT immOiHandle

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiFinalize(immOiHandle)

def saImmOiImplementerSet(immOiHandle, implementerName):
	"""Set implementer name for the handle and register invoking process as
	the object implementer.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmOiImplementerNameT implementerName

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiImplementerSet(immOiHandle, implementerName)

def saImmOiImplementerClear(immOiHandle):
	"""Clear implementer name for the handle and unregister the invoking
	process as the object implementer.

	type arguments:
		SaImmOiHandleT immOiHandle

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiImplementerClear(immOiHandle)

def saImmOiClassImplementerSet(immOiHandle, className):
	"""Assign object implementer associated with handle to the role(s)
	specified by the role parameter for all objects of class specified
	by className.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmClassNameT className

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiClassImplementerSet(immOiHandle, className)

def saImmOiClassImplementerRelease(immOiHandle, className):
	"""Release object implementer associated with handle from the role(s)
	specified by the role parameter for all objects of class specified
	by className.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmClassNameT className

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiClassImplementerRelease(immOiHandle, className)

def saImmOiObjectImplementerSet(immOiHandle, objectName, scope):
	"""Assign object implementer associated with handle to the role(s)
	specified by the role parameter for all objects identified by
	objectName and scope.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaNameT objectName
		SaImmScopeT scope

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiObjectImplementerSet(immOiHandle,
			BYREF(objectName),
			scope)

def saImmOiObjectImplementerRelease(immOiHandle, objectName, scope):
	"""Release object implementer associated with handle from the role(s)
	specified by the role parameter for all objects identified by
	objectName and scope.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaNameT objectName
		SaImmScopeT scope

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiObjectImplementerRelease(immOiHandle,
			BYREF(objectName),
			scope)

#ifdef IMM_A_01_01
def saImmOiRtObjectCreate(immOiHandle, className, parentName, attrValues):
	"""Create new IMM service runtime object.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmClassNameT className
		SaNameT parentName
		SaImmAttrValuesT attrValues

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiRtObjectCreate(immOiHandle,
			className,
			BYREF(parentName),
			BYREF(attrValues))
#endif

def saImmOiRtObjectCreate_2(immOiHandle, className, parentName, attrValues):
	"""Create new IMM service runtime object.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaImmClassNameT className
		SaNameT parentName
		SaImmAttrValuesT_2 attrValues

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiRtObjectCreate_2(immOiHandle,
			className,
			BYREF(parentName),
			BYREF(attrValues))

def saImmOiRtObjectDelete(immOiHandle, objectName):
	"""Delete object and subtree rooted at that object.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaNameT objectName

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiRtObjectDelete(immOiHandle, BYREF(objectName))

#ifdef IMM_A_01_01
def saImmOiRtObjectUpdate(immOiHandle, objectName, attrMods):
	"""Update runtime attributes of configuration or runtime object.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaNameT objectName
		SaImmAttrModificationT attrMods

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiRtObjectUpdate(immOiHandle,
			BYREF(objectName),
			BYREF(attrMods))
#endif

def saImmOiRtObjectUpdate_2(immOiHandle, objectName, attrMods):
	"""Update runtime attributes of configuration or runtime object.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaNameT objectName
		SaImmAttrModificationT_2 attrMods

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiRtObjectUpdate_2(immOiHandle,
			BYREF(objectName),
			BYREF(attrMods))

def saImmOiAdminOperationResult(immOiHandle, invocation, result):
	"""Respond to IMM with result of saImmOiAdminOperationCallback()
	invoked by IMM.

	type arguments:
		SaImmOiHandleT immOiHandle
		SaInvocationT invocation
		SaAisErrorT result

	returns:
		SaAisErrorT

	"""
	return oidll.saImmOiAdminOperationResult(immOiHandle, invocation, result)
