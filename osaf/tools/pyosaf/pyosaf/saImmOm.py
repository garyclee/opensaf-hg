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

omdll = CDLL('libSaImmOm.so.0')

SaImmHandleT = SaUint64T
SaImmAdminOwnerHandleT = SaUint64T
SaImmCcbHandleT = SaUint64T
SaImmSearchHandleT = SaUint64T
SaImmAccessorHandleT = SaUint64T

SaImmOmAdminOperationInvokeCallbackT = CFUNCTYPE(None,
	SaInvocationT, SaAisErrorT, SaAisErrorT)

class SaImmCallbacksT(Structure):
	"""Contain various callbacks IMM may invoke on registrant.
	"""
	_fields_ = [('saImmOmAdminOperationInvokeCallback',
			SaImmOmAdminOperationInvokeCallbackT)]

saImmOm = Const()

saImmOm.SA_IMM_ATTR_CLASS_NAME = "SaImmAttrClassName"
saImmOm.SA_IMM_ATTR_ADMIN_OWNER_NAME = "SaImmAttrAdminOwnerName"
saImmOm.SA_IMM_ATTR_IMPLEMENTER_NAME = "SaImmAttrImplementerName"

SaImmRepositoryInitModeT = SaEnumT
eSaImmRepositoryInitModeT = Enumeration((
	('SA_IMM_KEEP_REPOSITORY', 1),
	('SA_IMM_INIT_FROM_FILE', 2),
))

def saImmOmInitialize(immHandle, immCallbacks, version):
	"""Register invoking process with IMM.

	type arguments:
		SaImmHandleT immHandle
		SaImmCallbacksT immCallbacks
		SaVersionT version

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmInitialize(BYREF(immHandle),
			BYREF(immCallbacks),
			BYREF(version))

def saImmOmSelectionObjectGet(immHandle, selectionObject):
	"""Return operating system handle associated with IMM handle to detect
	pending callbacks.

	type arguments:
		SaImmHandleT immHandle
		SaSelectionObjectT selectionObject

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmSelectionObjectGet(immHandle,
			BYREF(selectionObject))

def saImmOmDispatch(immHandle, dispatchFlags):
	"""Invoke callbacks pending for the IMM handle.

	type arguments:
		SaImmHandleT immHandle
		SaDispatchFlagsT dispatchFlags

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmDispatch(immHandle, dispatchFlags)

def saImmOmFinalize(immHandle):
	"""Close association between IMM and the handle.

	type arguments:
		SaImmHandleT immHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmFinalize(immHandle)

#ifdef IMM_A_01_01
def saImmOmClassCreate(immHandle, className, classCategory, attrDefinitions):
	"""Create new configuration or runtime object class.

	type arguments:
		SaImmHandleT immHandle
		SaImmClassNameT className
		SaImmClassCategoryT classCategory
		SaImmAttrDefinitionT attrDefinitions

	returns:
		SaAisErrorT

	"""
	c_attrDefinitions = marshalNullArray(attrDefinitions)
	return omdll.saImmOmClassCreate(immHandle,
			className, classCategory, c_attrDefinitions)
#endif

def saImmOmClassCreate_2(immHandle, className, classCategory, attrDefinitions):
	"""Create new configuration or runtime object class.

	type arguments:
		SaImmHandleT immHandle
		SaImmClassNameT className
		SaImmClassCategoryT classCategory
		SaImmAttrDefinitionT_2 attrDefinitions

	returns:
		SaAisErrorT

	"""
	c_attrDefinitions = marshalNullArray(attrDefinitions)
	return omdll.saImmOmClassCreate_2(immHandle,
			className, classCategory, c_attrDefinitions)

