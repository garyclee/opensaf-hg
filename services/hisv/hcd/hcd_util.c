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
*  MODULE NAME:  hcd_util.c                                                  *
*                                                                            *
*                                                                            *
*  DESCRIPTION                                                               *
*  This module contains the functionality of Utility modules used by the     *
*  HPI Chassis Director. The function string_to_epath() is a utility         *
*  function used to convert the canonical string of entity path to the       *
*  SAF HPI defined entity path structure 'SaHpiEntityPathT'.                 *
*                                                                            *
*****************************************************************************/

#include "hcd.h"

/* local function declaration */
static SaHpiEntityTypeT get_entity_type(int8 *tok);
static void remove_spaces(uns8 **tok);

/* entity type list */
static ENTITY_TYPE_LIST  gl_etype_list[] = {

    /* Entity type string */                /* Entity type value */

    {"UNSPECIFIED",                SAHPI_ENT_UNSPECIFIED},
    {"OTHER",                      SAHPI_ENT_OTHER},
    {"UNKNOWN",                    SAHPI_ENT_UNKNOWN},
    {"PROCESSOR",                  SAHPI_ENT_PROCESSOR},
    {"DISK_BAY",                   SAHPI_ENT_DISK_BAY},
    {"PERIPHERAL_BAY",             SAHPI_ENT_PERIPHERAL_BAY},
    {"SYS_MGMNT_MODULE",           SAHPI_ENT_SYS_MGMNT_MODULE},
    {"SYSTEM_BOARD",               SAHPI_ENT_SYSTEM_BOARD},
    {"MEMORY_MODULE",              SAHPI_ENT_MEMORY_MODULE},
    {"PROCESSOR_MODULE",           SAHPI_ENT_PROCESSOR_MODULE},

    {"POWER_SUPPLY",               SAHPI_ENT_POWER_SUPPLY},
    {"ADD_IN_CARD",                SAHPI_ENT_ADD_IN_CARD},
    {"FRONT_PANEL_BOARD",          SAHPI_ENT_FRONT_PANEL_BOARD},
    {"BACK_PANEL_BOARD",           SAHPI_ENT_BACK_PANEL_BOARD},
    {"POWER_SYSTEM_BOARD",         SAHPI_ENT_POWER_SYSTEM_BOARD},
    {"DRIVE_BACKPLANE",            SAHPI_ENT_DRIVE_BACKPLANE},
    {"SYS_EXPANSION_BOARD",        SAHPI_ENT_SYS_EXPANSION_BOARD},
    {"OTHER_SYSTEM_BOARD",         SAHPI_ENT_OTHER_SYSTEM_BOARD},
    {"PROCESSOR_BOARD",            SAHPI_ENT_PROCESSOR_BOARD},
    {"POWER_UNIT",                 SAHPI_ENT_POWER_UNIT},

    {"POWER_MODULE",               SAHPI_ENT_POWER_MODULE},
    {"POWER_MGMNT",                SAHPI_ENT_POWER_MGMNT},
    {"CHASSIS_BACK_PANEL_BOARD",   SAHPI_ENT_CHASSIS_BACK_PANEL_BOARD},
    {"SYSTEM_CHASSIS",             SAHPI_ENT_SYSTEM_CHASSIS},
    {"SUB_CHASSIS",                SAHPI_ENT_SUB_CHASSIS},
    {"OTHER_CHASSIS_BOARD",        SAHPI_ENT_OTHER_CHASSIS_BOARD},
    {"DISK_DRIVE_BAY",             SAHPI_ENT_DISK_DRIVE_BAY},
    {"PERIPHERAL_BAY_2",           SAHPI_ENT_PERIPHERAL_BAY_2},
    {"DEVICE_BAY",                 SAHPI_ENT_DEVICE_BAY},
    {"COOLING_DEVICE",             SAHPI_ENT_COOLING_DEVICE},

    {"COOLING_UNIT",               SAHPI_ENT_COOLING_UNIT},
    {"INTERCONNECT",               SAHPI_ENT_INTERCONNECT},
    {"MEMORY_DEVICE",              SAHPI_ENT_MEMORY_DEVICE},
    {"SYS_MGMNT_SOFTWARE",         SAHPI_ENT_SYS_MGMNT_SOFTWARE},
    {"BIOS",                       SAHPI_ENT_BIOS},
    {"OPERATING_SYSTEM",           SAHPI_ENT_OPERATING_SYSTEM},
    {"SYSTEM_BUS",                 SAHPI_ENT_SYSTEM_BUS},
    {"GROUP",                      SAHPI_ENT_GROUP},
    {"REMOTE",                     SAHPI_ENT_REMOTE},
    {"EXTERNAL_ENVIRONMENT",       SAHPI_ENT_EXTERNAL_ENVIRONMENT},
    {"BATTERY",                    SAHPI_ENT_BATTERY},

#ifdef HPI_A
    {"CHASSIS_SPECIFIC",           SAHPI_ENT_CHASSIS_SPECIFIC},
    {"BOARD_SET_SPECIFIC",         SAHPI_ENT_BOARD_SET_SPECIFIC},
    {"OEM_SYSINT_SPECIFIC",        SAHPI_ENT_OEM_SYSINT_SPECIFIC},
    {"ROOT",                       SAHPI_ENT_ROOT},
    {"RACK",                       SAHPI_ENT_RACK},
    {"SUBRACK",                    SAHPI_ENT_SUBRACK},
    {"COMPACTPCI_CHASSIS",         SAHPI_ENT_COMPACTPCI_CHASSIS},
    {"ADVANCEDTCA_CHASSIS",        SAHPI_ENT_ADVANCEDTCA_CHASSIS},
    {"SYSTEM_SLOT",                SAHPI_ENT_SYSTEM_SLOT},

    {"SBC_BLADE",                  SAHPI_ENT_SBC_BLADE},
    {"IO_BLADE",                   SAHPI_ENT_IO_BLADE},
    {"DISK_BLADE",                 SAHPI_ENT_DISK_BLADE},
    {"DISK_DRIVE",                 SAHPI_ENT_DISK_DRIVE},
    {"FAN",                        SAHPI_ENT_FAN},
    {"POWER_DISTRIBUTION_UNIT",    SAHPI_ENT_POWER_DISTRIBUTION_UNIT},
    {"SPEC_PROC_BLADE",            SAHPI_ENT_SPEC_PROC_BLADE},
    {"IO_SUBBOARD",                SAHPI_ENT_IO_SUBBOARD},
    {"SBC_SUBBOARD",               SAHPI_ENT_SBC_SUBBOARD},
    {"ALARM_MANAGER",              SAHPI_ENT_ALARM_MANAGER},

    {"ALARM_MANAGER_BLADE",        SAHPI_ENT_ALARM_MANAGER_BLADE},
    {"SUBBOARD_CARRIER_BLADE",     SAHPI_ENT_SUBBOARD_CARRIER_BLADE}
#else
#ifdef HPI_B_02
    {"RESERVED_1",                 SAHPI_ENT_RESERVED_1},
    {"RESERVED_2",                 SAHPI_ENT_RESERVED_2},
    {"RESERVED_3",                 SAHPI_ENT_RESERVED_3},
    {"RESERVED_4",                 SAHPI_ENT_RESERVED_4},
    {"RESERVED_5",                 SAHPI_ENT_RESERVED_5},
    {"MC_FIRMWARE",                SAHPI_ENT_MC_FIRMWARE},
    {"IPMI_CHANNEL",               SAHPI_ENT_IPMI_CHANNEL},
    {"PCI_BUS",                    SAHPI_ENT_PCI_BUS},
    {"PCI_EXPRESS_BUS",            SAHPI_ENT_PCI_EXPRESS_BUS},

    {"SCSI_BUS",                   SAHPI_ENT_SCSI_BUS},
    {"SATA_BUS",                   SAHPI_ENT_SATA_BUS},
    {"PROC_FSB",                   SAHPI_ENT_PROC_FSB},
    {"CLOCK",                      SAHPI_ENT_CLOCK},
    {"SYSTEM_FIRMWARE",            SAHPI_ENT_SYSTEM_FIRMWARE},
#endif
    {"CHASSIS_SPECIFIC",           SAHPI_ENT_CHASSIS_SPECIFIC},
    {"BOARD_SET_SPECIFIC",         SAHPI_ENT_BOARD_SET_SPECIFIC},
    {"OEM_SYSINT_SPECIFIC",        SAHPI_ENT_OEM_SYSINT_SPECIFIC},
    {"ROOT",                       SAHPI_ENT_ROOT},
    {"RACK",                       SAHPI_ENT_RACK},

    {"SUBRACK",                    SAHPI_ENT_SUBRACK},
    {"COMPACTPCI_CHASSIS",         SAHPI_ENT_COMPACTPCI_CHASSIS},
    {"ADVANCEDTCA_CHASSIS",        SAHPI_ENT_ADVANCEDTCA_CHASSIS},
    {"RACK_MOUNTED_SERVER",        SAHPI_ENT_RACK_MOUNTED_SERVER},
    {"SYSTEM_BLADE",               SAHPI_ENT_SYSTEM_BLADE},
    {"SWITCH",                     SAHPI_ENT_SWITCH},
    {"SWITCH_BLADE",               SAHPI_ENT_SWITCH_BLADE},
    {"SBC_BLADE",                  SAHPI_ENT_SBC_BLADE},
    {"IO_BLADE",                   SAHPI_ENT_IO_BLADE},
    {"DISK_BLADE",                 SAHPI_ENT_DISK_BLADE},

    {"DISK_DRIVE",                 SAHPI_ENT_DISK_DRIVE},
    {"FAN",                        SAHPI_ENT_FAN},
    {"POWER_DISTRIBUTION_UNIT",    SAHPI_ENT_POWER_DISTRIBUTION_UNIT},
    {"SPEC_PROC_BLADE",            SAHPI_ENT_SPEC_PROC_BLADE},
    {"IO_SUBBOARD",                SAHPI_ENT_IO_SUBBOARD},
    {"SBC_SUBBOARD",               SAHPI_ENT_SBC_SUBBOARD},
    {"ALARM_MANAGER",              SAHPI_ENT_ALARM_MANAGER},
    {"SHELF_MANAGER",              SAHPI_ENT_SHELF_MANAGER},
    {"DISPLAY_PANEL",              SAHPI_ENT_DISPLAY_PANEL},
    {"SUBBOARD_CARRIER_BLADE",     SAHPI_ENT_SUBBOARD_CARRIER_BLADE},

    {"PHYSICAL_SLOT",              SAHPI_ENT_PHYSICAL_SLOT},
    {"AMC_SUB_SLOT",               AMC_SUB_SLOT_TYPE},

#ifdef HPI_B_02
    {"PICMG_FRONT_BLADE",          SAHPI_ENT_PICMG_FRONT_BLADE},
    {"SYSTEM_INVENTORY_DEVICE",    SAHPI_ENT_SYSTEM_INVENTORY_DEVICE},
    {"FILTRATION_UNIT",            SAHPI_ENT_FILTRATION_UNIT},
    {"AMC",                        SAHPI_ENT_AMC},
    {"BMC",                        SAHPI_ENT_BMC},
    {"IPMC",                       SAHPI_ENT_IPMC},
    {"MMC",                        SAHPI_ENT_MMC},
    {"SHMC",                       SAHPI_ENT_SHMC},
    {"CPLD",                       SAHPI_ENT_CPLD},

    {"EPLD",                       SAHPI_ENT_EPLD},
    {"FPGA",                       SAHPI_ENT_FPGA},
    {"DASD",                       SAHPI_ENT_DASD},
    {"NIC",                        SAHPI_ENT_NIC},
    {"DSP",                        SAHPI_ENT_DSP},
    {"UCODE",                      SAHPI_ENT_UCODE},
    {"NPU",                        SAHPI_ENT_NPU},
    {"OEM",                        SAHPI_ENT_OEM}
#endif
#endif
};

