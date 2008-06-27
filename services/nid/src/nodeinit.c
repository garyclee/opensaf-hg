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


/************************************************************************
*                                                                       *
*       Module Name:    nodeinit (Node Initialization Daemon)           *
*                                                                       *
*       Purpose:        Nodeinitd reads following info from             *
*                       /etc/opt/opensaf/nodeinit.conf file:           *
*                       * Application file name,with absolute path name.*
*                       * Application Name.                             *
*                       * Application Type.                             *
*                       * [OPTIONAL]: Cleanup application               *
*                       * Time-out for successful application initializ *
*                         ation.                                        *
*                       * [OPTIONAL]: Recovery action (respawn "N"      *
*                         times, Restart "X" times if respawn fails     *
*                         "N" times).                                   *
*                       * [OPTIONAL]: Input parameters for application. *
*                       * [OPTIONAL]: Input parameters for cleanup app. *
*                                                                       *
*                                                                       *
*                                                                       *
*            Spawns the services in the sequence listed in              *
*            this file,uses time-out against each service               *
*            spawned and spawn the next service only once               *
*            nodeinitd receives a successful initialization             *
*            notification from spawned servrice.                        *
*                                                                       *
*                       Nodeinitd invokes cleanup followed by recovery  *
*            if it receives initializ ation error from                  *
*            spawned service or time-out before it receives             *
*            any notification.                                          *
************************************************************************/


#include "node_init.h"
#include "rda_papi.h"  /* for RDF interfacing */
#include "nid_ipmc_cmd.h"


#define  SETSIG(sa,sig,fun,flags)\
        do{\
            sa.sa_handler = fun;\
            sa.sa_flags = flags;\
            m_NCS_POSIX_SIGEMPTYSET(&sa.sa_mask);\
            m_NCS_POSIX_SIGACTION(sig,&sa,NULL);\
        }while(0)


extern NID_BOARD_IPMC_TYPE  nid_board_type;
extern NID_BOARD_IPMC_INTF_CONFIG nid_ipmc_board[NID_MAX_BOARDS];
extern uns8 *nid_board_types[NID_MAX_BOARDS]; 


/***********************************
*     NID FIFO file descriptor      *
************************************/
int32 select_fd = -1;

/**********************************
*  To track NID current priority  *
**********************************/
int32 nid_current_prio;

/************************************
*    NIS FIFO handler, created      *
*    by NIS. NIS is waiting for     *
*    us to write DONE.              *
************************************/
int32 nis_fifofd = -1;

/************************************************************
*     List to store the info of application to be spawned   *
************************************************************/
NID_CHILD_LIST spawn_list = {NULL,NULL,0};

/************************************************************
*     Console needed for shell scripts to be spawned        *
************************************************************/
uns8 *cons_dev;
int32 cons_fd=-1;
/***************************************
*     just to track dead childs pid    *
***************************************/
uns32 dead_child = 0;

/********************************************
*    Used to depict if we are ACTIVE/STDBY. *
********************************************/
uns32 role = 0;
uns8  rolebuff[20];
uns8  svc_id[20];



static uns32           spawn_wait(NID_SPAWN_INFO *servicie, uns8 *strbuff);
int32                  fork_process(NID_SPAWN_INFO * service, uns8 * app, char * args[],uns8 *, uns8 * strbuff);
static int32           fork_script(NID_SPAWN_INFO * service, uns8 * app, char * args[],uns8 *, uns8 * strbuff);
static int32           fork_daemon(NID_SPAWN_INFO * service, uns8 * app, char * args[],uns8 *, uns8 * strbuff);
static void            collect_param(uns8 *, uns8 *, char* args[]);
void                   logme(uns32, uns8 *, ...);
static uns8 *          gettoken(uns8 **, uns32 );
static void            add2spawnlist(NID_SPAWN_INFO * );
static NID_SVC_CODE    get_servcode(uns8 * );
static NID_APP_TYPE    get_apptype(uns8 * );
static uns32           get_spawn_info(uns8 *,NID_SPAWN_INFO * ,uns8 *);
static uns32           parse_nodeinitconf(uns8 *strbuf);
static uns32           check_process(NID_SPAWN_INFO * service);
static void            cleanup(NID_SPAWN_INFO * service);
static uns32           recovery_action(NID_SPAWN_INFO *,uns8 *);
static void            insert_role_svcid(NID_SPAWN_INFO *);
static uns32           spawn_services(uns8 * );
static void            sigchld_handlr(int );
static void            daemonize_me(void);
static void            cons_init(void);
static uns32           cons_open(uns32);
#if 0
static void            print_nodeinitconf(void);
#endif
static void            notify_bis(uns8 *message);
static uns8 *          strtolower(uns8 * );
static uns32           getrole(void);
static uns32           get_role_from_rdf(void);
static void            nid_sleep(uns32);
       uns32           nid_get_board_type(uns8 *srcstr,uns8 *strbuf);
static void            sys_fw_prog(int32, NID_SVC_CODE,enum fw_prog_notify);
static uns32           setled_Inservice(void);


/******************************************************************
*       List of recovery strategies                               *
******************************************************************/
NID_FUNC      recovery_funcs[]={spawn_wait/*,bladereset*/};
NID_FORK_FUNC fork_funcs[]={fork_process,fork_script,fork_daemon};
uns8          *nid_log_path;


uns8 * nid_recerr[NID_MAXREC][4]={ {"Trying To RESPAWN","Could Not RESPAWN",\
                                    "Succeeded To RESPAWN","FAILED TO RESPAWN"},\
                                   {"Trying To RESET","Faild to RESET",\
                                    "suceeded To RESET","FAILED AFTER RESTART"}
                                 };
/* vivek_nid */
FW_PROG_SNSR  nid_serv_snsr_map[NID_MAXSERV][NID_MAX_NOTIFY]={ \
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: HPM Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_HPM_INIT_FAIL,\
                  "HAPS: HPM Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: IPMC Upgrade Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_IPMC_UPGRADE_FAIL,\
                  "HAPS: IPMC Upgrade Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_PHOENIXBIOS_UPGRADE_SUCCESS,\
                  "HAPS: BIOS Upgrade Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_PHOENIXBIOS_UPGRADE_FAIL,\
                  "HAPS: BIOS Upgrade Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: HLFM Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_HLFM_INIT_FAIL,\
                  "HAPS: HLFM Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: RDF Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_RDF_INIT_FAIL,\
                  "NCS: RDF Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: OpenHPI Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_OPENHPI_INIT_FAIL,\
                  "HAPS: OpenHPI Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: XND Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_XND_INIT_FAIL,\
                  "HAPS: XND Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: LHCPD Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_LHCD_INIT_FAIL,\
                  "HAPS: LHCD Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: LHCR Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_LHCR_INIT_FAIL,\
                  "HAPS: LHCR Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS: Networking Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_NWNG_INIT_FAIL,\
                  "HAPS: Networking Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_HAPS_INIT_SUCCESS,\
                  "HAPS Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_DRBD_INIT_FAIL,\
                  "HAPS: DRBD Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: TIPC Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_TIPC_INIT_FAIL,\
                  "NCS: TIPC Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: IFSVDeviceDiscovery Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_IFSVDD_INIT_FAIL,\
                  "NCS: IFSVDeviceDiscovery Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: SNMPD Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_SNMPD_INIT_FAIL,\
                  "NCS: SNMPD Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS initialization Successful"},\
                 {NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_FAIL,\
                  "NCS Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: DLSV Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_DLSV_INIT_FAIL,\
                  "NCS: DLSV Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: MASV Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_MASV_INIT_FAIL,\
                  "NCS: MASV Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: PSSV Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_PSSV_INIT_FAIL,\
                  "NCS: PSSV Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: GLND Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_GLND_INIT_FAIL,\
                  "NCS: GLND Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: EDSV Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_EDSV_INIT_FAIL,\
                  "NCS: EDSV Initialization Failed"}},\
                {{NID_NOTIFY_DISABLE,MOTO_FIRMWARE_NCS_INIT_SUCCESS,\
                  "NCS: SUBAGT Initialization Success"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_SUBAGT_INIT_FAIL,\
                  "NCS: SUBAGT Initialization Failed"}},\
                {{NID_NOTIFY_ENABLE,MOTO_FIRMWARE_SYSINIT_SUCCESS,\
                  "Node Initialization Successful"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_NCS_INIT_FAIL,\
                  "SCAP Initialization Failed"}},\
                {{NID_NOTIFY_ENABLE,MOTO_FIRMWARE_SYSINIT_SUCCESS,\
                  "Node initialization Successful"},\
                 {NID_NOTIFY_ENABLE,MOTO_FIRMWARE_NCS_INIT_FAIL,\
                  "PCAP Initialization Failed"}}\
   };



/****************************************************************************
 * Name          : nid_sleep                                                *
 *                                                                          *
 * Description   : Select based sleep upto milli secs granularity           *
 *                                                                          *
 * Arguments     : time_in _msec- time to sleep for milli secs              *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void nid_sleep(uns32 time_in_msec)
{
   struct timeval tv;

   tv.tv_sec = time_in_msec/1000;
   tv.tv_usec = ((time_in_msec)%1000)*1000;

   while(m_NCSSOCK_SELECT(0, 0, 0, 0, &tv)!= 0)
        if(errno == EINTR) continue;
}


/****************************************************************************
 * Name          : setled_Inservice                                         *
 *                                                                          *
 * Description   : Uses ipmi to set GREEN LED on                            *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
setled_Inservice(void)
{
   NID_BOARD_IPMC_INTF_CONFIG ipmc_inf_conf    = nid_ipmc_board[nid_board_type];
   NID_OPEN_IPMC_DEV          open_ipmc_dev    = ipmc_inf_conf.nid_open_ipmc_dev;
   NID_SEND_IPMC_CMD          send_ipmc_cmd    = ipmc_inf_conf.nid_send_ipmc_cmd;
   NID_PROC_IPMC_RSP          process_ipmc_rsp = ipmc_inf_conf.nid_process_ipmc_resp;
#if 0
   NIDGETLED                  lc_get_red_led,lc_get_green_led;
#endif
   NIDSETLED                  lc_set_led; 
   uns8                       strbuff[200];


   if ( open_ipmc_dev(strbuff) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;      
   /****************************************************
   * Get LED status before setting inservice LED on    *
   ****************************************************/