#ifdef IMM_A_01_01
def saImmOmClassDescriptionGet(immHandle,
		className, classCategory, attrDefinitions):
	"""Get descriptor of object class identified by className.

	type arguments:
		SaImmHandleT immHandle
		SaImmClassNameT className
		SaImmClassCategoryT classCategory
		POINTER(POINTER(SaImmAttrDefinitionT))() attrDefinitions

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmClassDescriptionGet(immHandle,
			className,
			BYREF(classCategory),
			BYREF(attrDefinitions))
#endif

def saImmOmClassDescriptionGet_2(immHandle,
		className, classCategory, attrDefinitions):
	"""Get descriptor of object class identified by className.

	type arguments:
		SaImmHandleT immHandle
		SaImmClassNameT className
		SaImmClassCategoryT classCategory
		POINTER(POINTER(SaImmAttrDefinitionT_2))() attrDefinitions

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmClassDescriptionGet_2(immHandle,
			className,
			BYREF(classCategory),
			BYREF(attrDefinitions))

#ifdef IMM_A_01_01
def saImmOmClassDescriptionMemoryFree(immHandle, attrDefinitions):
	"""Release memory allocated by previous call to
	saImmOmClassDescriptionGet().

	type arguments:
		SaImmHandleT immHandle
		POINTER(POINTER(SaImmAttrDefinitionT))() attrDefinitions

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmClassDescriptionMemoryFree(immHandle,
			attrDefinitions)
#endif

def saImmOmClassDescriptionMemoryFree_2(immHandle, attrDefinitions):
	"""Release memory allocated by previous call to
	saImmOmClassDescriptionGet_2().

	type arguments:
		SaImmHandleT immHandle
		POINTER(POINTER(SaImmAttrDefinitionT_2))() attrDefinitions

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmClassDescriptionMemoryFree_2(immHandle,
			attrDefinitions)

def saImmOmClassDelete(immHandle, className):
	"""Delete named object class provided no instances exist.

	type arguments:
		SaImmHandleT immHandle
		SaImmClassNameT className

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmClassDelete(immHandle, className)

#ifdef IMM_A_01_01
def saImmOmSearchInitialize(immHandle,
		rootName, scope, searchOptions, searchParams, attributeNames,
		searchHandle):
	"""Initialize a search operation limited to objects identified by
	rootName and scope.

	type arguments:
		SaImmHandleT immHandle
		SaNameT rootName
		SaImmScopeT scope
		SaImmSearchOptionsT searchOptions
		SaImmSearchParametersT searchParams
		SaImmAttrNameT attributeNames
		SaImmSearchHandleT searchHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmSearchInitialize(immHandle,
			BYREF(rootName),
			scope,
			searchOptions,
			BYREF(searchParams),
			BYREF(attributeNames),
			BYREF(searchHandle))
#endif

def saImmOmSearchInitialize_2(immHandle,
		rootName, scope, searchOptions, searchParams, attributeNames,
		searchHandle):
	"""Initialize a search operation limited to objects identified by
	rootName and scope.

	type arguments:
		SaImmHandleT immHandle
		SaNameT rootName
		SaImmScopeT scope
		SaImmSearchOptionsT searchOptions
		SaImmSearchParametersT_2 searchParams
		SaImmAttrNameT attributeNames
		SaImmSearchHandleT searchHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmSearchInitialize_2(immHandle,
			BYREF(rootName),
			scope,
			searchOptions,
			BYREF(searchParams),
			BYREF(attributeNames),
			BYREF(searchHandle))

#ifdef IMM_A_01_01
def saImmOmSearchNext(searchHandle, objectName, attributes):
	"""Get next object matching search criteria specified in corresponding
	saImmOmSearchInitialize().

	type arguments:
		SaImmSearchHandleT searchHandle
		SaNameT objectName
		SaImmAttrValuesT attributes

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmSearchNext(searchHandle,
			BYREF(objectName),
			BYREF(attributes))
#endif

