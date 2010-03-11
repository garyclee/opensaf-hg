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
 * A ClmHandle object represents a reference to the association between
 * the client and the Cluster Membership Service. The
 * client uses this handle in subsequent communication with the Cluster
 * Membership Service. AIS libraries must
 * support several ClmHandle instances from the same binary
 * program (e.g., process in the POSIX.1 world).
 *
 * <P><B>SAF Reference:</B> <code>SaClmHandleT</code>
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 * @see ClmHandleFactory
 * @see ClmHandleFactory#initializeHandle
 */
public interface ClmHandle extends Handle {

    /**
     * The Callbacks class bundles the callback interfaces supplied by a process
     * to the Cluster Membership Service.
     * <P><B>SAF Reference:</B> <code>SaClmCallbacksT</code>
     * <P><B>SAF Reference:</B> <code>SaClmCallbacksT_3</code>
     * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since CLM-B.01.01
     *
     */
    final class Callbacks extends org.saforum.ais.Callbacks {

        /**
         * Reference to the GetClusterNodeCallback object that
         * the Cluster Membership Service may invoke on the component.
         * @since CLM-B.01.01
         */
        public GetClusterNodeCallback getClusterNodeCallback;

        /**
         * Reference to the TrackClusterCallback object that
         * the Cluster Membership Service may invoke on the component.
         * @since CLM-B.01.01
         */
        public TrackClusterCallback trackClusterCallback;

    }

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