#if 0
   lc_get_red_led.fruid = 0x00;
   lc_get_red_led.led   = LED1;
   if ( send_ipmc_cmd(NID_IPMC_CMD_LED_GET, (NID_IPMC_EVT_INFO *)&lc_get_red_led) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;
   if ( process_ipmc_rsp((NID_IPMC_EVT_INFO*)&lc_get_red_led) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;

   lc_get_green_led.fruid = 0x00;
   lc_get_green_led.led   = LED2;
   if ( send_ipmc_cmd(NID_IPMC_CMD_LED_GET, (NID_IPMC_EVT_INFO *)&lc_get_green_led) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;
   if ( process_ipmc_rsp((NID_IPMC_EVT_INFO*)&lc_get_green_led) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;

   /****************************************************
   * Switch-off RED led if its on                      *
   ****************************************************/
   if ( lc_get_red_led.overrideFunction == LED_ON )
   {
#endif
     memset( &lc_set_led,0,sizeof(lc_set_led));
     lc_set_led.fruid = 0x00;
     lc_set_led.ledId   = LED1;
     lc_set_led.ledFunction = LED_OFF;
     lc_set_led.onDuration = 0x00;
     lc_set_led.ledColor = RED;

     if ( send_ipmc_cmd(NID_IPMC_CMD_LED_SET,(NID_IPMC_EVT_INFO *)&lc_set_led) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

     if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS )
         open_ipmc_dev(strbuff);      
#if 0
   }

   /****************************************************
   * Switch-on GREEN led if its on                     *
   ****************************************************/
   if ( lc_get_green_led.overrideFunction == LED_OFF )
   {
#endif
     memset( &lc_set_led,0,sizeof(lc_set_led));
     lc_set_led.fruid = 0x00;
     lc_set_led.ledId   = LED2;
     lc_set_led.ledFunction = LED_ON;
     lc_set_led.onDuration  = 0x00;


     /*********************************************
     * FIXME: Need to enable once AVM is enhanced *
     * to set LED's on standby and active         *
     * accordingly                                *
     *********************************************/
#if 0
     if (role == SA_AMF_HA_ACTIVE) 
     {
        lc_set_led.ledFunction = LED_ON;
        lc_set_led.onDuration  = 0x00; 
     }
     else
     {
       lc_set_led.ledFunction = 0x10;
       lc_set_led.onDuration  = 0x64;
     }
#endif
     lc_set_led.ledColor = GREEN;
     if ( send_ipmc_cmd(NID_IPMC_CMD_LED_SET,(NID_IPMC_EVT_INFO *)&lc_set_led) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;
     if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;
#if 0
   }
#endif
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : notify_bis                                               *
 *                                                                          *
 * Description   : Writes the bid status to NIS pipe                        *
 *                                                                          *
 * Arguments     : messgae - Message to be sent    to bis                   *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ****************************************************************************/
void notify_bis(uns8 *message)
{
  uns32 size;
   size = m_NCS_STRLEN(message);
   if(m_NCS_POSIX_WRITE(nis_fifofd,message,size) != size)
     logme(NID_LOG2FILE,"Error writing to nis FIFO! Error:%s\n",strerror(errno));
   m_NCS_POSIX_CLOSE(nis_fifofd);
}


/****************************************************************************
 * Name          : logme                                                    *
 *                                                                          *
 * Description   : Abstraction to log the error messages to file and console*
 *           optionally                                                     *
 *                                                                          *
 *                                                                          *
 * Arguments     : level - Input arg, to specify whether to log to console  *
 *               or file or both.                                           *
 *                 s     - Test to be loged.                                *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ****************************************************************************/
void
logme(uns32 level,uns8 *s, ...)
{

   va_list arg_list;
   uns8 buf[256];

   va_start(arg_list,s);
   vsnprintf(buf,sizeof(buf), s, arg_list);
   va_end(arg_list);

   m_NCS_POSIX_OPENLOG("NID",LOG_CONS,LOG_LOCAL3);
   switch(level){

   case NID_LOG2CONS:
      m_NCS_POSIX_WRITE(cons_fd,buf,m_NCS_STRLEN(buf));
      break;
   case NID_LOG2FILE:
      m_NCS_POSIX_SYSLOG(LOG_LOCAL3|LOG_INFO,"%s",buf);
      break;

   case NID_LOG2FILE_CONS:
      m_NCS_POSIX_SYSLOG(LOG_LOCAL3|LOG_INFO,"%s",buf);
      m_NCS_POSIX_WRITE(cons_fd,buf,m_NCS_STRLEN(buf));
      break;
   }

}

/****************************************************************************
 * Name          : gettoken                                                 *
 *                                                                          *
 * Description   :  Parse and return the string speperated by tok           *
 *                                                                          *
 * Arguments     : str - input string to be parsed                          *
 *           tok - tokenizing charecter, in our case ':'                    *
 *                                                                          *
 * Return Values : token string seperated by "tok" or NULL if nothing       *
 *                                                                          *
 * Notes         : None.                                                    *
 ****************************************************************************/
uns8 *
gettoken(uns8 **str, uns32 tok)
{
   uns8 *p, *q;

   if((str == NULL) || (*str == 0) ||\
      (**str == '\n') || (**str == '\0'))
   return(NULL);

   p = q = *str;
   if( **str == ':')
   {
     *p++ = 0;
     *str = p;
     return(NULL);
   }

   while((*p != tok) && (*p != '\n') && *p) p++;

   if((*p == tok) || (*p == '\n'))
   {
     *p++ = 0;
     *str = p;
   }

   return q;
}

/****************************************************************************
 * Name          : add2spawnlist                                            *
 *                                                                          *
 * Description   : Insert the childinfo into global list "spawn_list"       *
 *           in FIFO order.                                                 *
 *                                                                          *
 * Arguments     : childinfo - contains child info to be inserted into      *
 *                            spawn_list                                    *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
add2spawnlist(NID_SPAWN_INFO * childinfo)
{

   if(spawn_list.head == NULL)
   {
     spawn_list.head = spawn_list.tail = childinfo;
     childinfo->next = NULL;
     spawn_list.count++;
     return;
   }

   spawn_list.tail->next = childinfo;
   spawn_list.tail = childinfo;
   childinfo->next = NULL;
   spawn_list.count++;

}

/****************************************************************************
 * Name          : get_servcode                                             *
 *                                                                          *
 * Description   : Given a service name it returns corresponding service    *
 *                 code                                                     *
 *                                                                          *
 * Arguments     : p  - input parameter service name.                       *
 *                             spawn_list                                   *
 *                                                                          *
 * Return Values : service code/SERVERR                                     *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
NID_SVC_CODE
get_servcode(uns8 * p)
{
   NID_SVC_CODE opt=NID_HPM;
   uns8 * s2, *s1 = p;
   if(p == NULL) return NID_SERVERR;

   while(opt != NID_MAXSERV)
   {
      s1 = p;
      s2 = nid_serv_stat_info[opt].nid_serv_name;

      while(*s1++ == *s2++)

      if(*s1 == '\0')
      {
        if(*s2 == '\0') return opt;
        else return NID_SERVERR;
      } else if(*s2 == '\0') return NID_SERVERR;

      opt++;
   }
   return NID_SERVERR;
}


/****************************************************************************
 * Name          : get_apptype                                              *
 *                                                                          *
 * Description   : Given application type returns application internal code *
 *                                                                          *
 *                                                                          *
 * Arguments     : p  - input parameter application type.                   *
 *                                                                          *
 *                                                                          *
 * Return Values : Application type code/APPERR                             *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
NID_APP_TYPE
get_apptype(uns8 * p)
{
   NID_APP_TYPE type=NID_APPERR;

   if(p == NULL) return type;

   if(*p == 'E') type = NID_EXEC;
   else if(*p == 'S') type = NID_SCRIPT;
   else if(*p == 'D') type = NID_DAEMN;

   return type;
}



/****************************************************************************
 * Name          : get_spawn_info                                           *
 *                                                                          *
 * Description   : Parse one entry in /etc/opt/opensaf/nodeinit.conf file and  *
 *           extract the fields into "spawninfo".                           *
 *                                                                          *
 *                                                                          *
 * Arguments     : srcstr - One entry in /etc/opt/opensaf/nodeinit.conf to be  *
 *           parsed.                                                        *
 *           spawninfo - output buffer to fill with NID_SPAWN_INFO          *
 *                 sbuf - Buffer for returning error messages               *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
get_spawn_info(uns8 *srcstr,NID_SPAWN_INFO * spawninfo,uns8 *sbuf)
{
   uns8 *p,*q;
   NID_PLATCONF_PARS parse_state=NID_PLATCONF_SFILE;

   p = srcstr;

   while(parse_state != NID_PLATCONF_END)
   {

   switch(parse_state)
   {
   case NID_PLATCONF_SFILE:
      if(p[0] == ':')
      {
        sysf_sprintf(sbuf,": Missing application file name in file:"NID_PLAT_CONF);
        break;
      }
      q = gettoken(&p,':');
      if(m_NCS_STRLEN(q) > NID_MAXSFILE)
      {
        sysf_sprintf(sbuf,": App file name length exceeded max:%d in file" NID_PLAT_CONF,NID_MAXSFILE);
        break;
      }
      m_NCS_STRCPY(spawninfo->s_name,q);
      if(spawninfo->s_name[0] != '/')
      {
        sysf_sprintf(sbuf,": Not an absolute path: %s",spawninfo->s_name);
        break;
      }
      if((p == NULL) || (*p == '\0') )
      {
        sysf_sprintf(sbuf,": Missing app name in file:"NID_PLAT_CONF);
        break;
      }
      parse_state = NID_PLATCONF_SNAME;
      continue;

   case NID_PLATCONF_SNAME:
      if((p[0] == ':') || (p[0] == '\n'))
      {
        sysf_sprintf(sbuf,": Missing app name in file:"NID_PLAT_CONF);
        break;
      }
      q = gettoken(&p,':');
      if(m_NCS_STRLEN(q) > NID_MAXSNAME)
      {
        sysf_sprintf(sbuf,": App name length exceeded max:%d in file" NID_PLAT_CONF,NID_MAXSNAME);
        break;
      }
      spawninfo->servcode = get_servcode(q);
      if(spawninfo->servcode < 0)
      {
        sysf_sprintf(sbuf,": Not an identified service,\"%s\"",q);
        break;
      }
      if((p == NULL) || (*p == '\0'))
      {
        sysf_sprintf(sbuf,": Missing file type in file:"NID_PLAT_CONF);
        break;
      }
      parse_state = NID_PLATCONF_APPTYP;
      continue;

   case NID_PLATCONF_APPTYP:
      if((p[0] == ':') || (p[0] == '\n'))
      {
        sysf_sprintf(sbuf,": Missing file type in file:"NID_PLAT_CONF);
        break;
      }
      q = gettoken(&p,':');
      if(m_NCS_STRLEN(q) > NID_MAXAPPTYPE_LEN)
      {
        sysf_sprintf(sbuf,": File type length exceeded max:%d in file" NID_PLAT_CONF,NID_MAXAPPTYPE_LEN);
        break;
      }
      spawninfo->app_type = get_apptype(q);
      if(spawninfo->app_type < 0)
      {
        sysf_sprintf(sbuf,": Not an identified file type,\"%s\"",q);
        break;
      }
      if(( p == NULL) || (*p == '\0'))
      {
        if((spawninfo->app_type == NID_SCRIPT))
          sysf_sprintf(sbuf,": Missing cleanup script in file:" NID_PLAT_CONF);
        else
           if((spawninfo->app_type == NID_EXEC) || (spawninfo->app_type == NID_DAEMN))
             sysf_sprintf(sbuf,": Missing timeout value in file:"NID_PLAT_CONF);
             break;
      }

      parse_state = NID_PLATCONF_CLNUP;
      continue;

   case NID_PLATCONF_CLNUP:
      if((p[0] == ':') || (p[0] == '\n'))
      {
        if((spawninfo->app_type == NID_SCRIPT))
        {
          sysf_sprintf(sbuf,": Missing cleanup script in file:"NID_PLAT_CONF);
          break;
        }
        else if((spawninfo->app_type == NID_EXEC) || (spawninfo->app_type == NID_DAEMN))
        {
           spawninfo->cleanup_file[0] = '\0';
           gettoken(&p,':');
           if((p == NULL) || (*p == '\0'))
           {
             sysf_sprintf(sbuf,": Missing timeout value in file:"NID_PLAT_CONF);
             break;
           }
           parse_state = NID_PLATCONF_TOUT;
           continue;
        }
      }
      q = gettoken(&p,':');
      if(m_NCS_STRLEN(q) > NID_MAXSFILE)
      {
        sysf_sprintf(sbuf,": Cleanup app file name length exceeded max:%d in file" NID_PLAT_CONF,NID_MAXSFILE);
        break;
      }

      m_NCS_STRCPY(spawninfo->cleanup_file,q);
      if(spawninfo->cleanup_file[0] != '/')
      {
        sysf_sprintf(sbuf,": Not an absolute path: %s",spawninfo->cleanup_file);
        break;
      }
      if((p == NULL) || (*p == '\0'))
      {
        sysf_sprintf(sbuf,": Missing timeout value in file:"NID_PLAT_CONF);
        break;
      }
      parse_state = NID_PLATCONF_TOUT;
      continue;

   case NID_PLATCONF_TOUT:
      if((p[0] == ':') || (p[0] == '\n'))
      {
        sysf_sprintf(sbuf,": Missing timeout value in file:"NID_PLAT_CONF);
        break;
      }
      q = gettoken(&p,':');
      if(m_NCS_STRLEN(q) > NID_MAX_TIMEOUT_LEN)
      {
        sysf_sprintf(sbuf,": Timeout field length exceeded max:%d in file" NID_PLAT_CONF,NID_MAX_TIMEOUT_LEN);
        break;
      }
      spawninfo->time_out = atoi(q);

      parse_state = NID_PLATCONF_PRIO;
      continue;

   case NID_PLATCONF_PRIO:
      q = gettoken(&p,':');
      if(q == NULL)
      {
        spawninfo->priority = 0;
        parse_state = NID_PLATCONF_RSP;
        continue;
      }
      else
      {
         if(m_NCS_STRLEN(q) > NID_MAX_PRIO_LEN)
         {
           sysf_sprintf(sbuf,": Priority field length exceeded max:%d in file"NID_PLAT_CONF,NID_MAX_PRIO_LEN);
           break;
         }
         spawninfo->priority = atoi(q);
         parse_state = NID_PLATCONF_RSP;
         continue;
      }

   case NID_PLATCONF_RSP:
      q = gettoken(&p,':');
      if(q == NULL)
      {
        spawninfo->recovery_matrix[NID_RESPAWN].retry_count = 0;
        spawninfo->recovery_matrix[NID_RESPAWN].action = recovery_funcs[NID_RESPAWN];
        parse_state = NID_PLATCONF_RST;
        continue;
      }
      else
      {
         if(m_NCS_STRLEN(q) > NID_MAX_RESP_LEN)
         {
           sysf_sprintf(sbuf,": Respawn field length exceeded max:%d in file"NID_PLAT_CONF,NID_MAX_RESP_LEN);
           break;
         }
         if((*q < '0') || (*q > '9'))
         {
           sysf_sprintf(sbuf,": Not a digit");
           break;
         }
         spawninfo->recovery_matrix[NID_RESPAWN].retry_count = atoi(q);
         spawninfo->recovery_matrix[NID_RESPAWN].action = recovery_funcs[NID_RESPAWN];
         parse_state = NID_PLATCONF_RST;
         continue;
      }

   case NID_PLATCONF_RST:
      q = gettoken(&p,':');
      if(q == NULL)
      {
        spawninfo->recovery_matrix[NID_RESET].retry_count = 0;
#if 0
        spawninfo->recovery_matrix[NID_RESET].action = recovery_funcs[NID_RESET];
#endif
        parse_state = NID_PLATCONF_SPARM;
        continue;
      }
      else
      {
         if(m_NCS_STRLEN(q) > NID_MAX_REST_LEN)
         {
           sysf_sprintf(sbuf,": Restart field length exceeded max:%d in file" NID_PLAT_CONF,NID_MAX_REST_LEN);
           break;
         }
         if((*q < '0') || (*q > '1'))
         {
          sysf_sprintf(sbuf,": Not a valid digit");
          break;
         }
         spawninfo->recovery_matrix[NID_RESET].retry_count = atoi(q);
#if 0
         spawninfo->recovery_matrix[NID_RESET].action = recovery_funcs[NID_RESET];
#endif
         parse_state = NID_PLATCONF_SPARM;
         continue;
      }
   case NID_PLATCONF_SPARM:
      q = gettoken(&p,':');
      if(q == NULL)
      {
        m_NCS_STRCPY(spawninfo->s_parameters," ");
        spawninfo->serv_args[1] = NULL;
        spawninfo->serv_args[0] = spawninfo->s_name;
        parse_state = NID_PLATCONF_CLNPARM;
        continue;
      }
      else
      {
         if(m_NCS_STRLEN(q) > NID_MAXPARMS)
         {
           sysf_sprintf(sbuf,": App param length exceeded max:%d in file" NID_PLAT_CONF,NID_MAXPARMS);
           break;
         }

         m_NCS_STRNCPY(spawninfo->s_parameters,q,NID_MAXPARMS);
         collect_param(spawninfo->s_parameters,spawninfo->s_name,spawninfo->serv_args);
         parse_state = NID_PLATCONF_CLNPARM;
         continue;
      }

   case NID_PLATCONF_CLNPARM:
      q = gettoken(&p,':');
      if(q == NULL)
      {
        m_NCS_STRCPY(spawninfo->cleanup_parms," ");
        spawninfo->clnup_args[1] = NULL;
        spawninfo->clnup_args[0] = spawninfo->cleanup_file;
        parse_state = NID_PLATCONF_END;
        continue;
      }
      else
      {
         if(m_NCS_STRLEN(q) > NID_MAXPARMS)
         {
         sysf_sprintf(sbuf,": App param length exceeded max:%d in file" NID_PLAT_CONF,NID_MAXPARMS);
         break;
         }
         m_NCS_STRNCPY(spawninfo->cleanup_parms,q,NID_MAXPARMS);
         collect_param(spawninfo->cleanup_parms,spawninfo->cleanup_file,spawninfo->clnup_args);
         parse_state = NID_PLATCONF_END;
         continue;
      }

    case NID_PLATCONF_END:
                          break;
   }

   if(parse_state != NID_PLATCONF_END) return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : nid_get_board_type                                       *
 *                                                                          *
 * Description   : Parse to check if its an IPMC BOARD type entry           *
 *                 and extract the board type.                              *
 *                                                                          *
 * Arguments     : srcstr- string to parse                                  *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
nid_get_board_type(uns8 *srcstr,uns8 *strbuf)
{
   uns8    *p;
   uns32   length=strlen(NID_IPMC_BOARD_ENTRY),i; 
   p  = srcstr;

   if (!m_NCS_STRNCMP(p,NID_IPMC_BOARD_ENTRY,length))
   { 
      p += length;
      if ( *p == ':' )
      { 
         p++;
         for ( i=0; i<NID_MAX_BOARDS; i++)
         {
           /* Exclude the next-line char from comparision */
           if (!m_NCS_STRNCMP(p,nid_board_types[i],(m_NCS_STRLEN(p)-1)))
           {
              nid_board_type = i; 
              return NCSCC_RC_SUCCESS;
           }
         }
         if ( i >= NID_MAX_BOARDS )
         {
           sysf_sprintf(strbuf,"No such board type found");
           return NCSCC_RC_FAILURE;
         }
      }
      else
      {
        sysf_sprintf(strbuf,"IPMC Board type, incorrect format");
        return NCSCC_RC_FAILURE; 
      }
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
 * Name          : parse_nodeinitconf                                       *
 *                                                                          *
 * Description   : Parse all the entries in /etc/opt/opensaf/nodeinit.conf *
 *           file and return intermittently with lineno where parsing       *
 *           error was found.                                               *
 *                                                                          *
 * Arguments     : strbuf- To return the error message string               *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                        *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
parse_nodeinitconf(uns8 *strbuf)
{

   NID_SPAWN_INFO *childinfo;
   uns8 buff[256],sbuf[200],nid_plat_conf_file[255], *nid_plat_conf_path, *ch, *ch1;
   uns32 lineno=0,retry=0;
   struct nid_resetinfo info={-1,-1};
   NCS_OS_FILE plat_conf,plat_conf_close;

   /*******************************************
   *    Check if we have config file       *
   *******************************************/
   if ( (nid_plat_conf_path = getenv("NID_CONFIG_PATH")) == NULL )
   {
      logme(NID_LOG2FILE,"NID_CONFIG_PATH not set. Default:%s",NID_PLAT_CONF_PATH);
      nid_plat_conf_path = NID_PLAT_CONF_PATH;
   }
   sysf_sprintf(nid_plat_conf_file,"%s/"NID_PLAT_CONF,nid_plat_conf_path);
   plat_conf.info.open.i_file_name = nid_plat_conf_file;
   plat_conf.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;
   if(m_NCS_OS_FILE(&plat_conf, NCS_OS_FILE_OPEN) != NCSCC_RC_SUCCESS)
   {
     sysf_sprintf(strbuf,"FAILURE: No "NID_PLAT_CONF" file found\n");
     return NCSCC_RC_FAILURE;
   }

#if 0
   /****************************************************
   *       Need to replace the reset count from        *
   *       /tmp/nidresetinfo incase if we were reset   *
   *       while recovering.                           *
   ****************************************************/
   rst_file.info.open.i_file_name = NID_RST_FILE;
   rst_file.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;
   if( m_NCS_OS_FILE(&rst_file, NCS_OS_FILE_OPEN) == NCSCC_RC_SUCCESS)
   {
     NCS_OS_FILE rstfile,close_fd;
     rstfile.info.read.i_file_handle = rst_file.info.open.o_file_handle;
     rstfile.info.read.i_buf_size = sizeof(struct nid_resetinfo);
     rstfile.info.read.i_buffer = (uns8 *)&info;
     if(m_NCS_OS_FILE(&rstfile,NCS_OS_FILE_READ) == NCSCC_RC_SUCCESS)
       close_fd.info.close.i_file_handle = rst_file.info.open.o_file_handle;
     m_NCS_OS_FILE(&close_fd,NCS_OS_FILE_CLOSE);
   }

   if ( !info.count ) m_NCS_POSIX_UNLINK(NID_RST_FILE);
#endif

   while(m_NCS_OS_FGETS(buff,sizeof(buff),(FILE *)plat_conf.info.open.o_file_handle))
   {

      lineno++;


      /************************************************
       *       Skip Comments and tab spaces in the     *
       *       beginning                               *
       ************************************************/
      ch = buff;

      while(*ch == ' ' || *ch == '\t') ch++;

      if(*ch == '#' || *ch == '\n') continue;


      /**************************************************
       *   In case if we have # somewhere in this line  *
       *   lets truncate the string from there          *
       *************************************************/
      if((ch1 = strchr(ch,'#')) != NULL){ *ch1++ = '\n'; *ch1 = '\0';}

      /****************************************************
       *       Allocate mem for new child info             *
       ****************************************************/

       while((childinfo =(NID_SPAWN_INFO*) m_NCS_OS_MEMALLOC(sizeof(NID_SPAWN_INFO),NULL)) == NULL)
       {
           if(retry++ == 5)
           {
             sysf_sprintf(strbuf,"FAILURE: Out of memory\n");
             return NCSCC_RC_FAILURE;
           }
           nid_sleep(1000);
       }

       /* Check if this is a board type entry? */
       if ( nid_board_type == NID_MAX_BOARDS )
       {
          if ( nid_get_board_type(ch,sbuf) != NCSCC_RC_SUCCESS)
          {
             sysf_sprintf(strbuf,"%s, At: %d\n",sbuf,lineno);
             return NCSCC_RC_FAILURE;
          }
          if ( nid_board_type != NID_MAX_BOARDS ) continue;
       }
       /****************************************************
        *       Clear the new child info struct             *
        ****************************************************/
       m_NCS_OS_MEMSET(childinfo,0,sizeof(NID_SPAWN_INFO));


       /****************************************************
        *       Parse each entry in the nodeinit.conf file  *
        ****************************************************/
       if(get_spawn_info(ch,childinfo,sbuf) != NCSCC_RC_SUCCESS)
       {
         sysf_sprintf(strbuf,"%s, At: %d\n",sbuf,lineno);
         return NCSCC_RC_FAILURE;
       }


       if( childinfo->servcode == info.faild_serv_code)
         childinfo->recovery_matrix[NID_RESET].retry_count = info.count;

       /****************************************************
        *      Add the new child info to spawn_list        *
        ****************************************************/
       add2spawnlist(childinfo);
   }

   /* FIXME:Need to raise a bug  */
   /* close the node_init_conf file once it is read,
      else this open fd will be passed to all its children */
   plat_conf_close.info.close.i_file_handle =  plat_conf.info.open.o_file_handle;
   if(m_NCS_OS_FILE(&plat_conf_close, NCS_OS_FILE_CLOSE) != NCSCC_RC_SUCCESS)
   {
     sysf_sprintf(strbuf,"Failed to close "NID_PLAT_CONF"\n");
     return NCSCC_RC_FAILURE;
   }

   /* Lets get the log files path */
   if ( (nid_log_path = getenv("NID_NCS_LOG_PATH")) == NULL )
   {                                                                                   
      logme(NID_LOG2FILE,"No NID_NCS_LOG_PATH env set. Default:%s",NID_NCSLOGPATH);
      nid_log_path = NID_NCSLOGPATH;
   }
   if ( nid_board_type == NID_MAX_BOARDS )
   {
     logme(NID_LOG2FILE_CONS,"IPMC_BOARD Type missing in "NID_PLAT_CONF"\n");
     return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;

}



/****************************************************************************
 * Name          : cons_init                                                *
 *                                                                          *
 * Description   : Set console_dev to a working console.                    *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : None                                                     *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
cons_init(void)
{
   int32 fd;
   uns32 tried_devcons = 0;
   uns32 tried_vtmaster = 0;
   uns8 *s;

   if ((s = (uns8 *)getenv("CONSOLE")) != NULL)
      cons_dev = s;
   else
   {
      cons_dev = NID_CNSL;
      tried_devcons++;
   }

   while ((fd = m_NCS_POSIX_OPEN(cons_dev, O_RDONLY|O_NONBLOCK)) < 0)
   {
     if (!tried_devcons)
     {
        tried_devcons++;
        cons_dev = NID_CNSL;
        continue;
     }
     if (!tried_vtmaster)
     {
      tried_vtmaster++;
      cons_dev = NID_VT_MASTER;
      continue;
     }
     break;
   }
   if (fd < 0)
      cons_dev = "/dev/null";
   else
      m_NCS_POSIX_CLOSE(fd);
}


/****************************************************************************
 * Name          : cons_open                                                *
 *                                                                          *
 * Description   : Open the console device.                                 *
 *                                                                          *
 * Arguments     : mode - input parameter specifies open for readonly/read  *
 *           write etc...                                                   *
 *                                                                          *
 * Return Values : file descriptor.                                         *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
cons_open(uns32 mode)
{
   int32 f, fd = -1;
   uns32 m;

   cons_init();

   /*Open device in non blocking mode*/
   m = mode|O_NONBLOCK;

   for(f=0; f<5; f++)
    if((fd = m_NCS_POSIX_OPEN(cons_dev,m)) >= 0) break;

   if(fd < 0) return fd;

   if(m != mode)
    fcntl(fd,F_SETFL,mode);

   return fd;
}





/****************************************************************************
 * Name          : print_nodeinitconf                                       *
 *                                                                          *
 * Description   : Prints the contents of global spawn list.                *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
#if 0
void
print_nodeinitconf(void)
{
   NID_SPAWN_INFO *ch;
   NID_CHILD_LIST sp_list = spawn_list;
   while(sp_list.head != NULL)
   {
       ch = sp_list.head;
       printf("\t Aplication Name: %s\n    \
               \t Timeout:         %d\n    \
               \t Respawn Count:   %d\n    \
               \t Restart Count:   %d\n    \
               \t App Parameters:  %s\n"    \
              ,ch->s_name,ch->time_out,ch->recovery_matrix[NID_RESPAWN].retry_count\
              ,ch->recovery_matrix[NID_RESET].retry_count,ch->s_parameters);
       sp_list.head = sp_list.head->next;
   }
}
#endif



/****************************************************************************
 * Name          : fork_daemon                                              *
 *                                                                          *
 * Description   : Creates a daemon out of noraml process                   *
 *                                                                          *
 * Arguments     : app - Application file name to be spawned                *
 *           args - Application arguments                                   *
 *           strbuff - Return error message string, not used for now        *
 *                                                                          *
 * Return Values : Process ID of the daemon forked.                         *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
int32
fork_daemon(NID_SPAWN_INFO * service, uns8 * app,char * args[],uns8 *nid_log_filename,uns8 * strbuff) /* DEL */
{

   int32 pid = -1;
   uns32 f;
   int tmp_pid = -1;
   int32 prio_stat = -1;
   NID_CLOSE_IPMC_DEV close_ipmc_intf = nid_ipmc_board[nid_board_type].nid_close_ipmc_dev;
   sigset_t nmask,omask;
   struct sigaction sa;
   int filedes[2];
   int32 n;
   fd_set  set;
   struct timeval tv;

   /********************************************************
   *       Block sigchild while forking.                   *
   *******************************************************/
   m_NCS_POSIX_SIGEMPTYSET(&nmask);
   m_NCS_POSIX_SIGADDSET(&nmask, SIGCHLD);
   m_NCS_POSIX_SIGPROCMASK(SIG_BLOCK, &nmask, &omask);

   /******************************************************
   *  Need to run with the highest priority among spawned *
   *  else we may not get chance to try recovery before   *
   *  BMC watchdog expires.                               *
   *******************************************************/
   if (service)
   {
     if ( nid_current_prio > service->priority )
     {
       nid_current_prio = service->priority -1;
       setpriority(PRIO_PROCESS,0,nid_current_prio);
     }
   }

   m_NCS_POSIX_PIPE(filedes);

   if((pid = m_NCS_POSIX_FORK()) == 0)
   {

     if (nis_fifofd > 0) m_NCS_POSIX_CLOSE(nis_fifofd);
     if((tmp_pid = m_NCS_POSIX_FORK()) > 0)
     {
             exit(0);
     }

     /* We dont need reader open here */
     m_NCS_POSIX_CLOSE(filedes[0]);

     SETSIG(sa, SIGPIPE,  SIG_IGN, 0);

     tmp_pid = getpid();
     /*write tmp_pid to filedes[1]*/
     while(m_NCS_POSIX_WRITE(filedes[1],&tmp_pid,sizeof(int)) < 0)
        if (errno == EINTR) continue;
        else if(errno == EPIPE)
        {
           logme(NID_LOG2FILE,"Reader not available to return my PID\n");
           exit(2);
        }
        else logme(NID_LOG2FILE,"Problem writing to pipe! Error:%s",strerror(errno));

     setsid();

     m_NCS_POSIX_CLOSE(0);m_NCS_POSIX_CLOSE(1);m_NCS_POSIX_CLOSE(2);
     close_ipmc_intf();

     prio_stat = setpriority(PRIO_PROCESS,0,service->priority);
     if(prio_stat < 0) logme(NID_LOG2FILE,"Failed setting priority for %s",nid_serv_stat_info[service->servcode].nid_serv_name);
     else logme(NID_LOG2FILE,"%s Service has been set a priority value of %d",nid_serv_stat_info[service->servcode].nid_serv_name,prio_stat);

     if ( nid_log_filename )
     {
        if(( f = m_NCS_POSIX_OPEN(nid_log_filename,O_CREAT | O_RDWR | O_APPEND,S_IRWXU)) >= 0)
        {
          m_NCS_POSIX_DUP(f);
          m_NCS_POSIX_DUP(f);
        }
        else logme(NID_LOG2FILE,"Failed opening file: %s",nid_log_filename);
     }

     umask(027);

     /* Reset all the signals */
     for(f = 1; f < NSIG; f++) SETSIG(sa, f, SIG_DFL, SA_RESTART);
     m_NCS_POSIX_EXECVP(app,args);

     /***********************************************************
      *    Hope we never come here, incase if we are here       *
      *    Lets rest in peace.                                  *
      ***********************************************************/
     logme(NID_LOG2FILE_CONS,"Failed to exec while creating daemon:%s\n",strerror(errno));
     _exit(2);
   }

   /* We dont need writer open here */
   m_NCS_POSIX_CLOSE(filedes[1]);

   /*Lets not block indefinitely for reading pid */
   FD_ZERO(&set);
   FD_SET(filedes[0],&set);
   tv.tv_sec = 5;
   tv.tv_usec = 0;
   while((n = m_NCSSOCK_SELECT(filedes[0] + 1, &set, NULL, NULL, &tv)) <= 0)
   {
        if (n == 0)
        {
           logme(NID_LOG2FILE,"Writer couldn't return PID\n");
           m_NCS_POSIX_CLOSE(filedes[0]);
           return tmp_pid;
        }
        if (errno == EINTR) continue;
   }

   while(m_NCS_POSIX_READ(filedes[0],&tmp_pid,sizeof(int)) < 0)
       if(errno == EINTR) continue;
       else break;

   m_NCS_POSIX_CLOSE(filedes[0]);

   m_NCS_POSIX_SIGPROCMASK(SIG_SETMASK, &omask, NULL);
   return tmp_pid;

}



/****************************************************************************
 * Name          : fork_script                                              *
 *                                                                          *
 * Description   : Spawns shell scripts with console on                     *
 *                                                                          *
 * Arguments     : app - Application file name to be spawned                *
 *           args - Application arguments                                   *
 *           strbuff - Return error message string, not used for now        *
 *                                                                          *
 * Return Values : Process ID of the script forked.(not usedful).           *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
int32
fork_script(NID_SPAWN_INFO * service, uns8 * app,char * args[],uns8 *nid_log_filename,uns8 * strbuff)
{

   int32 pid= -1;
   int32 f;
   int32 prio_stat = -1;
   NID_CLOSE_IPMC_DEV close_ipmc_intf = nid_ipmc_board[nid_board_type].nid_close_ipmc_dev;
   sigset_t nmask,omask;
   struct sigaction sa;

   /********************************************************
   *       Block sigchild while forking.                   *
   *******************************************************/
   m_NCS_POSIX_SIGEMPTYSET(&nmask);
   m_NCS_POSIX_SIGADDSET(&nmask, SIGCHLD);
   m_NCS_POSIX_SIGPROCMASK(SIG_BLOCK, &nmask, &omask);

   /******************************************************
   *  Need to run with the highest priority among spawned *
   *  else we may not get chance to try recovery before   *
   *  BMC watchdog expires.                               *
   *******************************************************/
   if (service)
   {
     if ( nid_current_prio > service->priority )
     {
       nid_current_prio = service->priority -1;
       setpriority(PRIO_PROCESS,0,nid_current_prio);
     }
   }

   if ((pid = m_NCS_POSIX_FORK()) == 0)
   {

      if (nid_is_ipcopen() == NCSCC_RC_SUCCESS) nid_close_ipc();
      if (nis_fifofd > 0) m_NCS_POSIX_CLOSE(nis_fifofd);
      m_NCS_POSIX_SIGPROCMASK(SIG_SETMASK, &omask, NULL);

      setsid();
      m_NCS_POSIX_CLOSE(0);
      m_NCS_POSIX_CLOSE(1);
      m_NCS_POSIX_CLOSE(2);
      close_ipmc_intf();
 
      prio_stat = setpriority(PRIO_PROCESS,0,service->priority);
      if(prio_stat < 0) logme(NID_LOG2FILE,"Failed setting priority for %s",nid_serv_stat_info[service->servcode].nid_serv_name);
      else logme(NID_LOG2FILE,"%s Service has been set a priority value of %d",nid_serv_stat_info[service->servcode].nid_serv_name,prio_stat);

     if ( nid_log_filename )
     {
        if(( f = m_NCS_POSIX_OPEN(nid_log_filename,O_CREAT | O_RDWR | O_APPEND,S_IRWXU)) >= 0)
        {
           m_NCS_POSIX_DUP(f);
           m_NCS_POSIX_DUP(f);
        }
        else logme(NID_LOG2FILE,"Failed opening file : %s",nid_log_filename);
     }

      /* Reset all the signals */
      for(f = 1; f < NSIG; f++) SETSIG(sa, f, SIG_DFL, SA_RESTART);
      m_NCS_POSIX_EXECVP(app,args);
      /***********************************************************
       *    Hope we never come here, incase if we are here          *
       *    Lets rest in peace.                      *
       ***********************************************************/
      logme(NID_LOG2FILE_CONS,"Failed to exec while forking script:%s\n",strerror(errno));
      _exit(2);
   }

   m_NCS_POSIX_SIGPROCMASK(SIG_SETMASK, &omask, NULL);
   return pid;

}



/****************************************************************************
 * Name          : fork_process                                             *
 *                                                                          *
 * Description   : Spawns shell normal unix                                 *
 *                                                                          *
 * Arguments     : app - Application file name to be spawned                *
 *           args - Application arguments                                   *
 *           strbuff - Return error message string, not used for now        *
 *                                                                          *
 * Return Values : Process ID of the process forked.                        *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
int32
fork_process(NID_SPAWN_INFO * service, uns8 * app,char * args[],uns8 *nid_log_filename,uns8 * strbuff)/* DEL */
{
   int32 pid = -1;
   int32 f;
   int32 prio_stat = -1;
   NID_CLOSE_IPMC_DEV close_ipmc_intf = nid_ipmc_board[nid_board_type].nid_close_ipmc_dev;
   sigset_t nmask,omask;
   struct sigaction sa;

   /********************************************************
   *       Block sigchild while forking.                   *
   *******************************************************/
   m_NCS_POSIX_SIGEMPTYSET(&nmask);
   m_NCS_POSIX_SIGADDSET(&nmask, SIGCHLD);
   m_NCS_POSIX_SIGPROCMASK(SIG_BLOCK, &nmask, &omask);

   /******************************************************
   *  Need to run with the highest priority among spawned *
   *  else we may not get chance to try recovery before   *
   *  BMC watchdog expires.                               *
   *******************************************************/   
   if (service)
   {
     if ( nid_current_prio > service->priority )
     {
       nid_current_prio = service->priority -1;
       setpriority(PRIO_PROCESS,0,nid_current_prio);
     }
   }
    
   if ((pid = m_NCS_POSIX_FORK()) == 0)
   {

      if (nid_is_ipcopen() == NCSCC_RC_SUCCESS) nid_close_ipc();
      if (nis_fifofd > 0) m_NCS_POSIX_CLOSE(nis_fifofd);

      m_NCS_POSIX_SIGPROCMASK(SIG_SETMASK, &omask, NULL);

      m_NCS_POSIX_CLOSE(0);m_NCS_POSIX_CLOSE(1);m_NCS_POSIX_CLOSE(2);
      close_ipmc_intf();
      
     if ( nid_log_filename )
     {
         if(( f = m_NCS_POSIX_OPEN(nid_log_filename,O_CREAT | O_RDWR | O_APPEND,S_IRWXU)) >= 0)
         {
            m_NCS_POSIX_DUP(f);
            m_NCS_POSIX_DUP(f);
         }
         else logme(NID_LOG2FILE,"Failed opening file : %s",nid_log_filename);
     }
 
     if ( service )
     {
          prio_stat = setpriority(PRIO_PROCESS,0,service->priority);
          if(prio_stat < 0) logme(NID_LOG2FILE,"Failed setting priority for %s,nid_serv_stat_info[service->servcode].nid_serv_name");
          else logme(NID_LOG2FILE,"%s Service has been set a priority value of %d",nid_serv_stat_info[service->servcode].nid_serv_name,prio_stat);

     }
     else
     {
          /******************************************
          * We need ipmicmd to be procssed quickly  *
          * so we increase its priority by some     *
          * fixed number                            *
          ******************************************/
          setpriority(PRIO_PROCESS,0,-6);
          logme(NID_LOG2FILE,"Args: %s, %s, %s, %s, %s\n",args[0],args[1],args[2],args[3],args[4]);
     }
     /* Reset all the signals */
      for(f = 1; f < NSIG; f++) SETSIG(sa, f, SIG_DFL, SA_RESTART);
      m_NCS_POSIX_EXECVP(app,args);

      /***********************************************************
       *    Hope we never come here, incase if we are here          *
       *    Lets rest in peace.                      *
       ***********************************************************/
      logme(NID_LOG2FILE_CONS,"Failed to exec: %s\n",strerror(errno));
      _exit(2);
   }
   m_NCS_POSIX_SIGPROCMASK(SIG_SETMASK, &omask, NULL);
   return pid;

}


/****************************************************************************
 * Name          : collect_param                                            *
 *                                                                          *
 * Description   : Given a string of parameters, it seperates the parameters*
 *            into array of strings.                                        *
 *                                                                          *
 * Arguments     : params - string of parameters                            *
 *           s_name _ Application name.                                     *
 *           args   - To return an array of seperated parameter             *
 *                    strings                                               *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
collect_param(uns8 *params,uns8 *s_name,char *args[])
{

   uns32 f;
   uns8 *ptr;

   ptr = params;
   for(f = 1; f <= NID_MAXARGS; f++)
   {
      /* Skip white space */
      while(*ptr == ' ' || *ptr == '\t') ptr++;
      args[f] = ptr;

      /* May be trailing space.. */
      if (*ptr == 0) break;

      /* Skip this `word' */
      while(*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '#') ptr++;

      /* If end-of-line, break */
      if (*ptr == '#' || *ptr == 0)
      {
         f++;
         *ptr = 0;
         break;
      }
      /* End word with \0 and continue */
      *ptr++ = 0;
   }
   args[f] = NULL;
   args[0] = s_name;

}


