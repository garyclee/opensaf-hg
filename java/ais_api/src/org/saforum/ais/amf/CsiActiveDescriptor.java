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
 * The CsiActiveDescriptor type holds additional information about the
 * assignment of a component service instance to a component when the component
 * is requested to assume the active HA state for this component service
 * instance.
 *
 * When a component is requested to assume the active HA state for one or for
 * all component service instances assigned to the component,
 * CsiActiveDescriptor holds the information shown below:
 * <UL>
 * <LI>The Availability Management Framework uses the transitionDescriptor that
 * is appropriate for the redundancy model of the service group this component
 * belongs to.
 * <LI>If transitionDescriptor is set to NOT_QUIESCED or CSI_QUIESCED,
 * activeComponentName holds the name of the component that was previously
 * assigned the active state for the component service instances and no longer
 * has that assignment.
 * <LI>If transitionDescriptor is set to NEW_ASSIGN, activeComponentName is not
 * used.
 * <LI>If transitionDescriptor is set to STILL_ACTIVE, activeComponentName
 * holds the name of one of the components which is still assigned the active HA
 * state for all targeted component service instances. The choice of the
 * component selected in that case is arbitrary.
 * </UL>
 * <P><B>SAF Reference:</B> <code>SaAmfCSIActiveDescriptorT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class CsiActiveDescriptor extends CsiStateDescriptor {

    /**
     * This descriptor provides information on the component that was or is
     * still active for the one or all of the specified component service
     * instance.
     */
    public TransitionDescriptor transitionDescriptor;

    /**
     * The name of the component that was previously active for the specified
     * component service instance.
     */
    public String activeComponentName;

    /**
     *  This enum provides information on the component that was or still is
     *  active for the specified component service instance.
     * <P><B>SAF Reference:</B> <code>SaAmfCSITransitionDescriptorT</code>
     * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
     * @since AMF-B.01.01
     * @see #transitionDescriptor
     */
    public enum TransitionDescriptor {

        /**
         * This assignment is not the result of a switchover or fail-over of the
         * specified component service instance from another component to this
         * component. No component was previously active for this component service
         * instance.
         */
        NEW_ASSIGN(1),

        /**
         * This assignment is the result of a switch-over of the specified component
         * service instance from another component to this component. The component
         * that was previously active for this component service instance has been
         * quiesced.
         */
        QUIESCED(2),

        /**
         * This assignment is the result of a fail-over of the specified component
         * service instance from another component to this component. The component
         * that was previously active for this component service instance has not
         * been quiesced.
         */
        NOT_QUIESCED(3),

        /**
         * This assignment is not the result of a switchover or fail-over of the
         * specified component service instance from another component to this
         * component. At least one other component is still active for this
         * component service instance. This flag is used, for example, in the N-way
         * active redundancy model when a new component is assigned active for a
         * component service instance while other components are already assigned
         * active for that component service instance.
         */
        STILL_ACTIVE(4);



        /**
         * The numerical value assigned to this constant by the AIS specification.
         */
        private final int value;

        /**
         * Creates an enum constant with the numerical value assigned by the AIS specification.
         * @param value The numerical value assigned to this constant by the AIS specification.
         */
        private TransitionDescriptor( int value ){
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


