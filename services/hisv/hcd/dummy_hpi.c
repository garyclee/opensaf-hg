/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation 
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php 
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *   
 */


/*****************************************************************************
*                                                                            *
*  MODULE NAME:  dummy_hpi.c                                                 *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the dummy functions of Open HPI. This is used to     *
*  avoid function linking errors while building the HISv modules on windows  *
*  Open HPI is not available on Windows and hence these dummy functions are  *
*  used just for building and linking the HISv modules on Windows.           *
*  The code is commented while linking with actual Open HPI implementation.  *
*                                                                            *
*****************************************************************************/

#include "hcd.h"


/*******************************************************************************
**
** Name: saHpiInitialize
**
** Description:
**   This function allows the management service an opportunity to perform
**   platform-specific initialization. saHpiInitialize() must be called
**   before any other functions are called.
**
** Parameters:
**   HpiImplVersion - [out] Pointer to the version of the HPI
**      implementation.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_DUPLICATE is returned when the HPI has already
**   been initialized. Once one saHpiInitialize() call has been made,
**   another one cannot be made until after a saHpiFinalize() call is made.
**
**
** Remarks:
**   This function returns the version of the HPI implementation. Note: If
**   the HPI interface version is needed it can be retrieved from the
**   SAHPI_INTERFACE_VERSION definition.
**
*******************************************************************************/
SaErrorT saHpiInitialize (SaHpiVersionT *HpiImplVersion)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiInitialize\n");
   return SA_OK;
}



/*******************************************************************************
**
** Name: saHpiFinalize
**
** Description:
**   This function allows the management service an opportunity to perform
**   platform-specific cleanup. All sessions should be closed (see
**   saHpiSessionClose()), before this function is executed. All open
**   sessions will be forcibly closed upon execution of this command.
**
** Parameters:
**   None.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.   5  Domains
**
*******************************************************************************/
SaErrorT saHpiFinalize ()
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiFinalize\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSessionOpen
**
** Description:
**   This function opens a HPI session for a given domain and set of
**   security characteristics (future). This function call assumes that a
**   pre-arranged agreement between caller and the HPI implementation
**   identifies the resources within the specified domain. As a future
**   roadmap item, functions for discovery of domains and allocation of
**   resources within domains may be developed.
**
** Parameters:
**   DomainId - [in] Domain ID to be controlled by middleware/application.
**      A domain ID of SAHPI_DEFAULT_DOMAIN_ID indicates the default domain.
**   SessionId - [out] Pointer to a location to store a handle to the newly
**      opened session. This handle is used for subsequent access to domain
**      resources and events.
**   SecurityParams - [in] Pointer to security and permissions data
**      structure. This parameter is reserved for future use, and must be set
**      to NULL.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_DOMAIN is returned if no domain
**   matching the specified domain ID exists.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiSessionOpen (SaHpiDomainIdT   DomainId,
                           SaHpiSessionIdT  *SessionId,
                           void *SecurityParams)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSessionOpen\n");
   return SA_OK;
}



