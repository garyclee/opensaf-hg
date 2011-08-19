/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R6-A.01.01
**   SAI-Overview-B.05.01
**
** DATE: 
**   Wednesday November 19, 2008
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS.
**
** Copyright 2008 by the Service Availability Forum. All rights reserved.
**
** Permission to use, copy, and distribute this mapping specification for any
** purpose without fee is hereby granted, provided that this entire notice
** is included in all copies. No permission is granted for, and users are
** prohibited from, modifying or making derivative works of the mapping
** specification.
**
*******************************************************************************/
package org.saforum.ais;

/**
 * A status value indicating the result of a requested operation.
 *
 * <P><B>SAF Reference:</B> <code>SaAisErrorT</code>
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 *
 */
public enum AisStatus implements EnumValue {

    // B.01.01:

    /**
     * A status value indicating that the operation completed successfully.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_OK</code>
     * @since SAI-Overview-B.01.01
     *
     */
    OK(1),

    /**
     *
     * A status value indicating that an unexpected problem occurred in the AIS
     * library (such as corruption). The library cannot be used anymore.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_LIBRARY</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_LIBRARY(2),

    /**
     * A status value indicating any of the following two cases:
     * <UL>
     * <LI>The version specified in the call to initialize an instance of the service library is
     * not compatible with the version of the implementation of the particular service.
     * <LI>The invoked method is not supported in the version specified in the call to initialize
     * the used instance of the service library.
     * </UL>
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_VERSION</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_VERSION(3),

    /**
     * A status value indicating that the callback interface that is required for this
     * API has not been supplied during the initialization of the library handle.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_INIT</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_INIT(4),

    /**
     * A status value indicating that an implementation-dependent timeout or the
     * timeout specified in the API call occurred before the call could complete. It
     * is unspecified whether the call succeeded or whether it did not.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_TIMEOUT</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_TIMEOUT(5),

    /**
     * A status value indicating that the native AIS service cannot be provided at
     * this time. The component or process might try again later.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_TRY_AGAIN</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_TRY_AGAIN(6),

    /**
     * A status value indicating that a parameter is not set correctly.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_INVALID_PARAM</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_INVALID_PARAM(7),

    /**
     * A status value indicating that either the service library or the provider of
     * the service is out of memory and cannot provide the service.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NO_MEMORY</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NO_MEMORY(8),

    /**
     * A status value indicating that a handle is invalid.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_BAD_HANDLE</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_BAD_HANDLE(9),

    /**
     * A status value indicating that the resource is already in use or
     * the AIS Service is busy with another task.     *
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_BUSY</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_BUSY(10),

    /**
     * A status value indicating that the access is denied
     * due to a reason other than a security violation.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_ACCESS</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_ACCESS(11),

    /**
     * A status value indicating that the entity, referenced by the invoker, does not
     * exist.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NOT_EXIST</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NOT_EXIST(12),

    /**
     * A status value indicating that the specified name exceeds the allowed maximum
     * length.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NAME_TOO_LONG</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NAME_TOO_LONG(13),

    /**
     * A status value indicating that the entity, referenced by the invoker, already
     * exists.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_EXIST</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_EXIST(14),

    /**
     * A status value indicating that the buffer provided by the component or process
     * is too small.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NO_SPACE</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NO_SPACE(15),

    /**
     * A status value indicating that the request was canceled by a timeout or other
     * interrupt.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_INTERRUPT</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_INTERRUPT(16),

    /**
     * A status value indicating that there are not enough resources
     * to provide the service.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NO_RESOURCES</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NO_RESOURCES(18),

    /**
     * A status value indicating that the requested function is not supported.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NOT_SUPPORTED</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NOT_SUPPORTED(19),

    /**
     * A status value indicating that the requested operation is not allowed.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_BAD_OPERATION</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_BAD_OPERATION(20),

    /**
     * A status value indicating that the requested operation failed.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_FAILED_OPERATION</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_FAILED_OPERATION(21),

    /**
     * A status value indicating that a communication error occurred.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_MESSAGE</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_MESSAGE(22),

    /**
     * A status value indicating that the destination queue does not contain enough
     * space for the entire message. For a detailed the description of this error,
     * refer to the Message Service specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_QUEUE_FULL</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_QUEUE_FULL(23),

    /**
     * A status value indicating that the destination queue is not available.
     * <P>For the detailed description of this error, refer
     * to the Message Service specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NOT_AVAILABLE</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_QUEUE_NOT_AVAILABLE(24),

    /**
     * A status value indicating that the flags are invalid.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_BAD_FLAGS</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_BAD_FLAGS(25),

    /**
     * A status value indicating that value is larger than the maximum value
     * permitted.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_TOO_BIG</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_TOO_BIG(26),

    /**
     * A status value indicating that there are no or no more sections matching the
     * specified sections in the section iterator.
     * <P>For the detailed description of this error, refer
     * to the Checkpoint Service specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NO_SECTIONS</code>
     * @since SAI-Overview-B.01.01
     *
     */
    ERR_NO_SECTIONS(27),

