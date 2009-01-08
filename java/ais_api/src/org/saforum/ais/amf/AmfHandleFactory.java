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

import org.saforum.ais.FactoryImpl;

/**
 * This factory initializes the Availability Management Framework for the invoking client
 * and registers the various callback functions. It must be used prior to the invocation of
 * any other Availability Management Framework functionality. The <code>AmfHandle</code> object is
 * returned by the Availability Management Framework as the reference to this association between the
 * client and the Availability Management Framework. The client uses this handle in subsequent
 * communication with the Availability Management Framework. AIS libraries must
 * support several <code>AmfHandle</code> instances from the same binary
 * program (e.g., process in the POSIX.1 world). This allows support
 * for multithreaded dispatching of AIS callbacks.
 *
 * <P>The invoker of this factory must provide properties defining the location of the
 * Availability Management Framework implementation: SAF_AIS_AMF_IMPL_CLASSNAME and
 * SAF_AIS_AMF_IMPL_URL.
 *
 * <P><B>SAF Reference:</B> <code>saAmfInitialize</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public class AmfHandleFactory extends FactoryImpl<AmfHandle, AmfHandle.Callbacks>
implements org.saforum.ais.Factory<AmfHandle, AmfHandle.Callbacks> {
}
