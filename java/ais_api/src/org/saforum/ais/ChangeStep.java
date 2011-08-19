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
 * The following constants are used by the enhanced track API to indicate in
 * which step of the tracking process the track callback is invoked.
 * <P>
 * <B>SAF Reference:</B> see below
 * 
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.05.01
 * 
 */
public enum ChangeStep implements EnumValue {

	/**
	 * The track callback is invoked to allow the client to accept or reject the
	 * change.
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CHANGE_VALIDATE</code>
	 */
	VALIDATE(1),

	/**
	 * The change is occurring (it has not been rejected), and the track
	 * callback is invoked to let the client perform all necessary actions such
	 * that this change can occur with minimal service impact.
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CHANGE_START</code>
	 * 
	 */
	START(2),

	/**
	 * A proposed change has been rejected.
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CHANGE_ABORTED</code>
	 */
	ABORTED(3),

	/**
	 * A change in the cluster membership has occurred.
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CHANGE_COMPLETED</code>
	 */
	COMPLETED(4);

	/**
	 * The numerical value assigned to this constant by the AIS specification.
	 */
	private final int value;

	/**
	 * Creates an enum constant with the numerical value assigned by the AIS
	 * specification.
	 * 
	 * @param value
	 *            The numerical value assigned to this constant by the AIS
	 *            specification.
	 */
	private ChangeStep(int value) {
		this.value = value;
	}

	/**
	 * Returns the numerical value assigned to this constant by the AIS
	 * specification.
	 * 
	 * @return the numerical value assigned to this constant by the AIS
	 *         specification.
	 */
    @Override
	public int getValue() {
		return this.value;
	}

}
