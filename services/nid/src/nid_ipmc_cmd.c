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


#include "nid_ipmc_cmd.h"


extern int32 fork_process(NID_SPAWN_INFO * service, char * app, char * args[],uns8 *, uns8 * strbuff);
extern void  logme(uns32, uns8 *, ...);

NID_BOARD_IPMC_TYPE  nid_board_type = NID_MAX_BOARDS;

/* Buffer to read the response from IPMC */
static uns8          nid_ipmc_rsp_buff[255];

static uns32 nid_dummy_open_intf(uns8 *);
static void  nid_dummy_close_ipmc_intf( void );
static int32 nid_dummy_get_ipmc_dev_fd(void);
static int32 nid_dummy_send_ipmc_cmd(NID_IPMC_CMDS ipmc_cmd,NID_IPMC_EVT_INFO *evt_info);
static int32 nid_dummy_proc_ipmc_rsp(NID_IPMC_EVT_INFO *evt_info);
static uns32 nid_f101_open_intf ( uns8 *strbuff );
static void nid_f101_close_ipmc_intf( void );
static int32 nid_f101_get_ipmc_dev_fd(void);
static int32 nid_f101_send_ipmc(uns8 *cmdStr);
static int32 nid_f101_send_ipmc_cmd(NID_IPMC_CMDS ipmc_cmd,NID_IPMC_EVT_INFO *evt_info);
static uns8 nid_ipmi_parse_msg(ipmi_msg_t *ipmi_msg,const char  * response);
static int32 nid_f101_proc_ipmc_rsp(NID_IPMC_EVT_INFO *evt_info);
static void nid_ipmc_build_cmd(NID_IPMC_CMDS cmd, NID_IPMC_EVT_INFO *evt_info,uns8 *o_cmd);


/* String form of board types */
uns8 *nid_board_types[NID_MAX_BOARDS]={
                                         "F101_AVR",
                                         "IAS_ARM",
                                         "7221_AVR",
                                         "BLD_SNTR",
                                         "LINUX_PC_SCXB",
                                         "LINUX_PC_PLD"
                                       };

NID_F101_IPMC_DEV nid_f101_ipmc_dev_info={ NID_F101_IPMC_DEV_NAME,
                                           -1,
                                           NID_F101_IPMC_OPEN_FLAGS,
                                         };

/*******************************************************************
  The entires in command strings below indicate following:
  channel:lun:netfn:cmd:[data.......]
 *******************************************************************/
uns8 * nid_f101_ipmc_cmd_strs[NID_MAX_IPMC_CMD]=
       {
        "0f 00 2c 08 00 %02x %02x",                  /* IPMC command to get LED status */
        "0f 00 2c 07 00 %02x %02x %02x %02x %02x",   /* IPMC command to set LED */
        "0f 00 2e f0 00 00 a1 %02x %02x %02x",       /* IPMC command to send firmware progress event */
        "0f 00 06 24 %02x %02x %02x %02x %02x %02x", /* IPMC command to Start/Stop BMC WDT */
        "0f 00 06 22"                                /* IPMC Command to reset BMC WDT */
       };

/*******************************************************************
  The entires in command strings below indicate following:
  channel:lun:netfn:cmd:[data.......]
 *******************************************************************/
uns8 * nid_7221_ipmc_cmd_strs[NID_MAX_IPMC_CMD]=
       {
        "0f 00 2c 08 00 %02x %02x",                  /* IPMC command to get LED status */
        "0f 00 2c 07 00 %02x %02x %02x %02x %02x",   /* IPMC command to set LED */
        "0f 00 0a 44 00 00 02 00 00 00 00 01 00 04 0f 00 0f %02x %02x %02x",       /* IPMC command to send firmware progress event */
        "0f 00 06 24 %02x %02x %02x %02x %02x %02x", /* IPMC command to Start/Stop BMC WDT */
        "0f 00 06 22"                                /* IPMC Command to reset BMC WDT */
       };


