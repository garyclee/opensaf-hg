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

#ifndef NID_IPMC_CMD_H
#define NID_IPMC_CMD_H

#include <unistd.h>
#include "gl_defs.h"
#include "node_init.h"

/***************************************************
   structure to carry arguments for system firmware 
  progress event info 
****************************************************/
typedef struct nid_sys_fw_prog_info
{
   uns8 event_type;
   uns8 data2;
   uns8 data3;
}NID_SYS_FW_PROG_INFO;


/* Led id */
typedef enum led
{
   BLUE_LED = 0,
   ALL      = 0,
   LED1,
   LED2,
   LED3
}NID_IPMC_LED_ID;

/* led function */
typedef enum func
{
   LED_OFF = 0x00,
   LED_ON  = 0xFF,
}NID_IPMC_LED_FUNC;

/* Led color */
typedef enum nid_led_color
{
   BLUE = 1,
   RED,
   GREEN,
   AMBER,
   ORANGE,
   WHITE,
   DEFAULT = 0x0f 
}NID_LED_COLORS;

/* Sequence nuber generator */
#define NID_SEQ_NUM(ctx, num)    \
                             { \
                               num=ctx<< 2; \
                               ctx=(ctx==0x3F)?0:(ctx+1);\
                             }


#define   NID_IPMI_CMD_LENGTH          40
#define   NID_IPMI_RC                  4 /* IPMC command responce code */


typedef struct NidIpmiSerialMsg
{
   unsigned char msg[NID_IPMI_CMD_LENGTH];
   uns32 size;
}NID_IPMI_SERIAL_MSG;


/* Structure to carry argument info for LED get */

typedef struct nidGetLedT{
   uns8 fruid;                /* fru id (can be 0 or 1 depending on the board) */
   uns8 led;                  /* can be any of NID_IPMC_LEDS */
   uns8 ledState;             /* Bit field:
                                 0x01 : Local control enabled
                                 0x02 : Override enabled
                                 0x04 : LampTest enabled */
   uns8 lcFunction;           /* Local Function:
                                 0x00: Off
                                 0xFF: On
                                 0x01-0xFA: off duration if blinking (*10 = Xms)
                                 0xFB-0xFE: reserved */
   uns8 lcOnDuration;         /* Local OnDuration:
                                 0x01-0xFA: on duration if blinking      (*10 = Xms)
                                            else ignored (treated as zero) */
   uns8 lcColor;              /* can be any of NID_LED_COLORS
                                 can be 0x0F, which means default color */
   uns8 overrideFunction;     /* Override Function:
                                 0x00: Off
                                 0xFF: On
                                 0x01-0xFA: off duration if blinking  (*10 = Xms)
                                 0xFB-0xFE: reserved */
   uns8 overrideOnDuration;   /* Override OnDuration:
                                 0x01-0xFA: on duration if blinking       (*10 = Xms)
                                            else ignored (treated as zero) */
   uns8 overrideColor;        /* can be any of NID_LED_COLORS
                                 can be 0x0F, which means default color */
   uns8 lampTestDuration;     /* Override OnDuration:
                                 0x01-0x80: on duration if blinking       (*100 = Xms) */
} NIDGETLED;



/* Structure to carry argument info for LED set */

typedef struct nidSetLedT{
   uns8 fruid;         /* fru id (can be 0 or 1 depending on the board) */
   uns8 ledId;         /* can be any of NID_IPMC_LEDS */
   uns8 ledFunction;   /* can be any of NID_IPMC_LED_FUNCTIONS or
                          a value between 0x01 and 0xFA for blinking
                       */
   uns8 onDuration;    /* OnDuration:
                          0x01-0xFA: on duration if blinking       (*10 = Xms)
                                     else ignored (treated as zero)
                       */
   uns8 ledColor;      /* can be any of NID_LED_COLORS
                          can be 0x0F, which means default color
                       */
} NIDSETLED;


enum
{
    NID_IPMC_WDT_DONT_CLEAR_FLAGS=0x00,
    NID_IPMC_WDT_CLEAR_FLAGS=0x10,
}NID_IPMC_WDT_TIMER_USE_EXPIRATION_FLAGS;

enum
{
   NID_IPMC_WDT_NO_ACTION,
   NID_IPMC_WDT_HARD_RESET,
   NID_IPMC_WDT_POWER_DOWN,
   NID_IPMC_WDT_POWER_CYCLE,
}NID_IPMC_WDT_TIMER_ACTIONS;


enum
{
   NID_IPMC_WDT_DONT_STOP=0x44,
   NID_IPMC_WDT_STOP=0x04,
}NID_IPMC__WDT_TIMER_USE;

