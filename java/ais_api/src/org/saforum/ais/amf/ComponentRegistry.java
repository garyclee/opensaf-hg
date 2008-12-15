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
 * A ComponentRegistry object contains API methods that enable to register and unregister
 * components with the Availability Management Framework.
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface ComponentRegistry {

    /**
     * This method returns the name of the component the calling process belongs
     * to. Please note that this name is used internally by the other methods of
     * this ComponentRegistry object (i.e. you do not need to call this method
     * before registering this component).
     *
     * @return The name of the component the process invoking this method
     *         belongs to.
     */
    public String getComponentName();

    /**
     * This method can be used by an SA-aware component to register itself with
     * the Availability Management Framework.
     * <P><B>SAF Reference:</B> <code>saAmfComponentRegister()</code>
     *
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
     * @throws AisInitException The previous initialization of the associated
     *             AmfHandle object was incomplete, since one or more of
     *             the callbacks that are listed below were not supplied:
     *             TerminateComponentCallback, SetCsiCallback,
     *             RemoveCsiCallback.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisExistException The component has been registered previously,
     *             either via the AmfHandle object associated with this
     *             ComponentRegistry object or via another AmfHandle object.
     */
    public void registerComponent()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisExistException;

    /**
     * This method can be used by a proxy component to register a proxied
     * component with the Availability Management Framework.
     * <P><B>SAF Reference:</B> <code>saAmfComponentRegister()</code>
     *
     * @param proxiedComponentName [in] The name of the proxied component to be
     *            registered.
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
     * @throws AisInitException The previous initialization of the associated
     *             AmfHandle object was incomplete, since one or more of
     *             the callbacks that are listed below were not supplied:
     *             InstantiateProxiedComponentCallback and
     *             CleanupProxiedComponentCallback
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException This proxy component (represented by this
     *             ComponentRegistry instance) has not been registered
     *             previously.
     * @throws AisExistException The proxied component (identified by the
     *             specified proxiedComponentName parameter) has been registered
     *             previously, either via the associated AmfHandle object
     *             or via another AmfHandle object.
     * @throws AisBadOperationException This proxy component (represented by
     *             this ComponentRegistry instance) which is registering a
     *             proxied component has not been assigned the proxy CSI
     *             with the active HA state, through which the proxied component being
     *             registered is supposed to be proxied.
     */
    public void registerProxiedComponent( String proxiedComponentName )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException,
            AisExistException,
            AisBadOperationException;


    /**
     * The unregisterComponent() is used by an SA-aware component to unregister
     * itself. The SA-aware component indicates the Availability Management
     * Framework that it is unable to continue providing the service, possibly
     * because of a fault condition that is hindering its ability to provide service.
     * <P>
     * When an SA-aware component unregisters with the Availability Management Framework,
     * the framework treats such an unregistration as an error condition (similar to one
     * signaled by an ErrorReporting.reportComponentError() method) and engages the configured default
     * recovery action (recoveryOnError) on the component. As a consequence, its operational
     * state may become disabled, and, therefore, all of its component service instances
     * are removed from it.
     * <P>
     * During its life cycle, an SA-aware component can register or unregister
     * multiple times. It is understood that a failed component is implicitly
     * unregistered while it is cleaned up.
     *
     * <P><B>SAF Reference:</B> <code>saAmfComponentUnregister()</code>
     *
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
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component has not been registered
     *             previously.
     * @throws AisBadOperationException The requested unregistration is not
     *             acceptable because this component is a proxy component and
     *             it has not unregistered its proxied components before
     *             unregistering itself.
     */
    public void unregisterComponent()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException,
            AisBadOperationException;

    /**
     * The unregisterProxiedComponent() is used by a proxy component to
     * unregister one of its proxied components. This case will usually apply to
     * enable another proxy to register for the proxied component. Recall that
     * at a given time at most one proxy can exist for a component.
     * <P>
     * If a proxy component unregisters one of its proxied components,the operational state
     * of the latter does not change because unregistration does not indicate a failure in this
     * case. This is motivated by the
     * assumption that another proxy will soon take over the role of the
     * previous proxy.
     * <P>
     * During its life cycle, a proxy component can register or unregister a
     * proxied component multiple times. Before unregistering itself, a proxy
     * component must unregister all of its proxied components.
     * It is understood that a failed component is implicitly unregistered while it is cleaned
     * up.
     * <P><B>SAF Reference:</B> <code>saAmfComponentUnregister()</code>
     *
     * @param proxiedComponentName [in] The name of the proxied component to be
     *            unregistered.
     *
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
     * @throws AisNotExistException Either the proxy component (i.e. this
     *             component) or the proxied component, identified by
     *             proxiedCompName, has not been registered previously.
     * @throws AisBadOperationException The requested unregistration is not
     *             acceptable because this component is not the proxy of the
     *             proxied component identified by compName
     */
    public void unregisterProxiedComponent( String proxiedComponentName )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException,
            AisBadOperationException;

}



