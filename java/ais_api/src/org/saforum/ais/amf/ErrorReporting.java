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
 * An ErrorReporting object contains API methods that enable components to report an error and
 * provide a recovery recommendation to the Availability Management Framework.
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface ErrorReporting {



    /**
     * This method reports an error and provides a recovery recommendation to
     * the Availability Management Framework. The Availability Management
     * Framework will react on the error report and carry out a recovery
     * operation to retain the required availability of the component service
     * instances supported by the erroneous component. The Availability
     * Management Framework will not implement a weaker recovery action than
     * that recommended by the error report, but the Availability Management
     * Framework may decide to escalate to a higher recovery level.

     * <P><B>SAF Reference:</B> <code>saAmfComponentErrorReport()</code>
     *
     * @param erroneousComponent [in] The name of the erroneous component.
     * @param detectionTime [in] The absolute time when the reporting component
     *            detected the error. If this value is 0, it is assumed that the
     *            time at which the library received the error is the error
     *            detection time.
     * @param recommendedRecovery [in] Recommended recovery action.
     * @param ntfIdentifier [in] Identifier of the notification sent by the
     *            component to the Notification Service prior
     *            to reporting the error to the Availability Management
     *            Framework.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The handle amfHandle is invalid, since it
     *             is corrupted, uninitialized, or has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException
     *             The component specified by the name to which
     *             erroneousComponent refers is not contained in the
     *             Availability Management Framework's configuration.
     */
    public void reportComponentError(  String erroneousComponent,
                                              long detectionTime,
                                              RecommendedRecovery recommendedRecovery,
                                              long ntfIdentifier )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * This method cancels the previous errors reported about the component,
     * identified by componentName. The Availability Management Framework may
     * now change the component's operational state to "enabled", assuming that
     * nothing else prevents this. The Availability Management Framework may,
     * then, perform additional assignments of component service instances to
     * the component.
     * <P><B>SAF Reference:</B> <code>saAmfComponentErrorClear()</code>
     *
     * @param componentName [in] The name of the component to be cleared of any
     *            error.
     * @param ntfIdentifier [in] Identifier of the notification sent by the
     *            component to the System Management Notification Service.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The handle amfHandle is invalid, since it
     *             is corrupted, uninitialized, or has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException
     *             The component specified by the name to which
     *             componentName refers is not contained in the
     *             Availability Management Framework's configuration.
     */
    public void clearComponentError( String componentName, long ntfIdentifier )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

}