/****************************************************************************
 * Name          : strtolower                                               *
 *                                                                          *
 * Description   : Converts a given string to lower case.                   *
 *                                                                          *
 * Arguments     : p - Pointer to input string.                             *
 *                                                                          *
 * Return Values : NULL/pointer to converted string.                        *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns8 * strtolower(uns8 * p)
{
   uns8 * str = p;

   if(str == NULL) return NULL;

   while(*str != '\0')
   {
     if(*str >= 'A' && *str <= 'Z')
     *str = *str +'a' -'A';
     str++;
   }
   return p;
}

/****************************************************************************
 * Name          : nid_reset_bmc_watchdog                                   *
 *                                                                          *
 * Description   : Stop a BMC watchdog                                    *
 *                                                                          *
 * Arguments     :                                                          *
 *                 strbuff - Buffer to return error message if any.         *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
static uns32
nid_reset_bmc_watchdog(NID_SPAWN_INFO *service,uns8 *strbuf)
{
  NID_BOARD_IPMC_INTF_CONFIG ipmc_inf_conf    = nid_ipmc_board[nid_board_type];
  NID_OPEN_IPMC_DEV          open_ipmc_dev    = ipmc_inf_conf.nid_open_ipmc_dev;
  NID_SEND_IPMC_CMD          send_ipmc_cmd    = ipmc_inf_conf.nid_send_ipmc_cmd;
  NID_PROC_IPMC_RSP          process_ipmc_rsp = ipmc_inf_conf.nid_process_ipmc_resp;

  /**********************************************************
  *  Dont start watchdog if not enabled in nodeinit.conf    *
  **********************************************************/
  if ( !service->recovery_matrix[NID_RESET].retry_count ) return NCSCC_RC_SUCCESS;

  if ( open_ipmc_dev(strbuf) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

  /* Set the BMC watchdog parameters */
  if ( send_ipmc_cmd(NID_IPMC_CMD_WATCHDOG_RESET, NULL) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

  if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;

  return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : nid_stop_bmc_watchdog                                   *
 *                                                                          *
 * Description   : Stop a BMC watchdog                                    *
 *                                                                          *
 * Arguments     :                                                          *
 *                 strbuff - Buffer to return error message if any.         *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
static uns32
nid_stop_bmc_watchdog(NID_SPAWN_INFO *service,uns8 *strbuf)
{
  NID_BOARD_IPMC_INTF_CONFIG ipmc_inf_conf    = nid_ipmc_board[nid_board_type];
  NID_OPEN_IPMC_DEV          open_ipmc_dev    = ipmc_inf_conf.nid_open_ipmc_dev;
  NID_SEND_IPMC_CMD          send_ipmc_cmd    = ipmc_inf_conf.nid_send_ipmc_cmd;
  NID_PROC_IPMC_RSP          process_ipmc_rsp = ipmc_inf_conf.nid_process_ipmc_resp;
  NID_IPMC_EVT_INFO          lc_evt_info;

  /**********************************************************
  *  Dont start watchdog if not enabled in nodeinit.conf    *
  **********************************************************/
  if ( !service->recovery_matrix[NID_RESET].retry_count ) return NCSCC_RC_SUCCESS;

  if ( open_ipmc_dev(strbuf) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

  lc_evt_info.wdt_info.timer_use = NID_IPMC_WDT_STOP;
  lc_evt_info.wdt_info.timer_actions = NID_IPMC_WDT_NO_ACTION;
  lc_evt_info.wdt_info.pre_timeout_interval = 0x00;
  lc_evt_info.wdt_info.timer_expiration_flags_clear = NID_IPMC_WDT_DONT_CLEAR_FLAGS;
  lc_evt_info.wdt_info.initial_countdown_value_lsbyte = 0x00;
  lc_evt_info.wdt_info.initial_countdown_value_msbyte = 0x00;

  /* Set the BMC watchdog parameters */
  if ( send_ipmc_cmd(NID_IPMC_CMD_WATCHDOG_SET, &lc_evt_info) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

  if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;

  return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : nid_start_bmc_watchdog                                   *
 *                                                                          *
 * Description   : Starts a BMC watchdog                                    *
 *                                                                          *
 * Arguments     : service - service details for spawning.                  *
 *                 strbuff - Buffer to return error message if any.         *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
static uns32
nid_start_bmc_watchdog(NID_SPAWN_INFO *service,uns8 *strbuff)
{
  uns16                      timeout          = 0;
  NID_BOARD_IPMC_INTF_CONFIG ipmc_inf_conf    = nid_ipmc_board[nid_board_type];
  NID_OPEN_IPMC_DEV          open_ipmc_dev    = ipmc_inf_conf.nid_open_ipmc_dev;
  NID_SEND_IPMC_CMD          send_ipmc_dev    = ipmc_inf_conf.nid_send_ipmc_cmd;
  NID_PROC_IPMC_RSP          process_ipmc_rsp = ipmc_inf_conf.nid_process_ipmc_resp;
  NID_IPMC_EVT_INFO          lc_evt_info;

  /**********************************************************
  *  Dont start watchdog if not enabled in nodeinit.conf    *
  **********************************************************/
  if ( !service->recovery_matrix[NID_RESET].retry_count ) return NCSCC_RC_SUCCESS;

  if ( open_ipmc_dev(strbuff) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

  timeout = (service->time_out/10) + 300; /* is in units 10ms/unit convert to 100ms/unit*/  
  if ( service->time_out % 10 ) timeout++; /* roundoff */

  lc_evt_info.wdt_info.timer_use = NID_IPMC_WDT_DONT_STOP;
  lc_evt_info.wdt_info.timer_actions = NID_IPMC_WDT_HARD_RESET;
  lc_evt_info.wdt_info.pre_timeout_interval = 0x00;
  lc_evt_info.wdt_info.timer_expiration_flags_clear = NID_IPMC_WDT_DONT_CLEAR_FLAGS;
  lc_evt_info.wdt_info.initial_countdown_value_lsbyte = timeout & 0xFF;
  lc_evt_info.wdt_info.initial_countdown_value_msbyte =( timeout >> 8 )& 0xFF;

  /* Set the BMC watchdog parameters */
  if ( send_ipmc_dev(NID_IPMC_CMD_WATCHDOG_SET, &lc_evt_info) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

  if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;

  /* Reset the whatchdog, to start it */
  if ( send_ipmc_dev(NID_IPMC_CMD_WATCHDOG_RESET,NULL) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;
  
  if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS ) return NCSCC_RC_FAILURE;
  
  return NCSCC_RC_SUCCESS;    

}


/****************************************************************************
 * Name          : spawn_wait                                               *
 *                                                                          *
 * Description   : Spawns given service and waits for given time for        *
 *            service to respond. Error processing is done based on         *
 *            services response. Returns a failure if service doesent       *
 *            respond before timeout.                                       *
 *                                                                          *
 * Arguments     : service - service details for spawning.                  *
 *           strbuff - Buffer to return error message if any.               *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
spawn_wait(NID_SPAWN_INFO *service, uns8 *strbuff)
{
   int32 pid = -1,retry = 5;
   int32 n = 0;
   fd_set  set;
   NCS_OS_FILE fd1;
   struct timeval tv;
   NID_FIFO_MSG reqmsg;
   NID_SVC_CODE temp_serv_code;
   uns8 *magicno,*serv,*stat,*p;
   uns8 buff1[100],magic_str[15];
   uns8 nid_log_filename[100];

   /*******************************************************
   *    Clean previous messages in strbuff           *
   *******************************************************/
   strbuff[0] = '\0';


   /******************************************************
   *    Check if the service file exists only for the    *
   *    first time per service, as we may fail opening   *
   *    when we are here during recovery. because the    *
   *    process killed during cleanup might be still     *
   *    holding this file and we are trying to open here *
   *    for read and write which usually will not be     *
   *    allowed by OS reporting "file bussy" error.      *
   *        And Testing this opening for the first       *
   *    Time would serve following purposes:             *
   *    1. If the executable exists.                     *
   ******************************************************/
   if(service->pid == 0)
   {
     fd1.info.open.i_file_name = service->s_name;
     fd1.info.open.i_read_write_mask = NCS_OS_FILE_PERM_READ;
     if(m_NCS_OS_FILE(&fd1,NCS_OS_FILE_OPEN) != NCSCC_RC_SUCCESS)
     {
       if ( errno != ETXTBSY )
       {
          logme(NID_LOG2FILE_CONS,"Opening: %s Error: %s\n",service->s_name,strerror(errno));
          logme(NID_LOG2FILE_CONS,"Not spawning any services\n");
          logme(NID_LOG2FILE_CONS,"Please rectify"NID_PLAT_CONF" and restart the system\n");
          notify_bis("FAILED\n");
          _exit(1);
       }
     }
     else
     {
       NCS_OS_FILE close_fd;
       close_fd.info.close.i_file_handle = fd1.info.open.o_file_handle;
       m_NCS_OS_FILE(&close_fd,NCS_OS_FILE_CLOSE);
     }
   }


   /******************************************************
   *    By now fifo should be open, try once in case      *
   *    if its not.                                       *
   ******************************************************/
   if(nid_is_ipcopen() != NCSCC_RC_SUCCESS)
   {
     if(nid_create_ipc(strbuff) != NCSCC_RC_SUCCESS)   return NCSCC_RC_FAILURE;
     if(nid_open_ipc(&select_fd,strbuff) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;
   }

   /************************************************************
   *    Fork based on the application type, executable,        *
   *    script or daemon.                                      *
   ************************************************************/
   while(retry)
   {
        logme(NID_LOG2CONS,"Starting %s service... ",nid_serv_stat_info[service->servcode].nid_serv_name);
        sysf_sprintf(nid_log_filename,"%s/NID_%s.log",nid_log_path,nid_serv_stat_info[service->servcode].nid_serv_name);
        pid = (fork_funcs[service->app_type])(service,service->s_name,service->serv_args,nid_log_filename,strbuff);

        if(pid <= 0)
        {
          logme(NID_LOG2FILE_CONS,"Failed \nError forking: %s, Error: %s, PID: %d Retrying fork\n",\
          service->s_name,strerror(errno),pid);
          retry--;
          nid_sleep(1000);
          continue;
        } else break;

   }

   if(retry == 0)
   {
      logme(NID_LOG2FILE_CONS,"Unable to bring up: %s Error:%s\n",service->s_name,strerror(errno));
      logme(NID_LOG2FILE_CONS,"Exiting! Dropping to shell for trouble-shooting\n");
      notify_bis("FAILED\n");
      _exit(1);
   }

   service->pid = pid;


   /*******************************************************
   *    IF Everything is fine till now, wait till         *
   *    service notifies its initializtion status         *
   *******************************************************/

   FD_ZERO(&set);
   FD_SET(select_fd,&set);

   /**************************************
   *    its in centi sec                 *
   **************************************/
   tv.tv_sec = (service->time_out)/100;
   tv.tv_usec = ((service->time_out)%100)*10000;


   while((n = m_NCSSOCK_SELECT(select_fd + 1, &set, NULL, NULL, &tv)) <= 0)
   {
        if((errno == EINTR) && (n < 0))
        {
          FD_ZERO(&set);
          FD_SET(select_fd,&set);
          continue;
        }
        if(n == 0)
        {
          logme(NID_LOG2FILE_CONS,"Failed \n Timed-out for response from:%s\n",nid_serv_stat_info[service->servcode].nid_serv_name);
          return NCSCC_RC_FAILURE;
        }

   }

   /******************************************************************
   *  Read the message from FIFO and fill in structure.              *
   ******************************************************************/
     while((n = m_NCS_POSIX_READ(select_fd, buff1,sizeof(buff1))) <= 0)
     {
         if (errno == EINTR)
         {
            continue;
         }
         else
         {
            sysf_sprintf(strbuff,"Failed \nError reading NID FIFO: %d",errno);
            return NCSCC_RC_FAILURE;
         }
     }

     buff1[n] = '\0';
     p = buff1;
     if((magicno = gettoken(&p,':')) == NULL)
     {
       logme(NID_LOG2FILE,"Failed \nMissing magic no!!!\n");
       return NCSCC_RC_FAILURE;
     }
     if((serv = gettoken(&p,':')) == NULL)
     {
       logme(NID_LOG2FILE,"Failed \nMissing service code!!!\n");
       return NCSCC_RC_FAILURE;
     }
     if((stat = p) == NULL)
     {
       logme(NID_LOG2FILE,"Failed \nMissing status code!!!\n");
       return NCSCC_RC_FAILURE;
     }

     sysf_sprintf(magic_str,"%x",NID_MAGIC);
     strtolower(magicno);
     if(m_NCS_STRCMP(magic_str,magicno) == 0) reqmsg.nid_magic_no = NID_MAGIC;
     else reqmsg.nid_magic_no = -1;

     if(m_NCS_STRCMP(serv,nid_serv_stat_info[service->servcode].nid_serv_name) == 0)
       reqmsg.nid_serv_code = service->servcode;
     else reqmsg.nid_serv_code = get_servcode(serv);

     reqmsg.nid_stat_code = atoi(stat);

   if(reqmsg.nid_magic_no != NID_MAGIC)
   {
     sysf_sprintf(strbuff,"Failed \nReceived invalid message: %x",reqmsg.nid_magic_no);
     return NCSCC_RC_FAILURE;
   }


   /***********************************************************
   *    LOOKS LIKE CORRECT RESPONSE LETS PROCESS              *
   ***********************************************************/
   if((reqmsg.nid_serv_code >= NID_HPM) && (reqmsg.nid_serv_code < NID_MAXSERV))
   {

     if(reqmsg.nid_serv_code != service->servcode)
     {
       sysf_sprintf(strbuff,"Failed \nService code mismatch! Srvc spawned: %s, Srvc code received:%s",
                    nid_serv_stat_info[service->servcode].nid_serv_name,nid_serv_stat_info[reqmsg.nid_serv_code].nid_serv_name);
       return NCSCC_RC_FAILURE;
     }
     else if(reqmsg.nid_stat_code ==  NCSCC_RC_SUCCESS)
     {
        if (reqmsg.nid_serv_code == NID_BIOSUP)
        {
           sysf_sprintf(strbuff,"PHOENIX BIOS UPGRADED...");
           return NID_PHOENIXBIOS_UPGRADED;
        }
        {
           sysf_sprintf(strbuff,"Done.");
           return NCSCC_RC_SUCCESS;
        }
     }

     /* vivek_nid */
     /* If spawned script was for IPMC upgrade...and no upgrade is required...take that */
     /* as success case, but generate relevant log                                      */
     if ((reqmsg.nid_serv_code == NID_IPMCUP) && (reqmsg.nid_stat_code == NID_IPMC_VER_SAME))
     {
        sysf_sprintf(strbuff,"IPMC Version Same: Upgrade Not Required");
        return NCSCC_RC_SUCCESS;
     }
     /* If spawned script was for Phoenix BIOS upgrade...and no upgrade is required...take that */
     /* as success case, but generate relevant log                                              */
     if ((reqmsg.nid_serv_code == NID_BIOSUP) && (reqmsg.nid_stat_code == NID_PHOENIXBIOS_VER_SAME))
     {
        sysf_sprintf(strbuff,"BIOS Version Same: Upgrade Not Required");
        return NCSCC_RC_SUCCESS;
     }

     if((reqmsg.nid_serv_code > NID_NCS) && (reqmsg.nid_serv_code <= NID_PCAP))
        temp_serv_code = NID_NCS;
     else
        temp_serv_code = reqmsg.nid_serv_code;

     if((reqmsg.nid_stat_code > NCSCC_RC_SUCCESS) &&
        (reqmsg.nid_stat_code < nid_serv_stat_info[temp_serv_code].nid_max_err_code))
     {

       sysf_sprintf(strbuff,"Failed \n DESC:%s",\
           nid_serv_stat_info[temp_serv_code].stat_info[reqmsg.nid_stat_code - 2].nid_stat_msg);
       return NCSCC_RC_FAILURE;
      }
      else
      {
        sysf_sprintf(strbuff,"Failed \n Received invalid status code from: %s",\
            nid_serv_stat_info[reqmsg.nid_serv_code].nid_serv_name);
        return NCSCC_RC_FAILURE;
      }

   }
   else
   {
      sysf_sprintf(strbuff,"Failed \n Received invalid service code: %d",reqmsg.nid_serv_code);
      return NCSCC_RC_FAILURE;
   }

}

#if 0
/****************************************************************************
 * Name          : bladereset                                               *
 *                                                                          *
 * Description   : Invokes reboot command. Writes the reset count to a file *
 *            before going for reset. so that we wont reset beyond          *
 *            count times as a recovery.                                    *
 *                                                                          *
 * Arguments     : service - service details for refering reset count.      *
 *           strbuff - Not for any use right now.                           *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
bladereset(NID_SPAWN_INFO *service,uns8 *strbuff)
{
   char *args[3];
   uns32 i =0;
   NCS_OS_FILE rst_fd;

   /********************************************************
   *    Need To log the current reset count to             *
   *    RST_FILE, before we reset this blade, so that      *
   *    we know the count next time we come up             *
   ********************************************************/
   rst_fd.info.create.i_file_name = NID_RST_FILE;
   if(m_NCS_OS_FILE(&rst_fd, NCS_OS_FILE_CREATE) == NCSCC_RC_SUCCESS)
   {
     struct nid_resetinfo info;
     NCS_OS_FILE  write_rstfd,close_fd;
     info.faild_serv_code = service->servcode;
     info.count = service->recovery_matrix[NID_RESET].retry_count -1;
     write_rstfd.info.write.i_file_handle =  rst_fd.info.create.o_file_handle;
     write_rstfd.info.write.i_buf_size = sizeof(struct nid_resetinfo);
     write_rstfd.info.write.i_buffer = (uns8 *)&info;
     if(m_NCS_OS_FILE(&write_rstfd,NCS_OS_FILE_WRITE) != NCSCC_RC_SUCCESS)
       logme(NID_LOG2FILE,"WARNING: Couldn't write reset count to"NID_RST_FILE"\n");

     close_fd.info.close.i_file_handle = rst_fd.info.create.o_file_handle;
     m_NCS_OS_FILE(&close_fd,NCS_OS_FILE_CLOSE);
   }
   else
   {
      logme(NID_LOG2FILE,"WARNING: Couldn't open "NID_RST_FILE"\n");
   }

   /********************************************************
   *    Send system Firmware Progress Event                *
   ********************************************************/
   /* NID_SYSFW_PROG(service->servcode,NID_NOTIFY_FAILURE); */

   /********************************************************
   *    Bye Bye Going to suicide                           *
   ********************************************************/
   args[i++] = "reboot";
   args[i++] = NULL;
   m_NCS_POSIX_EXECVP("/sbin/reboot",args);
   m_NCS_POSIX_EXECVP("/bin/reboot",args);

   logme(NID_LOG2FILE_CONS,"Failed to reboot! Dropping Tt shell\n");
   logme(NID_LOG2FILE_CONS,"NID EXITING!!!!\n");
   notify_bis("FAILED\n");
   _exit(1);

}
#endif

/****************************************************************************
 * Name          : check_process                                            *
 *                                                                          *
 * Description   : Checks if the process is still running.                  *
 *                                                                          *
 * Arguments     : service - service details.                               *
 *                                                                          *
 * Return Values : 0 - process not running.                                 *
 *            1 - process running.                                          *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
check_process(NID_SPAWN_INFO * service)
{
   struct stat sb;
   uns8 buf[32];

   sysf_sprintf(buf, "/proc/%d",service->pid);
   if(stat(buf, &sb) != 0) return 0;
   else return 1;
}


/****************************************************************************
 * Name          : cleanup                                                  *
 *                                                                          *
 * Description   : Does cleanup of the process spawned as a previous        *
 *            retry.                                                        *
 *            * cleanup of unix daemons and noraml process is through       *
 *            kill.                                                         *
 *            * cleanup of services invoked by scripts is done through      *
 *            corresponding cleanup scripts.                                *
 *                                                                          *
 * Arguments     : service - service details of the service to be cleaned.  *
 *           strbuff - Buffer to return error message if any.               *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
cleanup(NID_SPAWN_INFO* service)
{
   uns8 strbuff[256];

   /*****************************************************
   *    Dont Allow anyone to write to this pipe till we *
   *    start recovery action.                          *
   *****************************************************/
   nid_close_ipc();
   select_fd = -1;

   if(check_process(service))
   {
     logme(NID_LOG2FILE,"Sending SIGKILL to %s, PID: %d\n",\
           nid_serv_stat_info[service->servcode].nid_serv_name,service->pid);
     kill(service->pid,SIGKILL);
   }

   if(service->cleanup_file[0] != '\0')
   {
       (fork_funcs[service->app_type])(service, service->cleanup_file,\
                                       service->clnup_args,NULL,strbuff);
       nid_sleep(15000);
   }

   /*******************************************************
   *    YEPPP!!!! we need to slowdown before spawning,       *
   *    cleanup task may take time before its done with      *
   *    cleaning. Spawning before cleanup may really lead *
   *    to CCHHHHAAAAOOOOOSSSSS!!!!!!!!              *
   *******************************************************/
   nid_sleep(100);
}



/****************************************************************************
 * Name          : recovery_action                                          *
 *                                                                          *
 * Description   : Invokes all the recovery actions in sequence according   *
 *            to the recovery options specified in /etc/opt/opensaf/-      *
 *           nodeinit.conf file                                             *
 *           It invokes recovery action for the count specified in          *
 *           /etc/opt/opensaf/nodeinit.conf if the recovery failes.        *
 *                                                                          *
 * Arguments     : service - service details for spawning.                  *
 *           strbuff - Buffer to return error message if any.               *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
recovery_action(NID_SPAWN_INFO *service,uns8 *strbuff)
{
   uns32 count = 0;
   NID_RECOVERY_OPT opt = NID_RESPAWN;
   FILE *fp=NULL;

   while(opt != NID_RESET)
   {
       count = service->recovery_matrix[opt].retry_count;
       while(service->recovery_matrix[opt].retry_count > 0)
       {
          logme(NID_LOG2FILE,"%s %s for: %d time...\n",nid_recerr[opt][0],\
          service->s_name,(count-service->recovery_matrix[opt].retry_count)+1);

          if ( nid_reset_bmc_watchdog(service, strbuff) != NCSCC_RC_SUCCESS )
             logme(NID_LOG2FILE,"Failed to reset BMC watchdog: %s\n",strbuff);


          /**************************************************************
           *    Just clean the stuff we created during prev retry        *
           **************************************************************/
          if(service->pid != 0) cleanup(service);

          /**************************************************************
           *    Done with cleanup so goahead with recovery               *
           **************************************************************/
          if((service->recovery_matrix[opt].action)(service,strbuff) != NCSCC_RC_SUCCESS)
          {
            service->recovery_matrix[opt].retry_count--;
            logme(NID_LOG2FILE,"%s %s\n",nid_recerr[opt][1],service->s_name);
            logme(NID_LOG2FILE_CONS,"%s\n",strbuff);
            continue;
          }
          else
          {
            /************************************************************
             *  Ahhhhhhh  lot of juggling, we wonn!!!!                  *
             *    lets go back home                                     *
             ************************************************************/
            logme(NID_LOG2CONS,"Done.\n");
            return NCSCC_RC_SUCCESS;
          }

       }

       if(service->recovery_matrix[opt].retry_count == 0)
       {
         if(count != 0)
         logme(NID_LOG2FILE_CONS,"%s\n",nid_recerr[opt][3]);
         opt++;
         continue;
       }

    }

    fp = fopen(NODE_HA_STATE,"w");
    logme(NID_LOG2FILE_CONS,"%s Initialization failed\n",nid_serv_stat_info[service->servcode].nid_serv_name);
    logme(NID_LOG2FILE_CONS,"Tried all recoveries, couldn't recover! NID exiting!!!!!\n");
    if(fp)
       fprintf(fp,"%s","OpenSAF Initialization Failed");
    if ( !service->recovery_matrix[NID_RESET].retry_count ) {
    if(fp)
       fprintf(fp,"%s","Dropping to shell for Trouble-shooting");
    logme(NID_LOG2CONS,"Dropping to shell for Trouble-shooting!!!\n");
    }
    if (fp)
    {
       fflush(fp); 
       fclose(fp);
    }
    return NCSCC_RC_FAILURE;

}


/****************************************************************************
 * Name          : sys_fw_prog                                              *
 *                                                                          *
 * Description   : Routine to log Display and send system firmware          *
 *            progress events.                                              *
 *                                                                          *
 * Arguments     :                                                          *
 *                                                                          *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
sys_fw_prog(int32 fw_prog_evt,NID_SVC_CODE servcode,enum fw_prog_notify status)
{
   NID_BOARD_IPMC_INTF_CONFIG ipmc_inf_conf    = nid_ipmc_board[nid_board_type];
   NID_OPEN_IPMC_DEV          open_ipmc_dev    = ipmc_inf_conf.nid_open_ipmc_dev;
   NID_SEND_IPMC_CMD          send_ipmc_dev    = ipmc_inf_conf.nid_send_ipmc_cmd;
   NID_PROC_IPMC_RSP          process_ipmc_rsp = ipmc_inf_conf.nid_process_ipmc_resp;
   NID_SYS_FW_PROG_INFO       lc_fw_prog;
   uns8  strbuff[255];
   strbuff[0] = '\0';

   if ( fw_prog_evt < 0 )  return;

   if(status == NID_NOTIFY_SUCCESS)
   {
     lc_fw_prog.event_type = 0x02;

     switch ( fw_prog_evt )
     {
       case MOTO_FIRMWARE_BOOT_SUCCESS:
                      lc_fw_prog.data2 = fw_prog_evt;
                      lc_fw_prog.data3 = 0xFF;
                      break;
       case MOTO_FIRMWARE_SYSINIT_SUCCESS:
                      lc_fw_prog.data2 = MOTO_FIRMWARE_OEM_CODE;
                      lc_fw_prog.data3 = fw_prog_evt;         
                      break;
/* vivek_nid */
       case MOTO_FIRMWARE_PHOENIXBIOS_UPGRADE_SUCCESS:
                      lc_fw_prog.data2 = MOTO_FIRMWARE_OEM_CODE;
                      lc_fw_prog.data3 = fw_prog_evt;
                      break; 
    
     }
   }
   else if(status == NID_NOTIFY_FAILURE)
   {
       lc_fw_prog.event_type = 0x00;
       lc_fw_prog.data2 = MOTO_FIRMWARE_OEM_CODE;
       lc_fw_prog.data3 = fw_prog_evt;
   }

   if ( open_ipmc_dev(strbuff) != NCSCC_RC_SUCCESS) return;

   /* Send firmware progress event */
   if ( send_ipmc_dev(NID_IPMC_CMD_SEND_FW_PROG, (NID_IPMC_EVT_INFO *)&lc_fw_prog) != NCSCC_RC_SUCCESS) return;

   if ( process_ipmc_rsp(NULL) != NCSCC_RC_SUCCESS ) return;

}

