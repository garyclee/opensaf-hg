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


#include "ifsv_demo.h"

static uns32 
file_get_word(FILE **fp, char *o_chword);
static uns32
ifsv_get_linux_stats(STATS_INFO *o_info, uns32 port_num);
static uns32 get_intf_type (char *intf);
static uns32
extract_port_status(FILE **fp, INTF_INFO *o_info);
static uns32 
skip_till_next_word(FILE **fp, char *i_chk_word, char *prev_word);
static uns32
check_char_present(char *i_str, char i_char);
static uns32
strip_n_bytes_in_str(char *i_str, char *o_str, uns32 start, uns32 n_bytes);
static uns32
ifsv_drv_set_get_info(struct ncs_ifsv_hw_drv_req *drv_req);
static uns32
get_word_after_marker(char *i_word, char *o_word, char marker);
static unsigned int
get_word_before_marker(char *i_word, char *o_word, char marker);
static uns32
extract_phy_addr(char *i_str, uns8 *o_phy);
static uns32 
conv_str_to_byte_hex(char *i_str);
static uns32
find_hex_value(char hex_char);

/****************************************************************************
 * Name          : ifsv_drv_set_get_info
 *
 * Description   : This is the function which sets and get information from 
 *                 the driver.
 *
 * Arguments     : drv_req     - driver request structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/ NCS_RCC_FAILURE.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32
ifsv_drv_set_get_info(struct ncs_ifsv_hw_drv_req *drv_req)
{
   NCS_IFSV_HW_INFO     hw_info;
   NCS_IFSV_DRV_SVC_REQ drv_svc_req;
   uns32                port_num;
   uns32                if_flag;
   uns32                res = NCSCC_RC_FAILURE;
   int                 sock;
   STATS_INFO          temp_stats;
   struct ifreq ifr;

   port_num = drv_req->port_type.port_id;
   memset(&drv_svc_req, 0, sizeof(drv_svc_req));
   switch(drv_req->req_type)
   {
      case NCS_IFSV_HW_DRV_STATS:
      {
         memset(&hw_info, 0, sizeof(hw_info));
         memset(&temp_stats, 0, sizeof(temp_stats));
         res = ifsv_get_linux_stats(&temp_stats, port_num);
         if (res == NCSCC_RC_FAILURE)
         {
            hw_info.info.stats.status = FALSE;
         }else
         {
             hw_info.info.stats.status = TRUE;
             hw_info.info.stats.stats.in_octs   = temp_stats.i_bytes;
             hw_info.info.stats.stats.in_upkts  = temp_stats.i_pack;
             hw_info.info.stats.stats.in_errs   = temp_stats.i_error;
             hw_info.info.stats.stats.in_dscrds = temp_stats.i_drop;
             hw_info.info.stats.stats.out_octs  = temp_stats.o_bytes;
             hw_info.info.stats.stats.out_upkts = temp_stats.o_pack;
             hw_info.info.stats.stats.out_errs  = temp_stats.o_error;
             hw_info.info.stats.stats.out_dscrds= temp_stats.o_drop;
         }
          hw_info.msg_type            = NCS_IFSV_HW_DRV_STATS;
          hw_info.port_type           = drv_req->port_type;
          /* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */
          hw_info.subscr_scope = drv_req->subscr_scope;
          /* Changes for INT/EXT scope for subscr_scope overwrite. EXT_INT */

          hw_info.info.stats.app_dest = drv_req->info.req_info.dest_addr;
          hw_info.info.stats.app_svc_id = drv_req->info.req_info.svc_id;
          hw_info.info.stats.usr_hdl    = drv_req->info.req_info.app_hdl;
          drv_svc_req.req_type          = NCS_IFSV_DRV_SEND_REQ;
          drv_svc_req.info.hw_info      = &hw_info;              
          res = ncs_ifsv_drv_svc_req(&drv_svc_req);
          break;
      }
      case NCS_IFSV_HW_DRV_SET_PARAM:
         sock = socket(PF_INET,SOCK_DGRAM,0);
         if (sock == -1)
         {
            printf("sorry coulnd't able to create a socket\n");
            break;
         }
         memset(&ifr, 0, sizeof(ifr));
         ifr.ifr_ifindex = port_num;
         if  (ioctl(sock, SIOCGIFNAME, ifr) < 0)
         {
            printf("sorry invalid interface name \n");
            close(sock);
            break;
         }
         /** At present this is used just to set MTU **/
         if ((drv_req->info.set_param.if_am & NCS_IFSV_IAM_MTU) != 0)
         {
            /* open a socket and set MTU */
            ifr.ifr_mtu = drv_req->info.set_param.mtu;
            if(ioctl(sock,SIOCSIFMTU,&ifr) < 0)
            {
               close(sock);
               perror("SIOCSIFMTU");
               break;
            }
         }
         /** This is to support Admin status UP/Down event from CLI **/
         if ((drv_req->info.set_param.if_am & NCS_IFSV_IAM_ADMSTATE) != 0)
         {
            /* open a socket and get the flags and reset/set the admin flag depending on the set */
            if(ioctl(sock,SIOCGIFFLAGS,&ifr) < 0)
            {
               perror("SIOCGIFFLAGS");
               close(sock);
               break;
            }
            if_flag = ifr.ifr_flags;

            if (drv_req->info.set_param.admin_state == TRUE)
            {
               if_flag = (if_flag | IFF_UP);
            } else
            {
               if_flag = (if_flag & (~IFF_UP));
            }
            ifr.ifr_flags = if_flag; 
            if(ioctl(sock,SIOCGIFFLAGS,&ifr) < 0)
            {
               perror("SIOCGIFFLAGS");
               close(sock);
               break;
            }
         }
        /* send a port register for the information to MOCK driver */ 
        close(sock);
      break;
      default:
      break;
   }
   return res;
}

