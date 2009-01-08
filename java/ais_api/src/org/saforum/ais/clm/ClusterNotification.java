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

/**
 * This class represents a notification of changes in the cluster membership or
 * of changes in an attribute of a cluster node.
 * <P><B>SAF Reference:</B> <code>SaClmClusterNotificationT</code>
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 */
public class ClusterNotification {

    /**
     * The information about a node that is a member of the cluster, or that was
     * a member and has just left the cluster.
     */
    public ClusterNode clusterNode;

    /**
     * The changes in the cluster membership.
     */
    public ClusterChange clusterChange;

    /**
     * This enumeration refers to a configured cluster node.
     * The cluster node may or may not be a member of the cluster.
     * <P><B>SAF Reference:</B> <code>SaClmClusterChangesT</code>
     * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since CLM-B.01.01
     */
    public enum ClusterChange {

        /**
         * The membership has not changed for the node.
         */
        NO_CHANGE(1),

        /**
         * The node joined the cluster membership.
         */
        JOINED(2),

        /**
         * The node left the cluster membership.
         */
        LEFT(3),

        /**
         * Some attributes associated with the node have changed, for instance, the
         * node name.
         */
        RECONFIGURED(4);

        /**
         * The numerical value assigned to this constant by the AIS specification.
         */
        private final int value;



        /**
         * Creates an enum constant with the numerical value assigned by the AIS specification.
         * @param value The numerical value assigned to this constant by the AIS specification.
         */
        private ClusterChange( int value ){
            this.value = value;
        }

        /**
         * Returns the numerical value assigned to this constant by the AIS specification.
         * @return the numerical value assigned to this constant by the AIS specification.
         */
        public int getValue() {
            return this.value;
        }

    }

}
