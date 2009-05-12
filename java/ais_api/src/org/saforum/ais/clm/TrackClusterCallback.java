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

import org.saforum.ais.AisStatus;

/**
 * This interface supports a callback by the Cluster Membership Service:
 * the callback returns information about the cluster.
 *
 * <P><B>SAF Reference:</B> <code>SaClmClusterTrackCallbackT</code>
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 */
public interface TrackClusterCallback {

    /**
     * This callback is invoked in the context of a thread issuing a dispatch()
     * call on the handle ClmHandle, which was specified when the process
     * requested tracking of the cluster membership using one of the
     * startClusterTracking() and getClusterAsync() calls. The
     * trackClusterCallback() callback returns information about the
     * cluster nodes in the notificationBuffer parameter. The kind of
     * information returned depends on the setting of the trackFlags parameter
     * of the startClusterTracking() or getClusterAsync() call.
     * <P>
     * If the callback is invoked on a process hosted on a node that is currently
     * not in the cluster membership, it can return information only about that node.
     * <P>
     * If two processes, on the same or different nodes, request tracking with
     * the same flags set then, for a particular view number, the Cluster
     * Membership Service shall provide the same information about the cluster
     * membership when it invokes the trackClusterCallback() method of those
     * processes.
     * <P>
     * The value of the numberOfItems attribute in the notificationBuffer
     * parameter might be greater than the value of the numberOfMembers
     * parameter, because nodes have left the cluster membership: If the
     * TRACK_CHANGES flag or the TRACK_CHANGES_ONLY flag is set, the
     * notificationBuffer might contain information about the current members of
     * the cluster and also about nodes that have recently left the cluster
     * membership.
     *
     * @param notificationBuffer [in] A buffer of cluster node structures. It is
     *            possible that there is a change in the view number and no
     *            change in the cluster membership. In this case, if the flag
     *            TRACK_CHANGES was set in the associated startClusterTracking()
     *            call, then the clusterChange field in each entry of
     *            notificationBuffer is set to SClusternotification.NO_CHANGE. If the
     *            flag TRACK_CHANGES_ONLY was set in the associated
     *            startClusterTracking() call, then notificationBuffer does not
     *            contain any items. If TRACK_CURRENT was specified, the view
     *            number of the most recent transition is delivered.
     * @param numberOfMembers [in] The current number of members in the cluster
     *            membership.
     * @param error [in] This parameter indicates whether the Cluster Membership
     *            Service was able to perform the operation. The parameter error
     *            has one of the values (defined in ais.AisStatus):
     *            <UL>
     *            <LI>OK - The method completed successfully.
     *            <LI>ERR_LIBRARY - An unexpected problem occurred in the library
     *            (such as corruption). The library cannot be used anymore.
     *            <LI>ERR_TIMEOUT - An implementation-dependent timeout occurred
     *            before the call could complete. It is unspecified whether the
     *            call succeeded or whether it did not.
     *            <LI>ERR_TRY_AGAIN - The service cannot be provided at this time.
     *            The process may retry the saClmClusterTrack() later.
     *            <LI>ERR_NO_MEMORY - Either the Cluster Membership Service library
     *            or the provider of the service is out of memory and cannot
     *            provide the service. The process that invoked
     *            startClusterTracking() or getClusterAsync() might have missed
     *            one or more notifications.
     *            <LI>ERR_NO_RESOURCES - Either the Cluster Membership Service
     *            library or the provider of the service is out of required
     *            resources (other than memory), and cannot provide the service.
     *            The process that invoked startClusterTracking() or
     *            getClusterAsync() might have missed one or more notifications.
     *            </UL>
     *            If the error returned is NO_MEMORY or NO_RESOURCES, the
     *            process that invoked startClusterTracking() or
     *            getClusterAsync() should invoke stopClusterTracking() and then
     *            invoke the appropriate request method again to resynchronize
     *            with the current state.
     * @see ClusterMembershipManager#getClusterAsync()
     * @see ClusterMembershipManager#getClusterThenStartTracking(org.saforum.ais.TrackFlags)
     * @see ClusterMembershipManager#getClusterAsyncThenStartTracking(org.saforum.ais.TrackFlags)
     */
    void trackClusterCallback( ClusterNotificationBuffer notificationBuffer,
                              int numberOfMembers,
                              AisStatus error );

}