def saImmOmSearchNext_2(searchHandle, objectName, attributes):
	"""Get next object matching search criteria specified in corresponding
	saImmOmSearchInitialize_2().

	type arguments:
		SaImmSearchHandleT searchHandle
		SaNameT objectName
		POINTER(POINTER(SaImmAttrValuesT_2))() attributes

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmSearchNext_2(searchHandle,
			BYREF(objectName),
			BYREF(attributes))

def saImmOmSearchFinalize(searchHandle):
	"""Finalize search initialized by previous call to
	saImmOmSearchInitialize().

	type arguments:
		SaImmSearchHandleT searchHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmSearchFinalize(searchHandle)

def saImmOmAccessorInitialize(immHandle, accessorHandle):
	"""Initialize an object accessor and return a handle for further
	references.

	type arguments:
		SaImmHandleT immHandle
		SaImmAccessorHandleT accessorHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAccessorInitialize(immHandle,
			BYREF(accessorHandle))

#ifdef IMM_A_01_01
def saImmOmAccessorGet(accessorHandle,
		objectName, attributeNames, attributes):
	"""Use object accessor handle to obtain values of object attributes.

	type arguments:
		SaImmAccessorHandleT accessorHandle
		SaNameT objectName
		SaImmAttrNameT attributeNames
		POINTER(POINTER(SaImmAttrValuesT))() attributes

	returns:
		SaAisErrorT

	"""
	c_attributeNames = marshalNullArray(attributeNames)
	return omdll.saImmOmAccessorGet(accessorHandle,
			BYREF(objectName),
			BYREF(c_attributeNames),
			BYREF(attributes))
#endif

def saImmOmAccessorGet_2(accessorHandle,
		objectName, attributeNames, attributes):
	"""Use object accessor handle to obtain values of object attributes.

	type arguments:
		SaImmAccessorHandleT accessorHandle
		SaNameT objectName
		SaImmAttrNameT attributeNames
		POINTER(POINTER(SaImmAttrValuesT_2))() attributes

	returns:
		SaAisErrorT

	"""
	c_attributeNames = marshalNullArray(attributeNames)
	return omdll.saImmOmAccessorGet_2(accessorHandle,
			BYREF(objectName),
			BYREF(c_attributeNames),
			BYREF(attributes))

def saImmOmAccessorFinalize(accessorHandle):
	"""Finalize object accessor.

	type arguments:
		SaImmAccessorHandleT accessorHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAccessorFinalize(accessorHandle)

def saImmOmAdminOwnerInitialize(immHandle,
		adminOwnerName, releaseOwnershipOnFinalize, ownerHandle):
	"""Initialize handle for a named administrative owner.

	type arguments:
		SaImmHandleT immHandle
		SaImmAdminOwnerNameT adminOwnerName
		SaBoolT releaseOwnershipOnFinalize
		SaImmAdminOwnerHandleT ownerHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAdminOwnerInitialize(immHandle,
			adminOwnerName, releaseOwnershipOnFinalize,
			BYREF(ownerHandle))

def saImmOmAdminOwnerSet(ownerHandle, objectNames, scope):
	"""Set the administrative owner of a set of objects identified by name
	and scope.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaNameT objectNames
		SaImmScopeT scope

	returns:
		SaAisErrorT

	"""
	c_objectNames = marshalNullArray(objectNames)
	return omdll.saImmOmAdminOwnerSet(ownerHandle,
			BYREF(c_objectNames),
			scope)

def saImmOmAdminOwnerRelease(ownerHandle, objectNames, scope):
	"""Release administrative ownership of a set of objects identified by
	name and scope.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaNameT objectNames
		SaImmScopeT scope

	returns:
		SaAisErrorT

	"""
	c_objectNames = marshalNullArray(objectNames)
	return omdll.saImmOmAdminOwnerRelease(ownerHandle,
			BYREF(c_objectNames),
			scope)

def saImmOmAdminOwnerFinalize(ownerHandle):
	"""Release owner handle.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAdminOwnerFinalize(ownerHandle)