    // B.02.01:

    /**
     * A status value indicating that the requested operation had no effect..
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NO_OP</code>
     * @since SAI-Overview-B.02.01
     *
     */
    ERR_NO_OP(28),

    /**
     * A status value indicating that the administrative operation is only partially
     * completed as some targeted components must be repaired..
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_REPAIR_PENDING</code>
     * @since SAI-Overview-B.02.01
     *
     */
    ERR_REPAIR_PENDING(29),

    // B.03.01:

    /**
     * A status value indicating that there are no more name-to-object bindings in the context.
     * <P>For the detailed description of this error, refer
     * to the Naming Service specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NO_BINDINGS</code>
     * @since SAI-Overview-B.03.01
     *
     */
    ERR_NO_BINDINGS(30),

    /**
     * A status value indicating that the operation requested in this call is unavailable on
     * this cluster node as the cluster node is not a member node, and the requested operation
     * is not permitted on a non-member node.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_UNAVAILABLE</code>
     * @since SAI-Overview-B.03.01
     *
     */
    ERR_UNAVAILABLE(31),

    // B.04.01:

    /**
     * A status value indicating that the upgrade campaign execution
     * failed due to an upgrade procedure of the upgrade campaign notifying a failure
     * caused by the step retry counter being exceeded during execution or caused by the
     * detection of an asynchronous failure of an upgraded entity. The upgrade campaign is
     * now in the Suspended by Error Detected state.
     * <P>For the detailed description of this error,
     * refer to the Software Management Framework specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_CAMPAIGN_ERROR_DETECTED(32),

    /**
     * <P>For the detailed description of this error,
     * refer to the Software Management Framework specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_CAMPAIGN_PROC_FAILED</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_CAMPAIGN_PROC_FAILED(33),

    /**
     * <P>For the detailed description of this error,
     * refer to the Software Management Framework specification .
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_CAMPAIGN_CANCELED</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_CAMPAIGN_CANCELED(34),

    /**
     * <P>For the detailed description of this error,
     * refer to the Software Management Framework specification.
     *
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_CAMPAIGN_FAILED</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_CAMPAIGN_FAILED(35),

    /**
     * A status value indicating that the rollback of the upgrade campaign was
     * suspended due to an AMF asynchronous error notification pertaining to the upgraded
     * entities. The upgrade campaign is now in the Rollback Suspended state.
     * <P>For the detailed description of this error,
     * refer to the Software Management Framework specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_CAMPAIGN_SUSPENDED</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_CAMPAIGN_SUSPENDED(36),

    /**
     * A status value indicating that the execution of the upgrade campaign
     * was suspended through an invocation of an SA_SMF_CAMPAIGN_SUSPEND operation.
     * The upgrade campaign is now in the Suspending Execution state.
     * <P>For the detailed description of this error,
     * refer to the Software Management Framework specification.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_CAMPAIGN_SUSPENDING</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_CAMPAIGN_SUSPENDING(37),

    /**
     * A status value indicating that the required access to a particular function of the
     * AIS Service is denied due to a security violation.
     *
     * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_ACCESS_DENIED</code>
     * @since SAI-Overview-B.04.01
     *
     */
    ERR_ACCESS_DENIED(38),
    
    // B.05.01:

    /**
	 * A status value indicating that an Availability Management Framework
	 * component cannot assume a specified HA state.
	 * 
	 * <P>
	 * <B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NOT_READY</code>
	 * @since SAI-Overview-B.05.01
	 * 
	 */
    ERR_NOT_READY(39),

    /**
	 * A status value indicating that the requested operation was accepted and
	 * applied at the information model level. However, its complete deployment
	 * in the running system may not be guaranteed at the moment.
	 * 
	 * <P>
	 * <B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_DEPLOYMENT</code>
	 * @since SAI-Overview-B.05.01
	 * 
	 */
    ERR_DEPLOYMENT(40);

    /**
     * The numerical value assigned to this constant by the AIS specification.
     */
    private final int value;

    /**
     * Creates an enum constant with the numerical value assigned by the AIS specification.
     * @param value The numerical value assigned to this constant by the AIS specification.
     */
    private AisStatus( int value ){
        this.value = value;
    }

    /**
     * Returns the numerical value assigned to this constant by the AIS specification.
     * @return the numerical value assigned to this constant by the AIS specification.
     */
    @Override
    public int getValue() {
        return this.value;
    }
}

/*  */
