/*******************************************************************************
**
** SPECIFICATION VERSION:
**   SAIM-AIS-R2-JD-A.01.01
**   SAI-Overview-B.01.01
**   SAI-AIS-CLM-B.01.01
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

package org.saforum.ais.clm;

import org.saforum.ais.FactoryImpl;

/**
 * This factory initializes the Cluster Membership Service for the invoking client
 * and registers the various callback objects. It must be used prior to the invocation of
 * any other Cluster Membership Service functionality. The <code>ClmHandle</code> object is
 * returned by the Cluster Membership Service as the reference to this association between the
 * client and the Cluster Membership Service. The client uses this handle in subsequent
 * communication with the Cluster Membership Service. AIS libraries must
 * support several <code>ClmHandle</code> instances from the same binary
 * program (e.g., process in the POSIX.1 world). This allows support
 * for multithreaded dispatching of AIS callbacks.
 *
 * <P>The invoker of this factory must provide properties defining the location of the
 * Cluster Membership Service implementation: SAF_AIS_CLM_IMPL_CLASSNAME and SAF_AIS_CLM_IMPL_URL.
 *
 * <P><B>SAF Reference:</B> <code>saClmInitialize</code>
 * @version CLM-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since CLM-B.01.01
 */
public class ClmHandleFactory extends FactoryImpl<ClmHandle, ClmHandle.Callbacks>
implements org.saforum.ais.Factory<ClmHandle, ClmHandle.Callbacks> {
}
