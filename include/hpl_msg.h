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

  DESCRIPTION:
  This file containts the message structures shared between AvM and other
  HPI clients with HISv

******************************************************************************
*/
#ifndef HPL_MSG_H
#define HPL_MSG_H

#include "ncsgl_defs.h"

#include <SaHpi.h>

#ifndef HPI_A
#define AMC_SUB_SLOT_TYPE  (SaHpiEntityTypeT)SAHPI_ENT_CHASSIS_SPECIFIC + 7
#endif


/* macros for base MAC addresses for payload blades */
#define MAX_MAC_ENTRIES          02
#define MAC_DATA_LEN             06

/* event channel name */
#define EVT_CHANNEL_NAME  "EVENTS"

#define SAHPI_CRITICAL_PATTERN_LEN    14
#define SAHPI_CRITICAL_PATTERN          "SAHPI_CRITICAL"

#define SAHPI_MAJOR_PATTERN_LEN       11
#define SAHPI_MAJOR_PATTERN             "SAHPI_MAJOR"

#define SAHPI_MINOR_PATTERN_LEN       11
#define SAHPI_MINOR_PATTERN          "SAHPI_MINOR"

#define SAHPI_INFORMATIONAL_PATTERN_LEN       19
#define SAHPI_INFORMATIONAL_PATTERN          "SAHPI_INFORMATIONAL"

#define SAHPI_OK_PATTERN_LEN       8
#define SAHPI_OK_PATTERN          "SAHPI_OK"

#define SAHPI_DEBUG_PATTERN_LEN   11
#define SAHPI_DEBUG_PATTERN          "SAHPI_DEBUG"

#define HISV_MAC_ADDR_MOT_OEM_MID       (0x0000A1)
#define HISV_MAC_ADDR_FORCE_OEM_MID     (0x000E48)
#define HISV_MAC_ADDR_EMERSON_OEM_MID   (0x0065cd)
#define HISV_MAC_ADDR_MOT_OEM_REC_ID    (0x01)
#define HISV_MAC_ADDR_FORCE_OEM_REC_ID  (0x11)
#define HISV_MAX_INV_STR_LEN            (30)
#define CTRL_NUM_BOOT_BANK              16

/* offset for firware progress error */
#define HPI_SE_FWPROG_CODE_OFFSET    0x80

/* version for backward compatibility */
#define HISV_SW_VERSION              (0x00010000)
#define HISV_EDS_INF_VERSION         (0x00000001)


/* MAC OEM Record Type */
typedef enum 
{
   MAC_OEM_UNKNOWN_TYPE,
   MAC_OEM_MOT_TYPE,
   MAC_OEM_FORCE_TYPE
} MAC_OEM_REC_TYPE;

/* enum commands for firmware progress code */
typedef enum hpi_evt_prg_state {
   HPI_EVT_FWPROG_ERROR,
   HPI_EVT_FWPROG_HANG,
   HPI_EVT_FWPROG_PROG
}HPI_EVT_FW_PRG_STATE;

#define HPI_IPMI_EVT_DATA2  0x02

/******************************************************************************
 firmware progress success events
 ******************************************************************************/
/* vivek_hisv */
typedef enum hpi_fwprog_events
{
   HPI_FWPROG_SYS_BOOT  = 0x13,
   HPI_FWPROG_BOOT_SUCCESS = HPI_SE_FWPROG_CODE_OFFSET,
   HPI_FWPROG_NODE_INIT_SUCCESS,
   HPI_FWPROG_PHOENIXBIOS_UPGRADE_SUCCESS

} HPI_FWPROG_EVENTS;

/******************************************************************************
 firmware progress error events
 ******************************************************************************/