/****************************************************************************
 * Name          : getrole                                                  *
 *                                                                          *
 * Description   : Calles the LHCPD provided API to determine the role      *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 ****************************************************************************/
uns32
getrole(void)
{
   uns8 *nid_role;

   /* obtain role from RDF */
   if(get_role_from_rdf() == NCSCC_RC_FAILURE)
   {
      logme(NID_LOG2FILE, "ERROR: couldn't get role from RDF!");

      /* if role can't be obtained from RDF, then get/set environment variables */
      if (( nid_role = getenv("NID_ROLE_CONFIG")) != NULL)
      {
         logme(NID_LOG2FILE_CONS,"Assuming default role %s\n",nid_role);
         if (!strcmp(nid_role,"ACTIVE"))
            role = SA_AMF_HA_ACTIVE;
         else
            role = SA_AMF_HA_STANDBY;
      }

      else
      {
         logme(NID_LOG2FILE,"NID_ROLE_CONFIG not set. Default:%s","ACTIVE");
         role = SA_AMF_HA_ACTIVE;
      }
   }

   return NCSCC_RC_SUCCESS;

}


/****************************************************************************
 * Name          : get_role_from_rdf                                        *
 *                                                                          *
 * Description   : Obtains the initial role from RDF                        *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         :                                                          *
 *                                                                          *
 ***************************************************************************/
