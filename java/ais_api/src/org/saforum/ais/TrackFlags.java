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
 * The following constants are used by the track API for all areas with track
 * APIs to specify what needs to be tracked and what information is supplied in
 * the notification callback.
 *
 * <P><B>SAF Reference:</B> see below
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.01.01
 *
 */
public enum TrackFlags implements EnumValue {

    /**
     * The notification callback is invoked each time at least one change
     * happens in the set of entities or one attribute of at least one entity in
     * the set changes. Information about all entities is passed to the
     * notification callback (both entities for which a change occurred and
     * entities for which no change occurred).
     *
     * <P><B>SAF Reference:</B> <code>SA_TRACK_CHANGES</code>
     *
     */
    TRACK_CHANGES( 2 ),

    /**
     * The notification callback is invoked each time at least one change
     * happens in the set of entities or one attribute of at least one entity in
     * the set changes. Only information about entities that changed is passed
     * in the notification callback.
     *
     * <P><B>SAF Reference:</B>  <code>SA_TRACK_CHANGES_ONLY</code>
     *
     */
    TRACK_CHANGES_ONLY( 4 );

    /**
     * The numerical value assigned to this constant by the AIS specification.
     */
    private final int value;

    /**
     * Creates an enum constant with the numerical value assigned by the AIS specification.
     * @param value The numerical value assigned to this constant by the AIS specification.
     */
    private TrackFlags( int value ) {
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
