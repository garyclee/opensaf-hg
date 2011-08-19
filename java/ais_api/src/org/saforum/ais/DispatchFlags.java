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
 * Indicates behaviour for "dispatch" calls.
 *
 * <P><B>SAF Reference:</B> <code>SaDispatchFlagsT</code>
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 * @see Handle#dispatch(DispatchFlags)
 *
 */
public enum DispatchFlags implements EnumValue {

    /**
     * Invoke a single pending callback in the context of the calling thread, if
     * there is a pending callback, and then return from the dispatch.
     */
    DISPATCH_ONE(1),

    /**
     * Invoke all of the pending callbacks in the context of the calling thread,
     * if there are any pending callbacks, before returning from dispatch.
     */
    DISPATCH_ALL(2),

    /**
     * One or more threads calling dispatch remain within dispatch and execute
     * callbacks as they become pending. The thread or threads do not return
     * from dispatch until the corresponding finalize method is executed by one
     * thread of the process.
     */
    DISPATCH_BLOCKING(3);



    /**
     * The numerical value assigned to this constant by the AIS specification.
     */
    private final int value;

    /**
     * Creates an enum constant with the numerical value assigned by the AIS specification.
     * @param value The numerical value assigned to this constant by the AIS specification.
     */
    private DispatchFlags( int value ){
        this.value = value;
    }

    /**
     * Returns the numerical value assigned to this constant by the AIS specification.
     * @return the numerical value assigned to this constant by the AIS specification.
     */
    @Override
    public int getValue() {
        return this.value;
    }

}

/*  */
