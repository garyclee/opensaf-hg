/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
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

package org.saforum.ais;


/**
 * An exception indicating that the entity, referenced by the invoker, already
 * exists.
 *
 * <P><B>SAF Reference:</B> <code>SaAisErrorT.SA_AIS_ERR_EXIST</code>
 * @version AIS-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AIS-B.01.01
 *
 */
public final class AisExistException extends AisException {

    private static final long serialVersionUID = 5L;

    /**
     * Constructs a new exception with null as its detail message.
     */
    public AisExistException() {
        super( AisStatus.ERR_EXIST );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param message
     * @param nested
     */
    public AisExistException( String message, Throwable nested ) {
        super( message, AisStatus.ERR_EXIST, nested );
    }

    /**
     * Constructs a new exception with the specified detail message.
     * @param message
     */
    public AisExistException( String message ) {
        super( message, AisStatus.ERR_EXIST );
    }

    /**
     * Constructs a new exception with the specified detail message and nested underlying exception.
     * @param nested
     */
    public AisExistException( Throwable nested ) {
        super( AisStatus.ERR_EXIST, nested );
    }

}


