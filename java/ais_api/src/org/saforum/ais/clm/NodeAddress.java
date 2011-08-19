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


/**
 * NodeAddress provides a string representation of the communication address
 * associated with a node.
 * <P><B>SAF Reference:</B> <code>SaClmNodeAddressT</code>
 * @version SAI-AIS-CLM-B.04.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-AIS-CLM-B.01.01
 *
 */
public abstract class NodeAddress {

    /**
     * A string representation of the communication address associated with a
     * node.
     */
    private String value;

    /**
     * @return Returns the string representation of the communication address associated with a
     * node.
     */
    public String toString() {
        return this.value;
    }

}

/*  */
