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
The following constants are used by the enhanced track API to ...
 * provide
a response to a track callback call that was previously
invoked with a ChangeStep parameter equal to either ChangeStep.VALIDATE or
ChangeStep.START. * 
 * 
 * <P>
 * <B>SAF Reference:</B> see below
 * 
 * @version SAI-Overview-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.05.01
 * 
 */
public enum CallbackResponse implements EnumValue {

	/**
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CALLBACK_RESPONSE_OK</code>
	 */
	OK(1),

	/**
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CALLBACK_RESPONSE_REJECTED</code>
	 * 
	 */
	REJECTED(2),

	/**
	 * <P>
	 * <B>SAF Reference:</B> <code>SA_&lt;AREA&gt;_CALLBACK_RESPONSE_ERROR</code>
	 */
	ERROR(3);

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
	private CallbackResponse(int value) {
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
