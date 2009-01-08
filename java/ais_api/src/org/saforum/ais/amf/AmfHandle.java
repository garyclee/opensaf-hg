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
 * An AmfHandle object represents a reference to the association between
 * a Component (the client) and the Availability Management Framework.
 * The client uses this handle in subsequent communication with the
 * Availability Management Framework. AIS libraries must
 * support several AmfHandle instances from the same binary
 * program (e.g., process in the POSIX.1 world).
 *
 * <P><B>SAF Reference:</B> <code>SaAmfHandleT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 * @see AmfHandleFactory
 * @see AmfHandleFactory#initializeHandle
 */
public interface AmfHandle extends Handle {


    /**
     * The Callbacks class bundles the callback interfaces supplied by a client
     * to the Availability Management Framework.
     * <P><B>SAF Reference:</B> <code>SaAmfCallbacksT</code>
     * <P><B>SAF Reference:</B> <code>SaAmfCallbacksT_3</code>
     * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since AMF-B.01.01
     *
     */
    final class Callbacks extends org.saforum.ais.Callbacks {

        /**
         * Reference to the HealthcheckCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public HealthcheckCallback healthcheckCallback;

        /**
         * Reference to the TerminateComponentCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public TerminateComponentCallback terminateComponentCallback;

        /**
         * Reference to the SetCsiCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public SetCsiCallback setCsiCallback;

        /**
         * Reference to the RemoveCsiCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public RemoveCsiCallback removeCsiCallback;

        /**
         * Reference to the TrackProtectionGroupCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public TrackProtectionGroupCallback trackProtectionGroupCallback;

        /**
         * Reference to the InstantiateProxiedComponentCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public InstantiateProxiedComponentCallback instantiateProxiedComponentCallback;

        /**
         * Reference to the CleanupProxiedComponentCallback object that
         * the Availability Management Framework may invoke on the component.
         * @since AMF-B.01.01
         */
        public CleanupProxiedComponentCallback cleanupProxiedComponentCallback;

    }

    /**
     * The component responds to the Availability Management Framework with the
     * result of its execution of a particular request of the Availability
     * Management Framework, designated by invocation. The request can be of one
     * of the following types:
     * <UL>
     * <LI>Request for executing a given healthcheck.
     * <LI>Request for terminating a component.
     * <LI>Request for adding/assigning a given state to a component on behalf
     * of a component service instance.
     * <LI>Request for removing a component service instance from a component.
     * <LI>Request for instantiating a proxied component.
     * <LI>Request for cleaning up a proxied component.
     * </UL>
     * The component replies to the Availability Management Framework when
     * either (i) it cannot carry out the request, or (ii) it has failed to
     * complete the execution of the request, or (iii) it has
     * completed the request. With the exception of the response to a
     * HealthcheckCallback.healthcheckCallback() callback, this method may be
     * called only by a registered process, that is, the amfHandle must be the
     * same that was used when the registered process registered this component
     * via the registerComponent() call of the ComponentRegistry class. The
     * response to a HealthcheckCallback.healthcheckCallback() callback may only
     * be issued by the process that started this healthcheck.
     * <P><B>SAF Reference:</B> <code>saAmfResponse()</code>
     *
     * @param invocation [in] This parameter associates an invocation of this
     *            response method with a particular invocation of a callback
     *            method by the Availability Management Framework.
     * @param error [in] The response of the process to the associated callback.
     *            It returns AisStatus.OK if the associated callback was
     *            successfully executed by the process; otherwise, it returns an
     *            appropriate error as described in the corresponding callback.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ComponentRegistry object is invalid, since it is corrupted or
     *             has already been finalized.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     */
    public void response( long invocation, AisStatus error )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * Returns a ComponentRegistry object that enables to register and
     * unregister components with the Availability Management Framework. Please
     * note that the returned instance has a one-to-one association with the
     * library handle: it is cached and returned by subsequent calls to this
     * method. Also note that a the name of the component is automatically
     * fetched from the Availability management Framework
     * if the invocation of this method causes the
     * creation of the returned ComponentRegistry object.
     *
     * <P>
     * <B>SAF Reference:</B> <code>saAmfComponentNameGet()</code>
     *
     * @return an object associated with this library handle that enables to
     *         register and unregister components with the Availability
     *         Management Framework.
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The Availability Management Framework is not
     *             aware of any component associated with the invoking process.
     */
    public ComponentRegistry getComponentRegistry()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * Returns a CsiManager object that enables to
     * manage the HA state of components on behalf of the component
     * service instances that they support.
     * Please note that the returned instance has a
     * one-to-one association with the library handle: it is cached
     * and returned by subsequent calls to this method.
     *
     * @return an object associated with this library handle that enables to
     *         manage the HA state of components on behalf of the component
     *         service instances that they support.
     */
    public CsiManager getCsiManager() ;

    /**
     * Returns a ProcessMonitoring object that enables
     * components to request passive monitoring of their processes by
     * the Availability Management Framework.
     * Please note that the returned instance has a
     * one-to-one association with the library handle: it is cached
     * and returned by subsequent calls to this method.
     *
     * @return an object associated with this library handle that enables
     *         components to request passive monitoring of their processes by
     *         the Availability Management Framework.
     */
    public ProcessMonitoring getProcessMonitoring() ;

    /**
     * Returns a ProtectionGroupManager object that enables
     *         tracking changes in the protection group associated with a
     *         component service instance, or changes of attributes of any
     *         component in the protection group.
     * Please note that the returned instance has a
     * one-to-one association with the library handle: it is cached
     * and returned by subsequent calls to this method.
     *
     * @return an object associated with this library handle that enables
     *         tracking changes in the protection group associated with a
     *         component service instance, or changes of attributes of any
     *         component in the protection group.
     */
    public ProtectionGroupManager getProtectionGroupManager() ;

    /**
     * Returns an ErrorReporting object that enables
     *         components to report an error and provide a recovery
     *         recommendation to the Availability Management Framework.
     * Please note that the returned instance has a
     * one-to-one association with the library handle: it is cached
     * and returned by subsequent calls to this method.
     *
     * @return an object associated with this library handle that enables
     *         components to report an error and provide a recovery
     *         recommendation to the Availability Management Framework.
     */
    public ErrorReporting getErrorReporting() ;

    /**
     * Returns a Healthcheck object that enables to
     *         monitor the health of a component.
     * Please note that the returned instance has a
     * one-to-one association with the library handle: it is cached
     * and returned by subsequent calls to this method.
     *
     * @return an object associated with this library handle that enables to
     *         monitor the health of a component.
     */
    public Healthcheck getHealthcheck() ;

}


