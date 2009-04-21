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
 * The information associated with the component member of the protection group.
 * <P><B>SAF Reference:</B> <code>SaAmfProtectionGroupMemberT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class ProtectionGroupMember {

    /**
     * The rank of the member component in the protection group if haState is
     * standby.
     */
    public int rank;

    /**
     * The HA state of the member component for the component service instance
     * supported by the member component.
     */
    public HaState haState;

    /**
     * The name of the component that is a member of the protection group.
     */
    public String componentName;

}


