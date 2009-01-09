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
 * This class provides information about the component service instances
 * targeted by the setCsiCallback() callback.
 * <P><B>SAF Reference:</B> <code>SaAmfCSIDescriptorT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class CsiDescriptor {


    /**
     * CsiFlags describes how the HA State is assigned to the Component.
     * Here are the possible assignments:
     * <UL>
     * <LI>a new CSI is assigned by using CsiFlags.ADD_ONE
     * <LI>an already assigned CSI state is changed by using CsiFlags.TARGET_ONE
     * <LI>all already assigned CSI states are changed by using CsiFlags.TARGET_ALL
     * </UL>
     */
    public CsiFlags csiFlags;

    /**
     * When TARGET_ALL is set in csiFlags, csiName is not used; otherwise,
     * csiName contains the name of the component service instance targeted b
     * the callback.
     */
    public String csiName;

    /**
     * When the component is requested to assume the active or standby state for
     * the targeted service instances, csiStateDescriptor holds additional
     * information relative to that state transition; otherwise,
     * csiStateDescriptor is not used.
     */
    public CsiStateDescriptor csiStateDescriptor;

    /**
     * When ADD_ONE is set in csiFlags, csiAttribute refers to the attributes of
     * the newly assigned component service instance; otherwise, no attributes
     * are provided and csiAttribute is not used.
     */
    public CsiAttribute[] csiAttribute;

    /**
     * This enum describes how the HA State is assigned to the Component.
     * Here are the possible assignments:
     * <UL>
     * <LI>a new CSI is assigned by using CsiFlags.ADD_ONE
     * <LI>an already assigned CSI state is changed by using CsiFlags.TARGET_ONE
     * <LI>all already assigned CSI states are changed by using CsiFlags.TARGET_ALL
     * </UL>
     * <P><B>SAF Reference:</B> <code>SaAmfCSIFlagsT</code>
     * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since AMF-B.01.01
     */
    public enum CsiFlags {

        /**
         * A new component service instance is
         * assigned to the component. The component is requested to assume a
         * particular HA state for the new component service instance.
         */
        ADD_ONE(1),

        /**
         * Valid value for csiFlags field: the request made to the component targets
         * only one of its component service instances.
         */
        TARGET_ONE(2),

        /**
         * The request made to the component targets
         * all of its component service instances. This flag is used for cases in
         * which all component service instances are managed as a bundle: The
         * component is assigned the same HA state for all component service
         * instances at the same time, or all component service instances are
         * removed at the same time. For assignments, this flag is set for
         * components providing the 'x_active_or_y_standby' capability model. The
         * Availability Management Framework can use this flag in other cases for
         * removing all component service instances at once, if it makes sense.
         */
        TARGET_ALL(4);



        /**
         * The numerical value assigned to this constant by the AIS specification.
         */
        private final int value;



        /**
         * Creates an enum constant with the numerical value assigned by the AIS specification.
         * @param value The numerical value assigned to this constant by the AIS specification.
         */
        private CsiFlags( int value ){
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