/****************************************************************************
 * Name          : ifsv_drv_get_specific_intf_info
 *
 * Description   : This is the wrapper function used get the interface 
 *                 specific information, which gets the information about 
 *                 specific the underlying interface.
 *
 * Arguments     : intf_info - Interface information which it got from polling.
 *                 port_num  - port number to be searched for. 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : In this demo we are just simulating the interface coming
 *                 up and going down using ifconfig command, so please don't
 *                 except any hot swapable capabilities.
 *****************************************************************************/
uns32 
ifsv_drv_get_specific_intf_info (INTF_INFO *intf_info, uns32 port_num)
{
   FILE *fp;
   char command_name[90];
   uns32 tmp_var;
   uns32 found_flag = FALSE;
   strcpy(command_name,"/sbin/ifconfig -a ");
   strcat(command_name,"> /etc/ncs_samp_drv");
   system(command_name);
   fp = fopen("/etc/ncs_samp_drv","r");
   if (fp == NULL)
   {
      printf("\ndriver file couldn't able to open ..\n");
      return found_flag;
   }
   while(1)
   {
      memset(&intf_info,0,sizeof(intf_info));
      tmp_var = extract_port_status(&fp, intf_info);
      if (tmp_var == IFSV_DRV_NOTSEND_END)
          break;
      if (tmp_var == IFSV_DRV_NOTSEND_CONT)
          continue;
      if (intf_info->port == port_num)
      {
         found_flag = TRUE;
         break;
      }
      if (tmp_var == IFSV_DRV_SEND_END)
          break;
   }
   fclose(fp);
   return found_flag;
}

/****************************************************************************
 * Name          : ifsv_drv_poll_intf_info
 *
 * Description   : This is the wrapper function used get the interface 
 *                 all information, which gets the information about 
 *                 specific the underlying interface.
 *
 * Arguments     : intf_info - Interface information which it got from polling.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : In this demo we are just simulating the interface coming
 *                 up and going down using ifconfig command, so please don't
 *                 except any hot swapable capabilities.
 *****************************************************************************/
