/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Nokia Siemens Networks, OptXware Research & Development LLC.
 */

package org.opensaf.ais.amf;

import java.util.HashMap;
import java.util.Map;

import org.opensaf.ais.HandleImpl;
import org.saforum.ais.AisBadHandleException;
import org.saforum.ais.AisInvalidParamException;
import org.saforum.ais.AisLibraryException;
import org.saforum.ais.AisNoMemoryException;
import org.saforum.ais.AisNoResourcesException;
import org.saforum.ais.AisNotExistException;
import org.saforum.ais.AisStatus;
import org.saforum.ais.AisTimeoutException;
import org.saforum.ais.AisTryAgainException;
import org.saforum.ais.AisVersionException;
import org.saforum.ais.DispatchFlags;
import org.saforum.ais.Version;
import org.saforum.ais.amf.AmfHandle;
import org.saforum.ais.amf.CleanupProxiedComponentCallback;
import org.saforum.ais.amf.ComponentRegistry;
import org.saforum.ais.amf.CsiDescriptor;
import org.saforum.ais.amf.CsiManager;
import org.saforum.ais.amf.ErrorReporting;
import org.saforum.ais.amf.HaState;
import org.saforum.ais.amf.Healthcheck;
import org.saforum.ais.amf.HealthcheckCallback;
import org.saforum.ais.amf.InstantiateProxiedComponentCallback;
import org.saforum.ais.amf.ProcessMonitoring;
import org.saforum.ais.amf.ProtectionGroupManager;
import org.saforum.ais.amf.ProtectionGroupNotification;
import org.saforum.ais.amf.RemoveCsiCallback;
import org.saforum.ais.amf.SetCsiCallback;
import org.saforum.ais.amf.TerminateComponentCallback;
import org.saforum.ais.amf.TrackProtectionGroupCallback;

public final class AmfHandleImpl extends HandleImpl implements AmfHandle {

	/**
     * A Map containing the entries of the following structure:
     * <UL>
     * <LI>The key is a thread reference
     * <LI>The value is a library instance for which the dispatch method is
     * currently being executed in the context of the thread referred by the
     * key.
     * </UL>
     * Map entries are created and destroyed dynamically in the dispatch method.
     *
     * @see #dispatch(DispatchFlags)
     */
    private static Map< Thread, AmfHandleImpl > s_handleMap;
    // TODO s_handleMap must be synchronised!

    static {
		String libraryName = System.getProperty("nativeLibrary");

		if (libraryName == null)
			throw new NullPointerException("AmfHandle initialization: System property 'nativeLibrary' is not set. Use -DnativeLibrary in the command line!");

		System.loadLibrary(libraryName);
        s_handleMap = new HashMap< Thread, AmfHandleImpl >();
        // System.out.println( "JAVA: Initialized hashtable." );
    }

    /**
     * TODO
     */
    private ComponentRegistry componentRegistry;

    /**
     * TODO
     */
    private CsiManager csiManager;

    /**
     * TODO
     */
    private ErrorReporting errorReporting;

    /**
     * TODO
     */
    private Healthcheck healthcheck;

    /**
     * TODO
     */
    private ProcessMonitoring processMonitoring;

    /**
     * TODO
     */
    private ProtectionGroupManager protectionGroupManager;

    /**
     *
     */
    private HealthcheckCallback healthcheckCallback;

    /**
     * TODO
     */
    private SetCsiCallback setCsiCallback;

    /**
     * TODO
     */
    private RemoveCsiCallback removeCsiCallback;

    /**
     * TODO
     */
    private TerminateComponentCallback terminateComponentCallback;

    /**
     * TODO
     */
    private InstantiateProxiedComponentCallback instantiateProxiedComponentCallback;

    /**
     * TODO
     */
    private CleanupProxiedComponentCallback cleanupProxiedComponentCallback;

    /**
     * TODO
     */
    private TrackProtectionGroupCallback trackProtectionGroupCallback;

