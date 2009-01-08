/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-AMF-B.01.01
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

package org.saforum.ais.amf;


/**
 * This enum represents the recommended recovery actions supported by the Availability Management
 * Framework. The Availability Management Framework always trusts the
 * recommended recovery action in the sense that it does not implement a weaker
 * recovery action than the recommended one; however, the Availability
 * Management Framework may decide to implement a stronger recovery action based
 * on its recovery escalation policy.
 * <P><B>SAF Reference:</B> <code>SaAmfRecommendedRecoveryT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public enum RecommendedRecovery {

    /**
     * This report makes no recommendation for recovery.
     * However, the Availability Management Framework should
     * engage the configured per-component recovery policy in
     * such a scenario.
     * @since AMF-B.01.01
     */
    NO_RECOMMENDATION(1),

    /**
     * The erroneous component should be terminated and reinstantiated.
     * @since AMF-B.01.01
     */
    COMPONENT_RESTART(2),

    /**
     * The error is related to the execution environment of the component on the
     * current node. Depending on the redundancy model used, either the
     * component or the service unit containing the component should fail over
     * to another node.
     * @since AMF-B.01.01
     */
    COMPONENT_FAILOVER(3),

    /**
     * The error has been identified as being at the node level, and no service
     * instance should be assigned to service units on that node. Service
     * instances containing component service instances assigned to the failed
     * component are failed over while other service instances are switched over
     * to other nodes (component service instances are not abruptly removed;
     * instead, they are brought to the quiesced state before being removed).
     * @since AMF-B.01.01
     */
    NODE_SWITCHOVER(4),

    /**
     * The error has been identified as being at the node level, and no service
     * instance should be assigned to service units on that node. All service
     * instances assigned to service units contained on the node are failed over
     * to other nodes (via an abrupt termination of all node-local components).
     * @since AMF-B.01.01
     */
    NODE_FAILOVER(5),

    /**
     * The error has been identified as being at the node level, and components
     * should not be in service on the node. The node should be rebooted via a
     * low-level interface.
     * @since AMF-B.01.01
     */
    NODE_FAILFAST(6),

    /**
     * The cluster should be reset. In order to execute this function, the Availability
     * Management Framework reboots all nodes that are part of the cluster through a
     * low level interface without trying to terminate the components individually.
     * To be effective, this operation must be performed such that all nodes are
     * first halted before any of the nodes boots again.
     * This recommendation should be used only in
     * the rare case in which a component (most likely itself involved in error
     * management) has enough knowledge to foresee a "cluster reset" as the only
     * viable recovery action from a global failure.
     * @since AMF-B.01.01
     */
    CLUSTER_RESET(7);



    /**
     * The numerical value assigned to this constant by the AIS specification.
     */
    private final int value;

    /**
     * Creates an enum constant with the numerical value assigned by the AIS specification.
     * @param value The numerical value assigned to this constant by the AIS specification.
     */
    private RecommendedRecovery( int value ){
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


