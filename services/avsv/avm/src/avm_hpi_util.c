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
..............................................................................
  
  ..............................................................................
 
  DESCRIPTION:This module deals with conversion of an SaHpiEntityPathT into a
  string.
  ..............................................................................

  Function Included in this Module:

  hpi_entitypath_string - 
  avm_get_reverse -
 
******************************************************************************
*/



#include "avm.h"

/* entity type list */
HPI_ENTITY_TYPE_LIST  gl_hpi_ent_type_list[] = {

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


/***********************************************************************
 ******
 * Name          : hpi_convert_entitypath_string
 *
 *
 * Description   : This function converts entitypath to string format
 *
 * Arguments     : SaHpiEntityPathT*    - Pointer to entity path
 *                 uns8*  - Pointer to destination string
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 ************************************************************************/

extern uns32
hpi_convert_entitypath_string(SaHpiEntityPathT *entity_path, uns8 *ent_path_str)
{
   uns32 i     = 0;
   uns32 index = 0;
   uns32 count; 
   uns32 rc = NCSCC_RC_SUCCESS;
    
   *(ent_path_str++) = '{';
   for(i = 0; i < SAHPI_MAX_ENTITY_PATH; i++)
   {
      if(entity_path->Entry[i].EntityType == SAHPI_ENT_ROOT)
      {
         break;
      }else
      {  
#ifdef HPI_A
        if((SAHPI_ENT_UNSPECIFIED > entity_path->Entry[i].EntityType) ||
            (SAHPI_ENT_SUBBOARD_CARRIER_BLADE < entity_path->Entry[i].EntityType))
         {
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path->Entry[i].EntityType);
            rc =  NCSCC_RC_FAILURE;
            break;
         }
         
         if((SAHPI_ENT_UNSPECIFIED <= entity_path->Entry[i].EntityType) &&
            (SAHPI_ENT_BATTERY >= entity_path->Entry[i].EntityType))
         {
            index = entity_path->Entry[i].EntityType; 
         }else
         {
            if((SAHPI_ENT_ROOT <= entity_path->Entry[i].EntityType) &&
               (SAHPI_ENT_SUBBOARD_CARRIER_BLADE >= entity_path->Entry[i].EntityType))
            {
               index = SAHPI_ENT_BATTERY + entity_path->Entry[i].EntityType - SAHPI_ENT_ROOT + 4;
            }else
            {
               if(SAHPI_ENT_CHASSIS_SPECIFIC == entity_path->Entry[i].EntityType)
               {
                  index = SAHPI_ENT_BATTERY + 1;
               }else 
               if(SAHPI_ENT_BOARD_SET_SPECIFIC == entity_path->Entry[i].EntityType)
               {
                  index = SAHPI_ENT_BATTERY + 2;
               }else
               if(SAHPI_ENT_OEM_SYSINT_SPECIFIC == entity_path->Entry[i].EntityType)
               {
                  index = SAHPI_ENT_BATTERY + 3;
               }else
               {
                  m_AVM_LOG_INVALID_VAL_FATAL(entity_path->Entry[i].EntityType);
                  rc =  NCSCC_RC_FAILURE;
                  break;
               }
            }
         }
#else
        if((SAHPI_ENT_UNSPECIFIED > entity_path->Entry[i].EntityType)
#ifdef HPI_B_02
            || (SAHPI_ENT_OEM < entity_path->Entry[i].EntityType))
#else  
            || (SAHPI_ENT_PHYSICAL_SLOT < entity_path->Entry[i].EntityType))
#endif
         {
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path->Entry[i].EntityType);
            rc =  NCSCC_RC_FAILURE;
            break;
         }

         index = 0;
         do {
            if (gl_hpi_ent_type_list[index].etype_val == entity_path->Entry[i].EntityType) {
               break;
            }
            index++;
#ifdef HPI_B_02
         } while (gl_hpi_ent_type_list[index -1].etype_val != SAHPI_ENT_OEM);

         if (gl_hpi_ent_type_list[index -1].etype_val == SAHPI_ENT_OEM) {
            /* reached end of list and did not find entity type */
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path->Entry[i].EntityType);
            rc = NCSCC_RC_FAILURE;
            break;
         }
#else
         } while (gl_hpi_ent_type_list[index -1].etype_val != SAHPI_ENT_PHYSICAL_SLOT);
         if (gl_hpi_ent_type_list[index -1].etype_val == SAHPI_ENT_PHYSICAL_SLOT) {
            /* reached end of list and did not find entity type */
            m_AVM_LOG_INVALID_VAL_FATAL(entity_path->Entry[i].EntityType);
            rc = NCSCC_RC_FAILURE;
            break;
         }
#endif

#endif

         *(ent_path_str++) = '{';
         memcpy(ent_path_str, gl_hpi_ent_type_list[index].etype_str, strlen(gl_hpi_ent_type_list[index].etype_str));
         
         ent_path_str += strlen(gl_hpi_ent_type_list[index].etype_str);
      
         *(ent_path_str++) = ',';

#ifdef HPI_A
         count = sprintf(ent_path_str, "%d", entity_path->Entry[i].EntityInstance);
#else
         count = sprintf(ent_path_str, "%d", entity_path->Entry[i].EntityLocation);
#endif
         ent_path_str += count;

         *(ent_path_str ++)= '}'; 
         *(ent_path_str ++)= ',';
      }    
   }     
   
   *(ent_path_str - 1)= '}'; 

   /* Must null-terminate this string if it is to be used by HISv. */
   *ent_path_str = '\0';
   return rc;
}