NCSCONTEXT
ifsv_drv_poll_intf_info (NCSCONTEXT info)
{
   FILE *fp;
   INTF_INFO o_info;
   char command_name[90];
   unsigned char phy_addr[6];
   uns32 tmp_var;
   int admin_status_down = 2;
try_again:
   strcpy(command_name,"/sbin/ifconfig -a ");
   strcat(command_name,"> /etc/ncs_samp_drv");
   system(command_name);
   fp = fopen("/etc/ncs_samp_drv","r");
   if (fp == NULL)
   {
      printf("\ndriver file couldn't able to open ..\n");
      return NULL;
   }
   /** Adding a forcible loopback address for NTT **/
   memset(&phy_addr,0,6);
   ifsv_demo_reg_port_info(1,24, phy_addr,
                    1,1,16436,
                    "lo",100000000);
   while(1)
   {
      m_NCS_TASK_SLEEP(500);
      memset(&o_info,0,sizeof(o_info));
      tmp_var = extract_port_status(&fp, &o_info);
      if (strcmp(o_info.name,"lo") == 0)
         continue;
      if(o_info.admin_status == admin_status_down && o_info.oper_status == NCS_STATUS_UP)
      break; 
      if (tmp_var == IFSV_DRV_NOTSEND_END)
          break;
      if (tmp_var == IFSV_DRV_NOTSEND_CONT)
          continue;
      /* send the updates to IfSv MOCK driver */
      if ((o_info.port != 0) && (o_info.MTU != 0))
      {
/*         printf(" \nDev Name : %s",o_info.name); */
         ifsv_demo_reg_port_info(o_info.port,o_info.port_type,o_info.phy,
                    o_info.oper_status,o_info.admin_status,o_info.MTU,
                    o_info.name,100000000);
      }
      if (tmp_var == IFSV_DRV_SEND_END)
          break;
   }
   fclose (fp);
   m_NCS_TASK_SLEEP(5000);
   goto try_again;
   return NULL;
}

/****************************************************************************
 * Name          : ifsv_demo_reg_port_info
 *
 * Description   : This is the wrapper function used register the port 
 *                 information with IfND.
 *
 * Arguments     : port_type - port type.
 *                 port_num  - port number.
 *                 i_phy     - physical MAC address.
 *                 oper_state - operational status.
 *                 admin_state - admin status.
 *                 MTU        - Maximun Transmision Unit.
 *                 if_name    - Interface name.
 *                 if_speed   - interface speed (default 100 Mbps).
 *                  
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : Here we assume the admin status as "1" always.
 *****************************************************************************/
uns32
ifsv_demo_reg_port_info (uns32 port_num, uns32 port_type, uns8 *i_phy, 
             uns32 oper_state, uns32 admin_state, uns32 MTU,char *if_name, 
             uns32 if_speed)
{
   NCS_IFSV_DRV_SVC_REQ drv_svc_req;
   NCS_IFSV_PORT_REG port_reg;
   NCS_IFSV_PORT_TYPE    type;
   uns32 res;

   type.port_id = port_num;
   type.type    = port_type;
   memset(&drv_svc_req, 0, sizeof(drv_svc_req));
   memset(&port_reg, 0, sizeof(port_reg));
   m_IFSV_ALL_ATTR_SET(port_reg.port_info.if_am);
   port_reg.hw_get_set_cb = ifsv_drv_set_get_info;
   port_reg.port_info.mtu         = MTU;
   strncpy(port_reg.port_info.if_name, if_name,IFSV_IF_NAME_SIZE-1);
   memcpy(port_reg.port_info.phy_addr,i_phy,6);
   port_reg.port_info.mtu         = MTU;
   port_reg.port_info.oper_state  = oper_state;
   port_reg.port_info.speed       = if_speed;
   port_reg.port_info.admin_state = admin_state;
   port_reg.port_info.port_type             = type;
   drv_svc_req.req_type      = NCS_IFSV_DRV_PORT_REG_REQ;
   drv_svc_req.info.port_reg = &port_reg;
   res = ncs_ifsv_drv_svc_req(&drv_svc_req);
   return (res);
}

/****************************************************************************
 * Name          : skip_till_eol
 *
 * Description   : This is the utill function used skip the characters till
 *                 the end of line. 
 *
 * Arguments     : fp - file pointer.
 *                  
 *
 * Return Values : 1 - error / 0 - found.
 *
 * Notes         : None.
 *****************************************************************************/
/** This function skips till the end of line **/
static uns32
skip_till_eol(FILE **fp)
{
   int temp_char;
   temp_char = getc(*fp);
    if (*fp == NULL)
    {
       return 1;
    }
   temp_char = getc(*fp);
   while((temp_char != EOF) && (temp_char != '\n'))
   {
      temp_char = getc(*fp);
   }
  return 0; 
}