    /**
     * The handle designating this particular initialization of the Availability
     * Management Framework, returned by the saAmfInitialize function of the
     * underlying AIS implementation.
     *
     * @see #invokeSaAmfInitialize(Version)
     */
    private long saAmfHandle = 0;

    /**
     * This method initializes the Availability Management Framework for the
     * invoking process and registers the various callback methods. This method
     * must be invoked prior to the invocation of any other Availability
     * Management Framework API method. The library handle is returned as the
     * reference to this association between the process and the Availability
     * Management Framework. The process uses this handle in subsequent
     * communication with the Availability Management Framework. Please note
     * that each invocation to initializeHandle() returns a new (i.e. different)
     * library handle.
     *
     * @param healthcheckCallback [in] If this parameter is set to NULL then no
     *            HealthcheckCallback callback is registered; otherwise it is a
     *            reference to a HealthcheckCallback object containing the
     *            callback of the process that the Availability Management
     *            Framework may invoke.
     * @param setCsiCallback [in] If this parameter is set to NULL then no
     *            SetCsiCallback callback is registered; otherwise it is a
     *            reference to a SetCsiCallback object containing the callback
     *            of the process that the Availability Management Framework may
     *            invoke.
     * @param removeCsiCallback [in] If this parameter is set to NULL then no
     *            RemoveCsiCallback callback is registered; otherwise it is a
     *            reference to a RemoveCsiCallback object containing the
     *            callback of the process that the Availability Management
     *            Framework may invoke.
     * @param terminateComponentCallback [in] If this parameter is set to NULL
     *            then no TerminateComponentCallback callback is registered;
     *            otherwise it is a reference to a TerminateComponentCallback
     *            object containing the callback of the process that the
     *            Availability Management Framework may invoke.
     * @param instantiateProxiedComponentCallback [in] If this parameter is set
     *            to NULL then no InstantiateProxiedComponentCallback callback
     *            is registered; otherwise it is a reference to a
     *            InstantiateProxiedComponentCallback object containing the
     *            callback of the process that the Availability Management
     *            Framework may invoke.
     * @param cleanupProxiedComponentCallback [in] If this parameter is set to
     *            NULL then no CleanupProxiedComponentCallback callback is
     *            registered; otherwise it is a reference to a
     *            CleanupProxiedComponentCallback object containing the callback
     *            of the process that the Availability Management Framework may
     *            invoke.
     * @param trackProtectionGroupCallback [in] If this parameter is set to NULL
     *            then no TrackProtectionGroupCallback callback is registered;
     *            otherwise it is a reference to a TrackProtectionGroupCallback
     *            object containing the callback of the process that the
     *            Availability Management Framework may invoke.
     * @param version [in/out] as an input parameter version is a reference to
     *            the required Availability Management Framework version. In
     *            this case minorVersion is ignored and should be set to 0x00.
     *            As an output parameter the version actually supported by the
     *            Availability Management Framework is delivered.
     * @return A reference to the handle designating this particular
     *         initialization of the Availability Management Framework.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it didn't.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisVersionException The version parameter is not compatible with
     *             the version of the Availability Management Framework
     *             implementation.
     */
    public static AmfHandleImpl initializeHandle( HealthcheckCallback healthcheckCallback,
                                                    SetCsiCallback csiSetCallback,
                                                    RemoveCsiCallback csiRemoveCallback,
                                                    TerminateComponentCallback componentTerminateCallback,
                                                    InstantiateProxiedComponentCallback proxiedComponentInstantiateCallback,
                                                    CleanupProxiedComponentCallback proxiedComponentCleanupCallback,
                                                    TrackProtectionGroupCallback protectionGroupTrackCallback,
                                                    Version version )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisVersionException {
        return new AmfHandleImpl( healthcheckCallback,
                                     csiSetCallback,
                                     csiRemoveCallback,
                                     componentTerminateCallback,
                                     proxiedComponentInstantiateCallback,
                                     proxiedComponentCleanupCallback,
                                     protectionGroupTrackCallback,
                                     version );
    }