NID_BOARD_IPMC_INTF_CONFIG nid_ipmc_board[NID_MAX_BOARDS]=
{
 {
  NID_IPMC_INTF_ENABLED,        /* Use the interface for sending IPMC commands */
  NID_MAX_IPMC_CMD,             /* No IPMC command is in progress */
  &nid_f101_ipmc_dev_info,      /* F101 IPMC device interface configuration */
  &nid_f101_ipmc_cmd_strs,      /* F101 IPMC command list */
  nid_f101_get_ipmc_dev_fd,     /* Routine to get IPMC device handle */
  nid_f101_open_intf,           /* Routine to open IPMC device interface */
  nid_f101_close_ipmc_intf,     /* Routine to close IPMC device interface */              
  nid_f101_send_ipmc_cmd,       /* Routine to send the IPMC command */ 
  nid_f101_proc_ipmc_rsp        /* Routine to process the responce from IPMC */ 
 },
 {
  NID_IPMC_INTF_ENABLED,        /* Use the interface for sending IPMC commands */
  NID_MAX_IPMC_CMD,             /* No IPMC command is in progress */
  &nid_f101_ipmc_dev_info,      /* F101 IPMC device interface configuration */
  &nid_f101_ipmc_cmd_strs,      /* F101 IPMC command list */
  nid_f101_get_ipmc_dev_fd,     /* Routine to get IPMC device handle */
  nid_f101_open_intf,           /* Routine to open IPMC device interface */
  nid_f101_close_ipmc_intf,     /* Routine to close IPMC device interface */              
  nid_f101_send_ipmc_cmd,       /* Routine to send the IPMC command */ 
  nid_f101_proc_ipmc_rsp        /* Routine to process the responce from IPMC */ 
 },
 {
  NID_IPMC_INTF_ENABLED,        /* Use the interface for sending IPMC commands */
  NID_MAX_IPMC_CMD,             /* No IPMC command is in progress */
  &nid_f101_ipmc_dev_info,      /* F101 IPMC device interface configuration */
  &nid_7221_ipmc_cmd_strs,      /* IPMI command strings */
  nid_f101_get_ipmc_dev_fd,     /* Routine to get IPMC device handle */
  nid_f101_open_intf,           /* Routine to open IPMC device interface */
  nid_f101_close_ipmc_intf,     /* Routine to close IPMC device interface */              
  nid_f101_send_ipmc_cmd,       /* Routine to send the IPMC command */ 
  nid_f101_proc_ipmc_rsp        /* Routine to process the responce from IPMC */ 
 },
 {
  NID_IPMC_INTF_DISABLED,
  NID_MAX_IPMC_CMD,
  NULL,
  NULL,
  nid_dummy_get_ipmc_dev_fd,
  nid_dummy_open_intf,
  nid_dummy_close_ipmc_intf,
  nid_dummy_send_ipmc_cmd,
  nid_dummy_proc_ipmc_rsp
 },
 {
  NID_IPMC_INTF_DISABLED,
  NID_MAX_IPMC_CMD,
  NULL,
  NULL,
  nid_dummy_get_ipmc_dev_fd,
  nid_dummy_open_intf,
  nid_dummy_close_ipmc_intf,
  nid_dummy_send_ipmc_cmd,
  nid_dummy_proc_ipmc_rsp
 },
 {
  NID_IPMC_INTF_DISABLED,
  NID_MAX_IPMC_CMD,
  NULL,
  NULL,
  nid_dummy_get_ipmc_dev_fd,
  nid_dummy_open_intf,
  nid_dummy_close_ipmc_intf,
  nid_dummy_send_ipmc_cmd,
  nid_dummy_proc_ipmc_rsp
 }
};

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_dummy_open_intf                                         *
 *                                                                             *
 * DESCRIPTION:    Dummy Interface                                             *
 *                                                                             *
 * INPUT:          None                                                        *
 *                                                                             *
 * OUTPUT:         Returns if the operation was successfull or not           *
 *                 NCSCC_RC_SUCCESS                                            *
 *                 NCSCC_RC_FAILURE                                            *
 ******************************************************************************/
