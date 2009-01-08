/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-CLM-B.01.01
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

package org.saforum.ais.clm;

import org.saforum.ais.*;

/**
 * <P>
 * A ClusterMembershipManager object contains API methods that
 * enable tracking changes in the cluster and/or in a node.
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 */
public interface ClusterMembershipManager {

    /**
     * This method is used to obtain the current cluster membership: information
     * about all nodes that are currently members in the cluster is returned.
     * <P>While a client process on a node that is not a cluster member may also use
     * this method, the client process can receive information consistent with the
     * semantics of the kind of tracking requested but limited to the node hosting the
     * process as long as this node does not itself join the cluster membership.
     *
     * <P><B>SAF Reference:</B> <code>saClmClusterTrack(..., SA_TRACK_CURRENT, notificationBuffer )</code>
     *
     * @return information about all nodes that are currently members in the
     *         cluster.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     */
    ClusterNotificationBuffer getCluster()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method is used to obtain the current cluster membership: information
     * about all nodes that are currently members in the cluster is returned a
     * by single subsequent invocation of the trackClusterCallback()
     * notification callback.
     * <P>While a client process on a node that is not a cluster member may also use
     * this method, the client process can receive information consistent with the
     * semantics of the kind of tracking requested but limited to the node hosting the
     * process as long as this node does not itself join the cluster membership.
     * <P><B>SAF Reference:</B> <code>saClmClusterTrack(..., SA_TRACK_CURRENT, NULL )</code>
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with
     *             of ClmHandle was incomplete, since the
     *             TrackClusterCallback() callback is missing.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @see TrackClusterCallback#trackClusterCallback(ClusterNotificationBuffer,
     *      int, org.saforum.ais.AisStatus)
     */
    void getClusterAsync()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method is used to obtain the current cluster membership <I>and</I>
     * to request notification of changes in the cluster membership or of
     * changes in an attribute of a cluster node, depending on the value of the
     * trackFlags parameter. The current cluster membership information is returned
     * synchronously. The changes are notified asynchronously, via the invocation of
     * the trackClusterCallback() callback method, which must have been supplied
     * when the process invoked the ClmHandle.initializeHandle() call.
     * <P>
     * An application may call this method repeatedly.
     * <P>
     * If this method is called with trackFlags containing TRACK_CHANGES_ONLY
     * when cluster changes are currently being tracked with TRACK_CHANGES,
     * the Cluster Membership Service will invoke further notification callbacks
     * according to the new value of trackFlags. The same is true vice versa.
     * <P>
     * Once this method has been called, notifications can only be stopped by an
     * invocation of stopClusterTracking() or ClmHandle.finalizeHandle().
     * <P>While a client process on a node that is not a cluster member may also use
     * this method, the client process can receive information consistent with the
     * semantics of the kind of tracking requested but limited to the node hosting the
     * process as long as this node does not itself join the cluster membership.
     *
     * <P><B>SAF Reference:</B> <code>saClmClusterTrack()</code>
     * <P>
     * This method corresponds to an invocation of
     * <UL>
     * <LI>saClmClusterTrack(..., SA_TRACK_CURRENT | SA_TRACK_CHANGES,
     * ntfBufferPtr ) or
     * <LI>saClmClusterTrack(..., SA_TRACK_CURRENT | SA_TRACK_CHANGES_ONLY,
     * ntfBufferPtr ).
     * </UL>
     *
     *
     * @param trackFlags [in] The kind of tracking that is requested, which is
     *            either TRACK_CHANGES or TRACK_CHANGES_ONLY, as defined in
     *            ais.TrackFlags, which have the following implementation here:
     *            <UL>
     *            <LI>TRACK_CHANGES - The callback method
     *            trackClusterCallback() is invoked each time a change in the
     *            cluster membership or in an attribute of a cluster node
     *            occurs. The notificationBuffer, returned by
     *            trackClusterCallback(), contains information <I>about all
     *            nodes that are currently members of the cluster</I>, and also
     *            about nodes that have left the cluster membership since the
     *            last invocation of trackClusterCallback().
     *            <LI>TRACK_CHANGES_ONLY - The callback method
     *            trackClusterCallback() is invoked each time a change in the
     *            cluster membership or in an attribute of a cluster node
     *            occurs. The notificationBuffer, returned by
     *            trackClusterCallback(), contains information about <I>new
     *            nodes in the cluster membership, nodes in the cluster
     *            membership whose attributes have changed</I>, and nodes that
     *            have left the cluster membership since the last invocation of
     *            trackClusterCallback().
     *            </UL>
     * @return information about all nodes that are currently members in the
     *         cluster.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with
     *             ClmHandle was incomplete, since the
     *             TrackClusterCallback() callback is missing.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisBadFlagsException The trackFlags parameter is invalid.
     * @see TrackClusterCallback#trackClusterCallback(ClusterNotificationBuffer,
     *      int, org.saforum.ais.AisStatus)
     */
    ClusterNotificationBuffer getClusterThenStartTracking( TrackFlags trackFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException;


    /**
     * This method is used to obtain the current cluster membership <I>and</I>
     * to request notification of changes in the cluster membership or of
     * changes in an attribute of a cluster node, depending on the value of the
     * trackFlags parameter. The current cluster membership information is
     * returned asynchronously, by a single invocation of the
     * trackClusterCallback() notification callback (which must have been
     * supplied when the process invoked the ClmHandle.initializeHandle() call),
     * preceding any other callbacks initiated by this call. The changes are
     * also notified asynchronously, via the invocation of the
     * trackClusterCallback() callback method.
     * <P>
     * An application may call this method repeatedly.
     * <P>
     * If this method is called with trackFlags containing TRACK_CHANGES_ONLY
     * when cluster changes are currently being tracked with TRACK_CHANGES, the
     * Cluster Membership Service will invoke further notification callbacks
     * according to the new value of trackFlags. The same is true vice versa.
     * <P>
     * Once this method has been called, notifications can only be stopped by an
     * invocation of stopClusterTracking() or ClmHandle.finalizeHandle().
     * <P>
     * While a client process on a node that is not a cluster member may also
     * use this method, the client process can receive information consistent
     * with the semantics of the kind of tracking requested but limited to the
     * node hosting the process as long as this node does not itself join the
     * cluster membership.
     * <P>
     * <B>SAF Reference:</B> <code>saClmClusterTrack()</code>
     * <P>
     * This method corresponds to an invocation of
     * <UL>
     * <LI>saClmClusterTrack(..., SA_TRACK_CURRENT | SA_TRACK_CHANGES, null )
     * or
     * <LI>saClmClusterTrack(..., SA_TRACK_CURRENT | SA_TRACK_CHANGES_ONLY,
     * null ).
     * </UL>
     *
     * @param trackFlags [in] The kind of tracking that is requested, which is
     *            either TRACK_CHANGES or TRACK_CHANGES_ONLY, as defined in
     *            ais.TrackFlags, which have the following implementation here:
     *            <UL>
     *            <LI>TRACK_CHANGES - The callback method
     *            trackClusterCallback() is invoked each time a change in the
     *            cluster membership or in an attribute of a cluster node
     *            occurs. The notificationBuffer, returned by
     *            trackClusterCallback(), contains information <I>about all
     *            nodes that are currently members of the cluster</I>, and also
     *            about nodes that have left the cluster membership since the
     *            last invocation of trackClusterCallback().
     *            <LI>TRACK_CHANGES_ONLY - The callback method
     *            trackClusterCallback() is invoked each time a change in the
     *            cluster membership or in an attribute of a cluster node
     *            occurs. The notificationBuffer, returned by
     *            trackClusterCallback(), contains information about <I>new
     *            nodes in the cluster membership, nodes in the cluster
     *            membership whose attributes have changed</I>, and nodes that
     *            have left the cluster membership since the last invocation of
     *            trackClusterCallback().
     *            </UL>
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with ClmHandle was
     *             incomplete, since the TrackClusterCallback() callback is
     *             missing.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisBadFlagsException The trackFlags parameter is invalid.
     * @see TrackClusterCallback#trackClusterCallback(ClusterNotificationBuffer,
     *      int, org.saforum.ais.AisStatus)
     */
    void getClusterAsyncThenStartTracking( TrackFlags trackFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method is used to request notification of changes in the cluster
     * membership or of changes in an attribute of a cluster node, depending on
     * the value of the trackFlags parameter. These changes are notified via the
     * invocation of the trackClusterCallback() callback method, which must have
     * been supplied when the process invoked the
     * ClmHandle.initializeHandle() call. An application may call this
     * method repeatedly.
     * <P>
     * If this method is called with trackFlags containing TRACK_CHANGES_ONLY
     * when cluster changes are currently being tracked with TRACK_CHANGES, the
     * Cluster Membership Service will invoke further notification callbacks
     * according to the new value of trackFlags. The same is true vice versa.
     * <P>
     * Once this method has been called, notifications can only be stopped by an
     * invocation of stopClusterTracking() or ClmHandle.finalizeHandle().
     * <P>While a client process on a node that is not a cluster member may also use
     * this method, the client process can receive information consistent with the
     * semantics of the kind of tracking requested but limited to the node hosting the
     * process as long as this node does not itself join the cluster membership.
     * <P><B>SAF Reference:</B> <code>saClmClusterTrack()</code>
     * <P>
     * This method corresponds to an invocation of
     * <UL>
     * <LI>saClmClusterTrack(..., SA_TRACK_CHANGES, null ) or
     * <LI>saClmClusterTrack(..., SA_TRACK_CHANGES_ONLY, null ).
     * </UL>
     *
     * @param trackFlags [in] The kind of tracking that is requested, which is
     *            either TRACK_CHANGES or TRACK_CHANGES_ONLY, as defined in
     *            ais.TrackFlags, which have the following implementation here:
     *            <UL>
     *            <LI>TRACK_CHANGES - The callback method
     *            trackClusterCallback() is invoked each time a change in the
     *            cluster membership or in an attribute of a cluster node
     *            occurs. The notificationBuffer, returned by
     *            trackClusterCallback(), contains information <I>about all
     *            nodes that are currently members of the cluster</I>, and also
     *            about nodes that have left the cluster membership since the
     *            last invocation of trackClusterCallback().
     *            <LI>TRACK_CHANGES_ONLY - The callback method
     *            trackClusterCallback() is invoked each time a change in the
     *            cluster membership or in an attribute of a cluster node
     *            occurs. The notificationBuffer, returned by
     *            trackClusterCallback(), contains information about <I>new
     *            nodes in the cluster membership, nodes in the cluster
     *            membership whose attributes have changed</I>, and nodes that
     *            have left the cluster membership since the last invocation of
     *            trackClusterCallback().
     *            </UL>
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with
     *             ClmHandle was incomplete, since the
     *             TrackClusterCallback() callback is missing.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisBadFlagsException The trackFlags parameter is invalid.
     * @see TrackClusterCallback#trackClusterCallback(ClusterNotificationBuffer,
     *      int, org.saforum.ais.AisStatus)
     */
    void startClusterTracking( TrackFlags trackFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method stops any further notifications through the associated
     * ClmHandle. Pending callbacks are removed.
     * <P><B>SAF Reference:</B> <code>saClmClusterTrackStop()</code>
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException No track of changes in the cluster
     *             membership was previously started, which is still in effect.
     */
    void stopClusterTracking()
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * This method provides the means for synchronously retrieving information
     * about a cluster member, identified by the nodeId parameter.
     * <P>
     * By invoking this method, a process can obtain the cluster node
     * information for the node, designated by nodeId, and can then check the
     * member field to determine whether this node is a member of the cluster.
     * If the constant LOCAL_NODE_ID is used as nodeId, the method returns
     * information about the cluster node that hosts the invoking process.
     * <P><B>SAF Reference:</B> <code>saClmClusterNodeGet()</code>
     *
     * @param nodeId [in] The identifier of the cluster node for which the
     *            clusterNode information structure is to be retrieved.
     * @param timeout [in] The invocation is considered to have failed if it
     *            does not complete by the time specified (in nanoseconds).
     *            Commonly used time duration constants defined in
     *            {@link org.saforum.ais.Consts} can be used here.
     * @return ClusterNode structure that contains information about a cluster
     *         node.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInvalidParamException The nodeId parameter is invalid.
     * @throws AisNoMemoryException Either the Cluster Membership Service
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @see ClusterNode#LOCAL_NODE_ID
     */
    ClusterNode getClusterNode( int nodeId, long timeout )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisNoMemoryException,
            AisNoResourcesException;

    /**
     * This method requests information, to be provided asynchronously, about
     * the particular cluster node, identified by the nodeId parameter. If
     * LOCAL_NODE_ID is used as nodeId, the method returns information about
     * the cluster node that hosts the invoking process. The process sets
     * invocation, which it uses subsequently to match the corresponding
     * callback, saClmClusterNodeGetCallback(), with this particular invocation.
     * The getClusterNodeCallback() callback method must have been supplied
     * when the process invoked the initializeHandle() call on the associated
     * ClmLibraryhandle instance.
     * <P><B>SAF Reference:</B> <code>saClmClusterNodeGetAsync()</code>
     *
     * @param invocation [in] This parameter allows the invoking process to
     *            match this invocation of getClusterNodeAsync() with the
     *            corresponding getClusterNodeCallback().
     * @param nodeId [in] The identifier of the cluster node for which the
     *            information is to be retrieved.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this ClusterMembershipManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInvalidParamException The nodeId parameter is invalid.
     * @throws AisInitException The previous initialization with
     *             ClmHandle was incomplete, since the
     *             GetClusterNodeCallback() callback is missing.
     * @see ClusterNode#LOCAL_NODE_ID
     * @see GetClusterNodeCallback#getClusterNodeCallback(long, ClusterNode,
     *      org.saforum.ais.AisStatus)
     */
    void getClusterNodeAsync( long invocation, int nodeId )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInvalidParamException,
            AisInitException;

}