/* BMC Watch Dog set information */
typedef  struct{
   uns8 timer_use;
   uns8 timer_actions;
   uns8 pre_timeout_interval;
   uns8 timer_expiration_flags_clear;
   uns8 initial_countdown_value_lsbyte;
   uns8 initial_countdown_value_msbyte;
   uns8 current_countdown_value_lsbyte;
   uns8 current_countdown_value_msbyte;
}NIDIPMC_WDT_INFO;


typedef union nid_ipmc_evt_info
{
   NIDGETLED            get_led_info;
   NIDSETLED            set_led_info;   
   NID_SYS_FW_PROG_INFO fw_prog_info;
   NIDIPMC_WDT_INFO     wdt_info;
} NID_IPMC_EVT_INFO;

/* Types of the IPMC commands supported */
typedef enum nid_ipmc_cmds
{
   NID_IPMC_CMD_LED_GET,
   NID_IPMC_CMD_LED_SET,
   NID_IPMC_CMD_SEND_FW_PROG,
   NID_IPMC_CMD_WATCHDOG_SET,
   NID_IPMC_CMD_WATCHDOG_RESET,
   NID_MAX_IPMC_CMD,
}NID_IPMC_CMDS;

/* IPMC interface can be used only if enabled */
typedef enum nid_ipmc_intf_stat
{
   NID_IPMC_INTF_ENABLED,
   NID_IPMC_INTF_DISABLED,
}NID_IPMC_INTF_STAT;


#define NID_IPMC_BOARD_ENTRY      "IPMC_BOARD"

/* Types of boards and corresponding IPMC's*/
typedef enum nid_board_ipmc_type
{
   NID_F101_AVR,
   NID_IAS_ARM,
   NID_7221_AVR,
   NID_BLD_SNTR,
   NID_PC_SCXB,
   NID_PC_PLD,
   NID_MAX_BOARDS,
}NID_BOARD_IPMC_TYPE;

/* F101 specific device informaion */
typedef struct nid_f101_ipmc_dev
{
   uns8  *dev_name;   /* payload interface device name */
   int32 dev_handle;  /* payload interface handle */
   int32 flags;       /* Flags used while opening payload interface */
}NID_F101_IPMC_DEV;


typedef struct ipmi_msg
{
    unsigned char  netfn;
    unsigned char  cmd;
    unsigned short data_len;
    unsigned char  data[40];
} ipmi_msg_t;

ipmi_msg_t ipmi_req_msg;
ipmi_msg_t ipmi_rsp_msg;


#define  NID_F101_IPMC_DEV_NAME   "/tmp/ipmc_rsp" 
#define  NID_F101_IPMC_OPEN_FLAGS (O_RDWR|O_NONBLOCK)
#define  NID_IPMI_CMD_SCRIPT      "/usr/bin/ipmicmd"


typedef void  (* NID_IPMC_BUILD_CMD)(NID_IPMC_CMDS,NID_IPMC_EVT_INFO);
typedef int32 (* NID_GET_IPMC_DEV_FD)();
typedef void  (* NID_CLOSE_IPMC_DEV)();
typedef uns32 (* NID_OPEN_IPMC_DEV)(uns8 *);
typedef int32 (* NID_SEND_IPMC_CMD)(NID_IPMC_CMDS,NID_IPMC_EVT_INFO *);
typedef int32 (* NID_PROC_IPMC_RSP)(NID_IPMC_EVT_INFO *);

/* Data sructure to hold board specific IPMC interface
   status and routines to open and send IPMC comands */
typedef struct nid_board_ipmc_intf_config
{
   NID_IPMC_INTF_STAT   status;                        /* To chaeck if the IPMC interface is enabled */
   uns32                nid_ipmc_cmd_prog;             /* To track if IPMC command resp is expected */
   void                 *nid_ipmc_dev;
   uns8                 *(*cmd_str)[NID_MAX_IPMC_CMD];
   NID_GET_IPMC_DEV_FD  nid_get_ipmc_dev_fd;        
   NID_OPEN_IPMC_DEV    nid_open_ipmc_dev;             /* Platform specific routine to
                                                          open Board specific IPMC device */
   NID_CLOSE_IPMC_DEV   nid_close_ipmc_dev;            /* Platform specific routine to
                                                          close Board specific IPMC device */
   NID_SEND_IPMC_CMD    nid_send_ipmc_cmd;             /* Platform specific routine to
                                                          send IPMC command */ 
   NID_PROC_IPMC_RSP    nid_process_ipmc_resp;         /* Platform specific routine
                                                          to process IPMC responce */ 
}NID_BOARD_IPMC_INTF_CONFIG; 

#endif /* NID_IPMC_CMD_H */