uns32
nid_dummy_open_intf(uns8 *strbuff)
{
  if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;
  return NCSCC_RC_SUCCESS;

}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_dummy_close_ipmc_intf                                    *
 *                                                                             *
 * DESCRIPTION:    Closes IPMC fifo                                            *
 *                                                                             *
 * INPUT:          None                                                        *
 *                                                                             *
 * OUTPUT:         None                                                        *
 ******************************************************************************/
static void
nid_dummy_close_ipmc_intf( void )
{
  if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return;
  return ;
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_dummy_get_ipmc_dev_fd                                   *
 *                                                                             *
 * DESCRIPTION:    Dummy Routine                                               *
 *                                                                             *
 * INPUT:          None                                                        *
 *                                                                             *
 * OUTPUT:                                                                     *
 ******************************************************************************/
static int32
nid_dummy_get_ipmc_dev_fd()
{
  if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;
  return NCSCC_RC_SUCCESS;
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_dummy_send_ipmc_cmd                                     *
 *                                                                             *
 * DESCRIPTION:    Dummy routine                                               *
 * INPUT:          None                                                        *
 *                                                                             *
 * OUTPUT:                                                                     *
 *                                                                             * 
 ******************************************************************************/
static int32
nid_dummy_send_ipmc_cmd(NID_IPMC_CMDS ipmc_cmd,NID_IPMC_EVT_INFO *evt_info)
{

  if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;
  return NCSCC_RC_SUCCESS;

}


/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_dummy_proc_ipmc_rsp                                     *
 *                                                                             *
 * DESCRIPTION:   Dummy routine                                                *
 *                                                                             *
 * INPUT:         None                                                         *
 *                                                                             *
 * OUTPUT:                                                                     *
 ******************************************************************************/
static int32
nid_dummy_proc_ipmc_rsp(NID_IPMC_EVT_INFO *evt_info)
{
  if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;
  return NCSCC_RC_SUCCESS;

}


/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_f101_open_intf                                          *
 *                                                                             *
 * DESCRIPTION:    Opens interface to get responce from IPMC on F101           *
 *                 This is the FIFO where IPMC command responce is expected    *
 *                                                                             *
 * INPUT:          None                                                        *
 *                                                                             *    
 * OUTPUT:         Returns if the operation was successfull or not           *
 *                 NCSCC_RC_SUCCESS                                            *
 *                 NCSCC_RC_FAILURE                                            *
 ******************************************************************************/
static uns32
nid_f101_open_intf ( uns8 *strbuff )
{
   NID_F101_IPMC_DEV *nid_ipmc_conf = (NID_F101_IPMC_DEV *)nid_ipmc_board[nid_board_type].nid_ipmc_dev;

   if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;

   /* check if fifo is already open? */
   if ( nid_ipmc_conf->dev_handle > 0 ) return NCSCC_RC_SUCCESS;

   /******************************************************
   *    Lets Remove any such file if it already exists   *
   ******************************************************/
   m_NCS_POSIX_UNLINK(nid_ipmc_conf->dev_name);

   /******************************************************
   *    Create ipmc fifo                                 *
   ******************************************************/
   if(m_NCS_POSIX_MKFIFO(nid_ipmc_conf->dev_name,0600) < 0){
     sysf_sprintf(strbuff," FAILURE: Unable To Create IPMC FIFO Error:%s\n",strerror(errno));
     return NCSCC_RC_FAILURE;
   }

   nid_ipmc_conf->dev_handle = open (nid_ipmc_conf->dev_name,
                                     nid_ipmc_conf->flags);
                                                                                                 
   if (nid_ipmc_conf->dev_handle < 0)
   {
      sysf_sprintf(strbuff, "Failed Opening Payload Interface:%s\n",strerror(errno));
      return NCSCC_RC_FAILURE;
   }
                   
   return NCSCC_RC_SUCCESS;
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_f101_close_ipmc_intf                                    *
 *                                                                             *
 * DESCRIPTION:    Closes IPMC fifo                                            *
 *                                                                             *
 * INPUT:          None                                                        *
 *                                                                             *    
 * OUTPUT:         None                                                        *
 ******************************************************************************/
static void 
nid_f101_close_ipmc_intf( void )
{
   NID_F101_IPMC_DEV *nid_ipmc_conf = (NID_F101_IPMC_DEV *)nid_ipmc_board[nid_board_type].nid_ipmc_dev;
   
   if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return;
   m_NCS_POSIX_CLOSE( nid_ipmc_conf->dev_handle );
   nid_ipmc_conf->dev_handle = -1;
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_f101_get_ipmc_dev_fd                                    *
 *                                                                             *
 * DESCRIPTION:    Returns the IPMC device FD to read the responce from        * 
 *                                                                             *
 * INPUT:          None                                                        *
 *                                                                             *    
 * OUTPUT:         Returns F101 payload interface device FD                    * 
 ******************************************************************************/
static int32 
nid_f101_get_ipmc_dev_fd()
{
   NID_F101_IPMC_DEV *f101_ipmc_dev = (NID_F101_IPMC_DEV *)nid_ipmc_board[nid_board_type].nid_ipmc_dev;

   if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return -1;
   return f101_ipmc_dev->dev_handle;  
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_f101_send_ipmc                                          *
 *                                                                             *
 * DESCRIPTION:    Sends given IPMC command                                    * 
 *                                                                             *
 * INPUT:                                                                       *
 *                 cmdStr- IPMC command string                                 *
 *                                                                             *    
 * OUTPUT:         Returns F101 payload interface device FD                    * 
 ******************************************************************************/
static int32
nid_f101_send_ipmc(uns8 *cmdStr)
{
  char  * args[6];
  uns8  strbuff[255];

  args[0] = NID_IPMI_CMD_SCRIPT;
  args[1] = "-k";
  args[2] = cmdStr;
  args[3] = "smi";
  args[4] = "0";
  args[5] = NULL;

  if ( fork_process(NULL,NID_IPMI_CMD_SCRIPT,args,NID_F101_IPMC_DEV_NAME,strbuff) < 0 )
  {
    logme(NID_LOG2FILE,"Failed to spawn, %s: %s\n",NID_IPMI_CMD_SCRIPT,strbuff);
    return NCSCC_RC_FAILURE;
  }
  
  return NCSCC_RC_SUCCESS;  

}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_f101_send_ipmc_cmd                                      *
 *                                                                             *
 * DESCRIPTION:    sends the given IPMC command through F101 payload interface *
 * INPUT:          None                                                        *
 *                                                                             *
 * OUTPUT:         Returns NCSCC_RC_SUCCESS if the command is send succefully  *
 *                 Returns NCSCC_RC_FAILURE if it failes to send IPMC command  *
 ******************************************************************************/
static int32
nid_f101_send_ipmc_cmd(NID_IPMC_CMDS ipmc_cmd,NID_IPMC_EVT_INFO *evt_info)
{
   NID_IPMC_EVT_INFO *lc_evt_info = evt_info;
   uns8              o_buff[150];

   if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;

   nid_ipmc_build_cmd(ipmc_cmd, lc_evt_info, o_buff);

   nid_ipmi_parse_msg(&ipmi_req_msg,o_buff);

   if ( nid_f101_send_ipmc(o_buff) < 0 )
      return NCSCC_RC_FAILURE;

   /* Mark that we are expecting for responce */
   nid_ipmc_board[nid_board_type].nid_ipmc_cmd_prog = ipmc_cmd;

   return NCSCC_RC_SUCCESS;

}


struct msg
{
  uns8 *head;
  uns8 *tail;
} nid_ipmc_rsp_msg = {
             nid_ipmc_rsp_buff,
             nid_ipmc_rsp_buff
            };


/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_ipmi_parse_msg                                          *
 *                                                                             *
 * DESCRIPTION:   Parses and segrigates the Responce from IPMC                 * 
 *                                                                             *
 * INPUT:         msg - buffer to fill the parsed responce codes               *
 *                response - string to be parsed, received from IPMC           *       
 *                                                                             *    
 * OUTPUT:        Returns the error code/or zero on succesful responce         *  
 ******************************************************************************/
static uns8 
nid_ipmi_parse_msg(ipmi_msg_t *ipmi_msg,const char  * response)
{
   int i;
   const char *str;
   uns8  outbuf[NID_IPMI_CMD_LENGTH];
   int val;


   for (i = 0, str = response;
        i < NID_IPMI_CMD_LENGTH;
        i++, str += 3)
   {
      if (sscanf (str, "%02x", &val) != 1)
         break;
      else
         outbuf[i] = (unsigned char) val;
   }

   ipmi_msg->netfn = outbuf[2];
   ipmi_msg->cmd   = outbuf[3];
   ipmi_msg->data_len = i - 4; /* remove channel, lun, netfn, and cmd */

   for (i = 4;i<(ipmi_msg->data_len + 4);i++)
       ipmi_msg->data[i-4] = outbuf[i];
   
   return ipmi_msg->data[0];
}


/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_ipmi_parse_resp                                         *
 *                                                                             *
 * DESCRIPTION:   Parses and segrigates the Responce from IPMC                 *
 *                                                                             *
 * INPUT:         msg - buffer to fill the parsed responce codes               *
 *                response - string to be parsed, received from IPMC           *
 *                                                                             *
 * OUTPUT:        Returns the error code/or zero on succesful responce         *
 ******************************************************************************/
#define NID_IPMI_INV_RSP 2

static uns8
nid_ipmi_parse_resp(ipmi_msg_t *ipmi_msg,const char  * response)
{
   int i;
   const char *str;
   uns8  outbuf[NID_IPMI_CMD_LENGTH];
   int val;
   
   outbuf[3] = 0;

   for (i = 0, str = response;
        i < NID_IPMI_CMD_LENGTH;
        i++, str += 3)
   {
      if (sscanf (str, "%02x", &val) != 1)
         break;
      else
         outbuf[i] = (unsigned char) val;
   }

   ipmi_msg->netfn = outbuf[1];
   ipmi_msg->cmd   = outbuf[3];
   /* Fix for IR83413 */
   if ( ipmi_msg->cmd != ipmi_req_msg.cmd ) return NID_IPMI_INV_RSP;

   ipmi_msg->data_len = i - 4; /* remove channel, lun, netfn, and cmd */
   for (i = 4;i<(ipmi_msg->data_len + 4);i++)
       ipmi_msg->data[i-4] = outbuf[i];

   return ipmi_msg->data[0];
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_f101_proc_ipmc_rsp                                      *
 *                                                                             *
 * DESCRIPTION:   Processes IPMC responce                                      * 
 *                                                                             *
 * INPUT:         None                                                         * 
 *                                                                             *    
 * OUTPUT:        NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE                            *  
 ******************************************************************************/
static int32
nid_f101_proc_ipmc_rsp(NID_IPMC_EVT_INFO *evt_info)
{
   uns32        rc        = NCSCC_RC_FAILURE;
   int          bytesRead = 0;
   int32        ipmc_fd   = nid_f101_get_ipmc_dev_fd(); 
   uns8         ipmi_status;
   fd_set       set;
   uns32        n=0;
   struct timeval tv;

   /* Check if this interface is enabled */
           
   if ( nid_ipmc_board[nid_board_type].status != NID_IPMC_INTF_ENABLED ) return NCSCC_RC_SUCCESS;

   /***************************************************************\
    *                                                             *
    *         Read from fifo that ipmccmd writes into             *
    *         appending to any characters previously read         *
    *                                                             *
   \***************************************************************/
 
   while(1) 
   {
     FD_ZERO(&set);
     FD_SET(ipmc_fd,&set);

     tv.tv_sec = 5; /* wait for 2 secs for IPMI response */
     tv.tv_usec = 0; 

     if ( (n = m_NCSSOCK_SELECT(ipmc_fd + 1, &set, NULL, NULL, &tv)) <= 0 )
     {
       if((errno == EINTR) && (n < 0))
        {
          logme(NID_LOG2FILE,"%s, Error: %d\n",strerror(errno),errno);
          FD_ZERO(&set);
          FD_SET(ipmc_fd,&set);
          continue;
        }
        if(n == 0)
        {
          logme(NID_LOG2FILE,"Timed-out for ipmi response\n");
          nid_ipmc_board[nid_board_type].nid_close_ipmc_dev();
          return NCSCC_RC_FAILURE;
        }

     }
 
     if ( (bytesRead = read(ipmc_fd, nid_ipmc_rsp_msg.tail,
                            (&nid_ipmc_rsp_buff[sizeof(nid_ipmc_rsp_buff)] - nid_ipmc_rsp_msg.tail))) < 0)
    {
      if ((errno != EINTR) && (errno != EWOULDBLOCK) && (errno != EAGAIN))
      {
         /* Non-benign error */
         logme(NID_LOG2FILE,"Failed reading IPMC Error: %s\n",strerror(errno));
      }
      else
      {
         /*logme(NID_LOG2FILE,"Failed reading IPMC device Error: %s\n",strerror(errno));*/
         continue;
      }
      return NCSCC_RC_FAILURE;
    }

    nid_ipmc_rsp_msg.tail+=bytesRead;
    if ( *(nid_ipmc_rsp_msg.tail-1) == '\n' )
    {
       *(nid_ipmc_rsp_msg.tail) = '\0';
       nid_ipmc_rsp_msg.tail = nid_ipmc_rsp_msg.head;
       break;
    }
 
   }

   /***************************************************************\
    *                          
    *         Check whether an IPMC event has been received         *
    *         or whether a complete IPMC response has been assembled*
    *                                                               *
   \***************************************************************/
   if ((ipmi_status = nid_ipmi_parse_resp(&ipmi_rsp_msg, nid_ipmc_rsp_msg.head)) == 0)
   {
 
     if ( (ipmi_rsp_msg.netfn - 1) != ipmi_req_msg.netfn ||
       ipmi_rsp_msg.cmd != ipmi_req_msg.cmd             )
     {
       logme(NID_LOG2FILE,"IPMI req/resp mismatch: rsp->netfn:%d req->netfn:%d rsp->cmd:%d req->cmd:%d\n",(ipmi_rsp_msg.netfn - 1),ipmi_req_msg.netfn,ipmi_rsp_msg.cmd,ipmi_req_msg.cmd);
       rc = NCSCC_RC_FAILURE;
     }
     switch (nid_ipmc_board[nid_board_type].nid_ipmc_cmd_prog)
     {
      case NID_IPMC_CMD_LED_GET:
           { 
             NIDGETLED   *ledget = (NIDGETLED *)&evt_info->get_led_info;

             if ( ipmi_rsp_msg.data_len == 10 )
             {
              ledget->ledState           = ipmi_rsp_msg.data[2];
              ledget->lcFunction         = ipmi_rsp_msg.data[3];
              ledget->lcOnDuration       = ipmi_rsp_msg.data[4];
              ledget->lcColor            = ipmi_rsp_msg.data[5] & 0x0f;
              ledget->overrideFunction   = ipmi_rsp_msg.data[6];
              ledget->overrideOnDuration = ipmi_rsp_msg.data[7];
              ledget->overrideColor      = ipmi_rsp_msg.data[8] & 0x0f;
              ledget->lampTestDuration   = ipmi_rsp_msg.data[9];
              rc                         = NCSCC_RC_SUCCESS;
             }
             else
             {
               logme(NID_LOG2FILE,"LED GET invalid data length: %d\n",ipmi_rsp_msg.data_len);
               rc =  NCSCC_RC_FAILURE;
             }

           }
           break;

      case NID_IPMC_CMD_LED_SET:
      case NID_IPMC_CMD_SEND_FW_PROG:
      case NID_IPMC_CMD_WATCHDOG_SET:
      case NID_IPMC_CMD_WATCHDOG_RESET:
               rc = NCSCC_RC_SUCCESS;
           break;
   }
  }
  else
  {
    logme(NID_LOG2FILE,"IPMC error responce: %x, %s", ipmi_status,nid_ipmc_rsp_msg.head);
    rc =  NCSCC_RC_FAILURE;
  }
   nid_ipmc_rsp_msg.head = nid_ipmc_rsp_buff;
   nid_ipmc_rsp_msg.tail = nid_ipmc_rsp_buff;
   nid_ipmc_board[nid_board_type].nid_ipmc_cmd_prog = NID_MAX_IPMC_CMD;
   return rc;
}

/*******************************************************************************
 *                                                                             *
 * PROCEDURE NAME: nid_ipmc_build_cmd                                          *
 *                                                                             *
 * DESCRIPTION: This function is used to build board specific IPMC commands    *
 *                                                                             *
 * INPUT:                                                                      *
 *       cmd        - get/set LED or send Firmware prog evt commands supported *
 *       evt_info   - IPMC command arguments                                   *
 *       o_cmd      - Frames input arguments to o/p buffer                     *
 *                                                                             *
 * OUTPUT:   None.                                                             *
 *                                                                             *
 *******************************************************************************
 */
static void 
nid_ipmc_build_cmd(NID_IPMC_CMDS cmd, NID_IPMC_EVT_INFO *evt_info,uns8 *o_cmd)
{
   uns8                 *cmd_str     = (* nid_ipmc_board[nid_board_type].cmd_str)[cmd];
   NIDGETLED            get_led_info;
   NIDSETLED            set_led_info;
   NID_SYS_FW_PROG_INFO fw_prog_info;
   NIDIPMC_WDT_INFO     wdt;

   if ( o_cmd == NULL ) return;

   if ( evt_info != NULL )
   {
     get_led_info = evt_info->get_led_info;
     set_led_info = evt_info->set_led_info;
     fw_prog_info = evt_info->fw_prog_info;
     wdt          = evt_info->wdt_info;
   }

   switch(cmd)
   {
   case  NID_IPMC_CMD_LED_GET:
         {

          sysf_sprintf(o_cmd,cmd_str,
                       get_led_info.fruid,
                       get_led_info.led);
         }
         break;

   case  NID_IPMC_CMD_LED_SET:
         {

          sysf_sprintf(o_cmd,cmd_str,
                       set_led_info.fruid,
                       set_led_info.ledId,
                       set_led_info.ledFunction,
                       set_led_info.onDuration,
                       set_led_info.ledColor);
         }
         break;

   case  NID_IPMC_CMD_SEND_FW_PROG:
         if ( (nid_board_type == NID_7221_AVR) && (fw_prog_info.data2 == MOTO_FIRMWARE_OEM_CODE) )
         {
           /* we have a hack here for 7221 */
            sysf_sprintf(o_cmd,cmd_str,fw_prog_info.event_type,fw_prog_info.data3,0xFF);
         }
         else
         {   
           sysf_sprintf(o_cmd,cmd_str,fw_prog_info.event_type,
                        fw_prog_info.data2,fw_prog_info.data3);
         }
         break;

   case  NID_IPMC_CMD_WATCHDOG_SET:
         sysf_sprintf(o_cmd,cmd_str,wdt.timer_use,\
                      wdt.timer_actions,\
                      wdt.pre_timeout_interval,\
                      wdt.timer_expiration_flags_clear,\
                      wdt.initial_countdown_value_lsbyte,\
                      wdt.initial_countdown_value_msbyte);
         break;

   case  NID_IPMC_CMD_WATCHDOG_RESET:
         sysf_sprintf(o_cmd,cmd_str);         
         break;

   default:
         return;

   }

}
