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

 MODULE NAME:  NCS_CLI.H

$Header: /ncs/software/leap/base/products/cli/inc/ncs_cli.h 6     6/19/01 3:20p Agranos $
..............................................................................

  DESCRIPTION:

  Published API's file for CLI

******************************************************************************
*/

#ifndef NCS_CLI_H
#define NCS_CLI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "ncsgl_defs.h"
#include "os_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_tsk.h"
#include "ncs_mib_pub.h"
#include "ncs_lib.h"

#define NCSCLI_ARG_COUNT             40
#define NCSCLI_CMDLIST_SIZE          100
#define NCSCLI_PWD_LEN               16
#define NCSCLI_MAC_ADDR_CNT          6

/* CLI instance and CEF Timer */
#define CLI_CEF_TIMEOUT             6000 /* 60 Seconds Wait period */

#define m_MMGR_ALLOC_CLI_DEFAULT_VAL(mem_size)  m_NCS_MEM_ALLOC(mem_size, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                0)

#define m_MMGR_FREE_CLI_DEFAULT_VAL(p)          m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                0)

/* access levels */
typedef enum {
    NCSCLI_VIEWER_ACCESS = 0,
    NCSCLI_ADMIN_ACCESS,
    NCSCLI_SUPERUSER_ACCESS,
    NCSCLI_ACCESS_END
}NCSCLI_ACCESS_LEVEL;  /* Needed to specify cli user access level (59359) */
/* Token types */
typedef enum {
    NCSCLI_KEYWORD = 1,  /* Keyword */
    NCSCLI_STRING,       /* String */
    NCSCLI_NUMBER,       /* Number */
    NCSCLI_CIDRv4,       /* IPV4-Address/Length */
    NCSCLI_IPv4,         /* IPV4 Address */
    NCSCLI_IPv6,         /* IPV6 Address */
    NCSCLI_MASKv4,       /* IPV4 Address Mask */
    NCSCLI_CIDRv6,       /* IPV6-Address/Length */
    NCSCLI_PASSWORD,     /* Password */
    NCSCLI_COMMUNITY,    /* Community */
    NCSCLI_WILDCARD,     /* Wildcard */
    NCSCLI_MACADDR,      /* MAC Address */
    NCSCLI_TOK_MAX,      /* This should at the last */    
} NCSCLI_TOKEN_TYPE;

typedef enum {
   NCSCLI_COMM_TYPE_ASN = 0,
   NCSCLI_COMM_TYPE_IPADDR,
   NCSCLI_COMM_TYPE_VALUE
} NCSCLI_COMMUNITY_TYPE;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        DATA STRUCTURES

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/****************************************************************************\
                    Subsystem data structure 
\****************************************************************************/
typedef struct ncscli_subsys_cb { 
   NCSCONTEXT   i_cef_data;   /* Void pointer for storing CEF DATA */ 
   NCSCONTEXT   i_cef_mode;   /* Void pointer for storing CEF MODE */ 
} NCSCLI_SUBSYS_CB;

/***************************************************************************\
    Bindery structure used by the CEF to populate the MAB arg
\**************************************************************************/
typedef struct ncscli_bindery {
  uns32           i_cli_hdl;     /* CLI handle; which CLI instance ?    */
  uns32           i_minst_hdl;   /* Subsystem handle; in case of multiple instance */
  uns32           i_mab_hdl;     /* MIB handle for NCSMIB_ARG::i_mib_key */
  NCSMIB_REQ_FNC  i_req_fnc;     /* MAB (to MAC) or the subsystem's  */
} NCSCLI_BINDERY;

/***************************************************************************
 The Substem function prototype
****************************************************************************/
typedef uns32 (*NCSCLI_SUBSYSTEM_FUNC) (struct ncscli_bindery *bindery); 

/**************************************************************************\
 The NCSCLI_CEF_DATA struct is passed as argument to CEF(Command Execution 
 Function). The CEF registers the NCSCLI_CEF_DATA struct with i_usr_key field 
 of NCSMIB_ARG. 
\**************************************************************************/
typedef struct ncscli_cef_data {
   NCSCLI_SUBSYS_CB   *i_subsys;    /* Subsystem control block which is 
                                       stored in CLI control block */
   NCSCLI_BINDERY     *i_bindery;   /* Bindary used by the CEF */
} NCSCLI_CEF_DATA;

/**************************************************************************\
Structure that define attributes of each argument record
   - NCSCLI_IPV4_PRFX store IP Address and mask lenght
   - NCSCLI_ARG_VAL store basic data unit found while parsing command
\***************************************************************************/
typedef struct ncscli_ipv4_pfx {   
   NCS_IPV4_ADDR  ip_addr; /* IP V4 address definition */
   uns8           len;     /* its lenght */   
} NCSCLI_IPV4_PFX;

#if (NCS_IPV6 ==1)
typedef struct ncscli_ipv6_pfx {   
   NCS_IPV6_ADDR  addr; /* IPV6 Address */ 
   uns8           len;  /* its length */
} NCSCLI_IPV6_PFX;
#endif

