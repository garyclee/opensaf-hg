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
 * CsiAttribute represents a single component service instance attribute by its
 * name and value strings. Each component service instance has a set of
 * attributes (name/value pair), which characterize the workload assigned to the
 * component. These attributes are not used by the Availability Management
 * Framework, and are just passed to the components.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfCSIAttributeT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class CsiAttribute {

    /**
     * Component service instance attribute name.
     */
    public String name;

    /**
     * Component service instance attribute value.
     */
    public String value;

}


