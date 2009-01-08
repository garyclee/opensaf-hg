/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-AMF-B.01.01
**
** DATE:
**   Wed Aug 6 2008
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

package org.saforum.ais.amf;

import org.saforum.ais.*;


/**
 * This class contains API methods that enable to monitor the health of a
 * component.
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface Healthcheck {



    /**
     * This method starts healthchecks via the invoking process for the
     * component designated by componentName. The type of the healthcheck
     * (component-invoked or Framework-invoked) is specified by invocationType.
     * If invocationType is INVOKED, the HealthcheckCallback object must have been
     * supplied when the process invoked the initializeHandle() call of AmfHandle.
     * <P>
     * If a component wants to start more than one healthcheck, it should invoke
     * this method once for each individual healthcheck.
     * It is, however, not possible to have at a given time and on the same
     * Healthcheck object two healthchecks started for the same component
     * name and healthcheck key.
     * <P><B>SAF Reference:</B> <code>saAmfHealthcheckStart()</code>
     *
     * @param componentName [in] The name of the component to be healthchecked.
     * @param healthcheckKey [in] The key of the healthcheck to be executed.
     *            Using this key, the Availability Management Framework can
     *            retrieve the corresponding healthcheck parameters.
     * @param invocationType [in] This parameter indicates whether the
     *            Availability Management Framework or the process itself will
     *            invoke the healthcheck calls.
     * @param recommendedRecovery [in] Recommended recovery to be performed by
     *            the Availability Management Framework if the component fails a
     *            healthcheck.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The handle amfHandle is invalid, since it
     *             is corrupted, uninitialized, or has already been finalized.
     * @throws AisInitException The previous initialization of the associated
     *             AmfHandle object was incomplete, since the
     *             HealthcheckCallback callback is missing and invocationType
     *             specifies Healthcheck.HealthcheckInvocation.AMF_INVOKED.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException Either one or both of the cases that follow
     *             apply:
     *             <UL>
     *             <LI>The Availability Management Framework is not aware of a
     *             component designated by componentName.
     *             <LI>The healthcheck with healthcheckKey is not configured
     *             for the component named componentName.
     *             </UL>
     * @throws AisExistException
     *                The healthcheck has already been started for
     *             the component, designated by componentName, and the same
     *             values of invocationType and healthcheckKey.
     */
    public void startHealthcheck( String componentName,
                                        byte[] healthcheckKey,
                                        HealthcheckInvocation invocationType,
                                        RecommendedRecovery recommendedRecovery )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException,
            AisExistException;

    /**
     * This method is used to stop the healthcheck, referred to by
     * healthcheckKey, for the component designated by componentName.
     * <P><B>SAF Reference:</B> <code>saAmfHealthcheckStop()</code>
     *
     * @param componentName [in] The name of the component for which
     *            healthchecks are to be stopped.
     * @param healthcheckKey [in] The key of the healthcheck to be stopped.
     *            Using this key, the Availability Management Framework can
     *            retrieve the corresponding healthcheck parameters.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The handle amfHandle is invalid, since it
     *             is corrupted, uninitialized, or has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly. A
     *             specific example is when the calling process is not the
     *             process that has started the associated healthcheck.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException Either one or both of the cases that follow
     *             apply:
     *             <UL>
     *             <LI>The Availability Management Framework is not aware of a
     *             component designated by componentName.
     *             <LI>No healthcheck has been started for the component,
     *             designated by componentName, and the specified parameters
     *             healthcheckKey.
     *             </UL>
     */
    public void stopHealthcheck( String componentName,
                                       byte[] healthcheckKey )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * This method allows a process to inform the Availability Management
     * Framework that it has performed the healthcheck identified by
     * healthcheckKey for the component designated by componentName, and whether
     * the healthcheck was successful or not.
     * <P><B>SAF Reference:</B> <code>saAmfHealthcheckConfirm()</code>
     *
     * @param componentName [in] The name of the component the healthcheck
     *            result is being reported for.
     * @param healthcheckKey [in] The key of the healthcheck to be executed.
     *            Using this key, the Availability Management Framework can
     *            retrieve the corresponding healthcheck parameters.
     * @param healthcheckResult - This parameter indicates the result of the
     *            healthcheck performed. It can take one of the following
     *            values:
     *            <UL>
     *            <LI>AisStatus.OK - The healthcheck completed
     *            successfully.
     *            <LI>AisStatus.FAILED_OPERATION: The component failed
     *            to successfully execute the given healthcheck and has reported
     *            an error on itself using the reportComponentError() of ErrorReporting.
     *            </UL>
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The handle amfHandle is invalid, since it
     *             is corrupted, uninitialized, or has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly. A
     *             specific example is when the calling process is not the
     *             process that has started the associated healthcheck.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException Either one or both of the cases that follow
     *             apply:
     *             <UL>
     *             <LI>The Availability Management Framework is not aware of a
     *             component designated by componentName.
     *             <LI>No healthcheck has been started for the component,
     *             designated by componentName, and the specified parameters
     *             healthcheckKey.
     *             </UL>
     */
    public void confirmHealthcheck( String componentName,
                                           byte[] healthcheckKey,
                                           int healthcheckResult )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;


    /**
     * This enum defines the possible ways healthchecks can be invoked.
     * <P><B>SAF Reference:</B> <code>SaAmfHeathcheckInvocationT</code>
     * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since AMF-B.01.01
     */
    public enum HealthcheckInvocation {

        /**
         * The healthchecks are invoked by the Availability Management Framework.
         */
        AMF_INVOKED(1),

        /**
         * The healthchecks are invoked by the Component.
         */
        COMPONENT_INVOKED(2);

        /**
         * The numerical value assigned to this constant by the AIS specification.
         */
        private final int value;



        /**
         * Creates an enum constant with the numerical value assigned by the AIS specification.
         * @param value The numerical value assigned to this constant by the AIS specification.
         */
        private HealthcheckInvocation( int value ){
            this.value = value;
        }



        /**
         * Returns the numerical value assigned to this constant by the AIS specification.
         * @return the numerical value assigned to this constant by the AIS specification.
         */
        public int getValue() {
            return this.value;
        }

    }

}