typedef struct ncscli_community_val {
   NCSCLI_COMMUNITY_TYPE   type;
   union {
      NCS_IPV4_ADDR        ip_addr;    /* IP V4 address definition */
      uns32                asn;        /* AS Number */
   }val1;   
   uns32                   num;        /* Assigned number */   
}NCSCLI_COMMUNITY_VAL;

typedef struct ncscli_arg_val {   
   uns32                   i_arg_type;                   /* argument type */ 
   union {
      uns32                intval;                       /* NUMBER */
      int8                 *strval;                      /* STRING */      
      NCSCLI_IPV4_PFX      ip_pfx;                       /* IPV4 format */
      NCSCLI_COMMUNITY_VAL community;                    /* Community format */
      uns8                 macaddr[NCSCLI_MAC_ADDR_CNT]; /* MAC Address */
#if (NCS_IPV6 ==1)
      NCSCLI_IPV6_PFX      ipv6_pfx;                     /* IPV6 format */
#endif
   } cmd;            
   uns8                    i_arg_sub_type; 
   uns8                    sym_valid;                /* Token matched the semantic */    
} NCSCLI_ARG_VAL;

/***************************************************************************\
      Structure that define attributes of argument set which are passed to 
      the CEF function when a command match has been found.
\***************************************************************************/
typedef struct ncscli_arg_set {
   uns32          i_pos_value;   /* set bit position to 1 if arg present.*/
   NCSCLI_ARG_VAL i_arg_record[NCSCLI_ARG_COUNT]; /* array if arg_val structures */
} NCSCLI_ARG_SET;

/***************************************************************************
    The Command Execution Function (CEF) API prototype
****************************************************************************/
typedef uns32 (*NCSCLI_EXEC)(struct ncscli_arg_set   *args, 
                             struct ncscli_cef_data  *data);

/***************************************************************************\
 The NCSCLI_CMD_DESC structure is used to define the command description and
 associated data. 

 The opfmtstring is used to store the display formatted string. The callback
 function registered for this command and the cmddescriptor structure which 
 contains command specific data that needs to be send back with the callback
 function. One structure is used to represent a command. 
\***************************************************************************/
typedef struct ncscli_cmd_desc {  
  NCSCLI_EXEC  i_cmd_exec_func;  /* invoke this when comand satisfied */
  int8         *i_cmdstr;        /* command described in Netplane 
                                    command language */
  NCSCLI_ACCESS_LEVEL i_cmd_access_level; /* access level of the command */ 
}NCSCLI_CMD_DESC;

/***************************************************************************\
    Password validation function
\****************************************************************************/
typedef int8 (*NCSCLI_PASSWD)(int8* i_passwd, int8* i_node);

/***************************************************************************\
 The passwd struct used to store password info. The 'encrptflag' indicates
 that there is a user registered function that takes care of password 
 encryption and storage functionality. If it is FALSE then CLI just compares
 the user entered password with the 'passwd' string.  
\****************************************************************************/
typedef struct ncscli_access_passwd 
{    
   NCS_BOOL          i_encrptflag;  /* if any encryption is there. */
   union {
      /* to store passwd */
      int8           i_passwd[NCSCLI_PWD_LEN];  
      NCSCLI_PASSWD  i_passfnc;         
   } passwd;
} NCSCLI_ACCESS_PASSWD;

/***************************************************************************\
 The NCSCLI_CMD_LIST structure is used to specify the node and add all 
 commands under that node. All commands belonging to a node are grouped 
 together and defined by the array of structures. Each node can be associated
 with a password. The 'access_req' flag indicates if a node is password 
 protected.
\***************************************************************************/
typedef struct ncscli_cmd_list {
   uns32                i_cmd_count;      /* count of commands */
   NCS_BOOL             i_access_req;/* deprecated */     /* if TRUE indicated passwd protected */
   NCSCLI_ACCESS_PASSWD *i_access_passwd; /* deprecated , password data associated with node */
   int8                 *i_node;          /* specify absolute node-path under 
                                             which to add commands */
   int8                 *i_command_mode;  /* Indiactes the mode the commnads are 
                                             associated with */ 
   NCSCLI_CMD_DESC      i_command_list[NCSCLI_CMDLIST_SIZE]; /* List of commands 
                                           that come under this node */
} NCSCLI_CMD_LIST;

/***************************************************************************\
 The NCSCLI_DEREG_CMD_LIST structure is used to specify the node and all 
 commands under that node that are to be cleaned. All commands belonging to 
 a node are grouped together and defined by the array of structures.  
\***************************************************************************/
typedef struct ncscli_dereg_cmd_list {
   uns32             i_cmd_count;   /* count of commands */
   int8              *i_node;       /* specify absolute node-path under which
                                       to add commands */
   NCSCLI_CMD_DESC   i_command_list[NCSCLI_CMDLIST_SIZE];   /* List of commands
                                       that come under this node */
} NCSCLI_DEREG_CMD_LIST;

