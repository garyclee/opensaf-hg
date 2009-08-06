/******************************************************************************
**
** FILE:
**   saImmOi.h
**
** DESCRIPTION: 
**   This file provides the C language binding for the Service 
**   Availability(TM) Forum Information Model Management Service (IMM). 
**   It contains all the prototypes and type definitions required 
**   by the IMM Object Implementer APIs. 
**   
** SPECIFICATION VERSION:
**   SAI-AIS-IMM-A.02.01
**
** DATE: 
**   Thurs  June   28  2007
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS. 
**   The Specification and all worldwide copyrights therein are
**   the exclusive property of Licensor.  You may not remove, obscure, or
**   alter any copyright or other proprietary rights notices that are in or
**   on the copy of the Specification you download.  You must reproduce all
**   such notices on all copies of the Specification you make.  Licensor
**   may make changes to the Specification, or to items referenced therein,
**   at any time without notice.  Licensor is not obligated to support or
**   update the Specification. 
**   
**   Copyright(c) 2007, Service Availability(TM) Forum. All rights
**   reserved. 
**
******************************************************************************/

#ifndef _SA_IMM_OI_H
#define _SA_IMM_OI_H

#include <saImm.h>

#ifdef  __cplusplus
extern "C" {
#endif

  /* 5.2.1 IMM Service Handle */
typedef SaUint64T SaImmOiHandleT;

  /* 5.2.2 SaImmOiImplementerNameT */
typedef SaStringT SaImmOiImplementerNameT;

  /* 5.2.3 SaImmOiCcbIdT */
typedef SaUint64T SaImmOiCcbIdT;

  /* 5.2.4 SaImmOiCallbacksT */ 

  /* From 5.5.4  */
  typedef SaAisErrorT 
  (*SaImmOiRtAttrUpdateCallbackT)(
				  SaImmOiHandleT immOiHandle,
				  const SaNameT *objectName,
				  const SaImmAttrNameT *attributeNames
				  );
  /* From 5.6.1 */
#ifdef IMM_A_01_01
  typedef SaAisErrorT 
  (*SaImmOiCcbObjectCreateCallbackT)(
				     SaImmOiHandleT immOiHandle,
				     SaImmOiCcbIdT ccbId,
				     const SaImmClassNameT className,
				     const SaNameT *parentName,
				     const SaImmAttrValuesT **attr
				     );
#endif

  typedef SaAisErrorT 
  (*SaImmOiCcbObjectCreateCallbackT_2)(
				       SaImmOiHandleT immOiHandle,
				       SaImmOiCcbIdT ccbId,
				       const SaImmClassNameT className,
				       const SaNameT *parentName,
				       const SaImmAttrValuesT_2 **attr
				       );

  /* From 5.6.2  */
  typedef SaAisErrorT 
  (*SaImmOiCcbObjectDeleteCallbackT)(
				     SaImmOiHandleT immOiHandle,
				     SaImmOiCcbIdT ccbId,
				     const SaNameT *objectName
				     );
  /* From 5.6.3  */
#ifdef IMM_A_01_01
 typedef SaAisErrorT 
  (*SaImmOiCcbObjectModifyCallbackT)(
				     SaImmOiHandleT immOiHandle,
				     SaImmOiCcbIdT ccbId,
				     const SaNameT *objectName,
				     const SaImmAttrModificationT **attrMods
				     );
#endif

  typedef SaAisErrorT 
  (*SaImmOiCcbObjectModifyCallbackT_2)(
				       SaImmOiHandleT immOiHandle,
				       SaImmOiCcbIdT ccbId,
				       const SaNameT *objectName,
				       const SaImmAttrModificationT_2 **attrMods
				       );

  /* From 5.6.4  */
 typedef SaAisErrorT 
  (*SaImmOiCcbCompletedCallbackT)(
				  SaImmOiHandleT immOiHandle,
				  SaImmOiCcbIdT ccbId
				  );
  
  /* From 5.6.5  */
  typedef void 
  (*SaImmOiCcbApplyCallbackT)(
			      SaImmOiHandleT immOiHandle,
			      SaImmOiCcbIdT ccbId
			      );

  /* From 5.6.6  */
 typedef void 
  (*SaImmOiCcbAbortCallbackT)(
			      SaImmOiHandleT immOiHandle,
			      SaImmOiCcbIdT ccbId
			      );
 
 /* From 5.7.1  */
#ifdef IMM_A_01_01
  typedef void 
  (*SaImmOiAdminOperationCallbackT) (
				     SaImmOiHandleT immOiHandle,
				     SaInvocationT invocation,
				     const SaNameT *objectName,
				     SaImmAdminOperationIdT operationId,
				     const SaImmAdminOperationParamsT **params
				     );
#endif

  typedef void 
  (*SaImmOiAdminOperationCallbackT_2) (
				       SaImmOiHandleT immOiHandle,
				       SaInvocationT invocation,
				       const SaNameT *objectName,
				       SaImmAdminOperationIdT operationId,
				       const SaImmAdminOperationParamsT_2 **params
				       );


   /* SaImmOiCallbacksT */
#ifdef IMM_A_01_01
  typedef struct {
    SaImmOiRtAttrUpdateCallbackT saImmOiRtAttrUpdateCallback;
    SaImmOiCcbObjectCreateCallbackT saImmOiCcbObjectCreateCallback;
    SaImmOiCcbObjectDeleteCallbackT saImmOiCcbObjectDeleteCallback;
    SaImmOiCcbObjectModifyCallbackT saImmOiCcbObjectModifyCallback;
    SaImmOiCcbCompletedCallbackT saImmOiCcbCompletedCallback;
    SaImmOiCcbApplyCallbackT saImmOiCcbApplyCallback;
    SaImmOiCcbAbortCallbackT saImmOiCcbAbortCallback;
    SaImmOiAdminOperationCallbackT saImmOiAdminOperationCallback;
  } SaImmOiCallbacksT;
#endif

   /* SaImmOiCallbacksT_2 */
  typedef struct {
    /*SaImmOiAdminOperationCallbackT saImmOiAdminOperationCallback;*/
    /*The downloaded includefile contained the above line which is a bug!!*/
    SaImmOiAdminOperationCallbackT_2 saImmOiAdminOperationCallback;
    SaImmOiCcbAbortCallbackT    saImmOiCcbAbortCallback;
    SaImmOiCcbApplyCallbackT    saImmOiCcbApplyCallback;
    SaImmOiCcbCompletedCallbackT    saImmOiCcbCompletedCallback;
    SaImmOiCcbObjectCreateCallbackT_2    saImmOiCcbObjectCreateCallback;
    SaImmOiCcbObjectDeleteCallbackT    saImmOiCcbObjectDeleteCallback;
    SaImmOiCcbObjectModifyCallbackT_2    saImmOiCcbObjectModifyCallback;
    SaImmOiRtAttrUpdateCallbackT    saImmOiRtAttrUpdateCallback;
  } SaImmOiCallbacksT_2;


  /* 5.3.1 saImmOiInitialize() */
#ifdef IMM_A_01_01
  extern SaAisErrorT
  saImmOiInitialize(
		    SaImmOiHandleT *immOiHandle,
		    const SaImmOiCallbacksT *immOiCallbacks,
		    SaVersionT *version
		    );
#endif

  extern SaAisErrorT 
  saImmOiInitialize_2(
		      SaImmOiHandleT *immOiHandle,
		      const SaImmOiCallbacksT_2 *immOiCallbacks,
		      SaVersionT *version
		      );

  /* 5.3.2 saImmOiSelectionObjectGet() */

  extern SaAisErrorT
  saImmOiSelectionObjectGet(
			    SaImmOiHandleT immOiHandle,
			    SaSelectionObjectT *selectionObject
			    );

  /* 5.3.3 saImmOiDispatch() */

  extern SaAisErrorT
  saImmOiDispatch(
		  SaImmOiHandleT immOiHandle,
		  SaDispatchFlagsT dispatchFlags
		  );

  /* 5.3.4 saImmOiFinalize() */
  
  extern SaAisErrorT
  saImmOiFinalize(
		  SaImmOiHandleT immOiHandle
		  );

  /* 5.4.1 saImmOiImplementerSet() */

  extern SaAisErrorT
  saImmOiImplementerSet(
			SaImmOiHandleT immOiHandle,
			const SaImmOiImplementerNameT implementerName
			);

  /* 5.4.2 saImmOiImplementerClear() */

  extern SaAisErrorT
  saImmOiImplementerClear(
			  SaImmOiHandleT immOiHandle
			  );

  /* 5.4.3 saImmOiClassImplementerSet() */
  
  extern SaAisErrorT
  saImmOiClassImplementerSet(
			     SaImmOiHandleT immOiHandle,
			     const SaImmClassNameT className
			     );

  /* 5.4.4 saImmOiClassImplementerRelease() */
  
  extern SaAisErrorT
  saImmOiClassImplementerRelease(
				 SaImmOiHandleT immOiHandle,
				 const SaImmClassNameT className
				 );

  /* 5.4.5 saImmOiObjectImplementerSet() */
  
  extern SaAisErrorT
  saImmOiObjectImplementerSet(
			      SaImmOiHandleT immOiHandle,
			      const SaNameT *objectName,
			      SaImmScopeT scope
			      );

  /* 5.4.6 saImmOiObjectImplementerRelease() */
  
  extern SaAisErrorT
  saImmOiObjectImplementerRelease(
				  SaImmOiHandleT immOiHandle,
				  const SaNameT *objectName,
				  SaImmScopeT scope
				  );
  /*
   *
   * Runtime Object Management Routines
   *
   */

  /* 5.5.1 saImmOiRtObjectCreate() */
#ifdef IMM_A_01_01
  extern SaAisErrorT
  saImmOiRtObjectCreate(
			SaImmOiHandleT immOiHandle,
			const SaImmClassNameT className,
			const SaNameT *parentName,
			const SaImmAttrValuesT **attrValues
			);
#endif

  extern SaAisErrorT
  saImmOiRtObjectCreate_2(
			  SaImmOiHandleT immOiHandle,
			  const SaImmClassNameT className,
			  const SaNameT *parentName,
			  const SaImmAttrValuesT_2 **attrValues
			  );

  /* 5.5.2 saImmOiRtObjectDelete() */
  
  extern SaAisErrorT
  saImmOiRtObjectDelete(
			SaImmOiHandleT immOiHandle,
			const SaNameT *objectName
			);

  /* 5.5.3 saImmOiRtObjectUpdate() */

#ifdef IMM_A_01_01  
  extern SaAisErrorT
  saImmOiRtObjectUpdate(

			SaImmOiHandleT immOiHandle,
			const SaNameT *objectName,
			const SaImmAttrModificationT **attrMods
			);
#endif
  extern  SaAisErrorT
  saImmOiRtObjectUpdate_2(
			  SaImmOiHandleT immOiHandle,
			  const SaNameT *objectName,
			  const SaImmAttrModificationT_2 **attrMods
			  );

  /* 5.5.4 SaImmOiRtAttrUpdateCallbackT see 5.2.4 above */


  /*
   *
   * Configuration Object Implementer Routines
   *
   */

  /* 5.6.1 SaImmOiCcbObjectCreateCallbackT see 5.2.4 above */

  /* 5.6.2 SaImmOiCcbObjectDeleteCallbackT see 5.2.4 above  */

  /* 5.6.3 SaImmOiCcbObjectModifyCallbackT see 5.2.4 above  */

 
  /* 5.6.4 SaImmOiCcbCompletedCallbackT see 5.2.4 above  */

 
  /* 5.6.5 SaImmOiCcbApplyCallbackT see 5.2.4 above  */

  /* 5.6.6 SaImmOiCcbAbortCallbackT see 5.2.4 above  */
  
  /*
   *
   * Administrative Operations
   *
   */


  /* 5.7.1 SaImmOiAdminOperationCallbackT see 5.2.4 above  */

  /* 5.7.2 saImmOiAdminOperationResult() */
  
  extern SaAisErrorT
  saImmOiAdminOperationResult(
			      SaImmOiHandleT immOiHandle,
			      SaInvocationT invocation,
			      SaAisErrorT result
			      );

#ifdef  __cplusplus
}
#endif

#endif  /* _SA_IMM_OI_H */