	public static AmfHandleImpl initializeHandle(AmfHandle.Callbacks callbacks,
			Version version) throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException, AisVersionException {
		return initializeHandle(callbacks.healthcheckCallback,
				callbacks.setCsiCallback, callbacks.removeCsiCallback,
				callbacks.terminateComponentCallback,
				callbacks.instantiateProxiedComponentCallback,
				callbacks.cleanupProxiedComponentCallback,
				callbacks.trackProtectionGroupCallback, version);
	}

    /**
     * TODO
     *
     * @param invocation
     * @param componentName
     * @param healthcheckKey
     */
    private static void s_invokeHealthcheckCallback( long invocation,
                                                    String componentName,
                                                    byte[] healthcheckKey ) {
    	AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );
        _amfLibraryHandle.healthcheckCallback.healthcheckCallback( invocation,
                                                                   componentName,
                                                                   healthcheckKey );
    }

    /**
     * TODO
     *
     * @param invocation
     * @param componentName
     * @param haState
     * @param csiDescriptor
     */
    private static void s_invokeSetCsiCallback( long invocation,
                                               String componentName,
                                               HaState haState,
                                               CsiDescriptor csiDescriptor ) {
    	AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );
        _amfLibraryHandle.setCsiCallback.setCsiCallback( invocation,
                                                         componentName,
                                                         haState,
                                                         csiDescriptor );
    }

    /**
     * TODO
     *
     * @param invocation
     * @param componentName
     * @param csiName
     * @param csiFlags
     */
    private static void s_invokeRemoveCsiCallback( long invocation,
                                                  String componentName,
                                                  String csiName,
                                                  CsiDescriptor.CsiFlags csiFlags ) {
    	AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );
        _amfLibraryHandle.removeCsiCallback.removeCsiCallback( invocation,
                                                               componentName,
                                                               csiName,
                                                               csiFlags );
    }

    /**
     * TODO
     *
     * @param invocation
     * @param componentName
     */
    private static void s_invokeTerminateComponentCallback( long invocation,
                                                           String componentName ) {
    	AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );
        _amfLibraryHandle.terminateComponentCallback.terminateComponentCallback( invocation,
                                                                                 componentName );
    }

    /**
     * TODO
     *
     * @param invocation
     * @param proxiedCompName
     */
    private static void s_invokeInstantiateProxiedComponentCallback( long invocation,
                                                                    String proxiedCompName ) {
    	AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );
        _amfLibraryHandle.instantiateProxiedComponentCallback.instantiateProxiedComponentCallback( invocation,
                                                                                                   proxiedCompName );
    }

    /**
     * TODO
     *
     * @param invocation
     * @param proxiedCompName
     */
    private static void s_invokeCleanupProxiedComponentCallback( long invocation,
                                                                String proxiedCompName ) {
        AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );
        _amfLibraryHandle.cleanupProxiedComponentCallback.cleanupProxiedComponentCallback( invocation,
                                                                                           proxiedCompName );
    }

    private static void s_invokeTrackProtectionGroupCallback( String csiName,
                                                             ProtectionGroupNotification[] notificationBuffer,
                                                             int nMembers,
                                                             int error ) {
    	AmfHandleImpl _amfLibraryHandle = s_handleMap.get( Thread.currentThread() );

    	AisStatus status = null;
		for (AisStatus s : AisStatus.values()) {
			if (s.getValue() == error) {
				status = s;
				break;
			}
		}
        _amfLibraryHandle.trackProtectionGroupCallback.trackProtectionGroupCallback( csiName,
                                                                                     notificationBuffer,
                                                                                     nMembers,
                                                                                     status );
    }

    /**
     * TODO constructor comment
     *
     * @param healthcheckCallback [in] If this parameter is set to NULL then no
     *            HealthcheckCallback callback is registered; otherwise it is a
     *            reference to a HealthcheckCallback object containing the
     *            callback of the process that the Availability Management
     *            Framework may invoke.
     * @param setCsiCallback [in] If this parameter is set to NULL then no
     *            SetCsiCallback callback is registered; otherwise it is a
     *            reference to a SetCsiCallback object containing the callback
     *            of the process that the Availability Management Framework may
     *            invoke.
     * @param removeCsiCallback [in] If this parameter is set to NULL then no
     *            RemoveCsiCallback callback is registered; otherwise it is a
     *            reference to a RemoveCsiCallback object containing the
     *            callback of the process that the Availability Management
     *            Framework may invoke.
     * @param terminateComponentCallback [in] If this parameter is set to NULL
     *            then no TerminateComponentCallback callback is registered;
     *            otherwise it is a reference to a TerminateComponentCallback
     *            object containing the callback of the process that the
     *            Availability Management Framework may invoke.
     * @param instantiateProxiedComponentCallback [in] If this parameter is set
     *            to NULL then no InstantiateProxiedComponentCallback callback
     *            is registered; otherwise it is a reference to a
     *            InstantiateProxiedComponentCallback object containing the
     *            callback of the process that the Availability Management
     *            Framework may invoke.
     * @param cleanupProxiedComponentCallback [in] If this parameter is set to
     *            NULL then no CleanupProxiedComponentCallback callback is
     *            registered; otherwise it is a reference to a
     *            CleanupProxiedComponentCallback object containing the callback
     *            of the process that the Availability Management Framework may
     *            invoke.
     * @param trackProtectionGroupCallback [in] If this parameter is set to NULL
     *            then no TrackProtectionGroupCallback callback is registered;
     *            otherwise it is a reference to a TrackProtectionGroupCallback
     *            object containing the callback of the process that the
     *            Availability Management Framework may invoke.
     * @param version [in/out] as an input parameter version is a reference to
     *            the required Availability Management Framework version. In
     *            this case minorVersion is ignored and should be set to 0x00.
     *            As an output parameter the version actually supported by the
     *            Availability Management Framework is delivered.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it didn't.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisInvalidParamException A parameter is not set correctly.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisVersionException The version parameter is not compatible with
     *             the version of the Availability Management Framework
     *             implementation.
     * @see #initializeHandle(HealthcheckCallback, SetCsiCallback,
     *      RemoveCsiCallback, TerminateComponentCallback,
     *      InstantiateProxiedComponentCallback,
     *      CleanupProxiedComponentCallback, TrackProtectionGroupCallback,
     *      SVersion)
     */
    private AmfHandleImpl( HealthcheckCallback healthcheckCallback,
                             SetCsiCallback csiSetCallback,
                             RemoveCsiCallback csiRemoveCallback,
                             TerminateComponentCallback componentTerminateCallback,
                             InstantiateProxiedComponentCallback proxiedComponentInstantiateCallback,
                             CleanupProxiedComponentCallback proxiedComponentCleanupCallback,
                             TrackProtectionGroupCallback protectionGroupTrackCallback,
                             Version version )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisVersionException {
        this.healthcheckCallback = healthcheckCallback;
        this.setCsiCallback = csiSetCallback;
        this.removeCsiCallback = csiRemoveCallback;
        this.terminateComponentCallback = componentTerminateCallback;
        this.instantiateProxiedComponentCallback = proxiedComponentInstantiateCallback;
        this.cleanupProxiedComponentCallback = proxiedComponentCleanupCallback;
        this.trackProtectionGroupCallback = protectionGroupTrackCallback;
        invokeSaAmfInitialize( version );
    }

	public ComponentRegistry getComponentRegistry() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException, AisNotExistException {
		if ( componentRegistry == null ) {
            componentRegistry = new ComponentRegistryImpl( this );
        }
        return componentRegistry;
	}

	public CsiManager getCsiManager() {
		if ( csiManager == null ) {
            csiManager = new CsiManagerImpl( this );
        }
        return csiManager;
	}

	public ErrorReporting getErrorReporting() {
		if ( errorReporting == null ) {
            errorReporting = new ErrorReportingImpl( this );
        }
        return errorReporting;
	}

	public Healthcheck getHealthcheck() {
		if ( healthcheck == null ) {
            healthcheck = new HealthcheckImpl( this );
        }
        return healthcheck;
	}

	public ProcessMonitoring getProcessMonitoring() {
		if ( processMonitoring == null ) {
            processMonitoring = new ProcessMonitoringImpl( this );
        }
        return processMonitoring;
	}

	public ProtectionGroupManager getProtectionGroupManager() {
		if ( protectionGroupManager == null ) {
            protectionGroupManager = new ProtectionGroupManagerImpl( this );
        }
        return protectionGroupManager;
	}

	public native void response(long invocation, AisStatus error)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException,
			AisInvalidParamException, AisNoMemoryException,
			AisNoResourcesException;

	public void dispatch(DispatchFlags dispatchFlags)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException {
		Thread _currentThread = Thread.currentThread();
        s_handleMap.put( _currentThread, this );
        invokeSaAmfDispatch( dispatchFlags.getValue() );
        s_handleMap.remove( _currentThread );
		super.dispatch(dispatchFlags);
	}

	public void dispatchBlocking() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		// make sure that we have a valid selection object
        ensureSelectionObjectObtained();
        // do the dispatching
        Thread _currentThread = Thread.currentThread();
        s_handleMap.put( _currentThread, this );
        invokeSaAmfDispatchWhenReady();
        s_handleMap.remove( _currentThread );
		super.dispatchBlocking();
	}

	public void dispatchBlocking(long timeout) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		super.dispatchBlocking(timeout);
	}

	public void finalizeHandle() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException {
		// the file handle has to be taken out of the selected fd list before closing it, OW select returns with "bad handle"
		super.finalizeHandle();
		finalizeAmfHandle();
	}

	private native void finalizeAmfHandle() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException;

	protected void invokeSelectionObjectGet() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		invokeSaAmfSelectionObjectGet();
	}

	/**
     * This native method invokes the saAmfInitialize function of the underlying
     * native AIS implementation.
     *
     * @param version [in/out] as an input parameter version is a reference to
     *            the required Availability Management Framework version. In
     *            this case minorVersion is ignored and should be set to 0x00.
     *            As an output parameter the version actually supported by the
     *            Availability Management Framework is delivered.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisInvalidParamException A parameter is not set correctly..
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisVersionException The version parameter is not compatible with
     *             the version of the Availability Management Framework
     *             implementation.
     * @see #saAmfHandle
     * @see #selectionObject
     */
    private native void invokeSaAmfInitialize( Version version )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisVersionException;

	/**
     * This native method invokes the saAmfDispatch function of the underlying
     * native AIS implementation.
     *
     * @param dispatchFlags TODO
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it didn't.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException This library handle is invalid, since it is
     *             corrupted or has already been finalized.
     * @see #dispatch(DispatchFlags)
     */
    private native void invokeSaAmfDispatch( int dispatchFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            // AisBadHandleException,
            // AisInvalidParamException; // TODO consider removing this...
            AisBadHandleException;

    /**
     * TODO (DISPATCH_ONE) This native method invokes the saAmfDispatch function
     * of the underlying native AIS implementation.
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
     * @see #dispatch(DispatchFlags)
     */
    private native void invokeSaAmfDispatchWhenReady()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            // AisBadHandleException,
            // AisInvalidParamException; // TODO consider removing this...
            AisBadHandleException;

    /**
     * TODO
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
     */
    private native void invokeSaAmfSelectionObjectGet()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            // AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException;

}
