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
 * Generic factory for AIS services. Note that a
 * concrete class must be used to implement this interface. The
 * implementation class must be defined in the appropriate AIS
 * Service specification.
 *
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 */
public interface Factory<S extends Handle, C extends Callbacks> {

    /**
     * This method initializes the appropriate AIS service for the
     * invoking client and registers the various callback methods. This method
     * must be invoked prior to the invocation of any other API method for the
     * requested AIS service. The library handle is returned as the
     * reference to this association between the client and the AIS service.
     * The client uses this handle in subsequent
     * communication with the AIS service. Please note
     * that each invocation to initializeHandle() returns a new (i.e. different)
     * library handle. This allows support
     * for multithreaded dispatching of AIS callbacks.
     * <P><B>SAF Reference:</B> <code>sa&lt;Area&gt;Initialize()</code>
     *
     * @param callbacks [in]  The callbacks parameter refers to an object that
     * collects references to the callback objects implemented by the process
     * that the <area> library can invoke.
     * @param version [in/out] As an <I>input parameter</I> version is a reference to
     *            the required AIS service version. In
     *            this case minorVersion is ignored and should be set to 0x00.
     *            As an <i>output parameter</i> the version actually supported by the
     *            AIS service is delivered, as specified:
     *            <UL>
     *            <LI>If the implementation supports the specified releaseCode and majorVersion,
     *            version is set by this method as follows:
     *              <UL>
     *              <LI>releaseCode = required release code
     *              <LI>majorVersion = highest value of the major version that this
     *              implementation can support for the required releaseCode
     *              <LI>minorVersion = highest value of the minor version that this
     *              implementation can support for the required value of releaseCode and
     *              the returned value of majorVersion
     *              </UL>
     *            <LI>
     *            If the preceding condition cannot be met, AisVersionException is
     *            thrown, and the fields of the version parameter are set as follows:
     *            <pre>
     *    if (implementation supports the required releaseCode)
     *        releaseCode = required releaseCode
     *    else {
     *        if (implementation supports releaseCode higher than the required releaseCode)
     *            releaseCode = the lowest value of the supported release codes that is higher than the required releaseCode
     *        else
     *            releaseCode = the highest value of the supported release codes that is lower than the required releaseCode
     *    }
     *    majorVersion = highest value of the major versions that this implementation can support for the returned releaseCode
     *    minorVersion = highest value of the minor versions that this implementation can support for the returned values of releaseCode and majorVersion
     *            </pre>
     *            </UL>
     *
     * @return A reference to the handle designating a particular
     *         initialization of a particular AIS service library.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisVersionException The version parameter is not compatible with
     *             the version of the requested AIS service implementation.
     * @throws AisUnavailableException (<B>since</B> AIS-B.03.01)
     *             The operation requested in this call is unavailable on
     *             this cluster node because it is not a member node.
     */
    public S initializeHandle(C callbacks, Version version)
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisVersionException,
            AisUnavailableException;
}

/*  */