uns32 get_role_from_rdf(void)
{
   int rc;
   PCS_RDA_REQ pcs_rda_req;
   uns32 ret_val;

   /* initialise the RDA library */
   pcs_rda_req.req_type = PCS_RDA_LIB_INIT;

   if((rc = pcs_rda_request(&pcs_rda_req)) != PCSRDA_RC_SUCCESS)
   {
      ret_val = NCSCC_RC_FAILURE;
      goto go_back;
   }

   /* get initial HA role */
   pcs_rda_req.req_type = PCS_RDA_GET_ROLE;

   if((rc = pcs_rda_request(&pcs_rda_req)) != PCSRDA_RC_SUCCESS)
   {
      /* if there's any error getting the role, then don't return right away; destroy the library first */
      logme(NID_LOG2FILE_CONS,"Failed to get role from RDF\n");
      ret_val = NCSCC_RC_FAILURE;
   }

   else
   {
      switch(pcs_rda_req.info.io_role)
      {
      case PCS_RDA_ACTIVE:
         role = SA_AMF_HA_ACTIVE;
         ret_val = NCSCC_RC_SUCCESS;
         logme(NID_LOG2FILE_CONS,"RDF-ROLE for this System Controller is: %d, %s\n",pcs_rda_req.info.io_role,"ACTIVE");
         break;

      case PCS_RDA_STANDBY:
         role = SA_AMF_HA_STANDBY;
         ret_val = NCSCC_RC_SUCCESS;
         logme(NID_LOG2FILE_CONS,"RDF-ROLE for this System Controller is: %d, %s\n",pcs_rda_req.info.io_role,"STANDBY");
         break;

      default:
         ret_val = NCSCC_RC_FAILURE;
      };
   }

   /* destroy the RDA library */
   pcs_rda_req.req_type = PCS_RDA_LIB_DESTROY;

   if((rc = pcs_rda_request(&pcs_rda_req)) != PCSRDA_RC_SUCCESS)
   {
      ret_val = NCSCC_RC_FAILURE;
   }

go_back:
   return ret_val;
}