/*******************************************************************************
**
** Name: saHpiSessionClose
**
** Description:
**   This function closes a HPI session. After closing a session, the
**   session ID will no longer be valid.
**
** Parameters:
**   SessionId - [in] Session handle previously obtained using
**      saHpiSessionOpen().
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiSessionClose (SaHpiSessionIdT SessionId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSessionClose\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourcesDiscover
**
** Description:
**   This function requests the underlying management service to discover
**   information about the resources it controls for the domain associated
**   with the open session. This function may be called during operation to
**   regenerate the RPT table. For those FRUs that must be discovered by
**   polling, latency between FRU insertion and actual addition of the
**   resource associated with that FRU to the RPT exists. To overcome this
**   latency, a discovery of all present resources may be forced by calling
**   saHpiResourcesDiscover ().
**
** Parameters:
**   SessionId - [in] Handle to session context.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiResourcesDiscover (SaHpiSessionIdT SessionId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourcesDiscover\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiRptInfoGet
**
** Description:
**   This function is used for requesting information about the resource
**   presence table (RPT) such as an update counter and timestamp. This is
**   particularly useful when using saHpiRptEntryGet() (see page 31).
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   RptInfo - [out] Pointer to the information describing the resource
**      presence table.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiRptInfoGet (SaHpiSessionIdT SessionId,
                          SaHpiRptInfoT *RptInfo)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiRptInfoGet\n");
   return SA_OK;
}



/*******************************************************************************
**
** Name: saHpiRptEntryGet
**
** Description:
**   This function retrieves resource information for the specified entry
**   of the resource presence table. This function allows the caller to
**   read the RPT entry-by-entry. If the EntryID parameter is set to
**   SAHPI_FIRST_ENTRY, the first entry in the RPT will be returned. When
**   an entry is successfully retrieved,  *NextEntryID will be set to the
**   ID of the next valid entry; however, when the last entry has been
**   retrieved, *NextEntryID will be set to SAHPI_LAST_ENTRY. To retrieve
**   an entire list of entries, call this function first with an EntryID of
**   SAHPI_FIRST_ENTRY and then use the returned NextEntryID in the next
**   call. Proceed until the NextEntryID returned is SAHPI_LAST_ENTRY. At
**   initialization, the user may not wish to turn on eventing, since the
**   context of the events, as provided by the RPT, is not known. In this
**   instance, if a FRU is inserted into the system while the RPT is being
**   read entry by entry, the resource associated with that FRU may be
**   missed. (Keep in mind that there is no specified ordering for the RPT
**   entries.)  The update counter provides a means for insuring that no
**   resources are missed when stepping through the RPT. In order to use
**   this feature, the user should invoke saHpiRptInfoGet(), and get the
**   update counter value before retrieving the first RPT entry. After
**   reading the last entry, the user should again invoke the
**   saHpiRptInfoGet() to get the update counter value. If the update
**   counter has not been incremented, no new records have been added.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   EntryId - [in] Handle of the entry to retrieve from the RPT. Reserved
**      entry ID values:  SAHPI_FIRST_ENTRY  Get first entry  SAHPI_LAST_ENTRY
**        Reserved as delimiter for end of list. Not a valid entry identifier.
**
**   NextEntryId - [out] Pointer to location to store the record ID of next
**      entry in RPT.
**   RptEntry - [out] Pointer to the structure to hold the returned RPT
**      entry.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiRptEntryGet (SaHpiSessionIdT SessionId,
                           SaHpiEntryIdT EntryId,
                           SaHpiEntryIdT  *NextEntryId,
                           SaHpiRptEntryT *RptEntry)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiRptEntryGet\n");
   *NextEntryId = SAHPI_LAST_ENTRY;
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiRptEntryGetByResourceId
**
** Description:
**   This function retrieves resource information from the resource
**   presence table for the specified resource using its resource ID.
**   Typically at start-up, the RPT is read entry-by-entry, using
**   saHpiRptEntryGet(). From this, the caller can establish the set of
**   resource IDs to use for future calls to the HPI functions. However,
**   there may be other ways of learning resource IDs without first reading
**   the RPT. For example, resources may be added to the domain while the
**   system is running in response to a hot-swap action. When a resource is
**   added, the application will receive a hot-swap event containing the
**   resource ID of the new resource. The application may then want to
**   search the RPT for more detailed information on the newly added
**   resource. In this case, the resource ID can be used to locate the
**   applicable RPT entry information.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource whose RPT entry should
**      be returned.
**   RptEntry  - [out] Pointer to structure to hold the returned RPT entry.
**
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiRptEntryGetByResourceId (SaHpiSessionIdT  SessionId,
                                       SaHpiResourceIdT ResourceId,
                                       SaHpiRptEntryT   *RptEntry)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiRptEntryGetByResourceId\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceSeveritySet
**
** Description:
**   This function allows the caller to set the severity level applied to
**   an event issued if a resource unexpectedly becomes unavailable to the
**   HPI. A resource may become unavailable for several reasons including:
**   ? The FRU associated with the resource is no longer present in the
**   system (a surprise extraction has occurred) ? A catastrophic failure
**   has occurred Typically, the HPI implementation will provide an
**   appropriate default value for this parameter, which may vary by
**   resource; management software can override this default value by use
**   of this function ? If a resource is removed from, then re-added to the
**   RPT (e.g., because of a hot-swap action), the HPI implementation may
**   reset the value of this parameter.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource for which the severity
**      level will be set.
**   Severity - [in] Severity level of event issued when the resource
**      unexpectedly becomes unavailable to the HPI.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiResourceSeveritySet (SaHpiSessionIdT  SessionId,
                                   SaHpiResourceIdT ResourceId,
                                   SaHpiSeverityT   Severity)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceSeveritySet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceTagSet
**
** Description:
**   This function allows the caller to set the resource tag for a
**   particular resource. The resource tag is an informational value that
**   supplies the caller with naming information for the resource. This
**   should be set to the "user-visible" name for a resource, which can be
**   used to identify the resource in messages to a human operator. For
**   example, it could be set to match a physical, printed label attached
**   to the entity associated with the resource. Typically, the HPI
**   implementation will provide an appropriate default value for this
**   parameter; this function is provided so that management software can
**   override the default, if desired. The value of the resource tag may be
**   retrieved from the resource's RPT entry. Note: If a resource is
**   removed from, then re-added to the RPT (e.g., because of a hot-swap
**   action), the HPI implementation may reset the value of this parameter.
**
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource for which the resource
**      tag should be set.
**   ResourceTag - [in] Pointer to string representing the resource tag.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiResourceTagSet (SaHpiSessionIdT    SessionId,
                              SaHpiResourceIdT   ResourceId,
                              SaHpiTextBufferT   *ResourceTag)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceTagSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceIdGet
**
** Description:
**   This function returns the resource ID of the resource associated with
**   the entity upon which the caller is running.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [out] Pointer to location to hold the returned resource
**      ID.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_NOT_PRESENT is returned if the entity the
**   caller is running on is not manageable in the addressed domain.
**   SA_ERR_HPI_UNKNOWN is returned if the domain controller cannot
**   determine an appropriate response. That is, there may be an
**   appropriate resource ID in the domain to return, but it cannot be
**   determined.
**
** Remarks:
**   This function must be issued within a session to a domain that
**   includes a resource associated with the entity upon which the caller
**   is running, or the SA_ERR_HPI_NOT_PRESENT return will be issued. Since
**   entities are contained within other entities, there may be multiple
**   possible resources that could be returned to this call. For example,
**   if there is a resource ID associated with a particular compute blade
**   upon which the caller is running, and another associated with the
**   chassis which contains the compute blade, either could logically be
**   returned as an indication of a resource associated with the entity
**   upon which the caller was running. The function should return the
**   resource ID of the "smallest" resource that is associated with the
**   caller. So, in the example above, the function should return the
**   resource ID of the compute blade. Once the function has returned the
**   resourceID, the caller may issue further HPI calls using that
**   resourceID to learn the type of resource that been identified.
**
*******************************************************************************/
SaErrorT saHpiResourceIdGet (SaHpiSessionIdT   SessionId,
                             SaHpiResourceIdT  *ResourceId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceIdGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEntitySchemaGet
**
** Description:
**   This function returns the identifier of the Entity Schema for the HPI
**   implementation. This schema defines valid Entity Paths that may be
**   returned by the HPI implementation.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   SchemaId - [out] Pointer to the ID of the schema in use; zero
**      indicates that a custom schema is in use.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   This function may be issued on any session opened to any domain in the
**   system, and will return the same identifier. The identifier returned
**   should either be zero, indicating that the HPI implementation uses a
**   custom schema; or one of the schema identifiers defined in Appendix A,
**   "Pre-Defined Entity Schemas," page 107. In the case of a custom
**   schema, the HPI implementation may use arbitrary entity paths to
**   describe resources in the system; in the case of a pre-defined schema,
**   all entity paths should conform to the schema.
**
*******************************************************************************/
SaErrorT saHpiEntitySchemaGet (SaHpiSessionIdT     SessionId,
                               SaHpiUint32T        *SchemaId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEntitySchemaGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogInfoGet
**
** Description:
**   This function retrieves the number of entries in the system event log,
**   total size of the event log, timestamp for the most recent entry, the
**   log's idea of the current time (i.e., timestamp that would be placed
**   on an entry at this moment), enabled/disabled status of the log (see
**   saHpiEventLogStateSet()), the overflow flag, the overflow action, and
**   whether the log supports deletion of individual entries.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log.
**   Info - [out] Pointer to the returned SEL information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiEventLogInfoGet (SaHpiSessionIdT  SessionId,
                               SaHpiResourceIdT ResourceId,
                               SaHpiSelInfoT    *Info)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogInfoGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogEntryGet
**
** Description:
**   This function retrieves an event log entry from a system event log.
**   The special entry IDs SAHPI_OLDEST_ENTRY and SAHPI_NEWEST_ENTRY are
**   used to select the oldest and newest entries, respectively, in the log
**   being read. A returned NextEntryID of SAHPI_NO_MORE_ENTRIES indicates
**   that the newest entry has been returned; there are no more entries
**   going forward (time-wise) in the log. A returned PrevEntryID of
**   SAHPI_NO_MORE_ENTRIES indicates that the oldest entry has been
**   returned. To retrieve an entire list of entries going forward (oldest
**   entry to newest entry) in the log, call this function first with an
**   EntryID of SAHPI_OLDEST_ENTRY and then use the returned NextEntryID as
**   the EntryID in the next call. Proceed until the NextEntryID returned
**   is SAHPI_NO_MORE_ENTRIES. To retrieve an entire list of entries going
**   backward (newest entry to oldest entry) in the log, call this function
**   first with an EntryID of SAHPI_NEWEST_ENTRY and then use the returned
**   PrevEntryID as the EntryID in the next call. Proceed until the
**   PrevEntryID returned is SAHPI_NO_MORE_ENTRIES.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be read. Set to SAHPI_DOMAIN_CONTROLLER_ID to address the
**      domain system event log.
**   EntryId - [in] Handle of the entry to retrieve from the SEL. Reserved
**      event log entry ID values:  SAHPI_OLDEST_ENTRY Oldest entry in the
**      log.  SAHPI_NEWEST_ENTRY Newest entry in the log.
**      SAHPI_NO_MORE_ENTRIES Not valid for this parameter. Used only when
**      retrieving the next and previous entry IDs.
**   PrevEntryId - [out] Handle of previous (older adjacent) entry in event
**      log. Reserved event log entry ID values:  SAHPI_OLDEST_ENTRY Not valid
**      for this parameter. Used only for the EntryID parameter.
**      SAHPI_NEWEST_ENTRY Not valid for this parameter. Used only for the
**      EntryID parameter.  SAHPI_NO_MORE_ENTRIES No more entries in the log
**      before the one referenced by the EntryId parameter.
**   NextEntryId - [out] Handle of next (newer adjacent) entry in event
**      log. Reserved event log entry ID values:  SAHPI_OLDEST_ENTRY Not valid
**      for this parameter. Used only for the EntryID parameter.
**      SAHPI_NEWEST_ENTRY Not valid for this parameter. Used only for the
**      EntryID parameter.  SAHPI_NO_MORE_ENTRIES No more entries in the log
**      after the one referenced by the EntryId parameter.
**   EventLogEntry - [out] Pointer to retrieved event log entry.
**   Rdr - [in/out] Pointer to structure to receive resource data record
**      associated with the event, if available. If NULL, no RDR data will be
**      returned.
**   RptEntry - [in/out] Pointer to structure to receive RPT Entry
**      associated with the event, if available. If NULL, no RPT entry data
**      will be returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Event logs may include RPT entries and resource data records
**   associated with the resource and sensor issuing an event along with
**   the basic event data in the log. Because the system may be
**   reconfigured after the event was entered in the log, this stored
**   information may be important to interpret the event. If the event log
**   includes logged RPT Entries and/or RDRs, and if the caller provides a
**   pointer to a structure to receive this information, it will be
**   returned along with the event log entry. If the caller provides a
**   pointer for an RPT entry, but the event log does not include a logged
**   RPT entry for the event being returned, RptEntry->ResourceCapabilities
**   will be set to zero. No valid RPTEntry will have a zero value here. If
**   the caller provides a pointer for an RDR, but the event log does not
**   include a logged RDR for the event being returned, Rdr->RdrType will
**   be set to SAHPI_NO_RECORD.
**
*******************************************************************************/
SaErrorT saHpiEventLogEntryGet (SaHpiSessionIdT     SessionId,
                                SaHpiResourceIdT    ResourceId,
                                SaHpiSelEntryIdT    EntryId,
                                SaHpiSelEntryIdT    *PrevEntryId,
                                SaHpiSelEntryIdT    *NextEntryId,
                                SaHpiSelEntryT      *EventLogEntry,
                                SaHpiRdrT           *Rdr,
                                SaHpiRptEntryT      *RptEntry)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogEntryGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogEntryAdd
**
** Description:
**   This function enables system management software to add entries to the
**   system event log.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the Domain System Event Log.
**   EvtEntry - [in] Pointer to event log entry data to write to the system
**      event log.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   This function forces a write of the event to the addressed event log.
**   Nothing else is done with the event. Specific implementations of HPI
**   may have restrictions on how much data may be passed to the
**   saHpiEventLogEntryAdd() function. These restrictions should be
**   documented by the provider of the HPI interface. If more event log
**   data is provided than can be written, an error will be returned.
**
*******************************************************************************/
SaErrorT saHpiEventLogEntryAdd (SaHpiSessionIdT      SessionId,
                                SaHpiResourceIdT     ResourceId,
                                SaHpiSelEntryT       *EvtEntry)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogEntryAdd\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogEntryDelete
**
** Description:
**   This function deletes an event log entry. This operation is only valid
**   if so indicated by saHpiEventLogInfoGet(), via the
**   DeleteEntrySupported field in the SaHpiSelInfoT structure.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in]  ResourceID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log.
**   EntryId - [in] Entry ID on the event log entry to delete. Reserved
**      event log entry ID values:  SAHPI_OLDEST_ENTRY - Oldest entry in the
**      log.  SAHPI_NEWEST_ENTRY - Newest entry in the log.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_CMD is returned if this log does not
**   support this operation.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiEventLogEntryDelete (SaHpiSessionIdT      SessionId,
                                   SaHpiResourceIdT     ResourceId,
                                   SaHpiSelEntryIdT     EntryId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogEntryDelete\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogClear
**
** Description:
**   This function erases the contents of the specified system event log.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in]  ResourceID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Note that all event logs must support the "clear" operation,
**   regardless of the setting of the DeleteEntrySupported field in the
**   SaHpiSelInfoT structure returned by saHpiEventLogInfoGet().
**
*******************************************************************************/
SaErrorT saHpiEventLogClear (SaHpiSessionIdT   SessionId,
                             SaHpiResourceIdT  ResourceId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogClear\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogTimeGet
**
** Description:
**   This function retrieves the current time from the event log's own time
**   clock. The value of this clock is used to timestamp log entries
**   written into the log.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] ResourceID of the resource that contains the System
**      Event Log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the Domain System Event Log.
**   Time - [out] Pointer to the returned SEL current time. If the
**      implementation cannot supply an absolute time value, then it may
**      supply a time relative to some system-defined epoch, such as system
**      boot. If the time value is less than or equal to
**      SAHPI_TIME_MAX_RELATIVE, but not SAHPI_TIME_UNSPECIFIED, then it is
**      relative; if it is greater than SAHPI_TIME_MAX_RELATIVE, then it is
**      absolute. The value SAHPI_TIME_UNSPECIFIED indicates that the time is
**      not set, or cannot be determined.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiEventLogTimeGet (SaHpiSessionIdT  SessionId,
                               SaHpiResourceIdT ResourceId,
                               SaHpiTimeT       *Time)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogTimeGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogTimeSet
**
** Description:
**   This function sets the event log's time clock, which is used to
**   timestamp events written into the log.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log.
**   Time - [in] time to set the SEL clock to. If the implementation cannot
**      supply an absolute time, then it may supply a time relative to some
**      system-defined epoch, such as system boot. If the timestamp value is
**      less than or equal to SAHPI_TIME_MAX_RELATIVE, but not
**      SAHPI_TIME_UNSPECIFIED, then it is relative; if it is greater than
**      SAHPI_TIME_MAX_RELATIVE, then it is absolute. The value
**      SAHPI_TIME_UNSPECIFIED indicates that the time of the event cannot be
**      determined.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiEventLogTimeSet (SaHpiSessionIdT   SessionId,
                               SaHpiResourceIdT  ResourceId,
                               SaHpiTimeT        Time)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogTimeSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogStateGet
**
** Description:
**   This function enables system management software to get the event log
**   state. If the event log is "disabled" no events generated within the
**   HPI implementation will be added to the event log. Events may still be
**   added to the event log with the saHpiEventLogEntryAdd() function. When
**   the event log is "enabled" events may be automatically added to the
**   event log as they are generated in a resource or a domain, however, it
**   is implementation-specific which events are automatically added to any
**   event log.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] ResourceID of the resource that contains the System
**      Event Log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the Domain System Event Log.
**   Enable - [out] Pointer to the current SEL state. True indicates that
**      the SEL is enabled; false indicates that it is disabled.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiEventLogStateGet (SaHpiSessionIdT  SessionId,
                                SaHpiResourceIdT ResourceId,
                                SaHpiBoolT       *Enable)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogStateGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventLogStateSet
**
** Description:
**   This function enables system management software to set the event log
**   enabled state. If the event log is "disabled" no events generated
**   within the HPI implementation will be added to the event log. Events
**   may still be added to the event log using the saHpiEventLogEntryAdd()
**   function. When the event log is "enabled" events may be automatically
**   added to the event log as they are generated in a resource or a
**   domain. The actual set of events that are automatically added to any
**   event log is implementation-specific. Typically, the HPI
**   implementation will provide an appropriate default value for this
**   parameter, which may vary by resource. This function is provided so
**   that management software can override the default, if desired. Note:
**   If a resource hosting an event log is re-initialized (e.g., because of
**   a hot-swap action), the HPI implementation may reset the value of this
**   parameter.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource that contains the system
**      event log to be managed. Set to SAHPI_DOMAIN_CONTROLLER_ID to address
**      the domain system event log.
**   Enable - [in] SEL state to be set. True indicates that the SEL is to
**      be enabled; false indicates that it is to be disabled.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiEventLogStateSet (SaHpiSessionIdT   SessionId,
                                SaHpiResourceIdT  ResourceId,
                                SaHpiBoolT        Enable)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventLogStateGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSubscribe
**
** Description:
**   This function allows the caller to subscribe for session events. This
**   single call provides subscription to all session events, regardless of
**   event type or event severity. Only one subscription is allowed per
**   session, and additional subscribers will receive an appropriate error
**   code. No event filtering will be done by the underlying management
**   service.
**
** Parameters:
**   SessionId - [in] Session for which event subscription will be opened.
**   ProvideActiveAlarms - [in] Indicates whether or not alarms which are
**      active at the time of subscription should be queued for future
**      retrieval via the saHpiEventGet() function.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_DUPLICATE is returned when a subscription is
**   already in place for this session.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiSubscribe (SaHpiSessionIdT  SessionId,
                         SaHpiBoolT       ProvideActiveAlarms)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSubscribe\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiUnsubscribe
