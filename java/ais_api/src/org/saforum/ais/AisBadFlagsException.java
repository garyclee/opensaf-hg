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
 * An exception indicating that the flags are invalid.
 *
 * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_BAD_FLAGS</code>
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 *
 */
public final class AisBadFlagsException extends AisException {

    private static final long serialVersionUID = 5L;

    /**
     * Constructs a new exception with null as its detail message.
     */
    public AisBadFlagsException() {
        super( AisStatus.ERR_BAD_FLAGS );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param message
     * @param nested
     */
    public AisBadFlagsException( String message, Throwable nested ) {
        super( message, AisStatus.ERR_BAD_FLAGS, nested );
    }

    /**
     * Constructs a new exception with the specified detail message.
     * @param message
     */
    public AisBadFlagsException( String message ) {
        super( message, AisStatus.ERR_BAD_FLAGS );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param nested
     */
    public AisBadFlagsException( Throwable nested ) {
        super( AisStatus.ERR_BAD_FLAGS, nested );
    }

}

/*  */
