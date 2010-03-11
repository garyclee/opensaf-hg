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
 * The CsiStandbyDescriptor type holds additional information about the
 * assignment of a component service instance to a component when the component
 * is requested to assume the standby HA state for this component service
 * instance.
 *
 * When a component is requested to assume the standby HA state for one or for
 * all component service instances assigned to the component,
 * activeComponentName holds the name of the component that is currently
 * assigned the active state for the one or all these component service
 * instances. In redundancy models where several components may assume the
 * standby HA state for the same component service instance at the same time,
 * standbyRank indicates to the component which rank it must assume. When the
 * Availability Management Framework selects a component to assume the active HA
 * state for a component service instance, the component assuming the standby
 * state for that component service instance with the lowest standbyRank value
 * is chosen.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfCSIStandbyDescriptorT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class CsiStandbyDescriptor extends CsiStateDescriptor {

    /**
     * The name of the component that is currently active for the one or all of
     * the specified component service instances. The name is empty if no active
     * component exists.
     */
    public String activeComponentName;

    /**
     * The rank of the component for assignments of the standby HA state to the
     * component for the one or all of the specified component service
     * instances.
     */
    public int standbyRank;

}


