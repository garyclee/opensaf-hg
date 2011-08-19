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
 * Global constants derived from the SAF saAIS.h file. The time values in this class
 * are always expressed as a positive number of nanoseconds (except for the SA_TIME_UNKNOWN constant).
 *
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 *
 */
public final class Consts {

    /**
     * This constant represents an unknown time value.
     */
    public static final long SA_TIME_UNKNOWN = 0x8000000000000000L;

    /**
     * END represents the largest timestamp value: Fri Apr 11 23:47:16.854775807
     * UTC 2262.
     */
    public static final long SA_TIME_END = 0x7FFFFFFFFFFFFFFFL;

    /**
     * BEGIN represents the smallest timestamp value: Thu 1 Jan 00:00:00 UTC
     * 1970.
     */
    public static final long SA_TIME_BEGIN = 0x0L;

    /**
     *
     */
    public static final long SA_TIME_ONE_MICROSECOND = 1000L;

    /**
     *
     */
    public static final long SA_TIME_ONE_MILLISECOND = 1000000L;

    /**
     *
     */
    public static final long SA_TIME_ONE_SECOND = 1000000000L;

    /**
     *
     */
    public static final long SA_TIME_ONE_MINUTE = 60000000000L;

    /**
     *
     */
    public static final long SA_TIME_ONE_HOUR = 3600000000000L;

    /**
     *
     */
    public static final long SA_TIME_ONE_DAY = 86400000000000L;

    /**
     * A duration of MAX is interpreted as an infinite duration. If a timeout
     * parameter is set to MAX when invoking an AIS API function, there will be
     * no time limit associated with this request. This value should be viewed
     * as a convenience value for programmers who do not care about timeouts
     * associated with various APIs. Typically, it is not advisable to use MAX
     * in timeout parameters, and other pre-defined constants should generally
     * suffice.
     */
    public static final long SA_TIME_MAX = SA_TIME_END;

    /**
	 * This constant is part of the enhanced tracking API: the client
	 * requests that the notification callback is called in the start step.
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_TRACK_START_STEP</code>
	 * 
	 * @since SAI-Overview-B.05.01
	 */
    public final static int TRACK_START_STEP = 0x10;

    /**
	 * This constant is part of the enhanced tracking API: the client requests
	 * that the notification callback is called in the validate step.
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_TRACK_VALIDATE_STEP</code>
	 * 
	 * @since SAI-Overview-B.05.01
	 */
    public final static int TRACK_VALIDATE_STEP = 0x20;


}

/*  */
