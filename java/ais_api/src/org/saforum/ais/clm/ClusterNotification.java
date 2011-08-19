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

import org.saforum.ais.EnumValue;

/**
 * This class represents a notification of changes in the cluster membership or
 * of changes in an attribute of a cluster node.
 * <P><B>SAF Reference:</B> <code>SaClmClusterNotificationT</code>
 * <P><B>SAF Reference:</B> <code>SaClmClusterNotificationT_4</code>
 * 
 * @version SAI-AIS-CLM-B.04.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-AIS-CLM-B.01.01
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
	 * This enumeration refers to a configured cluster node. The cluster node
	 * may or may not be a member of the cluster.
	 * <P><B>SAF Reference:</B> <code>SaClmClusterChangesT</code>
	 * 
	 * @version SAI-AIS-CLM-B.04.01 (SAIM-AIS-R6-A.01.01)
	 * @since SAI-AIS-CLM-B.01.01
	 */
	public enum ClusterChange implements EnumValue {

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
		 * Some attributes associated with the node have changed, for instance,
		 * the node name.
		 */
		RECONFIGURED(4),

		/**
		 * An administrative unlock operation is issued on the node before a
		 * previous shut down administrative operation on the node has
		 * completed, that is, the unlock operation interrupted the shut-down
		 * operation before the node left the cluster membership.
		 * 
		 * @since SAI-AIS-CLM-B.04.01
		 * 
		 */
		UNLOCK(5),

		/**
		 * An administrative shutdown operation was issued on the node or on
		 * another entity in the system that affects the node. For this change,
		 * the GetClusterNodeCallback callback is only invoked in the
		 * ChangeStep.START and ChangeStep.COMPLETED steps.
		 * 
		 * @since SAI-AIS-CLM-B.04.01
		 */
		SHUTDOWN(6);

		/**
		 * The numerical value assigned to this constant by the AIS
		 * specification.
		 */
		private final int value;

		/**
		 * Creates an enum constant with the numerical value assigned by the AIS
		 * specification.
		 * 
		 * @param value
		 *            The numerical value assigned to this constant by the AIS
		 *            specification.
		 */
		private ClusterChange(int value) {
			this.value = value;
		}

		/**
		 * Returns the numerical value assigned to this constant by the AIS
		 * specification.
		 * 
		 * @return the numerical value assigned to this constant by the AIS
		 *         specification.
		 */
	    @Override
		public int getValue() {
			return this.value;
		}

	}

}

/*  */
