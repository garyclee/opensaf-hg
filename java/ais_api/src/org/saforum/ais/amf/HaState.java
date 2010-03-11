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
 *
 * This enum defines the possible HA states for a component.
 * <P><B>SAF Reference:</B> <code>SaAmfHAStateT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public enum HaState {

    /**
     * The meaning of the ACTIVE HA state for a component service instance: the
     * component is currently responsible for providing the service
     * characterized by a this component service instance.
     */
    ACTIVE(1),

    /**
     * The meaning of the STANDBY HA state for a component service instance: The
     * component acts as a standby for the service characterized by this
     * component service instance.
     */
    STANDBY(2),

    /**
     * The meaning of the QUIESCED HA state for a component service instance:
     * The component, which had previously the active or quiescing HA state for
     * this component service instance has now quiesced its activity related to
     * this component service instance, and the Availability Management
     * Framework can safely assign the active HA state for this component
     * service instance to another component. The quiesced state is assigned in
     * the context of switch-over situations.
     */
    QUIESCED(3),

    /**
     * The meaning of the QUIESCING HA state for a component service instance:
     * The component that had previously an active HA state for this component
     * service instance is in the process of quiescing its activity related to
     * this service instance. In accordance with the semantics of the shutdown
     * administrative operations, this quiescing is performed by rejecting new
     * users of the service characterized by this component service instance
     * while still providing the service to existing users until they all
     * terminate using it. When there is no user left for that service, the
     * component indicates that fact to the Availability Management Framework,
     * which transitions the HA state to quiesced. The quiescing HA state is
     * assigned as a consequence of a shutdown administrative operation.
     */
    QUIESCING(4);



    /**
     * The numerical value assigned to this constant by the AIS specification.
     */
    private final int value;


    /**
     * Creates an enum constant with the numerical value assigned by the AIS specification.
     * @param value The numerical value assigned to this constant by the AIS specification.
     */
    private HaState( int value ){
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