/****************************************************************************
 * Name          : extract_port_status
 *
 * Description   : This is the utill function extract all the port information.
 *
 * Arguments     : fp - file pointer.
 *                 o_info - interface information which would be filled after
 *                          extraction.
 *                  
 *
 * Return Values : 4 - end 3 - don't send this and this is end 2 - don't send 
 *                this and still more to go 1 - still more interfaces are 
 *                available 0 - failure
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
extract_port_status(FILE **fp, INTF_INFO *o_info)
{
    static char prev_word[64] = "Done";
    static char pres_word[64];
    uns32 first_time = 0;
    uns32 temp_var =0;
    char temp_char[64];
    struct ifreq ifr;
    int fd;
    int admin_status_up = 1;
    int admin_status_down = 2;

    memset(&ifr, '\0', sizeof (ifr));
    memset(temp_char, 0, sizeof (temp_char));

    o_info->admin_status = admin_status_down;
    /** check for Link string in the file **/
    if (strcmp(prev_word,"Done") == 0)
    {
       first_time = 1;
    }

    if (first_time == 1)
    {
       /** get the interface name **/
        temp_var = file_get_word(fp,pres_word);
        if (temp_var == IFSV_DRV_EOF)
           return IFSV_DRV_SEND_END;
        strcpy(prev_word,pres_word);
        temp_var = file_get_word(fp,pres_word);
        if ((temp_var == IFSV_DRV_EOF) || (strcmp(pres_word,"Link") != 0))
        {           
           if (temp_var == IFSV_DRV_NOTSEND_END)
              return IFSV_DRV_SEND_END;
           else
              return 0;
        }
    }
    
    /** check for the logical interface **/
    if (check_char_present(prev_word,':') == IFSV_DRV_CHAR_FOUND)
    {
       temp_var = skip_till_next_word(fp,"Link",prev_word);
       if (temp_var == IFSV_DRV_NOTSEND_END)
       {
          return IFSV_DRV_SEND_END;
       } else if (temp_var == IFSV_DRV_SEND_CONT)
       {
          return IFSV_DRV_NOTSEND_CONT;
       } else
       {
          return 0;
       }
    }
    /** get the ifindex and fill in the port field **/
    if((fd = socket(PF_INET,SOCK_DGRAM,0)) < 0)
       return -1;
    ifr.ifr_ifindex = 0;
    strncpy(ifr.ifr_name,prev_word,IFNAMSIZ-1);
    if (ioctl(fd,SIOCGIFINDEX,&ifr) < 0)
    {
        printf ("error in ioctl \n");
        close(fd);
        return (-1);
    }
    o_info->port = ifr.ifr_ifindex;
    strcpy(o_info->name,ifr.ifr_name);

    if (ioctl(fd,SIOCGIFFLAGS,&ifr) < 0)
    {
        printf ("error in ioctl  SIOCGIFFLAGS \n");
        close(fd);
        return (-1);
    }
    if (ifr.ifr_flags & IFF_UP)
    {
       o_info->oper_status = NCS_STATUS_UP;
    } else
    {
       o_info->oper_status = NCS_STATUS_DOWN;
    }

    while(1)
    {
       temp_var = file_get_word(fp,pres_word);
       /** Check for status UP **/
       if (strcmp(pres_word,"UP") == 0)
       {
          o_info->admin_status = admin_status_up;
       } else if (strcmp(pres_word,"Link") == 0)
       {
          close(fd);
          return IFSV_DRV_SEND_CONT;
       } else if (strcmp(pres_word,"HWaddr") == 0)
       {
          temp_var = file_get_word(fp,pres_word);
          /** extract Hardware address and copy it to the struct **/
          extract_phy_addr(pres_word,o_info->phy);
       }
       
       if (strip_n_bytes_in_str(pres_word, temp_char, 0, 6) == 0)
       {
          if (strcmp(temp_char, "encap:") == 0)
          {
             /** extract the interface type **/
             if (strip_n_bytes_in_str(pres_word, temp_char, 6, 
                strlen(pres_word)) == 0)
             {
                o_info->port_type = get_intf_type(temp_char);                
             }
          }
       } 
       
       if (strip_n_bytes_in_str(pres_word, temp_char, 0, 4) == 0)
       {
          if (strcmp(temp_char, "MTU:") == 0)
          {
             /** extract the MTU **/
             if (strip_n_bytes_in_str(pres_word, temp_char, 4, 
                strlen(pres_word)) == 0)
             {
                o_info->MTU = atoi(temp_char);
             }
          }
       }

       if (temp_var == IFSV_DRV_NOTSEND_END)
       {
          strcpy(prev_word,"Done");
          close(fd);
          return IFSV_DRV_SEND_END;
       }
       strcpy(prev_word,pres_word);
    }
    close(fd);
    return 0;
}

