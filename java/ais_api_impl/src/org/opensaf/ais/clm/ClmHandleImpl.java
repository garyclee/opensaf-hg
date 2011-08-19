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

package org.opensaf.ais.clm;

import java.util.HashMap;
import java.util.Map;

import org.opensaf.ais.HandleImpl;
import org.saforum.ais.AisBadHandleException;
import org.saforum.ais.AisInvalidParamException;
import org.saforum.ais.AisLibraryException;
import org.saforum.ais.AisNoMemoryException;
import org.saforum.ais.AisNoResourcesException;
import org.saforum.ais.AisStatus;
import org.saforum.ais.AisTimeoutException;
import org.saforum.ais.AisTryAgainException;
import org.saforum.ais.AisVersionException;
import org.saforum.ais.AisUnavailableException;
import org.saforum.ais.DispatchFlags;
import org.saforum.ais.Version;
import org.saforum.ais.ChangeStep;
import org.saforum.ais.CorrelationIds;
import org.saforum.ais.CallbackResponse;
import org.saforum.ais.clm.ClmHandle;
import org.saforum.ais.clm.ClusterMembershipManager;
import org.saforum.ais.clm.ClusterNode;
import org.saforum.ais.clm.ClusterNotification;
import org.saforum.ais.clm.ClusterNotificationBuffer;
import org.saforum.ais.clm.GetClusterNodeCallback;
import org.saforum.ais.clm.TrackClusterCallback;
import org.saforum.ais.clm.ClusterNotification;

public final class ClmHandleImpl extends HandleImpl implements ClmHandle {

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
	private static Map<Thread, ClmHandleImpl> s_handleMap;
	// TODO s_handleMap must be synchronized!

	static {
		String libraryName = System.getProperty("nativeLibrary");

		if (libraryName == null)
			throw new NullPointerException(
					"ClmHandle initialization: System property 'nativeLibrary' is not set. Use -DnativeLibrary in the command line!");

		System.loadLibrary(libraryName);
		s_handleMap = new HashMap<Thread, ClmHandleImpl>();
		// System.out.println( "JAVA: Initialized hashtable." );
	}

	/**
	 * TODO
	 */
	private ClusterMembershipManager cmMgr;

	/**
	 * TODO
	 */
	private GetClusterNodeCallback getClusterNodeCallback;

	/**
	 * TODO
	 */
	private TrackClusterCallback trackClusterCallback;

	

    /**
     * The handle designating this particular initialization of the Cluster
     * Membership Service, returned by the saClmInitialize function of the
     * underlying AIS implementation.
     *
     * @see #invokeSaClmInitialize(Version)
     */
    private long saClmHandle = 0;

    private Version version;

    /**
	 * This method initializes the Cluster Membership Service for the invoking
	 * process and registers the various callback methods. This method must be
	 * invoked prior to the invocation of any other Cluster Membership Service
	 * API method. The library handle is returned as the reference to this
	 * association between the process and the Cluster Membership Service. The
	 * process uses this handle in subsequent communication with the Cluster
	 * Membership Service. Please note that each invocation to
	 * initializeHandle() returns a new (i.e. different) library handle.
	 *
	 *
	 * @param getClusterNodeCallback
	 *            [in] If this parameter is set to NULL then no
	 *            GetClusterNodeCallback callback is registered; otherwise it is
	 *            a reference to a GetClusterNodeCallback object containing the
	 *            callback of the process that the Cluster Membership Service
	 *            may invoke.
	 * @param trackClusterCallback
	 *            [in] If this parameter is set to NULL then no
	 *            TrackClusterCallback callback is registered; otherwise it is a
	 *            reference to a TrackClusterCallback object containing the
	 *            callback of the process that the Cluster Membership Service
	 *            may invoke.
	 * @param version
	 *            [in/out] as an input parameter version is a reference to the
	 *            required Cluster Membership Service version. In this case
	 *            minorVersion is ignored and should be set to 0x00. As an
	 *            output parameter the version actually supported by the Cluster
	 *            Membership Service is delivered.
	 * @return A reference to the handle designating this particular
	 *         initialization of the Cluster Membership Service.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisTimeoutException
	 *             An implementation-dependent timeout occurred before the call
	 *             could complete. It is unspecified whether the call succeeded
	 *             or whether it didn't.
	 * @throws AisTryAgainException
	 *             The service cannot be provided at this time. The process may
	 *             retry later.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly.
	 * @throws AisNoMemoryException
	 *             Either the Cluster Membership Service library or the provider
	 *             of the service is out of memory and cannot provide the
	 *             service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 * @throws AisVersionException
	 *             The version parameter is not compatible with the version of
	 *             the Cluster Membership Service implementation.
	 */
	public static ClmHandleImpl initializeHandle(
			GetClusterNodeCallback getClusterNodeCallback,
			TrackClusterCallback trackClusterCallback, Version version)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException, 
			AisVersionException, AisUnavailableException {
		return new ClmHandleImpl(getClusterNodeCallback, trackClusterCallback,
				version);
	}