static uns32 gl_num_etypes = sizeof(gl_etype_list)/sizeof(ENTITY_TYPE_LIST);


/****************************************************************************
 * Name          : string_to_epath
 *
 * Description   : This function is a utility function used to convert the
 *                 canonical string of entity path to the SAF HPI defined
 *                 entity path structure 'SaHpiEntityPathT'.
 *
 * Arguments     : Pointer to canonical entity path string.
 *
 * Return Values : Pointer to HPI's entity path structure.
 *                 NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32
string_to_epath(uns8 *epath_str, uns32 epath_len, SaHpiEntityPathT *epath_ptr)
{
   uns8 *tok, *ptr, *end_char;
   uns8 *epath;
   uns32 i = 0, rc = NCSCC_RC_SUCCESS;

   /* verify the arguments */
   if ((NULL == epath_str) || (NULL == epath_ptr) || (epath_len <= 0))
   {
      m_LOG_HISV_DTS_CONS("string_to_epath: Invalid arguments to string2entitypath\n");
      return NCSCC_RC_FAILURE;
   }
   memset(epath_ptr, 0, sizeof(SaHpiEntityPathT));

   /* allocate memory to make a duplicate of epath_str */
   if (NULL == (epath = m_MMGR_ALLOC_HISV_DATA(epath_len)))
   {
      m_LOG_HISV_DTS_CONS("string_to_epath: mem alloc error\n");
      return NCSCC_RC_FAILURE;
   }
   /* copy epath_str to epath */
   memcpy(epath, epath_str, epath_len);

   /* set the pointers */
   tok = epath;
   end_char = epath + epath_len;

   /* look for and skip the first '{' char in entity path */
   if (NULL == (ptr = strchr((int8 *)tok, EPATH_BEGIN_CHAR)))
   {
      m_MMGR_FREE_HISV_DATA(epath);
      return NCSCC_RC_FAILURE;
   }
   *ptr = '\0';

   if ((tok = ptr+1) >= end_char)
   {
      m_MMGR_FREE_HISV_DATA(epath);
      return NCSCC_RC_FAILURE;
   }
   /* get the tokens from epath and fill the epath_ptr structure */
   while (tok != NULL)
   {
      /* go to next tuple in entity path string */
      if (NULL == (ptr = strchr(tok, EPATH_BEGIN_CHAR)))
         break;

      if ((tok = ptr+1) >= end_char)
         break;

      /* gets the entity type */
      if (NULL == (ptr = strchr(tok, EPATH_SEPARATOR_CHAR)))
         break;

      *ptr = '\0';
      remove_spaces(&tok);

      /* fill corresponding value of entity type in the structure */
      if (SAHPI_ENT_UNSPECIFIED == (epath_ptr->Entry[i].EntityType = get_entity_type(tok)))
      {
         rc = NCSCC_RC_FAILURE;
         break;
      }
      if ((tok = ptr+1) >= end_char)
         break;

      /* get the entity instance value */
      if (NULL == (ptr = strchr(tok, EPATH_END_CHAR)))
         break;

      *ptr = '\0';
      remove_spaces(&tok);

      /* put the entity instance value in the epath_ptr structure */
#ifdef HPI_A
      sscanf(tok, "%d", &epath_ptr->Entry[i].EntityInstance);
#else
      sscanf(tok, "%d", &epath_ptr->Entry[i].EntityLocation);
#endif

      if ((tok = ptr+1) >= end_char)
         break;

      if (++i >= SAHPI_MAX_ENTITY_PATH) break;
   }
   /* fill in the root last */
   if (i < SAHPI_MAX_ENTITY_PATH)
   {
      epath_ptr->Entry[i].EntityType = SAHPI_ENT_ROOT;
#ifdef HPI_A
      epath_ptr->Entry[i++].EntityInstance = 0;
#else
      epath_ptr->Entry[i++].EntityLocation = 0;
#endif
   }
   /* free the duplicate string allocated for entity path */
   m_MMGR_FREE_HISV_DATA(epath);
   return rc;
}