/***************************************************************************
                    Notify Events
****************************************************************************/
typedef enum {
   NCSCLI_NOTIFY_EVENT_INVALID_CLI_CB,
   NCSCLI_NOTIFY_EVENT_MALLOC_CLI_CB_FAILED,
   NCSCLI_NOTIFY_EVENT_CEF_TMR_EXP,
   NCSCLI_NOTIFY_EVENT_CLI_INVALID_NODE,
   NCSCLI_NOTIFY_EVENT_CLI_DISPLAY_WRG_CMD
} NCSCLI_NOTIFY_EVENT;
 
/***************************************************************************
                    Notify structure
****************************************************************************/
typedef struct ncscli_notify_info_tag {
   uns32                i_notify_hdl;
   uns32                i_hdl;
   NCSCLI_NOTIFY_EVENT  i_evt_type;
   NCSCONTEXT           i_cb;
} NCSCLI_NOTIFY_INFO_TAG;

typedef enum {  /* Request types to be made to the CLI OP Routine */
   NCSCLI_OPREQ_REGISTER,
   NCSCLI_OPREQ_DEREGISTER,   
   NCSCLI_OPREQ_DONE,
   NCSCLI_OPREQ_MORE_TIME,
   NCSCLI_OPREQ_DISPLAY,
   NCSCLI_OPREQ_GET_DATA,
   NCSCLI_OPREQ_GET_MODE,
} NCSCLI_OPREQ;

/***************************************************************************\
     U S E R   P R O V I D E D   C A L L B A C K   F U N C T I O N S   
     
                 P E R   C L I   I N S T A N C E

    CLI Notify API prototype  : Inform system of trouble in CLI land
    CLI Getchar API prototype : fetch characters from console 
    CLI Putstr API prototype  : put string to console.. implies 'flush'
\****************************************************************************/

typedef void  (*NCSCLI_NOTIFY) (NCSCLI_NOTIFY_INFO_TAG *i_notif_info);
typedef uns32 (*NCSCLI_GETCHAR)(NCS_VRID id);
typedef uns32 (*NCSCLI_PUTSTR) (NCS_VRID id, uns8* str); /* SMM to replace m_CLI_FLUSH_STR */

/***************************************************************************\
                  OP  C O M M A N D S  R E G I S T E R
\***************************************************************************/
typedef struct ncscli_op_register {
   NCSCLI_BINDERY    *i_bindery;
   NCSCLI_CMD_LIST   *i_cmdlist;
} NCSCLI_OP_REGISTER;

/***************************************************************************\
                  OP  C O M M A N D S  D E R E G I S T E R  
\***************************************************************************/
typedef struct ncscli_op_deregister {
   NCSCLI_DEREG_CMD_LIST *i_cmdlist;
} NCSCLI_OP_DEREGISTER;

/***************************************************************************\
       OP  C O M M A N D S  D O N E  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_op_done {
   uns32 i_status;   
} NCSCLI_OP_DONE;

/***************************************************************************\
   OP  C O M M A N D S  M O R E - T I M E  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_op_more_time {
   uns32 i_delay;   
} NCSCLI_OP_MORE_TIME;

/***************************************************************************\
   OP  C O M M A N D S  D I S P L A Y  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_op_display {
   uns8 *i_str;   
} NCSCLI_OP_DISPLAY;

/***************************************************************************\
  OP  C O M M A N D S  M O D E - G E T  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_op_mode_get {
   uns16       i_lvl;   
   NCSCONTEXT  o_data;
} NCSCLI_OP_MODE_GET;

/***************************************************************************\
   OP  C O M M A N D S  D A T A - G E T  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_op_data_get {
   NCSCLI_SUBSYS_CB *o_info;
} NCSCLI_OP_DATA_GET;

/***************************************************************************\
                     OP  R E Q U E S T  S T R U C T U R E
\***************************************************************************/
typedef struct ncscli_op_info {
   uns32          i_hdl;
   NCSCLI_OPREQ   i_req;
   
   union {
      NCSCLI_OP_REGISTER   i_register;
      NCSCLI_OP_DEREGISTER i_deregister;      
      NCSCLI_OP_DONE       i_done;
      NCSCLI_OP_MORE_TIME  i_more;
      NCSCLI_OP_DISPLAY    i_display;
      NCSCLI_OP_MODE_GET   i_mode;
      NCSCLI_OP_DATA_GET   i_data;
   } info;   
} NCSCLI_OP_INFO;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                        EXTERNAL PUBLISHED API's
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
EXTERN_C CLIDLL_API uns32 ncscli_opr_request(struct ncscli_op_info *);

/*****************************************************************************\
    SE API used to initiate and create PWE's of CLI component. 
\*****************************************************************************/
EXTERN_C CLIDLL_API uns32 cli_lib_req(NCS_LIB_REQ_INFO *);
extern CLIDLL_API uns32 gl_cli_hdl;
extern CLIDLL_API NCSCONTEXT gl_pcode_sem_id;
extern CLIDLL_API NCS_BOOL gl_cli_valid;

#define m_MMGR_ALLOC_NCSCLI_OPAQUE     m_MMGR_ALLOC_CLI_DEFAULT_VAL
#define m_MMGR_FREE_NCSCLI_OPAQUE    m_MMGR_FREE_CLI_DEFAULT_VAL

#ifdef  __cplusplus
}
#endif

#endif
