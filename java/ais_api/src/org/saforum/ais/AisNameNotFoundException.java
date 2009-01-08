/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**
** DATE:
**   Thur May 29 2008
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
 * An exception indicating that the name could not be found.
 *
 * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_NAME_NOT_FOUND</code>
 * @version B.01.01 (deprecated in later versions)
 *
 */
public final class AisNameNotFoundException extends AisException {

    private static final long serialVersionUID = 5L;

    /**
     * Constructs a new exception with null as its detail message.
     */
    public AisNameNotFoundException() {
        super( AisStatus.ERR_NAME_NOT_FOUND );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param message
     * @param nested
     */
    public AisNameNotFoundException( String message, Throwable nested ) {
        super( message, AisStatus.ERR_NAME_NOT_FOUND, nested );
    }

    /**
     * Constructs a new exception with the specified detail message.
     * @param message
     */
    public AisNameNotFoundException( String message ) {
        super( message, AisStatus.ERR_NAME_NOT_FOUND );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param nested
     */
    public AisNameNotFoundException( Throwable nested ) {
        super( AisStatus.ERR_NAME_NOT_FOUND, nested );
    }

}


