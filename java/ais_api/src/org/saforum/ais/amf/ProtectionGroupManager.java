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
 * A ProtectionGroupManager object contains API methods that enable tracking changes in the
 * protection group associated with a component service instance, or changes of
 * attributes of any component in the protection group.
 * @version AMF-B.01.01 (SAIM-AIS-R2-JD-A.01.01)
 * @since AMF-B.01.01
 *
 */
public interface ProtectionGroupManager {



    /**
     * The Availability Management Framework is requested to return information
     * about all components in the protection group associated with the
     * component service instance, identified by csiName.
     *
     * <P><B>SAF Reference:</B> <code>saAmfProtectionGroupTrack(..., SA_TRACK_CURRENT, notificationBuffer )</code>
     *
     * @param csiName [in] The name of the component service instance, which is
     *            also the name of the protection group.
     * @return information about all components in the protection group
     *         identified by csiName.
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component service instance, designated by csiName,
     *             cannot be found.
     */
    public ProtectionGroupNotification[] getProtectionGroup( String csiName )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * This method is used to obtain information about all components in the
     * protection group associated with the component service instance,
     * identified by csiName: this information is returned by a single
     * subsequent invocation of the trackProtectionGroupCallback() notification
     * callback.
     * <P><B>SAF Reference:</B> <code>saAmfProtectionGroupTrack(..., SA_TRACK_CURRENT, NULL )</code>
     *
     * @param csiName [in] The name of the component service instance, which is
     *            also the name of the protection group.
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisInitException The previous initialization of
     *             AmfHandle was incomplete, since the
     *             TrackProtectionGroupCallback() callback is missing.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component service instance, designated by csiName,
     *             cannot be found.
     * @see TrackProtectionGroupCallback#trackProtectionGroupCallback(String,
     *      ProtectionGroupNotification[], int, AisStatus)
     */
    public void getProtectionGroupAsync( String csiName )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

    /**
     * This method is used to obtain the current protection group membership
     * <I>and</I> to request notification of changes in the protection group
     * membership or of changes in an attribute of a protection group, depending
     * on the value of the trackFlags parameter. The current protection group
     * membership information is returned synchronously. The changes are
     * notified asynchronously, via the invocation of the
     * trackProtectionGroupCallback() callback method, which must have been
     * supplied when the process invoked the AmfHandle.initializeHandle()
     * call.
     * <P>
     * An application may call this method repeatedly.
     * <P>
     * If this method is called with trackFlags containing TRACK_CHANGES_ONLY
     * when protection group changes are currently being tracked with
     * TRACK_CHANGES, the Availability Management Framework will invoke further
     * notification callbacks according to the new value of trackFlags. The same
     * is true vice versa.
     * <P>
     * Once this method has been called, notifications can only be stopped by an
     * invocation of stopProtectionGroupTracking() or
     * AmfHandle.finalizeHandle().
     * <P><B>SAF Reference:</B> <code>saAmfProtectionGroupTrack()</code>
     * <P>This method corresponds to an invocation of
     * <UL>
     * <LI>saAmfProtectionGroupTrack(..., SA_TRACK_CURRENT | SA_TRACK_CHANGES,
     * ntfBufferPtr ) or
     * <LI>saAmfProtectionGroupTrack(..., SA_TRACK_CURRENT |
     * SA_TRACK_CHANGES_ONLY, ntfBufferPtr ).
     * </UL>
     *
     * @param csiName [in] The name of the component service instance, which is
     *            also the name of the protection group, for which tracking is
     *            to start.
     * @param trackFlags [in] The kind of tracking that is requested, which is
     *            either TRACK_CHANGES or TRACK_CHANGES_ONLY, as defined in
     *            ais.TrackFlags, which have the following implementation here:
     *            <UL>
     *            <LI>TRACK_CHANGES - The notification callback is invoked each
     *            time at least one change occurs in the protection group
     *            membership, or one attribute of at least one component in the
     *            protection group changes. Information about all of the
     *            components is passed to the callback.
     *            <LI>TRACK_CHANGES_ONLY - The notification callback is invoked
     *            each time at least one change occurs in the protection group
     *            membership, or one attribute of at least one component in the
     *            protection group changes. Only information about components in
     *            the protection group that have changed is passed to this
     *            callback method.
     *            </UL>
     *            It is not permitted to set both TRACK_CHANGES and
     *            TRACK_CHANGES_ONLY in an invocation of this method.
     *            SaAmfProtectionGroupNotificationBufferT buffer is: group.
     * @return information about all components in the protection group
     *         identified by csiName.
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupMembershipManager object is invalid, since it
     *             is corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with
     *             AmfHandle was incomplete, since the
     *             TrackProtectionGroupCallback() callback is missing.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component service instance, designated by csiName,
     *             cannot be found.
     * @throws AisBadFlagsException The trackFlags parameter is invalid.
     * @see TrackProtectionGroupCallback#trackProtectionGroupCallback(String, ProtectionGroupNotification[],
     *      int, AisStatus)
     */
    public ProtectionGroupNotification[] getProtectionGroupThenStartTracking( String csiName,
                                                                              TrackFlags trackFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;


    /**
     * This method is used to obtain the current protection group membership
     * <I>and</I> to request notification of changes in the protection group
     * membership or of changes in an attribute of a protection group, depending
     * on the value of the trackFlags parameter. The current protection group
     * membership information is returned asynchronously, by a single invocation of the
     * trackProtectionGroupCallback() notification callback (which must have been
     * supplied when the process invoked the AmfHandle.initializeHandle() call). The changes are
     * also notified asynchronously, via the invocation of the
     * trackProtectionGroupCallback() callback method.
     * <P>
     * An application may call this method repeatedly.
     * <P>
     * If this method is called with trackFlags containing TRACK_CHANGES_ONLY
     * when protection group changes are currently being tracked with
     * TRACK_CHANGES, the Availability Management Framework will invoke further
     * notification callbacks according to the new value of trackFlags. The same
     * is true vice versa.
     * <P>
     * Once this method has been called, notifications can only be stopped by an
     * invocation of stopProtectionGroupTracking() or
     * AmfHandle.finalizeHandle().
     *
     * <P><B>SAF Reference:</B> <code>saAmfProtectionGroupTrack()</code>
     * <P>This method corresponds to an invocation of
     * <UL>
     * <LI>saAmfProtectionGroupTrack(..., SA_TRACK_CURRENT | SA_TRACK_CHANGES,
     * null ) or
     * <LI>saAmfProtectionGroupTrack(..., SA_TRACK_CURRENT |
     * SA_TRACK_CHANGES_ONLY, null ).
     * </UL>
     *
     * @param csiName [in] The name of the component service instance, which is
     *            also the name of the protection group, for which tracking is
     *            to start.
     * @param trackFlags [in] The kind of tracking that is requested, which is
     *            either TRACK_CHANGES or TRACK_CHANGES_ONLY, as defined in
     *            ais.TrackFlags, which have the following implementation here:
     *            <UL>
     *            <LI>TRACK_CHANGES - The notification callback is invoked each
     *            time at least one change occurs in the protection group
     *            membership, or one attribute of at least one component in the
     *            protection group changes. Information about all of the
     *            components is passed to the callback.
     *            <LI>TRACK_CHANGES_ONLY - The notification callback is invoked
     *            each time at least one change occurs in the protection group
     *            membership, or one attribute of at least one component in the
     *            protection group changes. Only information about components in
     *            the protection group that have changed is passed to this
     *            callback method.
     *            </UL>
     *            It is not permitted to set both TRACK_CHANGES and
     *            TRACK_CHANGES_ONLY in an invocation of this method.
     *            SaAmfProtectionGroupNotificationBufferT buffer is: group.
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupMembershipManager object is invalid, since it
     *             is corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with
     *             AmfHandle was incomplete, since the
     *             TrackProtectionGroupCallback() callback is missing.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component service instance, designated by csiName,
     *             cannot be found.
     * @throws AisBadFlagsException The trackFlags parameter is invalid.
     * @see TrackProtectionGroupCallback#trackProtectionGroupCallback(String,
     *      ProtectionGroupNotification[], int, AisStatus)
     */
    public void getProtectionGroupAsyncThenStartTracking( String csiName,
                                                    TrackFlags trackFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;


    /**
     * The Availability Management Framework is requested to start tracking
     * changes in the protection group associated with the component service
     * instance, identified by csiName, or changes of attributes of any
     * component in the protection group, depending on the value of the
     * trackFlags parameter. These changes are notified via the invocation of
     * the trackProtectionGroupCallback() callback method, which must have been
     * supplied when the process invoked the AmfHandle.initializeHandle()
     * call. An application may call this method repeatedly.
     * <P>
     * If this method is called with trackFlags containing TRACK_CHANGES_ONLY
     * when protection group changes are currently being tracked with
     * TRACK_CHANGES, the Availability Management Framework will invoke further
     * notification callbacks according to the new value of trackFlags. The same
     * is true vice versa.
     * <P>
     * Once this method has been called, notifications can only be stopped by an
     * invocation of stopProtectionGroupTracking() or
     * AmfHandle.finalizeHandle().
     *
     * <P><B>SAF Reference:</B> <code>saAmfProtectionGroupTrack()</code>
     * <P>This method corresponds to an invocation of
     * <UL>
     * <LI>saAmfProtectionGroupTrack(..., SA_TRACK_CHANGES, null ) or
     * <LI>saAmfProtectionGroupTrack(..., SA_TRACK_CHANGES_ONLY, null ).
     * </UL>
     *
     * @param csiName [in] The name of the component service instance, which is
     *            also the name of the protection group, for which tracking is
     *            to start.
     * @param trackFlags [in] The kind of tracking that is requested, which is
     *            either TRACK_CHANGES or TRACK_CHANGES_ONLY, as defined in
     *            ais.TrackFlags, which have the following implementation here:
     *            <UL>
     *            <LI>TRACK_CHANGES - The notification callback is invoked each
     *            time at least one change occurs in the protection group
     *            membership, or one attribute of at least one component in the
     *            protection group changes. Information about all of the
     *            components is passed to the callback.
     *            <LI>TRACK_CHANGES_ONLY - The notification callback is invoked
     *            each time at least one change occurs in the protection group
     *            membership, or one attribute of at least one component in the
     *            protection group changes. Only information about components in
     *            the protection group that have changed is passed to this
     *            callback method.
     *            </UL>
     *            It is not permitted to set both TRACK_CHANGES and
     *            TRACK_CHANGES_ONLY in an invocation of this method.
     *            SaAmfProtectionGroupNotificationBufferT buffer is: group.
     *
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupMembershipManager object is invalid, since it
     *             is corrupted or has already been finalized.
     * @throws AisInitException The previous initialization with
     *             AmfHandle was incomplete, since the
     *             TrackProtectionGroupCallback() callback is missing.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException The component service instance, designated by csiName,
     *             cannot be found.
     * @throws AisBadFlagsException The trackFlags parameter is invalid.
     * @see TrackProtectionGroupCallback#trackProtectionGroupCallback(String,
     *      ProtectionGroupNotification[], int, AisStatus)
     */
    public void startProtectionGroupTracking( String csiName,
                                                    TrackFlags trackFlags )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisInitException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;


    /**
     * The invoking process requests the Availability Management Framework to
     * stop tracking protection group changes for the component service instance
     * designated by csiName.
     * <P><B>SAF Reference:</B> <code>saAmfProtectionGroupTrackStop()</code>
     *
     * @param csiName [in] The name of the component service instance.
     * @throws AisLibraryException An unexpected problem occurred in the library
     *             (such as corruption). The library cannot be used anymore.
     * @throws AisTimeoutException An implementation-dependent timeout occurred,
     *             or the timeout, specified by the timeout parameter, occured
     *             before the call could complete. It is unspecified whether the
     *             call succeeded or whether it did not.
     * @throws AisTryAgainException The service cannot be provided at this time.
     *             The process may retry later.
     * @throws AisBadHandleException The library handle associated with this
     *             ProtectionGroupManager object is invalid, since it is
     *             corrupted or has already been finalized.
     * @throws AisNoMemoryException Either the Availability Management Framework
     *             library or the provider of the service is out of memory and
     *             cannot provide the service.
     * @throws AisNoResourcesException The system is out of required resources
     *             (other than memory).
     * @throws AisNotExistException One or both cases below occurred:
     *             <UL>
     *             <LI>The component service instance, designated by csiName,
     *             cannot be found,
     *             <LI>No track of changes in the protection group membership
     *             was previously started, which is still in effect.
     *             </UL>
     */
    public void stopProtectionGroupTracking( String csiName )
        throws AisLibraryException,
            AisTimeoutException,
            AisTryAgainException,
            AisBadHandleException,
            AisNoMemoryException,
            AisNoResourcesException,
            AisNotExistException;

}