**
** Description:
**   This function removes the event subscription for the session. After
**   removal of a subscription, additional saHpiEventGet() calls will not
**   be allowed unless the caller re-subscribes for events first. Any
**   events that are still in the event queue when this function is called
**   will be cleared from it.
**
** Parameters:
**   SessionId - [in] Session for which event subscription will be closed.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_REQUEST is returned if the caller is
**   not currently subscribed for events in this session.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiUnsubscribe (SaHpiSessionIdT SessionId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiUnsubscribe\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEventGet
**
** Description:
**   This function allows the caller to get an event. This call is only
**   valid within a session, which has subscribed for events. If the
**   ProvideActiveAlarms parameter was set in the subscription, the first
**   events retrieved will reflect the state of currently active alarms for
**   the resources belonging to the domain. After all active alarms are
**   retrieved this function will begin returning newly generated events as
**   the domain controller receives them. If there are one or more events
**   on the event queue when this function is called, it will immediately
**   return the next event on the queue. Otherwise, if the Timeout
**   parameter is SAHPI_TIMEOUT_IMMEDIATE, it will return
**   SA_ERR_HPI_TIMEOUT immediately. Otherwise, it will block for a time
**   specified by the timeout parameter; if an event is added to the queue
**   within that time, it will be returned immediately; if not,
**   saHpiEventGet() will return SA_ERR_HPI_TIMEOUT. If the Timeout
**   parameter is SAHPI_TIMEOUT_BLOCK, then saHpiEventGet() will block
**   indefinitely, until an event becomes available, and then return that
**   event. This provides for notification of events as they occur.
**
** Parameters:
**   SessionId - [in] Session for which events are retrieved.
**   Timeout - [in] The number of nanoseconds to wait for an event to
**      arrive. Reserved time out values: SAHPI_TIMEOUT_IMMEDIATE Time out
**      immediately if there are no events available (non-blocking call).
**      SAHPI_TIMEOUT_BLOCK Call should not return until an event is
**      retrieved.
**   Event - [out] Pointer to the next available event.
**   Rdr - [in/out] Pointer to structure to receive the resource data
**      associated with the event. If NULL, no RDR will be returned.
**   RptEntry - [in/out] Pointer to structure to receive the RPT entry
**      associated with the resource that generated the event. If NULL, no RPT
**      entry will be returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_TIMEOUT is returned if no event is available
**   to return within the timeout period. If SAHPI_TIMEOUT_IMMEDIATE is
**   passed in the Timeout parameter, this error return will be used if
**   there is no event queued when the function is called.
**
** Remarks:
**   If the caller provides a pointer for an RPT entry, but the event does
**   not include a valid resource ID for a resource in the domain (possible
**   on OEM or USER type event), then the ResourceCapabilities field in
**   *RptEntry will be set to zero. No valid RPT entry will have a zero
**   value here. If the caller provides a pointer for an RDR, but there is
**   no valid RDR associated with the event being returned (e.g., returned
**   event is not a sensor event), Rdr->RdrType will be set to
**   SAHPI_NO_RECORD. The timestamp reported in the returned event
**   structure is the best approximation an implementation has to when the
**   event actually occurred. The implementation may need to make an
**   approximation (such as the time the event was placed on the event
**   queue) because it may not have access to the actual time the event
**   occurred. The value SAHPI_TIME_UNSPECIFIED indicates that the time of
**   the event cannot be determined. If the implementation cannot supply an
**   absolute timestamp, then it may supply a timestamp relative to some
**   system-defined epoch, such as system boot. If the timestamp value is
**   less than or equal to SAHPI_TIME_MAX_RELATIVE, but not
**   SAHPI_TIME_UNSPECIFIED, then it is relative; if it is greater than
**   SAHPI_TIME_MAX_RELATIVE, then it is absolute.   6  Resource Functions
**
*******************************************************************************/
SaErrorT saHpiEventGet (SaHpiSessionIdT      SessionId,
                        SaHpiTimeoutT        Timeout,
                        SaHpiEventT          *Event,
                        SaHpiRdrT            *Rdr,
                        SaHpiRptEntryT       *RptEntry)
{
/*   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEventGet\n"); */
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiRdrGet
**
** Description:
**   This function returns a resource data record from the addressed
**   resource. Submitting an EntryId of SAHPI_FIRST_ENTRY results in the
**   first RDR being read. A returned NextEntryID of SAHPI_LAST_ENTRY
**   indicates the last RDR has been returned. A successful retrieval will
**   include the next valid EntryId. To retrieve the entire list of RDRs,
**   call this function first with an EntryId of SAHPI_FIRST_ENTRY and then
**   use the returned NextEntryId in the next call. Proceed until the
**   NextEntryId returned is SAHPI_LAST_ENTRY.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   EntryId - [in] Handle of the RDR to retrieve. Reserved entry ID
**      values: SAHPI_FIRST_ENTRY Get first entry SAHPI_LAST_ENTRY Reserved as
**      delimiter for end of list. Not a valid entry identifier.
**   NextEntryId - [out] Pointer to location to store Entry ID of next
**      entry in RDR repository.
**   Rdr - [out] Pointer to the structure to receive the requested resource
**      data record.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   A resource's RDR repository is static over the lifetime of the
**   resource; therefore no precautions are required against changes to the
**   content of the RDR repository while it is being accessed.
**
*******************************************************************************/
SaErrorT saHpiRdrGet (SaHpiSessionIdT       SessionId,
                      SaHpiResourceIdT      ResourceId,
                      SaHpiEntryIdT         EntryId,
                      SaHpiEntryIdT         *NextEntryId,
                      SaHpiRdrT             *Rdr)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiRdrGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorReadingGet
**
** Description:
**   This function is used to retrieve a sensor reading.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   SensorNum - [in] Sensor number for which the sensor reading is being
**      retrieved.
**   Reading - [out] Pointer to a structure to receive sensor reading
**      values.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiSensorReadingGet (SaHpiSessionIdT      SessionId,
                                SaHpiResourceIdT     ResourceId,
                                SaHpiSensorNumT      SensorNum,
                                SaHpiSensorReadingT  *Reading)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorReadingGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorReadingConvert
**
** Description:
**   This function converts between raw and interpreted sensor reading
**   values. The type of conversion done depends on the passed-in
**   ReadingInput parameter. If it contains only a raw value, then this is
**   converted to an interpreted value in ConvertedReading; if it contains
**   only an interpreted value, then this is converted to a raw value in
**   ConvertedReading. If it contains neither type of value, or both, then
**   an error is returned. The ReadingInput parameter is not altered in any
**   case. If the sensor does not use raw values - i.e., it directly
**   returns interpreted values - then this routine returns an error.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   SensorNum - [in] Sensor number for which reading is associated.
**   ReadingInput - [in] Pointer to the structure that contains raw or
**      interpreted reading to be converted.
**   ConvertedReading - [out] Pointer to structure to hold converted
**      reading.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_HPI_INVALID_PARAMS is returned if the ReadingInput
**   parameter is invalid; e.g. if it contains neither a raw nor an
**   interpreted value; or if it contains both; or if it contains an
**   invalid value. SA_ERR_HPI_INVALID_DATA is returned if the sensor does
**   not support raw readings. SA_ERR_HPI_NOT_PRESENT is returned if the
**   sensor is not present.
**
** Remarks:
**   The EventStatus field in ReadingInput is not used by this function. To
**   make conversions, sensor-specific data may be required. Thus, the
**   function references a particular sensor in the system through the
**   SessionID/ResourceID/SensorNum parameters. If this sensor is not
**   present, and sensor- specific information is required, the conversion
**   will fail and SA_ERR_HPI_NOT_PRESENT will be returned.
**
*******************************************************************************/
SaErrorT saHpiSensorReadingConvert (SaHpiSessionIdT      SessionId,
                                    SaHpiResourceIdT     ResourceId,
                                    SaHpiSensorNumT      SensorNum,
                                    SaHpiSensorReadingT  *ReadingInput,
                                    SaHpiSensorReadingT  *ConvertedReading)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorReadingConvert\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorThresholdsGet
**
** Description:
**   This function retrieves the thresholds for the given sensor.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   SensorNum - [in] Sensor number for which threshold values are being
**      retrieved.
**   SensorThresholds - [out] Pointer to returned sensor thresholds.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiSensorThresholdsGet (SaHpiSessionIdT        SessionId,
                                   SaHpiResourceIdT       ResourceId,
                                   SaHpiSensorNumT        SensorNum,
                                   SaHpiSensorThresholdsT *SensorThresholds)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorThresholdsGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorThresholdsSet
**
** Description:
**   This function sets the specified thresholds for the given sensor.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of addressed resource.
**   SensorNum - [in] Sensor number for which threshold values are being
**      set.
**   SensorThresholds - [in] Pointer to the sensor thresholds values being
**      set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   For each threshold or hysteresis value to be set, the corresponding
**   sensor reading structure must indicate whether a raw or interpreted
**   value is present. If neither are present, then that threshold or
**   hysteresis value will not be set. Each sensor may require settings to
**   be done with raw, or interpreted values, or may permit either; this is
**   defined by the field ThresholdDefn.TholdCapabilities in the sensor's
**   RDR (saHpiSensorRecT). If the interpreted value and raw value are both
**   provided, and both are legal for the sensor, the interpreted value
**   will be ignored and the raw value will be used.
**
*******************************************************************************/
SaErrorT saHpiSensorThresholdsSet (SaHpiSessionIdT        SessionId,
                                   SaHpiResourceIdT       ResourceId,
                                   SaHpiSensorNumT        SensorNum,
                                   SaHpiSensorThresholdsT *SensorThresholds)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorThresholdsSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorTypeGet
**
** Description:
**   This function retrieves the sensor type and event category for the
**   specified sensor.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   SensorNum - [in] Sensor number for which the type is being retrieved
**   Type - [out] Pointer to returned enumerated sensor type for the
**      specified sensor.
**   Category - [out] Pointer to location to receive the returned sensor
**      event category.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiSensorTypeGet (SaHpiSessionIdT     SessionId,
                             SaHpiResourceIdT    ResourceId,
                             SaHpiSensorNumT     SensorNum,
                             SaHpiSensorTypeT    *Type,
                             SaHpiEventCategoryT *Category)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorTypeGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorEventEnablesGet
**
** Description:
**   This function provides the ability to get the disable or enable event
**   message generation status for individual sensor events. The sensor
**   event states are relative to the event category specified by the
**   sensor. See the SaHpiEventCategoryT definition in section 7.3,
**   "Events, Part 1," on page 83 for more information. Within the
**   structure returned, there are two elements that contain bit flags; one
**   for assertion events and one for de-assertion events. A bit set to '1'
**   in the "AssertEvents" element in the structure indicates that an event
**   will be generated when the corresponding event state changes from
**   de-asserted to asserted on that sensor. A bit set to '1' in the
**   "DeassertEvents" element in the structure indicates that an event will
**   be generated when the corresponding event state changes from asserted
**   to de-asserted on that sensor. The saHpiSensorEventEnablesGet()
**   function also returns the general sensor status - whether the sensor
**   is completely disabled, or event generation is completely disabled.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   SensorNum - [in] Sensor number for which the event enable
**      configuration is being requested
**   Enables - [out] Pointer to the structure for returning sensor status
**      and event enable information.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Disabling events means that events are disabled for all sessions, not
**   just the session referenced by the SessionId parameter. For sensors
**   hosted by resources that have the "SAHPI_CAPABILITY_EVT_DEASSERTS"
**   flag set in its RPT entry, the "AssertEvents" element and the
**   "DeassertsEvents" element will always have same value.
**
*******************************************************************************/
SaErrorT saHpiSensorEventEnablesGet (SaHpiSessionIdT         SessionId,
                                     SaHpiResourceIdT        ResourceId,
                                     SaHpiSensorNumT         SensorNum,
                                     SaHpiSensorEvtEnablesT  *Enables)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorEventEnablesGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiSensorEventEnablesSet
**
** Description:
**   This function provides the ability to set the disable or enable event
**   message generation status for individual sensor events. The sensor
**   event states are relative to the event category specified by the
**   sensor. See the SaHpiEventCategoryT definition for more information.
**   Within the structure passed, there are two elements, which contain bit
**   flags; one for assertion events and one for de-assertion events.
**   However, the use of these two elements depends on whether the resource
**   addressed has the "SAHPI_CAPABILITY_EVT_DEASSERTS" flag set in its RPT
**   entry. This capability, if set, advertises that all sensors hosted by
**   the resource will always send a "de-assert" event when any state is
**   de-asserted whose assertion generates an "assert" event. Thus, for
**   sensors hosted by resources that advertise this behavior, it is not
**   meaningful to control assert events and de-assert events separately.
**   For sensors on resources that do not have the
**   "SAHPI_CAPABILITY_EVT_DEASSERTS" flag set, a bit set to '1' in the
**   "AssertEvents" element in the structure indicates that an event will
**   be generated when the corresponding event state changes from
**   de-asserted to asserted on that sensor., and a bit set to '1' in the
**   "DeassertEvents" element in the structure indicates that an event will
**   be generated when the corresponding event state changes from asserted
**   to de-asserted on that sensor. For sensors on resources, which do have
**   the "SAHPI_CAPABILITY_EVT_DEASSERTS" flag set, the "DeassertEvents"
**   element is not used. For sensors on these resources, a bit set to '1'
**   in the "AssertEvents" element in the structure indicates that an event
**   will be generated when the corresponding event state changes in either
**   direction (de-asserted to asserted or asserted to de-asserted). The
**   saHpiSensorEventEnablesSet() function also allows setting of general
**   sensor status - whether the sensor is completely disabled, or event
**   generation is completely disabled.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   SensorNum - [in] Sensor number for which the event enables are being
**      set.
**   Enables - [in] Pointer to the structure containing the enabled status
**      for each event.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Disabling events means that events are disabled for all sessions, not
**   just the session referenced by the SessionId parameter.
**   saHpiSensorEventEnablesGet () will return the values which were last
**   set by saHpiSensorEventEnablesSet() for the "AssertEvents" and
**   "DeassertEvents" elements in the passed data structures. However, for
**   sensors hosted by any resource that has the
**   SAHPI_CAPABILITY_EVT_DEASSERTS flag set in its RPT entry, the passed
**   "AssertEvents" element on the saHpiSensorEventEnablesSet () function
**   is used for both assertion and de-assertion event enable flags. In
**   this case, this value will be returned in both the "AssertEvents" and
**   "DeassertEvents" elements on a subsequent saHpiSensorEventEnablesGet
**   () call.
**
*******************************************************************************/
SaErrorT saHpiSensorEventEnablesSet (SaHpiSessionIdT        SessionId,
                                     SaHpiResourceIdT       ResourceId,
                                     SaHpiSensorNumT        SensorNum,
                                     SaHpiSensorEvtEnablesT *Enables)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiSensorEventEnablesSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiControlTypeGet
**
** Description:
**   This function retrieves the control type of a control object.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   CtrlNum - [in] Control number
**   Type - [out] Pointer to SaHpiCtrlTypeT variable to receive the
**      enumerated control type for the specified control.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   The Type parameter must point to a variable of type SaHpiCtrlTypeT.
**   Upon successful completion, the enumerated control type is returned in
**   the variable pointed to by Type.
**
*******************************************************************************/
SaErrorT saHpiControlTypeGet (SaHpiSessionIdT  SessionId,
                              SaHpiResourceIdT ResourceId,
                              SaHpiCtrlNumT    CtrlNum,
                              SaHpiCtrlTypeT   *Type)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiControlTypeGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiControlStateGet
**
** Description:
**   This function retrieves the current state (generally the last state
**   set) of a control object.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of addressed resource.
**   CtrlNum - [in] Number of the control for which the state is being
**      retrieved.
**   CtrlState - [in/out] Pointer to a control data structure into which
**      the current control state will be placed. For text controls, the line
**      number to read is passed in via CtrlState->StateUnion.Text.Line.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Note that Text controls are unique in that they have a state
**   associated with each line of the control - the state being the text on
**   that line. The line number to be read is passed in to
**   sahpiControlStateGet()via CtrlState- >StateUnion.Text.Line; the
**   contents of that line of the control will be returned in CtrlState-
**   >StateUnion.Text.Text. If the line number passed in is
**   SAHPI_TLN_ALL_LINES, then sahpiControlStateGet() will return the
**   entire text of the control, or as much of it as will fit in a single
**   SaHpiTextBufferT, in CtrlState- >StateUnion.Text.Text. This value will
**   consist of the text of all the lines concatenated, using the maximum
**   number of characters for each line (no trimming of trailing blanks).
**   Note that depending on the data type and language, the text may be
**   encoded in 2-byte Unicode, which requires two bytes of data per
**   character. Note that the number of lines and columns in a text control
**   can be obtained from the control's Resource Data Record.
**
*******************************************************************************/
SaErrorT saHpiControlStateGet (SaHpiSessionIdT  SessionId,
                               SaHpiResourceIdT ResourceId,
                               SaHpiCtrlNumT    CtrlNum,
                               SaHpiCtrlStateT  *CtrlState)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiControlStateGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiControlStateSet
**
** Description:
**   This function is used for setting the state of the specified control
**   object.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   CtrlNum - [in] Number of the control for which the state is being set.
**
**   CtrlState - [in] Pointer to a control state data structure holding the
**      state to be set
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   The CtrlState parameter must be of the correct type for the specified
**   control. Text controls include a line number and a line of text in the
**   CtrlState parameter, allowing update of just a single line of a text
**   control. If less than a full line of data is written, the control will
**   clear all spaces beyond those written on the line. Thus writing a
**   zero-length string will clear the addressed line. It is also possible
**   to include more characters in the text passed in the CtrlState
**   structure than will fit on one line; in this case, the control will
**   "wrap" to the next line (still clearing the trailing characters on the
**   last line written). Thus, there are two ways to write multiple lines
**   to a text control: (a) call saHpiControlStateSet() repeatedly for each
**   line, or (b) call saHpiControlStateSet() once and send more characters
**   than will fit on one line. The caller should not assume any "cursor
**   positioning" characters are available to use, but rather should always
**   write full lines and allow "wrapping" to occur. When calling
**   saHpiControlStateSet() for a text control, the caller may set the line
**   number to SAHPI_TLN_ALL_LINES; in this case, the entire control will
**   be cleared, and the data will be written starting on line 0. (This is
**   different from simply writing at line 0, which only alters the lines
**   written to.) This feature may be used to clear the entire control,
**   which can be accomplished by setting: CtrlState->StateUnion.Text.Line
**   = SAHPI_TLN_ALL_LINES; CtrlState->StateUnion.Text.Text.DataLength = 0;
**   Note that the number of lines and columns in a text control can be
**   obtained from the control's Resource Data Record.
**
*******************************************************************************/
SaErrorT saHpiControlStateSet (SaHpiSessionIdT  SessionId,
                               SaHpiResourceIdT ResourceId,
                               SaHpiCtrlNumT    CtrlNum,
                               SaHpiCtrlStateT  *CtrlState)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiControlStateSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEntityInventoryDataRead
**
** Description:
**   This function returns inventory data for a particular entity
**   associated with a resource.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   EirId - [in] Identifier for the entity inventory repository.
**   BufferSize - [in] Size of the InventData buffer passed in.
**   InventData - [out] Pointer to the buffer for the returned data.
**   ActualSize - [out] Pointer to size of the actual amount of data
**      returned.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned. SA_ERR_INVENT_DATA_TRUNCATED is returned if the buffer
**   passed in the InventData structure is not large enough (as indicated
**   by the "BufferSize" parameter) to hold the entire InventData
**   structure.
**
** Remarks:
**   Before calling saHpiEntityInventoryDataRead() the caller should
**   allocate a sufficiently large buffer to hold the data, and pass the
**   size of the buffer in the "BufferSize" parameter. The
**   saHpiEntityInventoryDataRead() function will return, at the location
**   pointed to by the ActualSize parameter, the actual space used in the
**   buffer to hold the returned data. If the data will not fit in the
**   buffer, as much as will fit will be returned, *ActualSize will be set
**   to indicated a suggested buffer size for the entire inventory data,
**   the "Validity" field in the InventData buffer will be set to
**   "SAHPI_INVENT_DATA_OVERFLOW," and an error return will be made. Since
**   it is impossible to know how large the inventory data may be without
**   actually reading and processing it from the entity inventory
**   repository, it may be advisable to err on the large side in allocating
**   the buffer. Note that the data includes many pointers to
**   SaHpiTextBufferT structures. The implementation of
**   saHpiEntityInventoryDataRead() may not reserve space for the maximum
**   size of each of these structures when formatting the data in the
**   returned buffer. Thus, if a user wishes to lengthen the data in one of
**   these structures, a new SaHpiTextBufferT structure should be
**   allocated, and the appropriate pointer reset to point to this new
**   structure in memory. See the description of the SaHpiInventoryDataT
**   structure in section 7.9, "Entity Inventory Data," on page 94, for
**   details on the format of the returned data.
**
*******************************************************************************/
SaErrorT saHpiEntityInventoryDataRead (SaHpiSessionIdT         SessionId,
                                       SaHpiResourceIdT        ResourceId,
                                       SaHpiEirIdT             EirId,
                                       SaHpiUint32T            BufferSize,
                                       SaHpiInventoryDataT     *InventData,
                                       SaHpiUint32T            *ActualSize)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEntityInventoryDataRead\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiEntityInventoryDataWrite
**
** Description:
**   This function writes the specified data to the inventory information
**   area. Note: If the resource hosting the inventory data is
**   re-initialized, or if the entity itself is removed and reinserted, the
**   inventory data may be reset to its default settings, losing data
**   written to the repository with this function.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   EirId - [in] Identifier for the entity inventory repository.
**   InventData - [in] Pointer to data to write to the repository.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   The SaHpiInventoryDataT structure consists of a Validity field and
**   then a set of pointers to record structures. It is not required that
**   all pointers point to data within a single contiguous buffer. The
**   "Validity" field in the SaHpiInventoryDataT structure must be set to
**   "SAHPI_INVENT_DATA_VALID," or else the saHpiEntityInventoryDataWrite()
**   function will take no action and return an error. This is to help
**   prevent invalid data returned by a saHpiEntityInventoryDataRead()
**   function from being inadvertently written to the resource. For this
**   protection to work, the caller should not change the value of the
**   "Validity" field in the SaHpiInventoryDataT structure unless building
**   an entire Inventory Data set from scratch. Some implementations may
**   impose limitations on the languages of the strings passed in within
**   the InventData parameter.  Implementation-specific documentation
**   should identify these restrictions.
**
*******************************************************************************/
SaErrorT saHpiEntityInventoryDataWrite (SaHpiSessionIdT          SessionId,
                                        SaHpiResourceIdT         ResourceId,
                                        SaHpiEirIdT              EirId,
                                        SaHpiInventoryDataT      *InventData)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiEntityInventoryDataWrite\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiWatchdogTimerGet
**
** Description:
**   This function retrieves the current watchdog timer settings and
**   configuration.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource, which contains the
**      watchdog timer being addressed.
**   WatchdogNum - [in] The watchdog number that specifies the watchdog
**      timer on a resource.
**   Watchdog - [out] Pointer to watchdog data structure.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   See the description of the SaHpiWatchdogT structure in 7.11,
**   "Watchdogs" on page 96 for details on what information is returned by
**   this function.
**
*******************************************************************************/
SaErrorT saHpiWatchdogTimerGet (SaHpiSessionIdT    SessionId,
                                SaHpiResourceIdT   ResourceId,
                                SaHpiWatchdogNumT  WatchdogNum,
                                SaHpiWatchdogT     *Watchdog)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiWatchdogTimerGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiWatchdogTimerSet
**
** Description:
**   This function provides a method for initializing the watchdog timer
**   configuration. Once the appropriate configuration has be set using
**   saHpiWatchdogTimerSet(), the user must then call
**   saHpiWatchdogTimerReset() to initially start the watchdog timer.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the resource that contains the
**      watchdog timer being addressed.
**   WatchdogNum - [in] The watchdog number specifying the specific
**      watchdog timer on a resource.
**   Watchdog - [in] Pointer to watchdog data structure.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   If the initial counter value in the SaHpiWatchdogT structure is set to
**   0, the Watchdog will immediately time out and take the pre-timeout and
**   timeout actions, as well as log an event. This provides a mechanism
**   for software to force an immediate recovery action should that be
**   dependent on a Watchdog timeout occurring. See the description of the
**   SaHpiWatchdogT structure for more details on the effects of this
**   command related to specific data passed in that structure.
**
*******************************************************************************/
SaErrorT saHpiWatchdogTimerSet (SaHpiSessionIdT    SessionId,
                                SaHpiResourceIdT   ResourceId,
                                SaHpiWatchdogNumT  WatchdogNum,
                                SaHpiWatchdogT     *Watchdog)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiWatchdogTimerGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiWatchdogTimerReset
**
** Description:
**   This function provides a method to start or restart the watchdog timer
**   from the initial countdown value.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID for the resource that contains the
**      watchdog timer being addressed.
**   WatchdogNum - [in] The watchdog number specifying the specific
**      watchdog timer on a resource.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   If the Watchdog has been configured to issue a Pre-Timeout interrupt,
**   and that interrupt has already occurred, the saHpiWatchdogTimerReset()
**   function will not reset  the watchdog counter. The only way to stop a
**   Watchdog from timing out once a Pre-Timeout interrupt has occurred is
**   to use the saHpiWatchdogTimerSet() function to reset and/or stop the
**   timer.
**
*******************************************************************************/
SaErrorT saHpiWatchdogTimerReset (SaHpiSessionIdT   SessionId,
                                  SaHpiResourceIdT  ResourceId,
                                  SaHpiWatchdogNumT WatchdogNum)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiWatchdogTimerReset\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiHotSwapControlRequest
**
** Description:
**   A resource supporting hot swap typically supports default policies for
**   insertion and extraction. On insertion, the default policy may be for
**   the resource to turn the associated FRU's local power on and to
**   de-assert reset. On extraction, the default policy may be for the
**   resource to immediately power off the FRU and turn on a hot swap
**   indicator. This function allows a caller, after receiving a hot swap
**   event with HotSwapState equal to SAHPI_HS_STATE_INSERTION_PENDING or
**   SAHPI_HS_STATE_EXTRACTION_PENDING, to request control of the hot swap
**   policy and prevent the default policy from being invoked. Because a
**   resource that supports the simplified hot swap model will never
**   transition into Insertion Pending or Extraction Pending states, this
**   function is not applicable to those resources.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiHotSwapControlRequest (SaHpiSessionIdT  SessionId,
                                     SaHpiResourceIdT ResourceId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiHotSwapControlRequest\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceActiveSet
**
** Description:
**   During insertion, a resource supporting hot swap will generate an
**   event to indicate that it is in the INSERTION PENDING state. If the
**   management middleware or other user software calls
**   saHpiHotSwapControlRequest() before the resource begins an auto-insert
**   operation, then the resource will remain in INSERTION PENDING state
**   while the user acts on the resource to integrate it into the system.
**   During this state, the user can instruct the resource to power on the
**   associated FRU, to de-assert reset, or to turn off its hot swap
**   indicator using the saHpiResourcePowerStateSet(),
**   saHpiResourceResetStateSet(), or saHpiHotSwapIndicatorStateSet()
**   functions, respectively. Once the user has completed with the
**   integration of the FRU, this function must be called to signal that
**   the resource should now transition into ACTIVE/HEALTHY or
**   ACTIVE/UNHEALTHY state (depending on whether or not there are active
**   faults). The user may also use this function to request a resource to
**   return to the ACTIVE/HEALTHY or ACTIVE/UNHEALTHY state from the
**   EXTRACTION PENDING state in order to reject an extraction request.
**   Because a resource that supports the simplified hot swap model will
**   never transition into Insertion Pending or Extraction Pending states,
**   this function is not applicable to those resources.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Only valid if resource is in INSERTION PENDING or EXTRACTION PENDING
**   state and an auto-insert or auto-extract policy action has not been
**   initiated.
**
*******************************************************************************/
SaErrorT saHpiResourceActiveSet (SaHpiSessionIdT  SessionId,
                                 SaHpiResourceIdT ResourceId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceActiveSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceInactiveSet
**
** Description:
**   During extraction, a resource supporting hot swap will generate an
**   event to indicate that it is in the EXTRACTION PENDING state. If the
**   management middleware or other user software calls
**   saHpiHotSwapControlRequest() before the resource begins an
**   auto-extract operation, then the resource will remain in EXTRACTION
**   PENDING state while the user acts on the resource to isolate the
**   associated FRU from the system. During this state, the user can
**   instruct the resource to power off the FRU, to assert reset, or to
**   turn on its hot swap indicator using the saHpiResourcePowerStateSet(),
**   saHpiResourceResetStateSet(), or saHpiHotSwapIndicatorStateSet()
**   functions, respectively. Once the user has completed the shutdown of
**   the FRU, this function must be called to signal that the resource
**   should now transition into INACTIVE state. The user may also use this
**   function to request a resource to return to the INACTIVE state from
**   the INSERTION PENDING state to abort a hot-swap insertion action.
**   Because a resource that supports the simplified hot swap model will
**   never transition into Insertion Pending or Extraction Pending states,
**   this function is not applicable to those resources.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Only valid if resource is in EXTRACTION PENDING or INSERTION PENDING
**   state and an auto-extract or auto-insert policy action has not been
**   initiated.
**
*******************************************************************************/
SaErrorT saHpiResourceInactiveSet (SaHpiSessionIdT  SessionId,
                                   SaHpiResourceIdT ResourceId)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceInactiveSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAutoInsertTimeoutGet
**
** Description:
**   This function allows the caller to request the auto-insert timeout
**   value. This value indicates how long the HPI implementation will wait
**   before the default auto-insertion policy is invoked. Further
**   information on the auto-insert timeout can be found in the function
**   saHpiAutoInsertTimeoutSet().
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   Timeout - [out] Pointer to location to store the number of nanoseconds
**      to wait before autonomous handling of the hotswap event. Reserved time
**      out values:  SAHPI_TIMEOUT_IMMEDIATE indicates autonomous handling is
**      immediate.  SAHPI_TIMEOUT_BLOCK indicates autonomous handling does not
**      occur.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiAutoInsertTimeoutGet(SaHpiSessionIdT SessionId,
                                   SaHpiTimeoutT    *Timeout)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAutoInsertTimeoutGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAutoInsertTimeoutSet
**
** Description:
**   This function allows the caller to configure a timeout for how long to
**   wait before the default auto-insertion policy is invoked. This
**   function accepts a parameter instructing the implementation to impose
**   a delay before a resource will perform its default hot swap policy for
**   auto-insertion. The parameter may be set to SAHPI_TIMEOUT_IMMEDIATE to
**   direct resources to proceed immediately to auto-insertion, or to
**   SAHPI_TIMEOUT_BLOCK to prevent auto-insertion from ever occurring. If
**   the parameter is set to another value, then it defines the number of
**   nanoseconds between the time a hot swap event with HotSwapState =
**   SAHPI_HS_STATE_INSERTION_PENDING is generated, and the time that the
**   auto-insertion policy will be invoked for that resource. If, during
**   this time period, a saHpiHotSwapControlRequest() function is
**   processed, the timer will be stopped, and the auto-insertion policy
**   will not be invoked. Once the auto-insertion process begins, the user
**   software will not be allowed to take control of the insertion process;
**   hence, the timeout should be set appropriately to allow for this
**   condition. Note that the timeout period begins when the hot swap event
**   with HotSwapState = SAHPI_HS_STATE_INSERTION_PENDING is initially
**   generated; not when it is received by a caller with a saHpiEventGet()
**   function call, or even when it is placed in a session event queue.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   Timeout - [in] The number of nanoseconds to wait before autonomous
**      handling of the hotswap event. Reserved time out values:
**      SAHPI_TIMEOUT_IMMEDIATE indicates proceed immediately to autonomous
**      handling.  SAHPI_TIMEOUT_BLOCK indicates prevent autonomous handling.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiAutoInsertTimeoutSet(SaHpiSessionIdT  SessionId,
                                   SaHpiTimeoutT    Timeout)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAutoInsertTimeoutSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAutoExtractTimeoutGet
**
** Description:
**   This function allows the caller to request the timeout for how long
**   the implementation will wait before the default auto-extraction policy
**   is invoked. Further information on auto-extract time outs is detailed
**   in saHpiAutoExtractTimeoutSet().
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   Timeout - [out] Pointer to location to store the number of nanoseconds
**      to wait before autonomous handling of the hotswap event. Reserved time
**      out values:  SAHPI_TIMEOUT_IMMEDIATE indicates autonomous handling is
**      immediate.  SAHPI_TIMEOUT_BLOCK indicates autonomous handling does not
**      occur.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiAutoExtractTimeoutGet(SaHpiSessionIdT   SessionId,
                                    SaHpiResourceIdT  ResourceId,
                                    SaHpiTimeoutT     *Timeout)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAutoExtractTimeoutGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAutoExtractTimeoutSet
**
** Description:
**   This function allows the caller to configure a timeout for how long to
**   wait before the default auto-extraction policy is invoked. This
**   function accepts a parameter instructing the implementation to impose
**   a delay before a resource will perform its default hot swap policy for
**   auto-extraction. The parameter may be set to SAHPI_TIMEOUT_IMMEDIATE
**   to direct the resource to proceed immediately to auto-extraction, or
**   to SAHPI_TIMEOUT_BLOCK to prevent auto-extraction from ever occurring
**   on a resource. If the parameter is set to another value, then it
**   defines the number of nanoseconds between the time a hot swap event
**   with HotSwapState = SAHPI_HS_STATE_EXTRACTION_PENDING is generated,
**   and the time that the auto- extraction policy will be invoked for the
**   resource. If, during this time period, a saHpiHotSwapControlRequest()
**   function is processed, the timer will be stopped, and the
**   auto-extraction policy will not be invoked. Once the auto-extraction
**   process begins, the user software will not be allowed to take control
**   of the extraction process; hence, the timeout should be set
**   appropriately to allow for this condition. Note that the timeout
**   period begins when the hot swap event with HotSwapState =
**   SAHPI_HS_STATE_EXTRACTION_PENDING is initially generated; not when it
**   is received by a caller with a saHpiEventGet() function call, or even
**   when it is placed in a session event queue. The auto-extraction policy
**   is set at the resource level and is only supported by resources
**   supporting the "Managed Hot Swap" capability. After discovering that a
**   newly inserted resource supports "Managed Hot Swap," middleware or
**   other user software may use this function to change the default
**   auto-extraction policy for that resource. If a resource supports the
**   simplified hot-swap model, setting this timer has no effect since the
**   resource will transition directly to "Not Present" state on an
**   extraction.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   Timeout - [in] The number of nanoseconds to wait before autonomous
**      handling of the hotswap event. Reserved time out values:
**      SAHPI_TIMEOUT_IMMEDIATE indicates proceed immediately to autonomous
**      handling.  SAHPI_TIMEOUT_BLOCK indicates prevent autonomous handling.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiAutoExtractTimeoutSet(SaHpiSessionIdT   SessionId,
                                    SaHpiResourceIdT  ResourceId,
                                    SaHpiTimeoutT     Timeout)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAutoExtractTimeoutSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiHotSwapStateGet
**
** Description:
**   This function allows the caller to retrieve the current hot swap state
**   of a resource. The returned state will be one of the following five
**   states: ? SAHPI_HS_STATE_INSERTION_PENDING ?
**   SAHPI_HS_STATE_ACTIVE_HEALTHY ? SAHPI_HS_STATE_ACTIVE_UNHEALTHY ?
**   SAHPI_HS_STATE_EXTRACTION_PENDING ? SAHPI_HS_STATE_INACTIVE The state
**   SAHPI_HS_STATE_NOT_PRESENT will never be returned, because a resource
**   that is not present cannot be addressed by this function in the first
**   place.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   State - [out] Pointer to location to store returned state information.
**
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   None.
**
*******************************************************************************/
SaErrorT saHpiHotSwapStateGet (SaHpiSessionIdT  SessionId,
                               SaHpiResourceIdT ResourceId,
                               SaHpiHsStateT    *State)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiHotSwapStateGet\n");
   return SA_OK;
}



/*******************************************************************************
**
** Name: saHpiHotSwapActionRequest
**
** Description:
**   A resource supporting hot swap typically requires a physical action on
**   the associated FRU to invoke an insertion or extraction process. An
**   insertion process is invoked by physically inserting the FRU into a
**   chassis. Physically opening an ejector latch or pressing a button
**   invokes the extraction process. This function allows the caller to
**   invoke an insertion or extraction process via software.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   Action - [in] Requested action: SAHPI_HS_ACTION_INSERTION or
**      SAHPI_HS_ACTION_EXTRACTION
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   There may be limitations on when saHpiHotSwapActionRequest() may be
**   called, and what value may be used for the "Action" parameter
**   depending on what state the resource is currently in. At the least,
**   this function may be called: ? To request an Insertion action when the
**   resource is in INACTIVE state ?    To request an Extraction action when
**   the resource is in the ACTIVE/HEALTHY or ACTIVE/ UNHEALTHY state.
**
*******************************************************************************/
SaErrorT saHpiHotSwapActionRequest (SaHpiSessionIdT  SessionId,
                                    SaHpiResourceIdT ResourceId,
                                    SaHpiHsActionT   Action)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiHotSwapActionRequest\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourcePowerStateGet
**
** Description:
**   A typical resource supporting hot swap will have the ability to
**   control local power on the FRU associated with the resource. During
**   insertion, the FRU can be instructed to power on. During extraction
**   the FRU can be requested to power off. This function allows the caller
**   to retrieve the current power state of the FRU associated with the
**   specified resource.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   State - [out] The current power state of the resource.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   This function returns the actual low-level power state of the FRU,
**   regardless of what hot-swap state the resource is in. Not all
**   resources supporting managed hot swap will necessarily support this
**   function. In particular, resources that use the simplified hot swap
**   model may not have the ability to control FRU power. An appropriate
**   error code will be returned if the resource does not support power
**   control on the FRU.
**
*******************************************************************************/
SaErrorT saHpiResourcePowerStateGet (SaHpiSessionIdT     SessionId,
                                     SaHpiResourceIdT    ResourceId,
                                     SaHpiHsPowerStateT  *State)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourcePowerStateGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourcePowerStateSet
**
** Description:
**   A typical resource supporting hot swap will have to ability to control
**   local power on the FRU associated with the resource. During insertion,
**   the FRU can be instructed to power on. During extraction the FRU can
**   be requested to power off. This function allows the caller to set the
**   current power state of the FRU associated with the specified resource.
**
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   State - [in] the new power state that the specified resource will be
**      set to.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   This function controls the hardware power on the FRU of what hot-swap
**   state the resource is in. For example, it is legal (and may be
**   desirable) to cycle power on the FRU even while it is in ACTIVE state
**   in order to attempt to clear a fault condition. Similarly, a resource
**   could be instructed to power on a FRU even while it is in INACTIVE
**   state, for example, in order to run off-line diagnostics. Not all
**   resources supporting managed hot swap will necessarily support this
**   function. In particular, resources that use the simplified hot swap
**   model may not have the ability to control FRU power. An appropriate
**   error code will be returned if the resource does not support power
**   control on the FRU.
**
*******************************************************************************/
SaErrorT saHpiResourcePowerStateSet (SaHpiSessionIdT     SessionId,
                                     SaHpiResourceIdT    ResourceId,
                                     SaHpiHsPowerStateT  State)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourcePowerStateSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiHotSwapIndicatorStateGet
**
** Description:
**   A FRU associated with a hot-swappable resource may include a hot swap
**   indicator such as a blue LED. This indicator signifies that the FRU is
**   ready for removal.. This function allows the caller to retrieve the
**   state of this indicator. The returned state is either
**   SAHPI_HS_INDICATOR_OFF or SAHPI_HS_INDICATOR_ON. This function will
**   return the state of the indicator, regardless of what hot swap state
**   the resource is in.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   State - [out] Pointer to location to store state of hot swap
**      indicator.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Not all resources supporting managed hot swap will necessarily support
**   this function. In particular, resources that use the simplified hot
**   swap model may not have the ability to control a FRU hot swap
**   indicator (it is likely that none exists). An appropriate error code
**   will be returned if the resource does not support control of a hot
**   swap indicator on the FRU.
**
*******************************************************************************/
SaErrorT saHpiHotSwapIndicatorStateGet (SaHpiSessionIdT         SessionId,
                                        SaHpiResourceIdT        ResourceId,
                                        SaHpiHsIndicatorStateT  *State)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiHotSwapIndicatorStateGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiHotSwapIndicatorStateSet
**
** Description:
**   A FRU associated with a hot-swappable resource may include a hot swap
**   indicator such as a blue LED. This indicator signifies that the FRU is
**   ready for removal. This function allows the caller to set the state of
**   this indicator. Valid states include SAHPI_HS_INDICATOR_OFF or
**   SAHPI_HS_INDICATOR_ON. This function will set the indicator regardless
**   of what hot swap state the resource is in, though it is recommended
**   that this function be used only in conjunction with moving the
**   resource to the appropriate hot swap state.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource
**   State - [in] State of hot swap indicator to be set.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Not all resources supporting managed hot swap will necessarily support
**   this function. In particular, resources that use the simplified hot
**   swap model may not have the ability to control a FRU hot swap
**   indicator (it is likely that none exists). An appropriate error code
**   will be returned if the resource does not support control of a hot
**   swap indicator on the FRU.
**
*******************************************************************************/
SaErrorT saHpiHotSwapIndicatorStateSet (SaHpiSessionIdT         SessionId,
                                        SaHpiResourceIdT        ResourceId,
                                        SaHpiHsIndicatorStateT  State)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiHotSwapIndicatorStateSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiParmControl
**
** Description:
**   This function allows the user to save and restore parameters
**   associated with a specific resource. Valid actions for this function
**   include: SAHPI_DEFAULT_PARM Restores the factory default settings for
**   a specific resource. Factory defaults include sensor thresholds and
**   configurations, and resource- specific configuration parameters.
**   SAHPI_SAVE_PARM Stores the resource configuration parameters in
**   non-volatile storage. Resource configuration parameters stored in
**   non-volatile storage will survive power cycles and resource resets.
**   SAHPI_RESTORE_PARM Restores resource configuration parameters from
**   non-volatile storage. Resource configuration parameters include sensor
**   thresholds and sensor configurations, as well as resource-specific
**   parameters.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   Action - [in] Action to perform on resource parameters.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code
**   is returned.
**
** Remarks:
**   Resource-specific parameters should be documented in an implementation
**   guide for the HPI implementation.
**
*******************************************************************************/
SaErrorT saHpiParmControl (SaHpiSessionIdT  SessionId,
                           SaHpiResourceIdT ResourceId,
                           SaHpiParmActionT Action)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiParmControl\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceResetStateGet
**
** Description:
**   This function gets the reset state of an entity, allowing the user to
**   determine if the entity is being held with its reset asserted. If a
**   resource manages multiple entities, this function will address the
**   entity which is identified in the RPT entry for the resource.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   ResetAction - [out] The current reset state of the entity. Valid reset
**      states are:  SAHPI_RESET_ASSERT: The entity's reset is asserted, e.g.,
**      for hot swap insertion/extraction purposes  SAHPI_RESET_DEASSERT: The
**      entity's reset is not asserted
**
** Return Value:
**   SA_OK is returned if the resource has reset control, and the reset
**   state has successfully been determined; otherwise, an error code is
**   returned. SA_ERR_HPI_INVALID_CMD is returned if the resource has no
**   reset control.
**
** Remarks:
**   SAHPI_RESET_COLD and SAHPI_RESET_WARM are pulsed resets, and are not
**   valid return values for ResetAction. If the entity is not being held
**   in reset (using SAHPI_RESET_ASSERT), the appropriate return value is
**   SAHPI_RESET_DEASSERT.
**
*******************************************************************************/
SaErrorT saHpiResourceResetStateGet (SaHpiSessionIdT    SessionId,
                                     SaHpiResourceIdT   ResourceId,
                                     SaHpiResetActionT *ResetAction)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceResetStateGet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiResourceResetStateSet
**
** Description:
**   This function directs the resource to perform the specified reset type
**   on the entity that it manages. If a resource manages multiple
**   entities, this function addresses the entity that is identified in the
**   RPT entry for the resource. Entities may be reset for a variety of
**   reasons. A misbehaving entity may be reset to bring it to a known
**   state. In these cases, either a warm reset or a cold reset may be
**   performed. A warm reset preserves entity state, whereas a cold reset
**   does not. Both of these reset types are pulsed asserted and then
**   de-asserted by the HPI implementation. This allows the HPI
**   implementation to hold the reset asserted for the appropriate length
**   of time, as needed by each entity. saHpiResourceResetStateSet() can
**   also be used for insertion and extraction scenarios. A typical
**   resource supporting hot swap will have to ability to control local
**   reset within the FRU. During insertion, a resource can be instructed
**   to assert reset, while the FRU powers on. During extraction a resource
**   can be requested to assert reset before the FRU is powered off. This
**   function allows the caller to set the reset state of the specified
**   FRU. SAHPI_RESET_ASSERT is used to hold the resource in reset; the FRU
**   is brought out of the reset state by using either SAHPI_COLD_RESET or
**   SAHPI_WARM_RESET.
**
** Parameters:
**   SessionId - [in] Handle to session context.
**   ResourceId - [in] Resource ID of the addressed resource.
**   ResetAction - [in] Type of reset to perform on the entity. Valid reset
**      actions are:  SAHPI_COLD_RESET: Perform a 'Cold Reset' on the entity
**      (pulse), leaving reset de-asserted  SAHPI_WARM_RESET: Perform a 'Warm
**      Reset' on the entity (pulse), leaving reset de-asserted
**      SAHPI_RESET_ASSERT: Put the entity into reset state and hold reset
**      asserted, e.g., for hot swap insertion/extraction purposes
**
** Return Value:
**   SA_OK is returned if the resource has reset control, and the requested
**   reset action has succeeded; otherwise, an error code is returned.
**   SA_ERR_HPI_INVALID_CMD is returned if the resource has no reset
**   control, or if the requested reset action is not supported by the
**   resource.
**
** Remarks:
**   Some resources may not support reset, or may only support a subset of
**   the defined reset action types. Also, on some resources, cold and warm
**   resets may be equivalent.    7 Data Type Definitions
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiResourceResetStateSet (SaHpiSessionIdT  SessionId,
                                               SaHpiResourceIdT ResourceId,
                                               SaHpiResetActionT ResetAction)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiResourceResetStateSet\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAlarmGetNext
**
** Description:
**   This function allows retrieval of an alarm from the current set of alarms
**   held in the Domain Alarm Table (DAT).
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously
**      obtained using saHpiSessionOpen().
**   Severity - [in] Severity level of alarms to retrieve.
**      Set to SAHPI_ALL_SEVERITIES to retrieve alarms of any
**      severity; otherwise, set to requested severity level.
**   UnacknowledgedOnly - [in] Set to True to indicate only unacknowledged alarms
**      should be returned. Set to False to indicate either an acknowledged or
**      unacknowledged alarm may be returned.
**   Alarm - [in/out] Pointer to the structure to hold the returned alarm entry.
**      Also, on input, Alarm->AlarmId and Alarm->Timestamp are used to identify
**      the previous alarmi.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned when:
**       - Severity is not one of the valid enumerated values for this type.
**       - The Alarm parameter is passed in as NULL.
**   SA_ERR_HPI_NOT_PRESENT is returned:
**       - If there are no additional alarms in the DAT that meet the criteria
**         specified by the Severity and UnacknowledgedOnly parameters.
**       - If the passed Alarm->AlarmId field was set to SAHPI_FIRST_ENTRY and
**         there are no alarms in the DAT that meet the criteria specified by
**         the Severity and UnacknowledgedOnly parameters.
**   SA_ERR_HPI_INVALID_DATA is returned if the passed Alarm->AlarmId matches
**       an alarm in the DAT, but the passed Alarm->Timestamp does not match
**       the timestamp of that alarm.
**
** Remarks:
**
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmGetNext(SAHPI_IN SaHpiSessionIdT SessionId,
                                     SAHPI_IN SaHpiSeverityT Severity,
                                     SAHPI_IN SaHpiBoolT UnacknowledgedOnly,
                                     SAHPI_INOUT SaHpiAlarmT *Alarm)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAlarmGetNext\n");
   return SA_OK;
}

/*******************************************************************************
**
** Name: saHpiAlarmGet
**
** Description:
**   This function allows retrieval of a specific alarm in the Domain Alarm Table (DAT)
**   by referencing its AlarmId.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously
**      obtained using saHpiSessionOpen().
**   AlarmId - [in] AlarmId of the alarm to be retrieved from the DAT.
**      should be returned. Set to False to indicate either an acknowledged or
**      unacknowledged alarm may be returned.
**   Alarm - [out] Pointer to the structure to hold the returned alarm entry.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is returned.
**   SA_ERR_HPI_NOT_PRESENT is returned if the requested AlarmId does not correspond
**   to an alarm contained in the DAT.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Alarm parameter is passed in as NULL.
**
** Remarks:
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmGet(SAHPI_IN SaHpiSessionIdT SessionId,
                                 SAHPI_IN SaHpiAlarmIdT AlarmId,
                                 SAHPI_OUT SaHpiAlarmT *Alarm)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAlarmGet\n");
   return SA_OK;
}

/*******************************************************************************
**
** Name: saHpiAlarmAcknowledge
**
** Description:
**   This function allows an HPI User to acknowledge a single alarm entry or
**   a group of alarm entries by severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously
**      obtained using saHpiSessionOpen().
**   AlarmId - [in] Identifier of the alarm to be acknowledged. Reserved AlarmId values:
**              SAHPI_ENTRY_UNSPECIFIED Ignore this parameter.
**   Severity - [in] Severity level of alarms to acknowledge.
**              Ignored unless AlarmId is SAHPI_ENTRY_UNSPECIFIED.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is returned.
**   SA_ERR_HPI_NOT_PRESENT is returned if the requested AlarmId does not correspond
**   to an alarm contained in the DAT.
**   SA_ERR_HPI_INVALID_PARAMS is returned if AlarmId is SAHPI_ENTRY_UNSPECIFIED
**   and Severity is not one of the valid enumerated values for this type.
**
** Remarks:
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmAcknowledge(SAHPI_IN SaHpiSessionIdT SessionId,
                                         SAHPI_IN SaHpiAlarmIdT AlarmId,
                                         SAHPI_IN SaHpiSeverityT Severity)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAlarmAcknowledge\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAlarmAdd
**
** Description:
**   This function is used to add a User Alarm to the DAT.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously
**      obtained using saHpiSessionOpen().
**   Alarm - [in/out] Pointer to the alarm entry structure that contains
**   the new User Alarm to add to the DAT.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned when the Alarm pointer is passed in as NULL.
**   SA_ERR_HPI_INVALID_PARAMS is returned when Alarm->Severity is not one of the following
**   enumerated values: SAHPI_MINOR, SAHPI_MAJOR, or SAHPI_CRITICAL.
**   SA_ERR_HPI_INVALID_PARAMS is returned when Alarm->AlarmCond.
**   Type is not SAHPI_STATUS_COND_TYPE_USER.
**   SA_ERR_HPI_OUT_OF_SPACE is returned if the DAT is not able to add an additional
**   User Alarm due to space limits or limits imposed on the number of User Alarms
**   permitted in the DAT.
**
** Remarks:
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmAdd(SAHPI_IN SaHpiSessionIdT SessionId,
                                 SAHPI_INOUT SaHpiAlarmT *Alarm)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAlarmAdd\n");
   return SA_OK;
}


/*******************************************************************************
**
** Name: saHpiAlarmDelete
**
** Description:
**   This function allows an HPI User to delete a single User Alarm or a group
**   of User Alarms from the DAT. Alarms may be deleted individually by specifying
**   a specific AlarmId, or they may be deleted as a group by specifying a Severity.
**
** Parameters:
**   SessionId - [in] Identifier for a session context previously
**      obtained using saHpiSessionOpen().
**   AlarmId - [in] Alarm identifier of the alarm entry to delete. Reserved values:
**                     - SAHPI_ENTRY_UNSPECIFIED Ignore this parameter.
**   Severity - [in] Severity level of alarms to delete. Ignored unless AlarmId
**      is SAHPI_ENTRY_UNSPECIFIED.
**
** Return Value:
**   SA_OK is returned on successful completion; otherwise, an error code is returned.
**   SA_ERR_HPI_INVALID_PARAMS is returned if AlarmId is SAHPI_ENTRY_UNSPECIFIED
**   and Severity is not one of the valid enumerated values for this type.
**   SA_ERR_HPI_NOT_PRESENT is returned if an alarm entry identified by the AlarmId
**   parameter does not exist in the DAT.
**   SA_ERR_HPI_READ_ONLY is returned if the AlarmId parameter indicates a non-User Alarm.
**
** Remarks:
**
*******************************************************************************/
SaErrorT SAHPI_API saHpiAlarmDelete(SAHPI_IN SaHpiSessionIdT SessionId,
                                    SAHPI_IN SaHpiAlarmIdT AlarmId,
                                    SAHPI_IN SaHpiSeverityT Severity)
{
   m_LOG_HISV_DEBUG("Dummy HPI function saHpiAlarmDelete\n");
   return SA_OK;
}

