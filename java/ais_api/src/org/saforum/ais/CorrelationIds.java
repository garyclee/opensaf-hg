/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R6-A.01.01
**   SAI-Overview-B.05.01
**   SAI-AIS-NTF-A.03.01
**
** DATE: 
**   Monday November 17, 2008
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
 * Correlation Id structure. Used to build a (distributed) hierarchical structure of correlated notifications. Note that (as of ntf-A.03.00.08) this type is not used).
 * <p>
 * <b>SAF Reference:</b> <code>SaNtfCorrelationIdsT</code>
 * @version SAI-AIS-NTF-A.03.01 (SAIM-AIS-R6-A.01.01)
 * @since SAI-AIS-NTF-A.03.01
 * @see Notification#UNUSED_IDENTIFIER
 */
public class CorrelationIds {
	long rootId;
	long parentId;
	long id;
}


