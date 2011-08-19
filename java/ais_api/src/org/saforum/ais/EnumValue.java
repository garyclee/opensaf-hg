/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R6-A.01.01
**   SAI-Overview-B.05.01
**
** DATE: 
**   Tuesday July 14, 2009
**
** LEGAL:
**   OWNERSHIP OF SPECIFICATION AND COPYRIGHTS.
**
** Copyright 2009 by the Service Availability Forum. All rights reserved.
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
 * Defines the interface used by all Java Enum types in the AIS specifications that have a corresponding integer value derived from the C language specification.
 * Pure Java clients do not, in general, need access to this underlying values.
 * However, if the clients communicate with non-Java clients, use a non-SAF channel for communication,
 * or need to store these value (e.g. in a log), then the values become relevant.
 * <p>
 * The Enum types that implement this interface facilitate the implementation of a generic type to map, as well, from Integer
 * values to Enum constants.
 * 
 * @version SAI-AIS-CPROG-B.05.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-Overview-B.05.01
 */
public interface EnumValue {
	
	/**
	 * @return the int value associated with this Enum constant.
	 */
	int getValue();
}

/*  */