	/*
	 * Factory method called by FactoryImpl.initializeHandle.
	 */
	public static ClmHandleImpl initializeHandle(ClmHandle.Callbacks callbacks,
			Version version) throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException, 
			AisVersionException, AisUnavailableException {
		return initializeHandle(callbacks.getClusterNodeCallback,
				callbacks.trackClusterCallback, version);
	}
	

	/**
	 * TODO
	 *
	 * @param invocation
	 *            TODO
	 * @param clusterNode
	 *            TODO
	 * @param error
	 *            TODO
	 */
	private static void s_invokeGetClusterNodeCallback(long invocation,
			ClusterNode clusterNode, int error) {
		ClmHandleImpl _clmLibraryHandle = s_handleMap.get(Thread
				.currentThread());
                AisStatus status = getAisStatusFromValue(error); 
		_clmLibraryHandle.getClusterNodeCallback.getClusterNodeCallback(
				invocation, clusterNode, status);
	}
	/**
         * TODO
         *
         * @param notificationBuffer
         *            TODO
         * @param numberOfMembers
         *            TODO
         * @param error
         *            TODO
         */

	private static void s_invokeTrackClusterCallback( ClusterNotificationBuffer notificationBuffer,
							  int numberOfMembers, 
							  int error ){
		ClmHandleImpl _clmLibraryHandle = s_handleMap.get( Thread.currentThread() );		
			AisStatus status = getAisStatusFromValue(error);
		_clmLibraryHandle.trackClusterCallback.trackClusterCallback( notificationBuffer, 
									     numberOfMembers,
									     status );
	}	
	/**
	 * TODO
	 *
	 * @param notificationBuffer
	 *            TODO



	 * @param numberOfMembers
	 *            TODO
	 * @param error
	 *            TODO
	 */

	private static void s_invokeTrackClusterCallback(
			ClusterNotificationBuffer notificationBuffer, int numberOfMembers, long invocation,java.lang.String rootCauseEntity, CorrelationIds correlationIds, int _step, long timeSupervision, int error) {
		ClmHandleImpl _clmLibraryHandle = s_handleMap.get(Thread
				.currentThread());
			AisStatus status = getAisStatusFromValue(error);
                        ChangeStep step = getChangeStepFromValue(_step); 
		_clmLibraryHandle.trackClusterCallback.trackClusterCallback(
	notificationBuffer, numberOfMembers, invocation, rootCauseEntity, correlationIds, step, timeSupervision, status);
	}

	private static ClusterNotification.ClusterChange s_getClusterChange(int value) {
		return getClusterChangeFromValue(value);
	}
      
        private static AisStatus getAisStatusFromValue(int value) {
                AisStatus aisStatus = null;
                for (AisStatus as : AisStatus.values()) {
                        if (as.getValue() == value) {
                                aisStatus = as;
                                break;
                        }
                }

                return aisStatus;
        }
        
        private static ChangeStep getChangeStepFromValue(int value) {
                ChangeStep changeStep = null;
                for (ChangeStep cs : ChangeStep.values()) {
                        if (cs.getValue() == value) {
                                changeStep = cs;
                                break;
                        }
                }

                return changeStep;
        }

