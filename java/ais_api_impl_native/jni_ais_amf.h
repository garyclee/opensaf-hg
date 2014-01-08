/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_opensaf_ais_amf_AmfHandleImpl */

#ifndef _Included_org_opensaf_ais_amf_AmfHandleImpl
#define _Included_org_opensaf_ais_amf_AmfHandleImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_AmfHandleImpl
 * Method:    response
 * Signature: (JLorg/saforum/ais/AisStatus;)V
 */
	JNIEXPORT void JNICALL Java_org_opensaf_ais_amf_AmfHandleImpl_response
	(JNIEnv *, jobject, jlong, jobject);

/*
 * Class:     org_opensaf_ais_amf_AmfHandleImpl
 * Method:    finalizeAmfHandle
 * Signature: ()V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_AmfHandleImpl_finalizeAmfHandle(JNIEnv *,
								 jobject);

/*
 * Class:     org_opensaf_ais_amf_AmfHandleImpl
 * Method:    invokeSaAmfInitialize
 * Signature: (Lorg/saforum/ais/Version;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_AmfHandleImpl_invokeSaAmfInitialize(JNIEnv
								     *,
								     jobject,
								     jobject);

/*
 * Class:     org_opensaf_ais_amf_AmfHandleImpl
 * Method:    invokeSaAmfDispatch
 * Signature: (I)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_AmfHandleImpl_invokeSaAmfDispatch(JNIEnv *,
								   jobject,
								   jint);

/*
 * Class:     org_opensaf_ais_amf_AmfHandleImpl
 * Method:    invokeSaAmfDispatchWhenReady
 * Signature: ()V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_AmfHandleImpl_invokeSaAmfDispatchWhenReady
	(JNIEnv *, jobject);

/*
 * Class:     org_opensaf_ais_amf_AmfHandleImpl
 * Method:    invokeSaAmfSelectionObjectGet
 * Signature: ()V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_AmfHandleImpl_invokeSaAmfSelectionObjectGet
	(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_opensaf_ais_amf_ComponentRegistryImpl */
#ifndef _Included_org_opensaf_ais_amf_ComponentRegistryImpl
#define _Included_org_opensaf_ais_amf_ComponentRegistryImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_ComponentRegistryImpl
 * Method:    registerComponent
 * Signature: ()V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ComponentRegistryImpl_registerComponent
	(JNIEnv *, jobject);