/****************************************************************************
 * Name          : SaHpiEntityTypeT
 *
 * Description   : This function returns the value of entity type
 *                 corresponding to the entity type string input.
 *
 * Arguments     : tok - entity type string token picked from entity path.
 *
 * Return Values : Value of entity type string or
 *                 SAHPI_ENT_UNSPECIFIED for invalid entity type string.
 *
 * Notes         : None.
 *****************************************************************************/

static
SaHpiEntityTypeT get_entity_type(int8 *tok)
{
   uns32 i;

   /* evaluate input */
   if (tok == NULL)
      return SAHPI_ENT_UNSPECIFIED;

   /** compare the token with available entity type and return the
    ** corresponding value for a matching entity type.
    **/
   for (i=1; i < gl_num_etypes; i++)
      if (0 == strcmp(tok, gl_etype_list[i].etype_str))
         break;

   /* entity type did not matched available types */
   if (i >= gl_num_etypes)
   {
      char *endptr;
      long int value;

      /* check and return if it is already a value */
      value = strtol(tok, &endptr, 0);
      if (*endptr == '\0')
         return value;
      else
         return SAHPI_ENT_UNSPECIFIED;
   }
   /* return the corresponding value for matching entity type */
   return gl_etype_list[i].etype_val;
}


/****************************************************************************
 * Name          : remove_spaces
 *
 * Description   : Local function used to remove the white spaces around
 *                 the token string read from entity path string.
 *
 * Arguments     : tok - pointer to token string.
 *
 * Return Values : string free of white spaces.
 *
 * Notes         : None.
 *****************************************************************************/

