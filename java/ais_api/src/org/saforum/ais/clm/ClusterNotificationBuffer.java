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
 * This class contains an array of notifications of changes in the cluster membership or
 * of changes in an attribute of a cluster node.
 * <P><B>SAF Reference:</B> <code>SaClmClusterNotificationBufferT</code>
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 */
public class ClusterNotificationBuffer {

    /**
     * The current view number of the cluster membership.
     * Some implementations may not use viewNumber. In this case
     * viewNumber must be set to zero.
     */
    public long viewNumber;

    /**
     * An array of notifications of changes in the cluster membership or
     * of changes in an attribute of a cluster node.
     */
    public ClusterNotification[] notifications;

}