	private static ClusterNotification.ClusterChange getClusterChangeFromValue(int value) {
		ClusterNotification.ClusterChange clusterChange = null;

		for (ClusterNotification.ClusterChange cc : ClusterNotification.ClusterChange.values()) {
			if (cc.getValue() == value) {
				clusterChange = cc;
				break;
			}
		}

		return clusterChange;
	}
	
	
	/*  TODO constructor
	 * @param getClusterNodeCallback
	 *            [in] If this parameter is set to NULL then no
	 *            GetClusterNodeCallback callback is registered; otherwise it is
	 *            a reference to a GetClusterNodeCallback object containing the
	 *            callback of the process that the Cluster Membership Service
	 *            may invoke.
	 * @param trackClusterCallback
	 *            [in] If this parameter is set to NULL then no
	 *            TrackClusterCallback callback is registered; otherwise it is a
	 *            reference to a TrackClusterCallback object containing the
	 *            callback of the process that the Cluster Membership Service
	 *            may invoke.
	 * @param version
	 *            [in/out] as an input parameter version is a reference to the
	 *            required Cluster Membership Service version. In this case
	 *            minorVersion is ignored and should be set to 0x00. As an
	 *            output parameter the version actually supported by the Cluster
	 *            Membership Service is delivered.
	 * @see #initializeHandle(GetClusterNodeCallback, TrackClusterCallback,
	 *      Version)
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisTimeoutException
	 *             An implementation-dependent timeout occurred before the call
	 *             could complete. It is unspecified whether the call succeeded
	 *             or whether it did not.
	 * @throws AisTryAgainException
	 *             The service cannot be provided at this time. The process may
	 *             retry later.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly..
	 * @throws AisNoMemoryException
	 *             Either the Cluster Membership Service library or the provider
	 *             of the service is out of memory and cannot provide the
	 *             service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 * @throws AisVersionException
	 *             The version parameter is not compatible with the version of
	 *             the Cluster Membership Service implementation.
	 */
	private ClmHandleImpl(GetClusterNodeCallback getClusterNodeCallback,
			TrackClusterCallback trackClusterCallback, Version version)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException, 
			AisVersionException, AisUnavailableException {
		this.getClusterNodeCallback = getClusterNodeCallback;
		this.trackClusterCallback = trackClusterCallback;
		invokeSaClmInitialize(version);
	}

	public ClusterMembershipManager getClusterMembershipManager() {
		if (cmMgr == null) {
			cmMgr = new ClusterMembershipManagerImpl(this);
		}
		return cmMgr;
	}

	public void dispatch(DispatchFlags dispatchFlags)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException {
		Thread _currentThread = Thread.currentThread();
		s_handleMap.put(_currentThread, this);
		invokeSaClmDispatch(dispatchFlags.getValue());
		s_handleMap.remove(_currentThread);
		super.dispatch(dispatchFlags);
	}

	public void dispatchBlocking() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, 
			AisBadHandleException, AisNoMemoryException, 
			AisNoResourcesException {
		// make sure that we have a valid selection object
		ensureSelectionObjectObtained();
		// do the dispatching
		Thread _currentThread = Thread.currentThread();
		s_handleMap.put(_currentThread, this);
		invokeSaClmDispatchWhenReady();
		s_handleMap.remove(_currentThread);
		super.dispatchBlocking();
	}

	public void dispatchBlocking(long timeout) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, 
			AisBadHandleException, AisNoMemoryException, 
			AisNoResourcesException {
		super.dispatchBlocking(timeout);
	}