/****************************************************************************
 * Name          : insert_role_svcid                                        *
 *                                                                          *
 * Description   : Just to insert the role  and service id as the last      *
 *                 arguments for the service being spawned.                 *
 *                                                                          *
 * Arguments     : service - Service to which role argument needs to be     *
 *                 appended.                                                *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
insert_role_svcid(NID_SPAWN_INFO *service)
{
   uns32 f;

   for(f = 1; f <= NID_MAXARGS; f++)
   if(service->serv_args[f] == NULL) break;

   if( f >= (NID_MAXARGS-4) )
   {
     logme(NID_LOG2FILE_CONS,"Exceeding parameters for %s\n",\
           nid_serv_stat_info[service->servcode].nid_serv_name);
     logme(NID_LOG2FILE_CONS,"NID Exiting\n");
     _exit(1);
   }

   /*************************************************************
    *      Insert the role only if its available                *
    ************************************************************/
   if( role ){
       sysf_sprintf(rolebuff,"ROLE=%d",role);
       service->serv_args[f++] = rolebuff;
   }
     /* Set the service-id to be passed */
   sysf_sprintf(svc_id,"NID_SVC_ID=%d",service->servcode);
   service->serv_args[f++] = svc_id;

   service->serv_args[f] = NULL;

   if(service->cleanup_file[0] != '\0')
   {
     for(f = 1; f <= NID_MAXARGS; f++)
     if(service->clnup_args[f] == NULL) break;

     if( f >= (NID_MAXARGS-4) ){
       logme(NID_LOG2FILE_CONS,"Exceeding parameters for %s\n",\
           nid_serv_stat_info[service->servcode].nid_serv_name);
       logme(NID_LOG2FILE_CONS,"NID Exiting\n");
       _exit(1);
     }

     if( role ){
       if(service->app_type == NID_SCRIPT){
         if(role == SA_AMF_HA_ACTIVE) service->clnup_args[f++] = "ACTIVE";
         else if(role == SA_AMF_HA_STANDBY) service->clnup_args[f++] = "STDBY";
       }
       else {
         service->clnup_args[f++] = rolebuff;
       }
     }

     if(service->app_type == NID_SCRIPT)
       service->clnup_args[f++] = nid_serv_stat_info[service->servcode].nid_serv_name;
     else {
       service->clnup_args[f++] = svc_id;
     }
     service->clnup_args[f] = NULL;
   }
}