/*
 * Class:     org_opensaf_ais_amf_ComponentRegistryImpl
 * Method:    registerProxiedComponent
 * Signature: (Ljava/lang/String;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ComponentRegistryImpl_registerProxiedComponent
	(JNIEnv *, jobject, jstring);

/*
 * Class:     org_opensaf_ais_amf_ComponentRegistryImpl
 * Method:    unregisterComponent
 * Signature: ()V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ComponentRegistryImpl_unregisterComponent
	(JNIEnv *, jobject);

/*
 * Class:     org_opensaf_ais_amf_ComponentRegistryImpl
 * Method:    unregisterProxiedComponent
 * Signature: (Ljava/lang/String;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ComponentRegistryImpl_unregisterProxiedComponent
	(JNIEnv *, jobject, jstring);

/*
 * Class:     org_opensaf_ais_amf_ComponentRegistryImpl
 * Method:    invokeSaAmfComponentNameGet
 * Signature: ()Ljava/lang/String;
 */
	JNIEXPORT jstring JNICALL
	Java_org_opensaf_ais_amf_ComponentRegistryImpl_invokeSaAmfComponentNameGet
	(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_opensaf_ais_amf_CsiManagerImpl */
#ifndef _Included_org_opensaf_ais_amf_CsiManagerImpl
#define _Included_org_opensaf_ais_amf_CsiManagerImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_CsiManagerImpl
 * Method:    completedCsiQuiescing
 * Signature: ((JLorg/saforum/ais/AisStatus;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_CsiManagerImpl_completedCsiQuiescing(JNIEnv
								      *,
								      jobject,
								      jlong,
								      jint);

/*
 * Class:     org_opensaf_ais_amf_CsiManagerImpl
 * Method:    getHaState
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Lorg/saforum/ais/amf/HaState;
 */
	JNIEXPORT jobject JNICALL
	Java_org_opensaf_ais_amf_CsiManagerImpl_getHaState(JNIEnv *,
							   jobject, jstring,
							   jstring);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_opensaf_ais_amf_ErrorReportingImpl */
#ifndef _Included_org_opensaf_ais_amf_ErrorReportingImpl
#define _Included_org_opensaf_ais_amf_ErrorReportingImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_ErrorReportingImpl
 * Method:    clearComponentError
 * Signature: (Ljava/lang/String;J)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ErrorReportingImpl_clearComponentError
	(JNIEnv *, jobject, jstring, jlong);

/*
 * Class:     org_opensaf_ais_amf_ErrorReportingImpl
 * Method:    reportComponentError
 * Signature: (Ljava/lang/String;JLorg/saforum/ais/amf/RecommendedRecovery;J)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ErrorReportingImpl_reportComponentError
	(JNIEnv *, jobject, jstring, jlong, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_opensaf_ais_amf_HealthcheckImpl */
#ifndef _Included_org_opensaf_ais_amf_HealthcheckImpl
#define _Included_org_opensaf_ais_amf_HealthcheckImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_HealthcheckImpl
 * Method:    confirmHealthcheck
 * Signature: (Ljava/lang/String;[BI)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_HealthcheckImpl_confirmHealthcheck(JNIEnv
								    *,
								    jobject,
								    jstring,
								    jbyteArray,
								    jint);

/*
 * Class:     org_opensaf_ais_amf_HealthcheckImpl
 * Method:    startHealthcheck
 * Signature: (Ljava/lang/String;[BLorg/saforum/ais/amf/Healthcheck/HealthcheckInvocation;Lorg/saforum/ais/amf/RecommendedRecovery;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_HealthcheckImpl_startHealthcheck(JNIEnv *,
								  jobject,
								  jstring,
								  jbyteArray,
								  jobject,
								  jobject);

/*
 * Class:     org_opensaf_ais_amf_HealthcheckImpl
 * Method:    stopHealthcheck
 * Signature: (Ljava/lang/String;[B)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_HealthcheckImpl_stopHealthcheck(JNIEnv *,
								 jobject,
								 jstring,
								 jbyteArray);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_opensaf_ais_amf_ProcessMonitoringImpl */
#ifndef _Included_org_opensaf_ais_amf_ProcessMonitoringImpl
#define _Included_org_opensaf_ais_amf_ProcessMonitoringImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_ProcessMonitoringImpl
 * Method:    startProcessMonitoring
 * Signature: (Ljava/lang/String;JIILorg/saforum/ais/amf/RecommendedRecovery;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ProcessMonitoringImpl_startProcessMonitoring
	(JNIEnv *, jobject, jstring, jlong, jint, jint, jobject);

/*
 * Class:     org_opensaf_ais_amf_ProcessMonitoringImpl
 * Method:    stopProcessMonitoring
 * Signature: (Ljava/lang/String;Lorg/saforum/ais/amf/ProcessMonitoring/PmStopQualifier;JI)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ProcessMonitoringImpl_stopProcessMonitoring
	(JNIEnv *, jobject, jstring, jobject, jlong, jint);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_opensaf_ais_amf_ProtectionGroupManagerImpl */
#ifndef _Included_org_opensaf_ais_amf_ProtectionGroupManagerImpl
#define _Included_org_opensaf_ais_amf_ProtectionGroupManagerImpl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_opensaf_ais_amf_ProtectionGroupManagerImpl
 * Method:    getProtectionGroup
 * Signature: (Ljava/lang/String;)[Lorg/saforum/ais/amf/ProtectionGroupNotification;
 */
	JNIEXPORT jobjectArray JNICALL
	Java_org_opensaf_ais_amf_ProtectionGroupManagerImpl_getProtectionGroup
	(JNIEnv *, jobject, jstring);

/*
 * Class:     org_opensaf_ais_amf_ProtectionGroupManagerImpl
 * Method:    getProtectionGroupAsync
 * Signature: (Ljava/lang/String;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ProtectionGroupManagerImpl_getProtectionGroupAsync
	(JNIEnv *, jobject, jstring);

/*
 * Class:     org_opensaf_ais_amf_ProtectionGroupManagerImpl
 * Method:    startProtectionGroupTracking
 * Signature: (Ljava/lang/String;Lorg/saforum/ais/TrackFlags;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ProtectionGroupManagerImpl_startProtectionGroupTracking
	(JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     org_opensaf_ais_amf_ProtectionGroupManagerImpl
 * Method:    stopProtectionGroupTracking
 * Signature: (Ljava/lang/String;)V
 */
	JNIEXPORT void JNICALL
	Java_org_opensaf_ais_amf_ProtectionGroupManagerImpl_stopProtectionGroupTracking
	(JNIEnv *, jobject, jstring);

/*
 * Class:     org_opensaf_ais_amf_ProtectionGroupManagerImpl
 * Method:    getProtectionGroupThenStartTracking
 * Signature: (Ljava/lang/String;Lorg/saforum/ais/TrackFlags;)[Lorg/saforum/ais/amf/ProtectionGroupNotification;
 */
	JNIEXPORT jobjectArray JNICALL
	Java_org_opensaf_ais_amf_ProtectionGroupManagerImpl_getProtectionGroupThenStartTracking
	(JNIEnv *, jobject, jstring, jobject);

#ifdef __cplusplus
}
#endif
#endif
