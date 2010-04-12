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

import org.saforum.ais.*;

/**
 * This interface supports a callback by the Availability Management Framework:
 * the callback returns information requested by the component earlier about a
 * protection group associated with a component service instance.
 *
 * <P><B>SAF Reference:</B> <code>SaAmfProtectionGroupTrackCallbackT</code>
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 */
public interface TrackProtectionGroupCallback {

    /**
     * This callback is invoked in the context of a thread issuing an dispatch()
     * call on the handle AmfHandle, which was used when the process
     * requested tracking of changes in the protection group associated with the
     * component service instance, identified by csiName, or in an attribute of
     * any component in this protection group via the
     * startProtectionGroupTracking() call. If successful, the
     * trackProtectionGroup callback returns the requested information in the
     * notificationBuffer parameter. The kind of information returned depends on
     * the setting of the trackFlags parameter of the
     * startProtectionGroupTracking() method.
     * <P>
     * The length of the array of the notificationBuffer parameter might be
     * greater than the value of the nMembers parameter, because some components
     * may no longer be members of the protection group: If the CHANGES flag or
     * the CHANGES_ONLY flag is set, the notificationBuffer might contain
     * information about the current members of the protection group and also
     * about components that have recently left the protection group.
     * <P>
     * If an error occurs, it is returned in the error parameter.
     *
     * @param csiName [in] The name of the component service instance.
     * @param notificationBuffer [in] An array that contains the requested
     *            information about components in the protection group.
     * @param nMembers [in] The number of the components that belong to the
     *            protection group associated with the component service
     *            instance, designated by csiName.
     * @param error [in] This parameter indicates whether the Availability
     *            Management Framework was able to perform the operation. The
     *            parameter error has one of the values:
     *            <UL>
     *            <LI>OK - The function completed successfully.
     *            <LI>ERR_LIBRARY - An unexpected problem occurred in the library
     *            (such as corruption). The library cannot be used anymore.
     *            <LI>ERR_TIMEOUT - An implementation-dependent timeout occurred
     *            before the call could complete. It is unspecified whether the
     *            call succeeded or whether it did not.
     *            <LI>ERR_TRY_AGAIN - The service cannot be provided at this time.
     *            The process may retry tracking later.
     *            <LI>ERR_NO_MEMORY - Either the Availability Management Framework
     *            library or the provider of the service is out of memory and
     *            cannot provide the service. The process that requested
     *            tracking might have missed one or more notifications.
     *            <LI>ERR_NO_RESOURCES - Either the Availability Management
     *            Framework library or the provider of the service is out of
     *            required resources (other than memory), and cannot provide the
     *            service. The process that requested tracking might have missed
     *            one or more notifications.
     *            </UL>
     *            If the error returned is ERR_NO_MEMORY or ERR_NO_RESOURCES, the
     *            process that requested tracking should invoke
     *            stopProtectionGroupTracking() (of ProtectionGroupManager) and
     *            then request tracking again to resynchronize with the
     *            current state.
     */
    void trackProtectionGroupCallback( String csiName,
                                      ProtectionGroupNotification[] notificationBuffer,
                                      int nMembers,
                                      AisStatus error );

}