/****************************************************************************
 * Name          : spawn_services                                           *
 *                                                                          *
 * Description   : Takes the global spawn list and calls spawn_wait for each*
 *            service in the list, spawn wait retrns only after the         *
 *            service is spawned successfully.                              *
 *                                                                          *
 * Arguments     : strbuff - Buffer to return error message if any.         *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
uns32
spawn_services(uns8 * strbuf)
{

   NID_SPAWN_INFO *service;
   NID_CHILD_LIST sp_list = spawn_list;
   uns8 sbuff[100];
   uns32 rc = NCSCC_RC_FAILURE;

   if(sp_list.head == NULL)
   {
     sysf_sprintf(strbuf,"No services to spawn\n");
     return NCSCC_RC_FAILURE;
   }

   /******************************************************
   *     Create bid fifo                                 *
   ******************************************************/
   if(nid_create_ipc(strbuf) != NCSCC_RC_SUCCESS) return NCSCC_RC_FAILURE;

   /******************************************************
   *    Try to open FIFO                                 *
   ******************************************************/
   if(nid_open_ipc(&select_fd,strbuf) != NCSCC_RC_SUCCESS)  return NCSCC_RC_FAILURE;

   logme(NID_LOG2FILE_CONS,"Starting OpenSAF Services...\n");
   while(sp_list.head != NULL)
   {
       service = sp_list.head;

       /**************************************************
        *    Passing role as an argument for DRBD and    *
        *    SCAP.                                       *
        *************************************************/
       insert_role_svcid(service);
  
       /************************************************************
       * Start Watchdog timer before we even start process         *
       * There could be a possibility that a task being spawned is *
       * realtime, and if it malfunctions to hang the system       *
       ************************************************************/
       if ( nid_start_bmc_watchdog(service,strbuf) != NCSCC_RC_SUCCESS)
            logme(NID_LOG2FILE,"BMC watchdog start failed: %s\n",strbuf);    

       /* vivek_nid */   
       rc = spawn_wait(service,sbuff);

       if((service->servcode == NID_BIOSUP) && (rc == NID_PHOENIXBIOS_UPGRADED))
       {
          /* Send Firmware Progress Code */
          sys_fw_prog(((nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].notify == NID_NOTIFY_ENABLE) ? nid_serv_snsr_map[service->servcode][NID_NOTIFY_SUCCESS].data3:-1) ,service->servcode, NID_NOTIFY_SUCCESS);
          logme(NID_LOG2FILE_CONS,"NID Sent PHOENIX BIOS UPGRADE SUCCESS System Firmware Code\n");
          _exit(1);
       }

       if (rc != NCSCC_RC_SUCCESS)
       {
         logme(NID_LOG2FILE_CONS,"%s\n",sbuff);
         logme(NID_LOG2FILE_CONS,"Going for recovery\n",sbuff);
         if ((service->servcode == NID_BIOSUP) || (service->servcode == NID_IPMCUP))
         {
       sys_fw_prog(((nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].notify == NID_NOTIFY_ENABLE) ? nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].data3:-1)
                       ,service->servcode,NID_NOTIFY_FAILURE);
           notify_bis("FAILED\n");
           _exit(1);
         }
         else
         {
           if(recovery_action(service,sbuff) != NCSCC_RC_SUCCESS)
           {
             sys_fw_prog(((nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].notify == NID_NOTIFY_ENABLE) ? nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].data3:-1) ,service->servcode,NID_NOTIFY_FAILURE);
             notify_bis("FAILED\n");
             _exit(1);
           }
         }
       }