/****************************************************************************
 * Name          : get_intf_type
 *
 * Description   : This is the utill function used extract interface type.
 *
 * Arguments     : intf - interface string type (like "ethernet").
 *                  
 *
 * Return Values : interface type / 0 - for unknow type.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 get_intf_type (char *intf)
{
   uns32 type = 1;
   if (strcmp(intf,"Ethernet") == 0)
   {
      type = NCS_IFSV_INTF_ETHERNET;
   } else if(strcmp(intf,"Local") == 0)
   {
      type = NCS_IFSV_INTF_LOOPBACK;
   }
   return type;
}

/****************************************************************************
 * Name          : skip_till_next_word
 *
 * Description   : This is the utill function used skip till next matching word.
 *
 * Arguments     : fp         - file pointer.
 *                 i_chk_word - word to be matched.
 *                 prev_word  - previous word if the matching word is lying
 *                              at more then 1st possition. 
 *
 * Return Values : IFSV_DRV_NOTSEND_END/IFSV_DRV_SEND_CONT.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32 
skip_till_next_word(FILE **fp, char *i_chk_word, char *prev_word)
{
   uns32 temp_var;
   char pres_word[64];
   while(1)
   {
      temp_var = file_get_word(fp, pres_word);
      if (temp_var == IFSV_DRV_EOF)
      {
         if (strcmp(pres_word,i_chk_word) != 0)
            return IFSV_DRV_NOTSEND_END;
         else
            return IFSV_DRV_SEND_CONT;
      }
      if (strcmp(pres_word,i_chk_word) == 0)
         break;
      strcpy(prev_word, pres_word);
   }
   return IFSV_DRV_SEND_CONT;
}

/****************************************************************************
 * Name          : check_char_present
 *
 * Description   : This is the utill function used to check whether the given
 *                 character is found are not in the given string.
 *
 * Arguments     : i_str   - input string.
 *                 i_char  - charecter to be matched with.
 *
 * Return Values : IFSV_DRV_CHAR_FOUND/IFSV_DRV_CHAR_NOTFOUND.
 *
 * Notes         : None.
 *****************************************************************************/
static uns32
check_char_present(char *i_str, char i_char)
{
   uns32 temp_var;
   for(temp_var = 0; temp_var < strlen(i_str); temp_var++)
   {
      if (i_str[temp_var] == i_char)
         return IFSV_DRV_CHAR_FOUND;
   }
   return IFSV_DRV_CHAR_NOTFOUND;
}
/****************************************************************************
 * Name          : strip_n_bytes_in_str
 *
 * Description   : This is the utill function used to strip "n" bytes from
 *                 the given string.
 *
 * Arguments     : fp   - File pointer.
 *                 i_word - word to be matched.
 *                 o_prev_word - previous word found.
 *
 * Return Values : IFSV_DRV_WORD_FOUND/IFSV_DRV_WORD_NOTFOUND.
 *
 * Notes         :  here offset starts from "0".
 *****************************************************************************/
static uns32
strip_n_bytes_in_str(char *i_str, char *o_str, uns32 start, uns32 n_bytes)
{
   uns32 temp_var;
   uns32 str_len;

   if ((i_str == NULL) || (o_str == NULL))
   {
       return 2;
   }
   str_len = strlen(i_str);
   if (n_bytes < str_len)
       str_len = n_bytes;
           
   for (temp_var = start; temp_var < str_len; temp_var++)
   {
       o_str[temp_var-start] = i_str[temp_var];
   }
   o_str[temp_var-start] = '\0';
   return 0;
}