def saImmOmAdminOwnerClear(immHandle, objectNames, scope):
	"""Clear owner handle of the set of objects identified by name and
	scope.

	type arguments:
		SaImmHandleT immHandle
		SaNameT objectNames
		SaImmScopeT scope

	returns:
		SaAisErrorT

	"""
	c_objectNames = marshalNullArray(objectNames)
	return omdll.saImmOmAdminOwnerClear(immHandle,
			BYREF(c_objectNames),
			scope)

def saImmOmCcbInitialize(ownerHandle, ccbFlags, ccbHandle):
	"""Initialize a new CCB (change control block) handle.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaImmCcbFlagsT ccbFlags
		SaImmCcbHandleT ccbHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmCcbInitialize(ownerHandle, ccbFlags,
			BYREF(ccbHandle))

#ifdef IMM_A_01_01
def saImmOmCcbObjectCreate(ccbHandle, className, parentName, attrValues):
	"""Add creation of new configuration object to CCB requests.

	type arguments:
		SaImmCcbHandleT ccbHandle
		SaImmClassNameT className
		SaNameT parentName
		SaImmAttrValuesT attrValues

	returns:
		SaAisErrorT

	"""
	c_attrValues = marshalNullArray(attrValues)
	return omdll.saImmOmCcbObjectCreate(ccbHandle,
			className,
			BYREF(parentName),
			BYREF(c_attrValues))
#endif

def saImmOmCcbObjectCreate_2(ccbHandle, className, parentName, attrValues):
	"""Add creation of new configuration object to CCB requests.

	type arguments:
		SaImmCcbHandleT ccbHandle
		SaImmClassNameT className
		SaNameT parentName
		SaImmAttrValuesT_2 attrValues

	returns:
		SaAisErrorT

	"""
	c_attrValues = marshalNullArray(attrValues)
	return omdll.saImmOmCcbObjectCreate_2(ccbHandle,
			className,
			BYREF(parentName),
			BYREF(c_attrValues))

def saImmOmCcbObjectDelete(ccbHandle, objectName):
	"""Add deletion of named configuration object to CCB requests.

	type arguments:
		SaImmCcbHandleT ccbHandle
		SaNameT objectName

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmCcbObjectDelete(ccbHandle, BYREF(objectName))

#ifdef IMM_A_01_01
def saImmOmCcbObjectModify(ccbHandle, objectName, attrMods):
	"""Add modification of configuration object's attributes to CCB requests.

	type arguments:
		SaImmCcbHandleT ccbHandle
		SaNameT objectName
		SaImmAttrModificationT attrMods

	returns:
		SaAisErrorT

	"""
	c_attrMods = marshalNullArray(attrMods)
	return omdll.saImmOmCcbObjectModify(ccbHandle,
			BYREF(objectName),
			BYREF(c_attrMods))
#endif

def saImmOmCcbObjectModify_2(ccbHandle, objectName, attrMods):
	"""Add modification of configuration object's attributes to CCB requests.

	type arguments:
		SaImmCcbHandleT ccbHandle
		SaNameT objectName
		SaImmAttrModificationT_2 attrMods

	returns:
		SaAisErrorT

	"""
	c_attrMods = marshalNullArray(attrMods)
	return omdll.saImmOmCcbObjectModify_2(ccbHandle,
			BYREF(objectName),
			BYREF(c_attrMods))

def saImmOmCcbApply(ccbHandle):
	"""Apply all CCB requests associated with handle.

	type arguments:
		SaImmCcbHandleT ccbHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmCcbApply(ccbHandle)

def saImmOmCcbFinalize(ccbHandle):
	"""Finalize the handle associated with the CCB.

	type arguments:
		SaImmCcbHandleT ccbHandle

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmCcbFinalize(ccbHandle)

