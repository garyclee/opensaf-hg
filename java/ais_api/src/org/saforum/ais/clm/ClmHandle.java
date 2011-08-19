/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R6-A.01.01
**   SAI-Overview-B.05.01
**   SAI-AIS-CLM-B.04.01
**
** DATE: 
**   Monday December 1, 2008
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

package org.saforum.ais.clm;

import org.saforum.ais.*;

/**
 * A ClmHandle object represents a reference to the association between
 * the client and the Cluster Membership Service. The
 * client uses this handle in subsequent communication with the Cluster
 * Membership Service. AIS libraries must
 * support several ClmHandle instances from the same binary
 * program (e.g., process in the POSIX.1 world).
 *
 * <P><B>SAF Reference:</B> <code>SaClmHandleT</code>
 * @version SAI-AIS-CLM-B.04.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-AIS-CLM-B.01.01
 * @see ClmHandleFactory
 * @see ClmHandleFactory#initializeHandle
 */
public interface ClmHandle extends Handle {

    /**
     * The Callbacks class bundles the callback interfaces supplied by a client
     * to the Cluster Membership Service.
     * <P><B>SAF Reference:</B> <code>SaClmCallbacksT</code>, <code>SaClmCallbacksT_3</code>, <code>SaClmCallbacksT_4</code>
     * @version SAI-AIS-CLM-B.04.01 (SAIM-AIS-R6-A.01.01)
     * @since SAI-AIS-CLM-B.01.01
     *
     */
    final class Callbacks extends org.saforum.ais.Callbacks {

        /**
         * Reference to the GetClusterNodeCallback object that
         * the Cluster Membership Service may invoke on the component.
         * @since SAI-AIS-CLM-B.01.01
         */
        public GetClusterNodeCallback getClusterNodeCallback;

        /**
         * Reference to the TrackClusterCallback object that
         * the Cluster Membership Service may invoke on the component.
         * @since SAI-AIS-CLM-B.01.01
         */
        public TrackClusterCallback trackClusterCallback;

        /**
		 * Reference to the ClusterNodeEvictionCallback object that the Cluster
		 * Membership Service may invoke on the component.
		 * 
		 * @since SAI-AIS-CLM-B.03.01
		 * @deprecated (since SAI-AIS-CLM-B.04.01: If B.04.01 version is requested when
		 *             the library is initialized, implementations will ignore
		 *             any non-null value of this field.)
		 */
    }

    /**
	 * This method is used by the client to provide a response to the
	 * TrackClusterCallback() callback call that was previously invoked with a
	 * step parameter equal to either ChangeStep.VALIDATE or ChangeStep.START.
	 * The invocation parameter must be set to the value passed in the
	 * invocation parameter of the track callback. The response parameter holds
	 * the response of the process. The response parameter can be set to
	 * CallbackResponse.REJECTED only for a response to the
	 * TrackClusterCallback() callback in the ChangeStep.VALIDATE step. This
	 * method can only be called by the process for which its
	 * TrackClusterCallback() callback has been invoked.
	 * <P><B>SAF Reference:</B><code>saClmResponse_4()</code>
	 * 
	 * @since SAI-AIS-CLM-B.04.01
	 * 
	 * @param invocation
	 *            [in] Used to match this invocation of response() with the
	 *            previous corresponding invocation of the
	 *            clusterNodeEvictionCallback() callback.
	 * @param response
	 *            [in] Result of the callback. This parameter has one of the
	 *            following values:
	 *            <UL>
	 *            <LI>OK - The process carried out all necessary actions
	 *            successfully.
	 *            <LI>ERR_FAILED_OPERATION - The process was unable to carry
	 *            out necessary actions (towards preparing for the proposed
	 *            administrative state change).
	 *            </UL>
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
	 * @throws AisInvalidParamException
	 *             A parameter is not set correctly. In particular, this value
	 *             is rs method is used by the client to provide a response to the^M
         * TrackClusterCallback() callback call that was previously invoked with a^M
         * step parameter equal to either ChangeStep.VALIDATE or ChangeStep.START.^M
         * The invocation parameter must be set to the value passed in the^M
         * invocation parameter of the track callback. The response parameter holds^M
         * the response of the process. The response parameter can be set to^M
         * CallbackResponse.REJECTED only for a response to the^M
         * TrackClusterCallback() callback in the ChangeStep.VALIDATE step. This^M
         * method can only be called by the process for which its^M
         * TrackClusterCallback() callback has been invoked.^M
         * <P><B>SAF Reference:</B><code>saClmResponse_4()</code>^M
         * ^M
         * @since SAI-AIS-CLM-B.04.01^M
         * ^M
         * @param invocation^M
         *            [in] Used to match this invocation of response() with the^M
         *            previous corresponding invocation of the^M
         *            clusterNodeEvictionCallback() callback.^M
         * @param response^M
         *            [in] Result of the callback. This parameter has one of the^M
         *            following values:^M
         *            <UL>^M
         *            <LI>OK - The process carried out all necessary actions^M
         *            successfully.^M
         *            <LI>ERR_FAILED_OPERATION - The process was unable to carry^M
         *            out necessary actions (towards preparing for the proposed^M
         *            administrative state change).^M
         *            </UL>^M
         * @throws AisLibraryException^M
         *             An unexpected problem occurred in the library (such as^M
         *             corruption). The library cannot be used anymore.^M
                                                                        eturned if the invocation parameter is invalid or
	 *             identifies an invocation of the TrackClusterCallback()
	 *             function with a step parameter that does not require a
	 *             response.
	 * @throws AisNoMemoryException
	 *             Either the Cluster Membership Service library or the provider
	 *             of the service is out of memory and cannot provide the
	 *             service.
	 * @throws AisNoResourcesException
	 *             The system is out of required resources (other than memory).
	 * @throws AisVersionException
	 *             The invoked method is not supported in the version specified
	 *             in the call to initialize this instance of the Cluster
	 *             Membership Service library.
	 * @throws AisUnavailableException
	 *             The invoking process is not executing on a member node.
	 */
    public void  response( long invocation, CallbackResponse response )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisVersionException,
            AisUnavailableException;


    /**
     * Returns a ClusterMembershipManager object that enables to
     * track and obtain cluster membership information.
     * Please note that the returned instance has a
     * one-to-one association with the library handle: it is cached
     * and returned by subsequent calls to this method.
     *
     * @return an object associated with this library handle that enables to
     *         track and obtain cluster membership information.
     */
    public ClusterMembershipManager getClusterMembershipManager();

}

/*  */