/****************************************************************************
 * Name          : file_get_word
 *
 * Description   : This is the utill function used to get a word from the file.
 *
 * Arguments     : fp       - File pointer.
 *                 o_chword - word which got extracted.
 *
 * Return Values : IFSV_DRV_EOF/IFSV_DRV_EOL/.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32 
file_get_word(FILE **fp, char *o_chword)
{
   int temp_char;
   uns32 temp_ctr=0;
   try_again:
   temp_ctr = 0;
   temp_char = getc(*fp);
   while ((temp_char != EOF) && (temp_char != '\n') && (temp_char != ' ') && (temp_char != '\0'))
   {
      o_chword[temp_ctr] = (char)temp_char;
      temp_char = getc(*fp);
      temp_ctr++;
   }
   o_chword[temp_ctr] = '\0';
   if (temp_char == EOF)
   {
      return(IFSV_DRV_EOF);
   }
   if (temp_char == '\n')
   {
      return(IFSV_DRV_EOL);
   }
   if(o_chword[0] == 0x0)
      goto try_again;
   return(0);
}


/****************************************************************************
 * Name          : ifsv_get_linux_stats
 *
 * Description   : This is the utill function used to get interface statistics
 *                 information from the linux driver.
 *
 * Arguments     : o_info    - interface statistics which needs to be fetched.
 *                 port_num  - port number for which statistics needs to be
 *                             fetched.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCS_RCC_FAILURE.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32
ifsv_get_linux_stats(STATS_INFO *o_info, uns32 port_num)
{
   FILE *fp;
   uns32 temp_var;
   char pres_word[64];
   char temp_word[16];
   struct ifreq ifr;
   int sock;
   uns32 res =  NCSCC_RC_FAILURE;
   system("cp /proc/net/dev /etc/ncs_drv_stats");
   fp = fopen("/etc/ncs_drv_stats","r");
   if (fp == NULL)
   {
       printf ("devices are not available \n");
       return 0;
   }
   /** skip first 2 lines **/
   skip_till_eol(&fp);
   skip_till_eol(&fp);
   do
   {
       sock = socket(PF_INET,SOCK_DGRAM,0);
       if (sock == -1)
       {
           printf ("sorry couldn't able to open the socket \n");
           return res;
       }
try_again:
       /** get word **/
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           res = 0;
           break;
       }
       get_word_before_marker(pres_word,temp_word,':');
       /** check whether the given name is a valid interface **/
       printf("the interface name is %s\n",temp_word);
       memset(&ifr, '\0', sizeof ifr);
       strncpy(ifr.ifr_name, temp_word, IFNAMSIZ-1);

       if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
       {
          printf ("wrong IOCTL index \n");
          close (sock);
          return res;
       }
       if (ifr.ifr_ifindex != port_num)
       {
          skip_till_eol(&fp);
          goto try_again;
       }
       close (sock);
       res =  NCSCC_RC_SUCCESS;
 
       if (get_word_after_marker(pres_word, temp_word,':')==  NCSCC_RC_SUCCESS)
       {
           temp_var = atoi(temp_word);
       }
       if (temp_var == 0)
       {
          temp_var = file_get_word(&fp,pres_word);
          if (temp_var == IFSV_DRV_EOF)
             break;
          temp_var = atoi(pres_word);
       }
       o_info->i_bytes = temp_var;
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->i_pack = atoi(pres_word);
           break;
       }
       o_info->i_pack = atoi(pres_word);
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->i_error = atoi(pres_word);
           break;
       }
       o_info->i_error = atoi(pres_word);
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->i_drop = atoi(pres_word);
           break;
       }
       o_info->i_drop = atoi(pres_word);
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           break;
       }
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           break;
       }
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           break;
       }
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           break;
       }
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->o_bytes = atoi(pres_word);
           break;
       }
       o_info->o_bytes = atoi(pres_word);
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->o_pack = atoi(pres_word);
           break;
       }
       o_info->o_pack = atoi(pres_word);
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->o_error = atoi(pres_word);
           break;
       }
       o_info->o_error = atoi(pres_word);
       temp_var = file_get_word(&fp,pres_word);
       if (temp_var == IFSV_DRV_EOF)
       {
           o_info->o_drop = atoi(pres_word);
           break;
       }
       o_info->o_drop = atoi(pres_word);
       break;
   }while(0);
   close (sock);
   fclose(fp);
   return res;
}

/****************************************************************************
 * Name          : get_word_after_marker
 *
 * Description   : This is the utill function used to get word after the marker.
 *
 * Arguments     : i_word    - word in which we need to grep for a marker.
 *                 o_word    - output word after marker.
 *                 marker    - marker.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCS_RCC_FAILURE.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32
get_word_after_marker(char *i_word, char *o_word, char marker)
{
    uns32 len = strlen(i_word);
    uns32 temp_var;

    for (temp_var = 0; temp_var < len; temp_var++)
    {
        if (i_word[temp_var] == marker)
        {
            strip_n_bytes_in_str(i_word,o_word,(temp_var+1),len);
            return NCSCC_RC_SUCCESS;
        }
    }
    return NCSCC_RC_FAILURE;
}

/****************************************************************************
 * Name          : get_word_before_marker
 *
 * Description   : This is the utill function used to get word before the marker
 *
 * Arguments     : i_word    - word in which we need to grep for a marker.
 *                 o_word    - output word before marker.
 *                 marker    - marker.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCS_RCC_FAILURE.
 *
 * Notes         :  None.
 *****************************************************************************/
