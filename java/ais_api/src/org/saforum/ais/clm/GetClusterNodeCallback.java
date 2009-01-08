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
 * the callback returns information about a particular cluster node.
 *
 * <P><B>SAF Reference:</B> <code>SaClmClusterNodeGetCallbackT</code>
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 */
public interface GetClusterNodeCallback {

    /**
     * The Cluster Membership Service invokes this callback method when the
     * operation requested by the invocation of getClusterNodeAsync() completes.
     * This callback is invoked in the context of a thread issuing an dispatch()
     * call on the associated ClmHandle. If successful, this callback
     * method returns information in clusterNode about a particular cluster
     * node, identified by the nodeId field of the clusterNode parameter,
     * provided by the process in the invocation of getClusterNodeAsync(). The
     * process sets the value of invocation when it invokes the
     * getClusterNodeAsync() method. The Cluster Membership Service uses
     * invocation when it invokes this callback method, so that the process
     * can associate its request with this callback. Upon receipt of this
     * method, the member field of the clusterNode parameter must be inspected
     * to determine whether this node is a member of the cluster. If an error
     * occurs, it is returned in the error parameter.
     *
     * @param invocation [in] This parameter makes the association between an
     *            invocation of the getClusterNodeCallback() method by the
     *            Cluster Membership Service and an invocation of the
     *            getClusterNodeAsync() method by a process.
     * @param clusterNode [in] A reference to a structure containing
     *            information about the node of the cluster, identified by
     *            nodeId, in the invocation of the getClusterNodeAsync()
     *            method.
     * @param error [in] This parameter indicates whether the
     *            getClusterNodeAsync() method was successful. The possible
     *            return values are (defined in ais.AisStatus):
     *            <UL>
     *            <LI>OK - The method completed successfully.
     *            <LI>ERR_LIBRARY - An unexpected problem occurred in the library
     *            (such as corruption). The library cannot be used anymore.
     *            <LI>ERR_TIMEOUT - An implementation-dependent timeout occurred
     *            before the call could complete. It is unspecified whether the
     *            call succeeded or whether it did not.
     *            <LI>ERR_TRY_AGAIN - The service cannot be provided at this time.
     *            The process may retry later.
     *            <LI>ERR_INVALID_PARAM -
     *            A parameter was not set correctly in the
     *            corresponding invocation of the getClusterNodeAsync() method.
     *            <LI>ERR_NO_MEMORY - Either the Cluster Membership Service library
     *            or the provider of the service is out of memory and cannot
     *            provide the service.
     *            <LI>ERR_NO_RESOURCES - The system is out of required resources
     *            (other than memory).
     *            </UL>
     * @see ClusterMembershipManager#getClusterNodeAsync(long, int)
     */
    void getClusterNodeCallback( long invocation,
                                ClusterNode clusterNode,
                                AisStatus error );

}