#if 0
/* vivek_hisv */
typedef enum hpi_fwerr_events
{
   /* Firmware error event logging messages */
   HPI_FWERR_HPM_INIT_FAILED = HPI_SE_FWPROG_CODE_OFFSET,
   HPI_FWERR_IPMC_UPGRADE_FAILED,
   HPI_FWERR_PHOENIXBIOS_UPGRADE_FAILED,
   HPI_FWERR_HLFM_INIT_FAILED,
   HPI_FWERR_OPENHPI_INIT_FAILED,
   HPI_FWERR_SWITCH_INIT_FAILED,
   HPI_FWERR_LHC_DMN_INIT_FAILED,
   HPI_FWERR_LHC_RSP_INIT_FAILED,
   HPI_FWERR_NW_SCRIPT_INIT_FAILED,
   HPI_FWERR_DRBD_INIT_FAILED,
   HPI_FWERR_TIPC_INIT_FAILED,
   HPI_FWERR_IFSD_INIT_FAILED,
   HPI_FWERR_DTSV_INIT_FAILED,
   HPI_FWERR_MASV_INIT_FAILED,
   HPI_FWERR_PSSV_INIT_FAILED,
   HPI_FWERR_GLND_INIT_FAILED,
   HPI_FWERR_EDSV_INIT_FAILED,
   HPI_FWERR_SUBAGT_INIT_FAILED,
   HPI_FWERR_SNMPD_INIT_FAILED,
   HPI_FWERR_NCS_INIT_FAILED,
   HPI_FWERR_UNKNOWN_EVT_FAILED

} HPI_FWERR_EVENTS;
#endif

/* Moving IPMC/BIOS codes to end */
typedef enum hpi_fwerr_events
{
   /* Firmware error event logging messages */
   HPI_FWERR_HPM_INIT_FAILED = HPI_SE_FWPROG_CODE_OFFSET,
   HPI_FWERR_HLFM_INIT_FAILED,
   HPI_FWERR_OPENHPI_INIT_FAILED,
   HPI_FWERR_SWITCH_INIT_FAILED,
   HPI_FWERR_LHC_DMN_INIT_FAILED,
   HPI_FWERR_LHC_RSP_INIT_FAILED,
   HPI_FWERR_NW_SCRIPT_INIT_FAILED,
   HPI_FWERR_DRBD_INIT_FAILED,
   HPI_FWERR_TIPC_INIT_FAILED,
   HPI_FWERR_IFSD_INIT_FAILED,
   HPI_FWERR_DTSV_INIT_FAILED,
   HPI_FWERR_MASV_INIT_FAILED,
   HPI_FWERR_PSSV_INIT_FAILED,
   HPI_FWERR_GLND_INIT_FAILED,
   HPI_FWERR_EDSV_INIT_FAILED,
   HPI_FWERR_SUBAGT_INIT_FAILED,
   HPI_FWERR_SNMPD_INIT_FAILED,
   HPI_FWERR_NCS_INIT_FAILED,
   HPI_FWERR_IPMC_UPGRADE_FAILED,
   HPI_FWERR_PHOENIXBIOS_UPGRADE_FAILED,
   HPI_FWERR_UNKNOWN_EVT_FAILED

} HPI_FWERR_EVENTS;


/* OEM inventory data structure to hold MAC address entries */
typedef struct oem_inv_data
{
   SaHpiUint8T         type;               /* OEM Type */
   SaHpiUint32T        mId;                /* Manufacturer Id */
   SaHpiUint8T         mot_oem_rec_id;     /* Motorola OEM Record Id */
   SaHpiUint8T         rec_format_ver;     /* record format version */
   SaHpiUint8T         num_mac_entries;    /* number of MAC address entries
                                             (max 2 supported for backplane base MAC addresses) */
   SaHpiUint8T         interface_mac_addr[MAX_MAC_ENTRIES][MAC_DATA_LEN];  /* base MAC address */
} OEM_INV_DATA;

/* data structure to hold inventory data */
typedef struct hisv_inv_data
{
    SaHpiTextBufferT      product_name;       /* product name used for validation */
    SaHpiTextBufferT      product_version;    /* product version being used for validation */
    OEM_INV_DATA          oem_inv_data;       /* OEM inventory data holding the
                                                 backplane base MAC addresses */
} HISV_INV_DATA;

/* HISv event publish structure */
typedef struct hpi_hisv_evt_type
{
   SaHpiEventT            hpi_event;          /* HPI event structure */
   SaHpiEntityPathT       entity_path;        /* HPI entity path */
   HISV_INV_DATA          inv_data;           /* HPI inventory data required for validation */
   uns32                  version;            /* versioning; see the macros defined above */

}HPI_HISV_EVT_T;

/* minimum length of the HISv packet
 * useful just in case if you do not want to publish inventory data; but after versioning its 
 * always required to publish inventory data now. At least empty data
 */
#define  HISV_MIN_EVT_LEN     (sizeof(HPI_HISV_EVT_T) - sizeof(HISV_INV_DATA))


#endif  /* HPL_MSG_H */