static unsigned int
get_word_before_marker(char *i_word, char *o_word, char marker)
{
    unsigned int len = strlen(i_word);
    unsigned int temp_var;

    for (temp_var = 0; temp_var < len; temp_var++)
    {
        if (i_word[temp_var] == marker)
        {
            o_word[temp_var] = '\0';
            return 1;
        }
        o_word[temp_var] = i_word[temp_var];
    }
    return 0;
}
/****************************************************************************
 * Name          : extract_phy_addr
 *
 * Description   : This is the utill function used to extract physical 
 *                 address from the string.
 *
 * Arguments     : i_str    - Input string.
 *                 o_phy    - extracted physical address.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCS_RCC_FAILURE.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32
extract_phy_addr(char *i_str, uns8 *o_phy)
{
   uns32 temp_var;
   uns32 temp_cnt;
   char temp_word[9];
   uns32 str_offset = 0;
   uns32 phy_offset = 0;

   if ((i_str == NULL) || (o_phy == NULL))
   {
      return NCSCC_RC_FAILURE;
   }
   
   temp_var = strlen(i_str);
   for (temp_cnt =0; ((temp_cnt < temp_var) && (phy_offset < 6)); temp_cnt++)
   {
      if (i_str[temp_cnt] != ':')
      {
         temp_word[str_offset] = i_str[temp_cnt];
         str_offset++;
      } else
      {
         temp_word[str_offset]='\0';
         o_phy[phy_offset] = (uns8)conv_str_to_byte_hex(temp_word);
         str_offset = 0;
         phy_offset++;
      }
   } 
   temp_word[str_offset]='\0';
   o_phy[phy_offset] = (uns8)conv_str_to_byte_hex(temp_word);
   
   return NCSCC_RC_SUCCESS;
}
/****************************************************************************
 * Name          : conv_str_to_byte_hex
 *
 * Description   : This is the utill function used to convert the hex string 
 *                 into decimal.
 *
 * Arguments     : i_str    - Input string.
 *
 * Return Values : hex value/0-onerror.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32 
conv_str_to_byte_hex(char *i_str)
{
   uns32 temp_cnt;
   uns32 temp_var;
   uns32 str_size;
   uns32 value = 0;

   str_size = strlen(i_str);
   for (temp_cnt = 0; temp_cnt < str_size; temp_cnt++)
   {
      temp_var = find_hex_value(i_str[temp_cnt]);
      if (temp_var != 0)
         temp_var = (temp_var * pow(16,(str_size - (temp_cnt +1))));
      value = (value + temp_var);  
   }
   return (value);
}

/****************************************************************************
 * Name          : find_hex_value
 *
 * Description   : This is the utill function used to find the value of 
 *                  the given hex ascii character. 
 *
 * Arguments     : hex_char- Input hex character.
 *
 * Return Values : hex value/0-onerror.
 *
 * Notes         :  None.
 *****************************************************************************/
static uns32
find_hex_value(char hex_char)
{
   uns32 value = 0;

   if (hex_char == '0')
   {
      value = 0;
   } else if (hex_char == '1')
   {
      value = 1;
   }else if (hex_char == '2')
   {
      value = 2;
   }else if (hex_char == '3')
   {
      value = 3;
   }else if (hex_char == '4')
   {
      value = 4;
   }else if (hex_char == '5')
   {
      value = 5;
   }else if (hex_char == '6')
   {
      value = 6;
   }else if (hex_char == '7')
   {
      value = 7;
   }else if (hex_char == '8')
   {
      value = 8;
   }else if (hex_char == '9')
   {
      value = 9;
   }else if (tolower(hex_char) == 'a')
   {
      value = 10;
   }else if (tolower(hex_char) == 'b')
   {
      value = 11;
   }else if (tolower(hex_char) == 'c')
   {
      value = 12;
   }else if (tolower(hex_char) == 'd')
   {
      value = 13;
   }else if (tolower(hex_char) == 'e')
   {
      value = 14;
   }else if (tolower(hex_char) == 'f')
   {
      value = 15;
   }
   return (value);
}