#ifdef IMM_A_01_01
def saImmOmAdminOperationInvoke(ownerHandle,
		objectName, operationId, params, operationReturnValue, timeout):
	"""Request registered runtime owner of named object to perform the
	identified administration operation.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaNameT objectName
		SaImmAdminOperationIdT operationId
		SaImmAdminOperationParamsT params
		SaAisErrorT operationReturnValue
		SaTimeT timeout

	returns:
		SaAisErrorT

	"""
	c_params = marshalNullArray(params)
	return omdll.saImmOmAdminOperationInvoke(ownerHandle,
			BYREF(objectName),
			operationId,
			c_params,
			BYREF(operationReturnValue),
			timeout)
#endif

def saImmOmAdminOperationInvoke_2(ownerHandle,
		objectName, continuationId, operationId, params,
		operationReturnValue, timeout):
	"""Request registered runtime owner of named object to perform the
	identified administration operation.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaNameT objectName
		SaImmContinuationIdT continuationId
		SaImmAdminOperationIdT operationId
		SaImmAdminOperationParamsT_2 params
		SaAisErrorT operationReturnValue
		SaTimeT timeout

	returns:
		SaAisErrorT

	"""
	c_params = marshalNullArray(params)
	return omdll.saImmOmAdminOperationInvoke_2(ownerHandle,
			BYREF(objectName),
			continuationId,
			operationId,
			c_params,
			BYREF(operationReturnValue),
			timeout)

#ifdef IMM_A_01_01
def saImmOmAdminOperationInvokeAsync(ownerHandle,
		invocation, objectName, operationId, params):
	"""Request registered runtime owner of named object to perform the
	identified administration operation asynchronously.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaInvocationT invocation
		SaNameT objectName
		SaImmAdminOperationIdT operationId
		SaImmAdminOperationParamsT params

	returns:
		SaAisErrorT

	"""
	c_params = marshalNullArray(params)
	return omdll.saImmOmAdminOperationInvokeAsync(ownerHandle,
			invocation,
			BYREF(objectName),
			operationId,
			c_params)
#endif

def saImmOmAdminOperationInvokeAsync_2(ownerHandle,
		invocation, objectName, continuationId, operationId, params):
	"""Request registered runtime owner of named object to perform the
	identified administration operation asynchronously.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaInvocationT invocation
		SaNameT objectName
		SaImmContinuationIdT continuationId
		SaImmAdminOperationIdT operationId
		SaImmAdminOperationParamsT_2 params

	returns:
		SaAisErrorT

	"""
	c_params = marshalNullArray(params)
	return omdll.saImmOmAdminOperationInvokeAsync_2(ownerHandle,
			invocation,
			BYREF(objectName),
			continuationId,
			operationId,
			c_params)

def saImmOmAdminOperationContinue(ownerHandle,
		objectName, continuationId, operationReturnValue):
	"""Continue invocation of an administrative operation initiated with
	an administrative handle that has since been finalized.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaNameT objectName
		SaImmContinuationIdT continuationId
		SaAisErrorT operationReturnValue

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAdminOperationContinue(ownerHandle,
			BYREF(objectName),
			continuationId,
			BYREF(operationReturnValue))

def saImmOmAdminOperationContinueAsync(ownerHandle,
		invocation, objectName, continuationId):
	"""Continue asynchronous invocation of an administrative operation
	initiated with an administrative handle that has since been finalized.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaInvocationT invocation
		SaNameT objectName
		SaImmContinuationIdT continuationId

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAdminOperationContinueAsync(ownerHandle,
			invocation,
			BYREF(objectName),
			continuationId)

def saImmOmAdminOperationContinuationClear(ownerHandle,
		objectName, continuationId):
	"""Clear information associated with the identified continuation of an
	administration operation for the named object.

	type arguments:
		SaImmAdminOwnerHandleT ownerHandle
		SaNameT objectName
		SaImmContinuationIdT continuationId

	returns:
		SaAisErrorT

	"""
	return omdll.saImmOmAdminOperationContinuationClear(ownerHandle,
			BYREF(objectName),
			continuationId)
