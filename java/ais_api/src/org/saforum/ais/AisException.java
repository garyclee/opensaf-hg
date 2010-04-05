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
 * Root class for exceptions representing error messages coming from the underlying AIS service.
 */
public class AisException extends Exception implements java.io.Serializable {

    private static final long serialVersionUID = 5L;

    public final AisStatus cause;

    /**
     * @param cause --
     *            refer to the SAF AIS Spec.
     */
    public AisException(AisStatus cause) {
        super();
        this.cause = cause;
    }

    /**
     * @param explain
     *            additional explanation
     * @param cause
     *            See SAF AIS Spec
     */
    public AisException(String explain, AisStatus cause) {
        super(explain);
        this.cause = cause;
    }

    /**
     * @param cause See SAF AIS Spec
     * @param nested underlying exception that provoked this AisException
     */
    public AisException(AisStatus cause, Throwable nested) {
        super(nested);
        this.cause = cause;
    }
    /**
     * @param explain additional explanation
     * @param cause See SAF AIS Spec
     * @param nested underlying exception that provoked this AisException
     */
    public AisException(String explain, AisStatus cause, Throwable nested) {
        super(explain,nested);
        this.cause = cause;
    }
}