	public boolean hasPendingCallback() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		return super.hasPendingCallback();
	}

	public boolean hasPendingCallback(long timeout) throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException {
		return super.hasPendingCallback(timeout);
	}

	public void finalizeHandle() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException {
		// the file handle has to be taken out of the selected fd list before closing it, OW select returns with "bad handle"
		super.finalizeHandle();
		finalizeClmHandle();
	}

	private native void finalizeClmHandle() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, AisBadHandleException;

	protected void invokeSelectionObjectGet() throws AisLibraryException,
			AisTimeoutException, AisTryAgainException, 
			AisBadHandleException, AisNoMemoryException, 
			AisNoResourcesException {
		invokeSaClmSelectionObjectGet();
	}

	/**
	 * This native method invokes the saClmInitialize function of the underlying
	 * native AIS implementation.
	 *
	 * @param version
	 *            [in/out] as an input parameter version is a reference to the
	 *            required Cluster Membership Service version. In this case
	 *            minorVersion is ignored and should be set to 0x00. As an
	 *            output parameter the version actually supported by the Cluster
	 *            Membership Service is delivered.
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisTimeoutException
	 *             An implementation-dependent timeout occurred before the call
	 *             could complete. It is unspecified whether the call succeeded
	 *             or whether it did not.
	 * @throws AisTryAgainException
	 *             The service cannot be provided at this time. The process may
	 *             retry later.
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly..
	 * @throws AisNoMemoryException
	 *             Either the Cluster Membership Service library or the provider
	 *             of the service is out of memory and cannot provide the
	 *             service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 * @throws AisVersionException
	 *             The version parameter is not compatible with the version of
	 *             the Cluster Membership Service implementation.
	 * @see #saClmHandle
	 * @see #selectionObject
	 */
	private native void invokeSaClmInitialize(Version version)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisInvalidParamException,
			AisNoMemoryException, AisNoResourcesException, 
			AisVersionException, AisUnavailableException;

	/**
	 * This native method invokes the saClmDispatch function of the underlying
	 * native AIS implementation.
	 *
	 * @param dispatchFlags
	 *            TODO
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisTimeoutException
	 *             An implementation-dependent timeout occurred before the call
	 *             could complete. It is unspecified whether the call succeeded
	 *             or whether it didn't.
	 * @throws AisTryAgainException
	 *             The service cannot be provided at this time. The process may
	 *             retry later.
	 * @throws AisBadHandleException
	 *             This library handle is invalid, since it is corrupted or has
	 *             already been finalized.
	 * @see #dispatch(DispatchFlags)
	 */
	private native void invokeSaClmDispatch(int dispatchFlags)
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException,
			// AisBadHandleException,
			//AisInvalidParamException, // TODO consider removing this...
			AisBadHandleException;

	/**
	 * TODO (DISPATCH_ONE) This native method invokes the saClmDispatch function
	 * of the underlying native AIS implementation.
	 *
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisTimeoutException
	 *             An implementation-dependent timeout occurred before the call
	 *             could complete. It is unspecified whether the call succeeded
	 *             or whether it did not.
	 * @throws AisTryAgainException
	 *             The service cannot be provided at this time. The process may
	 *             retry later.
	 * @throws AisBadHandleException
	 *             This library handle is invalid, since it is corrupted or has
	 *             already been finalized.
	 * @see #dispatch(DispatchFlags)
	 */
	private native void invokeSaClmDispatchWhenReady()
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException,
			// AisBadHandleException,
			//AisInvalidParamException, // TODO consider removing this...
			AisBadHandleException;

	/**
	 * TODO
	 *
	 * @throws AisLibraryException
	 *             An unexpected problem occurred in the library (such as
	 *             corruption). The library cannot be used anymore.
	 * @throws AisTimeoutException
	 *             An implementation-dependent timeout occurred before the call
	 *             could complete. It is unspecified whether the call succeeded
	 *             or whether it did not.
	 * @throws AisTryAgainException
	 *             The service cannot be provided at this time. The process may
	 *             retry later.
	 * @throws AisBadHandleException
	 *             This library handle is invalid, since it is corrupted or has
	 *             already been finalized.
	 * @throws AisNoMemoryException
	 *             Either the Cluster Membership Service library or the provider
	 *             of the service is out of memory and cannot provide the
	 *             service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 */
	private native void invokeSaClmSelectionObjectGet()
			throws AisLibraryException, AisTimeoutException,
			AisTryAgainException, AisBadHandleException,
			AisNoMemoryException, AisNoResourcesException;
			

	/* This is part of CLM lifecycle method to respond to the trackcallback 
         *
	 * SAF Reference: saClmResponse_4
         *
	 */
      
	public void response(long invocation, CallbackResponse response) throws AisLibraryException,
                        AisTimeoutException, AisTryAgainException, AisBadHandleException,
                        AisInvalidParamException, AisNoMemoryException, AisNoResourcesException,
                        AisVersionException, AisUnavailableException{
			
				invokeSaClmResponse(invocation,response);
	}
         
        /* This is the Native method */
	private native void invokeSaClmResponse(long invocation, CallbackResponse response)
		 throws AisLibraryException,
                        AisTimeoutException,
                        AisTryAgainException,
                        AisBadHandleException,
                        AisInvalidParamException,
                        AisNoMemoryException,
                        AisNoResourcesException,
                        AisVersionException,
                        AisUnavailableException;       
                         

}