static
void remove_spaces(uns8 **tok)
{
   uns8 *p = *tok;
   if (p == NULL)
      return;

   /* remove spaces before token */
   while ((*p != '\0') && (*p == ' '))
      p++;

   /* mark the first non-space character as beginning of token */
   *tok = p;

   /* move to end of token */
   while ((*p != '\0') && (*p != ' '))
      p++;

   /* terminate at the end of token, ignore rest of spaces if any */
   if (*p == ' ')
      *p = '\0';

   return;
}


/****************************************************************************
 * Name          : get_chassis_id
 *
 * Description   : This function is used by HCD to get the chassis_id from
 *                 the entity path of the resource
 *
 * Arguments     : Pointer to some entity path.
 *
 * Return Values : NCSCC_RC_FAILURE or NCSCC_RC_SUCCESS with chassis_id value.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
get_chassis_id(SaHpiEntityPathT *epath, int32 *chassis_id)
{
   uns32 i;

   /* reach to the root element of the entity path */
   for (i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
   if (epath->Entry[i].EntityType == SAHPI_ENT_ROOT)
      break;

   if ((i == 0) || (i >= SAHPI_MAX_ENTITY_PATH))
      return NCSCC_RC_FAILURE;

#ifdef HPI_A
   *chassis_id = epath->Entry[i-1].EntityInstance;
#else
   *chassis_id = epath->Entry[i-1].EntityLocation;
#endif

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : discover_domain
 *
 * Description   : This function is used by HCD to discover the set of
 *                 resources that are available on the domain over which
 *                 it manages the HPI session.
 *
 * Arguments     : Pointer argument strucutre contains session & domain id.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/

uns32
discover_domain(HPI_SESSION_ARGS *ptr)
{
   SaErrorT        err, autoinsert_err;
#ifdef HPI_A
   SaHpiRptInfoT   rpt_info_before;
#endif
   SaHpiEntryIdT   current;
   SaHpiEntryIdT   next;

   SaHpiDomainIdT  domain_id;
   SaHpiSessionIdT session_id;
   SaHpiRptEntryT  entry;
   int32 chassis_id;
   SaHpiInt64T auto_instime = SAHPI_TIMEOUT_BLOCK;
   uns32  rc = NCSCC_RC_SUCCESS;

    m_LOG_HISV_DTS_CONS("discover_domain: Function invoked\n");
   /* get the HPI session and domain identifiers */
   domain_id = ptr->domain_id;
   session_id = ptr->session_id;
   ptr->discover_domain_err = FALSE;

   autoinsert_err = saHpiAutoInsertTimeoutSet(session_id, auto_instime);

   if (autoinsert_err != SA_OK)
   {
      if (autoinsert_err == SA_ERR_HPI_READ_ONLY)
      {
         /* Allow for the case where the insertion timeout is read-only. */
         m_LOG_HISV_DTS_CONS("discover_domain: saHpiAutoInsertTimeoutSet is read-only\n");
      }
      else
      {
         m_LOG_HISV_DTS_CONS("discover_domain: Error in setting Auto Insertion timeout\n");
      }
   }

   /* discover the resources on this session */
#ifdef HPI_A
   err = saHpiResourcesDiscover(session_id);
#else
   err = saHpiDiscover(session_id);
#endif
   if (SA_OK != err)
   {
#ifdef HPI_A
      m_LOG_HISV_DTS_CONS("discover_domain: saHpiResourcesDiscover Error...\n");
#else
      m_LOG_HISV_DTS_CONS("discover_domain: saHpiDiscover Error...\n");
#endif
      ptr->discover_domain_err = TRUE;
      return NCSCC_RC_FAILURE;
   }

#ifdef HPI_A
   /* grab copy of the update counter before traversing RPT */
   err = saHpiRptInfoGet(session_id, &rpt_info_before);
   if (SA_OK != err)
   {
      m_LOG_HISV_DTS_CONS("discover_domain: saHpiRptInfoGet Err...\n");
      ptr->discover_domain_err = TRUE;
      return NCSCC_RC_FAILURE;
   }
#endif
   /** process the list of resource pointer tables on this session.
    ** verifies the existence of RPT entries.
    **/
   m_LOG_HISV_DTS_CONS("discover_domain: Scanning RPT...\n");
   next = SAHPI_FIRST_ENTRY;
   do
   {
      current = next;
      /* get the HPI RPT entry */
      err = saHpiRptEntryGet(session_id, current, &next, &entry);
      if (SA_OK != err)
      {
         /* error getting RPT entry */
         if (current != SAHPI_FIRST_ENTRY)
         {
            m_LOG_HISV_DTS_CONS("discover_domain: saHpiRptEntryGet Error\n");
            return NCSCC_RC_FAILURE;
         }
         else
         {
            m_LOG_HISV_DTS_CONS("discover_domain: Empty RPT\n");
            rc = NCSCC_RC_FAILURE;
            break;
         }
      }
      /* break here as it will be processed again in HSM thread */
      break;
   } while (next != SAHPI_LAST_ENTRY);
   /* pick the chassis_id value from entity path of last discovered resource */
   if (get_chassis_id(&entry.ResourceEntity,
                      &chassis_id) == NCSCC_RC_SUCCESS)
      ptr->chassis_id = chassis_id;
   else
      m_LOG_HISV_DTS_CONS("discover_domain: Could not find the chassis-id\n");

   return rc;
}

/****************************************************************************
 * Name          : Print-Hotswap states of the resource
 *
 * Description   : This function maps and prints the hotswap state to the enum
 *
 * Arguments     : Hotswap state value.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
print_hotswap (SaHpiHsStateT cur_state, SaHpiHsStateT prev_state, uns32 board_num, SaHpiEntityTypeT type)
{
   /* check if it is normal board */
#ifdef HPI_A
   if ((type != SAHPI_ENT_SYSTEM_BOARD) && (type != SAHPI_ENT_OTHER_SYSTEM_BOARD)
        && (type != SAHPI_ENT_PROCESSOR_BOARD) && (type != SAHPI_ENT_SUBBOARD_CARRIER_BLADE))
#else
   /* Allow for the case where blades are ATCA or non-ATCA.                        */
   if ((type != SAHPI_ENT_PHYSICAL_SLOT) && (type != SAHPI_ENT_SYSTEM_BLADE) &&
       (type != SAHPI_ENT_SWITCH_BLADE) && (AMC_SUB_SLOT_TYPE != type))
#endif
   {
      m_LOG_HISV_DTS_CONS ("Current Hotswap state of non-board resource is: ");
      m_NCS_CONS_PRINTF("Current Hotswap state of non-board resource of type %d is: ",type);
   }
   else
   {
      m_NCS_CONS_PRINTF("Current Hotswap state of board in physical slot %d is: ",board_num);
      m_LOG_HISV_DEBUG_VAL(HCD_HOTSWAP_CURR_STATE, board_num);
   }
   switch (cur_state)
   {
      case SAHPI_HS_STATE_INACTIVE:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_INACTIVE\n");
         break;
      case SAHPI_HS_STATE_INSERTION_PENDING:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_INSERTION_PENDING\n");
         break;
#ifdef HPI_A
      case SAHPI_HS_STATE_ACTIVE_HEALTHY:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_ACTIVE_HEALTHY\n");
         break;
      case SAHPI_HS_STATE_ACTIVE_UNHEALTHY:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_ACTIVE_UNHEALTHY\n");
         break;
#else
      case SAHPI_HS_STATE_ACTIVE:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_ACTIVE\n");
         break;
#endif
      case SAHPI_HS_STATE_EXTRACTION_PENDING:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_EXTRACTION_PENDING\n");
         break;
      case SAHPI_HS_STATE_NOT_PRESENT:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_NOT_PRESENT\n");
         break;
      default:
         m_LOG_HISV_DTS_CONS("UnKnown\n");
         break;
   }
   /* check if it is normal board */
#ifdef HPI_A
   if ((type != SAHPI_ENT_SYSTEM_BOARD) && (type != SAHPI_ENT_OTHER_SYSTEM_BOARD)
        && (type != SAHPI_ENT_PROCESSOR_BOARD) && (type != SAHPI_ENT_SUBBOARD_CARRIER_BLADE))
#else
   /* Allow for the case where blades are ATCA or non-ATCA.                        */
   if ((type != SAHPI_ENT_PHYSICAL_SLOT) && (type != SAHPI_ENT_SYSTEM_BLADE) &&
       (type != SAHPI_ENT_SWITCH_BLADE) && (AMC_SUB_SLOT_TYPE != type))
#endif
   {
      m_LOG_HISV_DTS_CONS ("Previous Hotswap state of non-board resource is: ");
   }
   else
   {
      m_NCS_CONS_PRINTF("Previous Hotswap state of board in physical slot %d is: ",board_num);
      m_LOG_HISV_DEBUG_VAL(HCD_HOTSWAP_PREV_STATE, board_num);
   }
   switch (prev_state)
   {
      case SAHPI_HS_STATE_INACTIVE:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_INACTIVE\n");
         break;
      case SAHPI_HS_STATE_INSERTION_PENDING:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_INSERTION_PENDING\n");
         break;
#ifdef HPI_A
      case SAHPI_HS_STATE_ACTIVE_HEALTHY:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_ACTIVE_HEALTHY\n");
         break;
      case SAHPI_HS_STATE_ACTIVE_UNHEALTHY:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_ACTIVE_UNHEALTHY\n");
         break;
#else
      case SAHPI_HS_STATE_ACTIVE:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_ACTIVE\n");
         break;
#endif
      case SAHPI_HS_STATE_EXTRACTION_PENDING:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_EXTRACTION_PENDING\n");
         break;
      case SAHPI_HS_STATE_NOT_PRESENT:
         m_LOG_HISV_DTS_CONS("SAHPI_HS_STATE_NOT_PRESENT\n");
         break;
      default:
         m_LOG_HISV_DTS_CONS("UnKnown\n");
         break;
   }
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : hpi_decode_bcd_plus
 *
 * Description   : This function decodes BCD+ data
 *
 * Arguments     : inbuf - data in BCD+ code
 *                 inlen - data length
 *
 * Return Values : outbuf - result ASCII data array
 *                 outlen - result data length.
 *
 * Notes         : None.
 *
 *****************************************************************************/
int 
hpi_decode_bcd_plus(unsigned char *inbuf, unsigned char inlen, unsigned char *outbuf) 
{
   int  i;
   unsigned char c;

   for ( i=0; i < inlen*2; i++ ) 
   {
      if ( i%2 )
         c = (inbuf[i/2]&0xF0)>>4;
      else
         c = inbuf[i/2]&0x0F;
      switch ( c ) 
      {
         case 0xA:   outbuf[i] = ' ';    break;
         case 0xB:   outbuf[i] = '-';    break;
         case 0xC:   outbuf[i] = '.';    break;
         case 0xD:
         case 0xE:
         case 0xF:   outbuf[i] = '?';    break;
         default:    outbuf[i] = '0' + c;    /* digits 0..9 */
      }
   }
   outbuf[i] = '\0';
   return(inlen*2);
}

/****************************************************************************
 * Name          : hpi_decode_6bitpack
 *
 * Description   : This function decodes 6 bit packed ASCII data
 *
 * Arguments     : inbuf - data in 6bit packed ASCII code
 *                 inlen - data length
 *
 * Return Values : outbuf - result ASCII data array
 *                 outlen - result data length.
 *
 * Notes         : None.
 *
 *****************************************************************************/
int 
hpi_decode_6bitpack(unsigned char *inbuf, unsigned char inlen, unsigned char *outbuf) 
{
   int  i, j,
   ret = 0;

   for ( i=0, j=1; i < inlen; i++, j++ ) 
   {
      switch ( j ) 
      {
         case 1: /* process 1st char */
            outbuf[ret++] = 0x20 + (inbuf[i]&0x3F);
            break;
         case 2: /* process 2nd char */
            outbuf[ret++] = 0x20 + ((inbuf[i]&0x0F)<<2) + ((inbuf[i-1]&0xC0)>>6);
            break;
         case 3: /* process 3rd and 4th char */
            outbuf[ret++] = 0x20 + ((inbuf[i]&0x03)<<4) + ((inbuf[i-1]&0xF0)>>4);
            outbuf[ret++] = 0x20 + ((inbuf[i]&0xFC)>>2);
            break;
      }
      if (j == 3) 
         j = 0;
   }
   ret = inlen;
   outbuf[ret] = '\0';
   return(ret);
}

/****************************************************************************
 * Name          : hpi_decode_to_ascii
 *
 * Description   : This function decodes ASCII 6bit+ and BCD+ data to ASCII
 *
 * Arguments     : Encode Type: ASCII or BCD+
 *                 inbuf - data in ASCII or BCD+ code
 *                 inlen - data length
 *
 * Return Values : outbuf - result ASCII data array
 *                 outlen - result data length or -1.
 *
 * Notes         : None.
 *
 *****************************************************************************/
int 
hpi_decode_to_ascii(SaHpiTextTypeT data_type, unsigned char *inbuff, 
                    unsigned char inlen, unsigned char *outbuff) 
{
   switch (data_type)
   {
      case SAHPI_TL_TYPE_ASCII6:
         return hpi_decode_6bitpack(inbuff, inlen, outbuff);
         break;
      case SAHPI_TL_TYPE_BCDPLUS:
         return hpi_decode_bcd_plus(inbuff, inlen, outbuff);
         break;
      default:
         m_NCS_CONS_PRINTF("Unsuported decode type %d may result in FRU Invalidation\n", data_type);
         return -1;
         break;
   }
   return -1;
}

/****************************************************************************
 * Name          : print_invdata
 *
 * Description   : This function prints the inventory data of the resource
 *
 * Arguments     : Hotswap state value.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
uns32
print_invdata(HISV_INV_DATA *inv_data)
{
   /* check for proper argument */
   if (inv_data == NULL)
   {
      m_NCS_CONS_PRINTF("null variable in print_invdata\n");
      return NCSCC_RC_FAILURE;
   }
   /* print the inventory data */
   if (inv_data->product_name.Data != NULL)
   {
      m_NCS_CONS_PRINTF("Product Name = %s\n",inv_data->product_name.Data);
      m_NCS_CONS_PRINTF("Product Name Length = %d\n",inv_data->product_name.DataLength);
   }
   if (inv_data->product_version.Data != NULL)
   {
      m_NCS_CONS_PRINTF("Product Version = %s\n",inv_data->product_version.Data);
      m_NCS_CONS_PRINTF("Product Version Length = %d\n",inv_data->product_version.DataLength);
   }


   /* check for mac address entries and print if exists */
   if (inv_data->oem_inv_data.num_mac_entries == 2)
   {
      uns32 i;
      m_NCS_CONS_PRINTF("OEM Type = %d\n",inv_data->oem_inv_data.type);
      m_NCS_CONS_PRINTF("mId = %d\n",inv_data->oem_inv_data.mId);
      m_NCS_CONS_PRINTF("mot_oem_rec_id = %d\n",inv_data->oem_inv_data.mot_oem_rec_id);
      m_NCS_CONS_PRINTF("rec_format_ver = %d\n",inv_data->oem_inv_data.rec_format_ver);
      m_NCS_CONS_PRINTF("num_mac_entries = %d\n",inv_data->oem_inv_data.num_mac_entries);
      m_NCS_CONS_PRINTF("Base Mac Addr 1 :\n");
      for (i=0; i<6; i++)
         m_NCS_CONS_PRINTF("0x%2x  ",inv_data->oem_inv_data.interface_mac_addr[0][i]);

      m_NCS_CONS_PRINTF("\nBase Mac Addr 2 :\n");
      for (i=0; i<6; i++)
         m_NCS_CONS_PRINTF("0x%2x  ",inv_data->oem_inv_data.interface_mac_addr[1][i]);
      m_NCS_CONS_PRINTF("\n");
   }
   else
      m_NCS_CONS_PRINTF("Mac address entries does not exist in inventory data\n");

   return NCSCC_RC_SUCCESS;
}

