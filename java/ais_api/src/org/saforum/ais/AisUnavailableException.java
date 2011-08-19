/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R6-A.01.01
**   SAI-Overview-B.05.01
**
** DATE: 
**   Wednesday November 19, 2008
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

package org.saforum.ais;


/**
 * An exception indicating that the operation requested in this call is unavailable on
 * this cluster node as the cluster node is not a member node, and the requested operation
 * is not permitted on a non-member node.
 *
 * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_UNAVAILABLE</code>
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.03.01
 *
 */
public final class AisUnavailableException extends AisException {

    private static final long serialVersionUID = 5L;

    /**
     * Constructs a new exception with null as its detail message.
     */
    public AisUnavailableException() {
        super( AisStatus.ERR_UNAVAILABLE );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param message
     * @param nested
     */
    public AisUnavailableException( String message, Throwable nested ) {
        super( message, AisStatus.ERR_UNAVAILABLE, nested );
    }

    /**
     * Constructs a new exception with the specified detail message.
     * @param message
     */
    public AisUnavailableException( String message ) {
        super( message, AisStatus.ERR_UNAVAILABLE );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param nested
     */
    public AisUnavailableException( Throwable nested ) {
        super( AisStatus.ERR_UNAVAILABLE, nested );
    }

}

/*  */