#if 0
       if(spawn_wait(service,sbuff) != NCSCC_RC_SUCCESS)
       {
         logme(NID_LOG2FILE_CONS,"%s\n",sbuff);
         logme(NID_LOG2FILE_CONS,"Going for recovery\n",sbuff);

         if(recovery_action(service,sbuff) != NCSCC_RC_SUCCESS)
         {
           sys_fw_prog(((nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].notify == NID_NOTIFY_ENABLE) ? nid_serv_snsr_map[service->servcode][NID_NOTIFY_FAILURE].data3:-1)
                       ,service->servcode,NID_NOTIFY_FAILURE);
           notify_bis("FAILED\n");
           _exit(1);
         }

         /******************************************************
          * We might never return here, in case we have to take *
          * ultimate decision..... YES... RESET                 *
          *******************************************************/
       }
#endif
       /*********************************************************
       * Service successfully spawned, lets stop BMC watchdog   *
       *********************************************************/
       if ( nid_stop_bmc_watchdog(service,strbuf) != NCSCC_RC_SUCCESS)
            logme(NID_LOG2FILE,"BMC watchdog stop failed: %s\n",strbuf);

       logme(NID_LOG2CONS,"%s\n",sbuff);

       /****************************************
        *  send system firmware progress        *
        ****************************************/
       {
         sys_fw_prog(((nid_serv_snsr_map[service->servcode][NID_NOTIFY_SUCCESS].notify 
                     == NID_NOTIFY_ENABLE) ? nid_serv_snsr_map[service->servcode][NID_NOTIFY_SUCCESS].data3:-1),
                     service->servcode,NID_NOTIFY_SUCCESS);
       }

       /*********************************************************
        *    We need to save the SCAP or PCAP PID in files,     *
        *    which will be used during shutdown.                *
        ********************************************************/
       if((service->servcode == NID_SCAP) || (service->servcode == NID_PCAP))
       {
         int32 lfd;
         uns8 filename[30],str[15];
         sysf_sprintf(filename,"/var/run/%s.pid","ncsspcap");
         m_NCS_POSIX_UNLINK(filename);
         lfd = m_NCS_POSIX_OPEN(filename,O_CREAT|O_WRONLY,S_IRWXU);
         sysf_sprintf(str,"%d\n",service->pid);
         m_NCS_POSIX_WRITE(lfd,str,m_NCS_STRLEN(str));
         m_NCS_POSIX_CLOSE(lfd);
       }

       /***********************************************************
        *    Determine the role from LHCPD If we are ACTIVE/STDBY.*
        *    Needed while initializing DRBD and SCAP/PCAP.        *
        ***********************************************************/
         if( ((service->servcode == NID_HLFM) || (service->servcode == NID_RDF)) && (!m_NCS_STRCMP(nid_board_types[nid_board_type],nid_board_types[NID_F101_AVR]) || (!m_NCS_STRCMP(nid_board_types[nid_board_type],nid_board_types[NID_PC_SCXB]))) )
       {
         if(getrole() != NCSCC_RC_SUCCESS)
         {
           logme(NID_LOG2FILE_CONS,"FAILED to determine role, stopped spawning\n");
           logme(NID_LOG2FILE_CONS,"NID Exiting\n");
           notify_bis("FAILED\n");
           _exit(1);
         }
       }

   sp_list.head = sp_list.head->next;

   }

   return NCSCC_RC_SUCCESS;
}



/****************************************************************************
 * Name          : sigchld_handlr                                           *
 *                                                                          *
 * Description   : Handler for SIGCHLD signal, to free zoombie process.     *
 *            stores pid of dead child in dead_child. Which ofcourse        *
 *            now is not in use.                                            *
 *                                                                          *
 * Arguments     : i - signal number received.                              *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
sigchld_handlr(int i)
{
   int j;
   dead_child = wait(&j);
}



/****************************************************************************
 * Name          : daemonize_me                                             *
 *                                                                          *
 * Description   : This routine daemonizes nodeinitd.Sets the sessio        *
 *            group leader and does some signal settings.                   *
 *                                                                          *
 * Arguments     : None.                                                    *
 *                                                                          *
 * Return Values : None.                                                    *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
void
daemonize_me(void)
{
   int32 pid,lfd;
   struct sigaction sa;
   uns8 str[10];

   /*****************************************************************
   *    Check if we are already daemon?                             *
   ******************************************************************/
   if(getppid() == 1) return;

   /******************************************************************
   *    exit if we fail to fork or if we are parent                  *
   *******************************************************************/
   if((pid = m_NCS_POSIX_FORK()) < 0)
   {
     logme(NID_LOG2FILE_CONS,"fork failed: %s\n",strerror(errno));
     exit(1);
   }
   else if (pid > 0) exit(0);

   /*******************************************************************
   *    We are shure to be child from here on Be process group leader *
   *    Be a session leader DOnt need a controllong terminal          *
   ********************************************************************/
#ifndef NID_LINUX_PC
   setsid();
#endif

   /********************************************************************
   *    Shell would have duplicated NIS fifo to our O/P descriptor "1".*
   *    Lets duplicate it before closing. I cant trust LEAP if it      *
   *    tries writing something to O/P desc I mean if it were only our *
   *    code things would have been in control but we have leaps       *
   *    library comming in so it may write something to O/P desc.      *
   *********************************************************************/
   if((nis_fifofd = m_NCS_POSIX_DUP(1)) < 0)
     logme(NID_LOG2FILE,"Failed To duplicate NIS FIFO\n");

   m_NCS_POSIX_CLOSE(0);m_NCS_POSIX_CLOSE(1);m_NCS_POSIX_CLOSE(2);
   if((cons_fd = cons_open(O_RDWR|O_NOCTTY)) >= 0)
   {
   /*  (void)ioctl(cons_fd, TIOCSCTTY, 1);*/
      m_NCS_POSIX_DUP(cons_fd);
      m_NCS_POSIX_DUP(cons_fd);
   }
   else logme(NID_LOG2FILE,"Failed to open console: %s\n",strerror(errno));

   umask(022);

   chdir(NID_RUNNING_DIR);
   lfd = m_NCS_POSIX_OPEN(NID_PID_FILE,O_CREAT|O_WRONLY,S_IRWXU);
   sysf_sprintf(str,"%d\n",getpid());
   m_NCS_POSIX_WRITE(lfd,str,m_NCS_STRLEN(str));
   m_NCS_POSIX_CLOSE(lfd);

   SETSIG(sa, SIGALRM,  SIG_IGN, 0);
   SETSIG(sa, SIGHUP,   SIG_IGN, 0);
   SETSIG(sa, SIGINT,   SIG_IGN, 0);
   SETSIG(sa, SIGCHLD,  sigchld_handlr, SA_RESTART);
   SETSIG(sa, SIGSTOP,  SIG_IGN, SA_RESTART);
   SETSIG(sa, SIGTSTP,  SIG_IGN, SA_RESTART);
   SETSIG(sa, SIGTERM,  SIG_IGN, 0);
   SETSIG(sa, SIGTTOU,  SIG_IGN, SA_RESTART);
   SETSIG(sa, SIGTTIN,  SIG_IGN, SA_RESTART);

   /*setpriority(PRIO_PROCESS,0,-7);*/
}

/****************************************************************************
 * Name          : main                                                      *
 *                                                                          *
 * Description   : daemonizes the nodeinitd.                                *
 *            parses the nodeinit.conf file.                                *
 *            invokes the services in the order mentioned in conf file      *
 *                                                                          *
 * Arguments     : argc - no of command line args                           *
 *           argv- List of arguments.                                       *
 *                                                                          *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.                       *
 *                                                                          *
 * Notes         : None.                                                    *
 ***************************************************************************/
int
main(int argc, char **argv)
{
   uns8 sbuf[256];


   /****************************************************************
   *    Daemonize because we dont want to be controlled by         *
   *    a terminal                                                 *
   ****************************************************************/
   daemonize_me();

   /****************************************************************
   *    TBD: Once we have LHC working, we are supposed to get role *
   *    after LHC comes up. For now we get it here/anywhere        *
   ****************************************************************/
   /*getrole();*/

   if(parse_nodeinitconf(sbuf) != NCSCC_RC_SUCCESS)
   {
      logme(NID_LOG2FILE_CONS,"Failed parsing "NID_PLAT_CONF" file\n%s",sbuf);
      logme(NID_LOG2FILE_CONS,"Not spawning any services\n");
      logme(NID_LOG2FILE_CONS,"Please rectify"NID_PLAT_CONF" and restart the system\n");
      notify_bis("FAILED\n");
      _exit(1);
   }

   /********************************************
    *  Send completed os boot firmware progress*
    *  event                                   *
    *******************************************/
   sys_fw_prog(MOTO_FIRMWARE_BOOT_SUCCESS,NID_MAXSERV,NID_NOTIFY_SUCCESS);


   if(spawn_services(sbuf) != NCSCC_RC_SUCCESS)
   {
      logme(NID_LOG2FILE_CONS,"FAILURE: Failed spawning service, %s\n",sbuf);
      notify_bis("FAILED\n");
      _exit(1);
   }

   /*******************************
   *    Lets set blade to IS      *
   *******************************/
   if ( setled_Inservice() != NCSCC_RC_SUCCESS)
    logme(NID_LOG2FILE,"Failed to switch on Inservice LED\n");

   logme(NID_LOG2FILE_CONS,"Node Initialization Successful. \n");
   logme(NID_LOG2CONS,"SUCCESSFULLY SPAWNED ALL SERVICES!!!\n");

   FILE *fp=NULL;
   fp=fopen(NODE_HA_STATE,"a");
   if(fp)
   {
     fprintf(fp,"%s","Node Initialization Successful.\n");
     fflush(fp);
     fclose(fp);
   }

   notify_bis("SUCCESS\n");

   return 0;
}
