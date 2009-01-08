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
 * A class that contains the information about components in a protection group.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfProtectionGroupNotificationT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class ProtectionGroupNotification {

    /**
     * The kind of change in the associated component member.
     */
    public ProtectionGroupChanges change;

    /**
     * The information associated with the component member of the protection
     * group.
     */
    public ProtectionGroupMember member;

    /**
     * This enum describes possible protection group related changes in a component.
     * <P><B>SAF Reference:</B> <code>SaAmfProtectionGroupChangesT</code>
     * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since AMF-B.01.01
     * @see #change
     */
    public enum ProtectionGroupChanges {

        /**
         * No change in this member component has
         * occurred since the last callback.
         */
        NO_CHANGE(1),

        /**
         * The associated component service instance
         * has been added to the member component.
         */
        ADDED(2),

        /**
         * The associated component service instance
         * has been removed from the member component.
         */
        REMOVED(3),

        /**
         * Any of the elements
         * haState or rank of the ProtectionGroupMember object for the member
         * component have changed.
         */
        STATE_CHANGE(4);

        /**
         * The numerical value assigned to this constant by the AIS specification.
         */
        private final int value;

        /**
         * Creates an enum constant with the numerical value assigned by the AIS specification.
         * @param value The numerical value assigned to this constant by the AIS specification.
         */
        private ProtectionGroupChanges( int value ){
            this.value = value;
        }

        /**
         * Returns the numerical value assigned to this constant by the AIS specification.
         * @return the numerical value assigned to this constant by the AIS specification.
         */
        public int getValue() {
            return this.value;
        }
    }

}


