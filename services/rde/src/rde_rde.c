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

  $$

  MODULE NAME: rde_rde.c

..............................................................................

 DESCRIPTION:

  This file contains an implementation of the RDE rde_rde_cb Interface
  The rde_rde_cb Interface is used for communication between RDE RDE


******************************************************************************/


#include "rde.h"
#include "rde_cb.h"
static uns32 rde_rde_process_send_slot_number (void);
static uns32 rde_rde_exchange_slot_number(RDE_CONTROL_BLOCK * rde_cb);
/*****************************************************************************

  PROCEDURE NAME:       rde_rde_parse_config_file

  DESCRIPTION:          To read the Initial RDE RDE role, IP and PORT numbers

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully parse the config file
    RDE_RDE_RC_FAILURE:   Failure to parse the config file

  NOTES:

*****************************************************************************/

uns32 rde_rde_parse_config_file()
 {
  char line[256];
  char *str=NULL;
  int rc,i;
   
  FILE *fp;
 RDE_CONTROL_BLOCK * rde_cb; 
 rde_cb = rde_get_control_block();
  
  fp = fopen(RDE_RDE_CONFIG_FILE, "r");  /*This CONFIGPATH can be declared in etc/rde file*/
  if (fp == NULL)
  {
        m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_UNABLE_TO_OPEN_CONFIG_FILE , RDE_RDE_CONFIG_FILE); 
        return RDE_RDE_RC_FAILURE;
  }
  while(fgets (line, sizeof (line), fp) != NULL)
  {
       i = strncmp (line, "HOSTIPADDRESS",13);
       if (i == 0) 
       {
          str=strstr(line,"=");
          str++;
          strcpy(rde_cb->rde_rde_cb.hostip,str);
          rde_rde_strip(rde_cb->rde_rde_cb.hostip);
       }      
       i = strncmp (line, "HOSTPORTNUM",11);
       if (i == 0)
       {
          str=strstr(line,"=");
          str++;
          rde_cb->rde_rde_cb.hostportnum = atoi(str++);
       }
       i = strncmp (line, "SERVERIPADDRESS",15);
       if (i == 0) 
       {
          str=strstr(line,"=");
          str++;
          strcpy(rde_cb->rde_rde_cb.servip,str++);
          rde_rde_strip(rde_cb->rde_rde_cb.servip);
       }      
       i = strncmp (line, "SERVERPORTNUM",13);
       if (i == 0)
       {
          str=strstr(line,"=");
          str++;
          rde_cb->rde_rde_cb.servportnum = atoi(str++);
       }
       i = strncmp (line,"HA_ROLE",7);
       if (i == 0)
       {
          str=strstr(line,"=");
          str++;
          rde_cb->ha_role = atoi(str++);
       }
       i = strncmp (line,"MAX_CLIENT_RETRIES",11);
       if (i == 0)
       {
          str=strstr(line,"=");
          str++;
          rde_cb->rde_rde_cb.maxNoClientRetries= atoi(str++);
       }
     }
      
     rc = fclose(fp);
     if ( rc != 0 )
     {
        m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_FAIL_TO_CLOSE_CONFIG_FILE, "config_file");
        return RDE_RDE_RC_FAILURE;
     }
    return  RDE_RDE_RC_SUCCESS;   
    
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_open

  DESCRIPTION:            Open the socket and initialization of 
                          the RDE Interface

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully opened the socket
    RDE_RDE_RC_FAILURE:   Failure opening socket

  NOTES:

*****************************************************************************/

uns32 rde_rde_open (RDE_RDE_CB *rde_rde_cb)
{
   char  log[RDE_LOG_MSG_SIZE] = {0};
   /***************************************************************
    *                                                               *
    *         Open socket for communication with rde_rde_cb         *
    *                                                               *
    *****************************************************************/

   if (rde_rde_sock_open(rde_rde_cb) != RDE_RDE_RC_SUCCESS)
   {
      sprintf(log,"rde_rde_sock_open fail");
      m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_ERROR, log);
      return RDE_RDE_RC_FAILURE;
   }
   /***************************************************************\
    *                                                               *
    *         Initialize socket settings                            *
    *                                                               *
   \***************************************************************/
   
   rde_rde_cb->host_addr.sin_family = AF_INET;
   rde_rde_cb->host_addr.sin_addr.s_addr = inet_addr(rde_rde_cb->hostip);
   rde_rde_cb->host_addr.sin_port = htons(rde_rde_cb->hostportnum);

   
   if (rde_rde_sock_init(rde_rde_cb) != RDE_RDE_RC_SUCCESS)
   {
      sprintf(log,"rde_rde_sock_init fail");
      m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_ERROR, log);
      return RDE_RDE_RC_FAILURE;
   }
   return RDE_RDE_RC_SUCCESS;
}
/*****************************************************************************

  PROCEDURE NAME:         rde_rde_sock_open

  DESCRIPTION:            To Create the socket interface

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully created socket
    RDE_RDE_RC_FAILURE:   Failure to create socket

  NOTES:

*****************************************************************************/

uns32 rde_rde_sock_open (RDE_RDE_CB *rde_rde_cb)
{
   uns32 rc= RDE_RDE_RC_SUCCESS;

   rde_rde_cb->fd = socket (AF_INET, SOCK_STREAM, 0) ;

   if (rde_rde_cb->fd < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CREATE_FAIL, errno);       
      return RDE_RDE_RC_FAILURE;
   }

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_sock_init

  DESCRIPTION:            Initializing the socket

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully initialized socket
    RDE_RDE_RC_FAILURE:   Failure to initialize socket

  NOTES:

*****************************************************************************/
uns32 rde_rde_sock_init (RDE_RDE_CB *rde_rde_cb)
{  
   int rc,opt_value;

   opt_value = 1;

   /* To allow binding on a socket which is already in a TIME_WAIT state */
   if (setsockopt(rde_rde_cb->fd,SOL_SOCKET, SO_REUSEADDR, (char *)&opt_value, sizeof(opt_value)) < 0)
   {
     m_RDE_LOG_COND_C (RDE_SEV_INFO, RDE_RDE_SET_SOCKOPT_ERROR, "SETSOCKOPT ERROR");
   }
 
   rc = bind (rde_rde_cb->fd,
             (struct sockaddr *) &rde_rde_cb-> host_addr,
              sizeof(rde_rde_cb-> host_addr));

   if (rc < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_BIND_FAIL, errno);
      return RDE_RDE_RC_FAILURE;
   }
   
   rc = listen(rde_rde_cb->fd, 2);
   if (rc < 0)
   {
     m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_LISTEN_FAIL, errno); 
     return RDE_RDE_RC_FAILURE;
   }

   return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_process_msg

  DESCRIPTION:            To accept the clinet interface

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully accept the other RDE 
    RDE_RDE_RC_FAILURE:   Failure to accept the other RDE 

  NOTES:

*****************************************************************************/
uns32 rde_rde_process_msg (RDE_RDE_CB *rde_rde_cb)
{  
   int newsockfd,cli_addr_len,rc;
   cli_addr_len = sizeof(rde_rde_cb->cli_addr);
   rc = 0;
   if(rde_rde_cb->connRecv == TRUE)
   {
     rc = close(rde_rde_cb->recvFd);
   }
   if (rc < 0)
   {
   /*    m_RDE_LOG_COND_C (RDE_SEV_INFO, RDE_RDE_SOCK_CLOSE_FAIL, "recvFd Faild"); */
   }
   newsockfd = accept (rde_rde_cb->fd, (struct sockaddr *) &rde_rde_cb->cli_addr,&cli_addr_len);

   if (newsockfd < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_ACCEPT_FAIL, 0);
      return RDE_RDE_RC_FAILURE;
   }
   m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Accepted Connection from other RDE \n");

   rde_rde_cb->connRecv = TRUE;
   rde_rde_cb->recvFd       = newsockfd;
   return RDE_RDE_RC_SUCCESS;
}
/*****************************************************************************

  PROCEDURE NAME:         rde_rde_sock_close

  DESCRIPTION:            To Close the socket interface

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully closed socket interface
    RDE_RDE_RC_FAILURE:   Failure to close socket interface

  NOTES:

*****************************************************************************/
uns32 rde_rde_sock_close (RDE_RDE_CB *rde_rde_cb)
{
/* All the active fds should be closed here */
   int rc;
   rc = close (rde_rde_cb-> fd);
   if (rc < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
      return RDE_RDE_RC_FAILURE;
   }
  if (rde_rde_cb->clientConnected == TRUE)
  {
    rc = close (rde_rde_cb-> clientfd);
    if (rc < 0)
    {
            m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
            return RDE_RDE_RC_FAILURE;
    }
  }
  if (rde_rde_cb->connRecv == TRUE)
  {
    rc = close (rde_rde_cb->recvFd);
    if (rc < 0)
    {
            m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
            return RDE_RDE_RC_FAILURE;
    }
  }
   return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_close

  DESCRIPTION:            To Close all the socket interface

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully closed socket interface
    RDE_RDE_RC_FAILURE:   Failure to close socket interface

  NOTES:

*****************************************************************************/
uns32 rde_rde_close (RDE_RDE_CB *rde_rde_cb)
{

/* close all the open fds */
   if (rde_rde_sock_close(rde_rde_cb) != RDE_RDE_RC_SUCCESS)
   {
       return RDE_RDE_RC_FAILURE;
   }

   rde_rde_cb-> fd = -1;
   rde_rde_cb-> clientfd = -1;
   rde_rde_cb-> recvFd = -1;

   return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_write_msg

  DESCRIPTION:            Write a message to other RDE 

  ARGUMENTS:
    fd                    File descriptor to send  
    msg                   Message to write

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully write a message
    RDE_RDE_RC_FAILURE:   Failure to write a messge 

  NOTES:

*****************************************************************************/
 uns32 rde_rde_write_msg (int fd, char *msg)
{
   int         rc         = RDE_RDE_MSG_WRT_SUCCESS;
   int         msg_size   = 0;
   /* int errno; */

   msg_size = write  (fd, msg, strlen(msg));
   if (msg_size < 0)
   {
     if (errno != EINTR && errno != EWOULDBLOCK)
         /* Non-benign error */
    /* Do the processing that is done when the client connection is lost here */
     m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno); 
     return RDE_RDE_MSG_WRT_FAILURE;
   }
   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_read_msg

  DESCRIPTION:            Read message from the other RDE

  ARGUMENTS:
    fd                    File descriptor to send
    msg                   Message to write
    size                  Size of the message
  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully read a message
    RDE_RDE_RC_FAILURE:   Failure to read a messge

  NOTES:

*****************************************************************************/
uns32 rde_rde_read_msg (int fd, char *msg, int size)
{
   int msg_size  = 0;
   msg_size = read (fd, msg, size);
   if (msg_size < 0)
   {
       m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_READ_FAIL, errno);  
       return RDE_RDE_RC_FAILURE;
   }
/* If the client connection is lost then need not maintain the fd and wait for fresh connection. */
   if (msg_size == 0)
   {
    return RDE_RDE_RC_FAILURE;
   }   
   return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_client_process_msg

  DESCRIPTION:            To process the message and and send the role to 
                          other RDE

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully process the message
    RDE_RDE_RC_FAILURE:   Failure to process the messge

  NOTES:

*****************************************************************************/
uns32 rde_rde_client_process_msg(RDE_RDE_CB  * rde_rde_cb)
{
    char  msg [RDE_RDE_MSG_SIZE] = {0};
    uns32 rc        = RDE_RDE_RC_SUCCESS;

    if (rde_rde_read_msg (rde_rde_cb->recvFd, msg, sizeof (msg)) != RDE_RDE_RC_SUCCESS)
    {
      rc = close(rde_rde_cb->recvFd);
      if ( rc < 0)
      {
          m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
          rde_rde_cb->connRecv = FALSE;
          return RDE_RDE_RC_FAILURE;
      }
         rde_rde_cb->connRecv = FALSE;
         return RDE_RDE_RC_FAILURE;
    }
    /*
    ** Parse the message for cmd type and value
    */
    rc = rde_rde_parse_msg(msg,rde_rde_cb);
    if (rc == RDE_RDE_CLIENT_ROLE_REQUEST)
     {
    /* send the role */
        rc = rde_rde_process_send_role ();

    /* Return value not set, ok to lose the connection*/
        if (rc != RDE_RDE_RC_SUCCESS)
        {
              m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno);
              return RDE_RDE_RC_FAILURE; 
        }
        else
        {
              return RDE_RDE_RC_SUCCESS; 
        }
     }
     else if(rc == RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_REQ)
     {
        rc = rde_rde_process_send_slot_number();
        if (rc != RDE_RDE_RC_SUCCESS)
        {
              m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno);
              return RDE_RDE_RC_FAILURE; 
        }
        else
        {
              return RDE_RDE_RC_SUCCESS; 
        }
     }
     else
     {
         /* The message received from the other rde client is not valid */
         return RDE_RDE_RC_FAILURE; 
     }
    
  return rc; 
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_parse_msg

  DESCRIPTION:            To evaluate the other RDE's message

  ARGUMENTS:
      msg                 Message   

  RETURNS:

    Request Type      :   Valid request type
    RDE_RDE_RC_FAILURE:   RDE request is not valid message

  NOTES:

*****************************************************************************/
uns32 rde_rde_parse_msg(char * msg,RDE_RDE_CB  * rde_rde_cb)
{
    int req = 0;
    char  log[RDE_LOG_MSG_SIZE] = {0};
    uns32 slot_number = 0;
    char *ptr = NULL;

    ptr = strchr (msg, ' ');

    if(ptr == NULL)
    {
      req = atoi (msg);
    }
    else
    {
        *ptr = '\0';
        req = atoi (msg);
        slot_number = atoi (++ptr);
    }
    if (req==RDE_RDE_CLIENT_ROLE_REQUEST)
    {
        /* The client request is a valid request for the role */
        m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Received RDE_RDE_CLIENT_ROLE_REQUEST from other RDE\n");
        return req;
    }
    else if(req == RDE_RDE_REBOOT_CMD)
    {
      m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Received RDE_RDE_REBOOT_CMD from other RDE\n");
      sleep(5);
      rde_rde_close(rde_rde_cb);
      sleep(5);
      m_NCS_REBOOT;  
      exit(0); 
    }
   else if(req == RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_REQ)
   {
    sprintf(log,"Received RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_REQ from other RDE. Peer Slot Number %d\n",slot_number);
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
    rde_rde_cb->peer_slot_number = slot_number; 
    return req;
   }
   else
   {
     m_RDE_LOG_COND_C (RDE_SEV_INFO, RDE_RDE_RECV_INVALID_MESSAGE, msg);
   }
        return RDE_RDE_RC_FAILURE;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_process_send_role

  DESCRIPTION:            To SEND the ha_role to the requested RDA 

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully SEND the role to the RDA 
    RDE_RDE_RC_FAILURE:   Failure to SEND the role to RDA

  NOTES:

*****************************************************************************/

uns32 rde_rde_process_send_role ()
{
    int rc;
    char msg [RDE_RDE_MSG_SIZE] = {0};
    RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();
    RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
    char  log[RDE_LOG_MSG_SIZE] = {0};
    NCS_BOOL conn_estab = FALSE;

    conn_estab = rde_rde_get_set_established(FALSE);
    if(conn_estab == FALSE)
    {
      rde_cb->ha_role = PCS_RDA_ACTIVE;
      rde_rde_get_set_established(TRUE);
      rde_rde_update_config_file(rde_cb->ha_role);
      rde_rda_send_role (rde_cb->ha_role);
    }

    sprintf (msg, "%d %d", RDE_RDE_CLIENT_ROLE_RESPONSE,rde_cb->ha_role);
    if (rde_rde_write_msg (rde_rde_cb->recvFd, msg) != RDE_RDE_MSG_WRT_SUCCESS)
    {
      rc = close(rde_rde_cb->recvFd);   
      rde_rde_cb->connRecv = FALSE;
      if ( rc < 0)
      {
          m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
          
      }
    
    /* close the fd */
        return RDE_RDE_RC_FAILURE;
    }
    sprintf(log,"Role %d sent to other RDE\n",rde_cb->ha_role);
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);

    return RDE_RDE_RC_SUCCESS;

}
/*****************************************************************************

  PROCEDURE NAME:         rde_rde_client_socket_init

  DESCRIPTION:            Initializing the other RDEs' socket

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully initialized other RDE socket
    RDE_RDE_RC_FAILURE:   Failure to initialize other RDE socket

  NOTES:

*****************************************************************************/

uns32 rde_rde_client_socket_init(RDE_RDE_CB *rde_rde_cb)
{
   uns32 rc =  RDE_RDE_RC_SUCCESS;

   rde_rde_cb->clientfd = socket (AF_INET, SOCK_STREAM, 0) ;

   if (rde_rde_cb->clientfd < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_CLIENT_SOCK_CREATE_FAIL, errno);
      return RDE_RDE_RC_FAILURE;
   }

   bzero( &rde_rde_cb->serv_addr, sizeof(rde_rde_cb->serv_addr));

   rde_rde_cb->serv_addr.sin_family = AF_INET;
   rde_rde_cb->serv_addr.sin_addr.s_addr = inet_addr(rde_rde_cb->servip);
   rde_rde_cb->serv_addr.sin_port = htons(rde_rde_cb->servportnum);

   return rc;   
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_get_set_established

  DESCRIPTION:            To know the establisment of the role

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:   
    RDE_RDE_RC_FAILURE:   

  NOTES:

*****************************************************************************/
NCS_BOOL rde_rde_get_set_established(NCS_BOOL e)
{
    static NCS_BOOL role_established = FALSE;

    if (e==FALSE)
    {
        /* request to get the flag */
        return role_established;
    }
    else
    {
        /* request to set the flag */
        role_established = TRUE; /* role is established */ 
        return role_established;
    }
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_clCheckConnect

  DESCRIPTION:            Connection establishment with other RDE 
                          and to send the role request

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully to connected to other RDE
    RDE_RDE_RC_FAILURE:   Failure to connected to other RDE

  NOTES:

*****************************************************************************/

uns32 rde_rde_clCheckConnect(RDE_RDE_CB *rde_rde_cb)
{
        int status;
        int msg_size   = 0;
        char msg [RDE_RDE_MSG_SIZE] = {0};
        char  log[RDE_LOG_MSG_SIZE] = {0};

        if (((rde_rde_cb->clientReconnCount <  rde_rde_cb->maxNoClientRetries) && (FALSE == rde_rde_cb->clientConnected)) ||
            rde_rde_cb->retry == TRUE)
        {
           if(rde_rde_connect(rde_rde_cb)!=RDE_RDE_RC_SUCCESS) 
           {
             sprintf(log,"Connect Failed. Retry Count %d\n",rde_rde_cb->clientReconnCount);
             m_RDE_LOG_COND_C(RDE_SEV_INFO, RDE_RDE_INFO, log);
             return RDE_RDE_RC_FAILURE;
           }
      
        }
                /* send the request for role here */
                /* if the send fails then we need to reconnect again  */
                /* if ( rde_rde_cb->clientConnected == TRUE )  */
           if ((rde_rde_cb->clientConnected == TRUE) && (rde_rde_cb->connRecv == FALSE))
            {
                sprintf(msg,"%d",RDE_RDE_CLIENT_ROLE_REQUEST);
                msg_size = write  (rde_rde_cb->clientfd, msg, strlen(msg));
                if (msg_size < 0)
                {
                    /* Do the processing that is done when the client connection is lost here */
                    if (errno != EINTR && errno != EWOULDBLOCK)
                    /* Non-benign error */
                     m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno);
                 
                     status = close(rde_rde_cb->clientfd);
                        if ( status < 0 )
                        {
                            m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
                        }
                   rde_rde_cb->clientReconnCount = 0;
                    rde_rde_cb->retry = TRUE;
                    rde_rde_cb->clientConnected = FALSE;

                    if( rde_rde_client_socket_init(rde_rde_cb)!= RDE_RDE_RC_SUCCESS)
                    {
                        /* If the socket initialization fails then we need to exit */
                        return RDE_RDE_RC_FAILURE;
                    }
                    else 
                    {
                         return RDE_RDE_RC_SUCCESS;
                    }


              }
              else 
              {
                    sprintf(log,"RDE_RDE_CLIENT_ROLE_REQUEST sent. Size %d\n",msg_size);
                    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
                    return RDE_RDE_RC_SUCCESS;
              }

            }
     return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_client_read_role

  DESCRIPTION:            get the role of other RDE and update the current role 
                          based on the role of other RDE

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully initialize the role 
    RDE_RDE_RC_FAILURE:   Failure to initialize the role

  NOTES:

*****************************************************************************/
uns32 rde_rde_client_read_role ()
{
   int msg_size; 
   PCS_RDA_ROLE role;
   char msg[RDE_RDE_MSG_SIZE] = {0};
   int status ;  
   RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();
   char  log[RDE_LOG_MSG_SIZE] = {0};
   char              *ptr;
   uns32 msg_type = 0;
   uns32 slot_number = 0;
   uns32 value = 0;

   msg_size = read (rde_cb->rde_rde_cb.clientfd, msg, sizeof(msg));
   if (msg_size < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_READ_FAIL, errno);
      status = close(rde_cb->rde_rde_cb.clientfd);
      rde_cb->rde_rde_cb.retry = TRUE;
      rde_cb->rde_rde_cb.clientConnected = FALSE;
      rde_cb->rde_rde_cb.clientReconnCount = 0;
      if(status < 0)
      {
           m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
      }
      if( rde_rde_client_socket_init(&rde_cb->rde_rde_cb)!= RDE_RDE_RC_SUCCESS) 
      {
         /* If the socket initialization fails then we need to exit */
          return RDE_RDE_RC_FAILURE;
      }
   return RDE_RDE_RC_SUCCESS; /* it is okay if th client is not able to connect this time */
   }

   if (msg_size == 0)
   {
        m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_READ_FAIL, errno);
        status = close(rde_cb->rde_rde_cb.clientfd);
    /* Check for the return status of close and log message as required */
        if(status < 0)
        {
        m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
        }
        if( rde_rde_client_socket_init(&rde_cb->rde_rde_cb)!= RDE_RDE_RC_SUCCESS)   
        {
             /* If the socket initialization fails then we need to exit */
             return RDE_RDE_RC_FAILURE;
        }
   rde_cb->rde_rde_cb.retry = TRUE;
   rde_cb->rde_rde_cb.clientConnected = FALSE;
   rde_cb->rde_rde_cb.clientReconnCount = 0;
   }    
   else
   {
    ptr = strchr (msg, ' ');
    if (ptr == NULL)
    {
      msg_type = atoi (msg);
    }
    else
    {
        *ptr = '\0';
        msg_type = atoi (msg);
        value    = atoi (++ptr);
    }
    if(msg_type == RDE_RDE_CLIENT_ROLE_RESPONSE)
    {
     role = value;
     if ( (role == PCS_RDA_ACTIVE) || (role == PCS_RDA_STANDBY) )
     {
     /*  update the config file, update the ha_role in the control block and update the rda about the changed role */
        if (role == PCS_RDA_ACTIVE)
        {
            /* The other server is active , so the current server is standby*/
            /* check how to update rda about role */
            m_NCS_OS_MEMSET(log,0,RDE_LOG_MSG_SIZE);
            sprintf(log,"Present RDE Role = %d. Changing role to PCS_RDA_STANDBY\n",rde_cb->ha_role);
            m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
            rde_cb->ha_role = PCS_RDA_STANDBY;
        }

        else
        {
            m_NCS_OS_MEMSET(log,0,RDE_LOG_MSG_SIZE);
            sprintf(log,"Present RDE Role = %d. Changing role to PCS_RDA_ACTIVE\n",rde_cb->ha_role);
            m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
            rde_cb->ha_role = PCS_RDA_ACTIVE;
        }
        /* set that the role is established */
        rde_rde_get_set_established(TRUE);
        /* update the entry in the config file */
        rde_rde_update_config_file(rde_cb->ha_role);
        rde_rda_send_role (rde_cb->ha_role);
        rde_rde_exchange_slot_number(rde_cb);

     }
     else
     {
        /* erroneous message the request needs to be sent again */
        sprintf(log,"Wrong Role = %d received from other RDE.\n",role);
        m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);
        return RDE_RDE_RC_FAILURE;
     }
    }        
    else if(msg_type == RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_RESP)
    {
        slot_number = value;     
        m_NCS_OS_MEMSET(log,0,RDE_LOG_MSG_SIZE);
        sprintf(log,"Recieved RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_RESP. Peer Slot Number = %d\n",slot_number);
        m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
        rde_cb->rde_rde_cb.peer_slot_number = slot_number; 
    }
    else
    {
        /* erroneous message the request needs to be sent again */
        sprintf(log,"Wrong Request Msg = %d received from other RDE.\n",msg_type);
        m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);
        return RDE_RDE_RC_FAILURE;
    }
   }    
    /* Return success in any case for now, ok to lose the connection */
      return RDE_RDE_RC_SUCCESS;
}
/*****************************************************************************

  PROCEDURE NAME:         rde_rde_strip

  DESCRIPTION:            To truncate the space in the string

  ARGUMENTS:
     name                 String to be truncated
  
  RETURNS:

    RDE_RDE_RC_SUCCESS:   
    RDE_RDE_RC_FAILURE:   

  NOTES:

*****************************************************************************/
void rde_rde_strip(char* name)
{
  char* s = name ;
  size_t n ;
  while (isspace(*s))
    ++s ;
  if ((n = strlen(s)) > 0)
    while (isspace(s[n-1]))
      --n ;
  ((char*)memmove(name, s, n))[n] = '\0' ;
  return;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_update_config_file

  DESCRIPTION:            set the updated role in the config_file 

  ARGUMENTS:
      role                Role to update configuration file

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully updated the config file
    RDE_RDE_RC_FAILURE:   Failure to update the config file 

  NOTES:

*****************************************************************************/
uns32 rde_rde_update_config_file(PCS_RDA_ROLE role)
{

  int i=0,rc =0,index,actPos;
  char line[256],tmp[256];
  char *lines[10];
  index = 0;
  char  log[RDE_LOG_MSG_SIZE] = {0};
  RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();
  RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
  
  /* Check that the NID reponse has been sent or not. */ 
  if(rde_rde_cb->nid_ack_sent == FALSE)
  {
    /* NID ACK has not been sent yet, so sent it now. */
      NID_STATUS_CODE nid_stat_code;
      uns32 error;
      nid_stat_code.hlfm_status =  NCSCC_RC_SUCCESS;
      nid_notify(NID_RDF, nid_stat_code, &error);
      rde_rde_cb->nid_ack_sent = TRUE;
  }

  sprintf(log,"Updating Node's HA state to %d\n",role);
  m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);

/* Update the HA state to the /var/opt/opensaf/node_ha_state file */
   FILE *fp = fopen(NODE_HA_STATE,"w");
   if (fp ==NULL)
   {
       m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_UNABLE_TO_OPEN_CONFIG_FILE,NODE_HA_STATE);
       return RDE_RDE_RC_FAILURE;
   }
   fprintf(fp,"%s %s","This System Controller is in",((role == 0) ? "HA ACTIVE State":"HA STANDBY State"));
   rc = fclose(fp);
   if ( rc != 0 )
   {
       m_RDE_LOG_COND_C(RDE_SEV_ERROR,RDE_RDE_FAIL_TO_CLOSE_CONFIG_FILE,NODE_HA_STATE);
   }
      return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_count_testing

  DESCRIPTION:            Attempts for connectin to other RDE  
                          and set the role

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:   
    RDE_RDE_RC_FAILURE:   

  NOTES:

*****************************************************************************/
uns32 rde_count_testing ()
{

 RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();
 char  log[RDE_LOG_MSG_SIZE] = {0};

 RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
                                                                                
 /* If there was a request to connect but the maximum no of retries is
 done then set the role to active*/
                                                                                
 if ( rde_rde_cb->clientReconnCount == rde_rde_cb->maxNoClientRetries)
 {
         rde_cb->ha_role = PCS_RDA_ACTIVE;
         rde_rde_get_set_established(TRUE);
         sprintf(log,"Peer RDE not found, tried %d times, being ACTIVE\n",rde_rde_cb->maxNoClientRetries);
         m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
         rde_rde_update_config_file(rde_cb->ha_role);
         rde_rda_send_role (rde_cb->ha_role);
         return RDE_RDE_RC_SUCCESS;
                                                                                
 }
                                                                                
 return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_set_role

  DESCRIPTION:            Attempts for connectin to other RDE
                          and set the role

  ARGUMENTS:
     role                 updated role

  RETURNS:

    RDE_RDE_RC_SUCCESS:   
    RDE_RDE_RC_FAILURE:   

  NOTES:

*****************************************************************************/

uns32 rde_rde_set_role (PCS_RDA_ROLE role)
{
   char  log[RDE_LOG_MSG_SIZE] = {0};
   RDE_CONTROL_BLOCK *rde_cb =  rde_get_control_block();

   sprintf(log,"rde_rde_set_role: role set to %d",role);
   m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);

       rde_cb->ha_role = role;

       if (role == PCS_RDA_QUIESCED)
       {
           return NCSCC_RC_SUCCESS;
       }
       /*
       ** Send new role to RDA
       */
       rde_rde_update_config_file(rde_cb->ha_role);
       rde_rda_send_role (rde_cb->ha_role);
   /* send the role to the other rde as well */

      rde_rde_process_send_role(); 
       return NCSCC_RC_SUCCESS;
}
/*****************************************************************************

  PROCEDURE NAME:         rde_rde_hb_err

  DESCRIPTION:            

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:
    RDE_RDE_RC_FAILURE:

  NOTES:

*****************************************************************************/

uns32 rde_rde_hb_err ()
{
                                                                                                                              
   RDE_CONTROL_BLOCK *rde_cb =  rde_get_control_block();
   RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
   char msg [RDE_RDE_MSG_SIZE] = {0}; 
   int msg_size   = 0;
    
   if (rde_cb->ha_role == PCS_RDA_ACTIVE)
   {
       /* Check about the link health.*/
       if(TRUE == rde_rde_cb->clientConnected)
       {
         /* Send reboot command to STDBY. */
            m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Role ACTIVE. Heart Beat Loss Msg received. Sending Reboot Command to other RDE\n");
            rde_rda_send_node_reset_to_avm(rde_rde_cb);
            sprintf(msg,"%d",RDE_RDE_REBOOT_CMD);
            msg_size = write  (rde_rde_cb->clientfd, msg, strlen(msg)); 
            if (msg_size < 0)
            {
               m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno);
               return RDE_RDE_RC_FAILURE;
            }
            else
            {
               return RDE_RDE_RC_SUCCESS;
            }
       }
       else
       {
         rde_rda_send_node_reset_to_avm(rde_rde_cb);
         m_RDE_LOG_COND_C(RDE_SEV_CRITICAL, RDE_RDE_INFO, "Role ACTIVE. Heart Beat Loss Msg received. RDE-RDE not connected\n");
       }
       /*
       ** We have nothing to do
       */
       return NCSCC_RC_SUCCESS;
   }
   if (rde_cb->ha_role == PCS_RDA_STANDBY)
   {
       /* Check about the link health.*/
       if(TRUE == rde_rde_cb->clientConnected)
       {
         /* Wait for reboot command to come.
            If it doesn't get reboot command then send reboot command to ACT. */
         m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Role STDBY. Heart Beat Loss Msg received.  Waiting for reboot cmd from other RDE\n");
           rde_rde_cb->hb_loss = TRUE;
       }
       else
       {
         rde_rde_cb->hb_loss = TRUE;
         /* Log a message. */
         m_RDE_LOG_COND_C(RDE_SEV_CRITICAL, RDE_RDE_INFO, "Role STDBY. Heart Beat Loss Msg received. RDE-RDE not connected\n");
       }
     
   }
   /*
   ** It is a standby server and got heartbeat error
   ** so change the role of the server 
   */
   return RDE_RDE_RC_SUCCESS;
                                                                                                                              
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_Connect

  DESCRIPTION:            Connection establishment with other RDE

  ARGUMENTS:
     rde_rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully to connected to other RDE
    RDE_RDE_RC_FAILURE:   Failure to connected to other RDE

  NOTES:

*****************************************************************************/
uns32 rde_rde_connect(RDE_RDE_CB * rde_rde_cb)
{
           int rc;
           char  log[RDE_LOG_MSG_SIZE] = {0};

           rc = connect(rde_rde_cb->clientfd, (struct sockaddr *) &rde_rde_cb->serv_addr, sizeof(rde_rde_cb->serv_addr));
           rde_rde_cb->clientReconnCount++;

           if (rc < 0)
           {
                m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CONN_FAIL, errno);
                close(rde_rde_cb->clientfd);
                if( rde_rde_client_socket_init(rde_rde_cb)!= RDE_RDE_RC_SUCCESS)
                {
                    return RDE_RDE_RC_FAILURE;
                }
           }
           else
           {
                sprintf(log,"rde_rde_connect:Connect SUCCESS. Retry Count %d\n",rde_rde_cb->clientReconnCount);
                m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO,log); 
                rde_rde_cb->clientConnected = TRUE;
                rde_rde_cb->clientReconnCount = 0;
                rde_rde_cb->retry = FALSE;
               /* connection is successful */
           }
           return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_process_hb_loss_stdby

  DESCRIPTION:            Wait for some time and then sends Reboot to ACT. 

  ARGUMENTS:
     rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully to connected to other RDE
    RDE_RDE_RC_FAILURE:   Failure to connected to other RDE

  NOTES:

*****************************************************************************/
uns32 rde_rde_process_hb_loss_stdby(RDE_CONTROL_BLOCK * rde_cb)
{
  char msg [RDE_RDE_MSG_SIZE] = {0};
  int msg_size   = 0;
  RDE_RDE_CB  *rde_rde_cb  = &rde_cb->rde_rde_cb;

  if(rde_rde_cb->hb_loss == TRUE)
  {
   if(rde_rde_cb->count == RDE_RDE_WAIT_CNT)
   {
    rde_rde_cb->count = 0;
    rde_rde_cb->hb_loss = FALSE;
    rde_cb->ha_role = PCS_RDA_ACTIVE;
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Wait Time Over : Switching STDBY to ACTIVE\n");
    rde_rde_update_config_file(rde_cb->ha_role);
    rde_rda_send_role (rde_cb->ha_role);
    /* Check about the link health.*/
    if(TRUE == rde_rde_cb->clientConnected)
    {
         rde_rda_send_node_reset_to_avm(rde_rde_cb);
         /* Send reboot command to STDBY. */
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, "Reboot Cmd Not received from ACT. So sending Reboot Cmd to ACT\n");
            sprintf(msg,"%d",RDE_RDE_REBOOT_CMD);
            msg_size = write  (rde_rde_cb->clientfd, msg, strlen(msg));
            if (msg_size < 0)
            {
               m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno);
               return RDE_RDE_RC_FAILURE;
            }
            else
            {
               return RDE_RDE_RC_SUCCESS;
            }
    }
    else
    {
       rde_rda_send_node_reset_to_avm(rde_rde_cb);
       /* Log a message. */
       m_RDE_LOG_COND_C(RDE_SEV_CRITICAL, RDE_RDE_INFO, "Reboot Cmd Not received from ACT. Reboot Cmd couldn't be sent to ACT as RDE-RDE Connection not Available\n");
    }

   }/* rde_rde_cb->count == 2 */
   else 
   {
     sleep(1);
   }

    rde_rde_cb->count++;
  }/* rde_rde_cb->hb_loss == TRUE  */  
 return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_exchange_slot_number

  DESCRIPTION:            Will send its slot id to its peer.

  ARGUMENTS:
     rde_cb           Pointer to RDE Socket Config structure

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully wrote to connected to other RDE
    RDE_RDE_RC_FAILURE:   Failure to write to connected to other RDE

  NOTES:

*****************************************************************************/
static uns32 rde_rde_exchange_slot_number(RDE_CONTROL_BLOCK * rde_cb)
{
   int status;
   int msg_size   = 0;
   RDE_RDE_CB  *rde_rde_cb  = &rde_cb->rde_rde_cb;
   char msg [RDE_RDE_MSG_SIZE] = {0};
   char  log[RDE_LOG_MSG_SIZE] = {0};

   sprintf(msg,"%d %d",RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_REQ,rde_cb->options.slot_number);
   msg_size = write  (rde_cb->rde_rde_cb.clientfd, msg, strlen(msg));
   if (msg_size < 0)
   {
    /* Do the processing that is done when the client connection is lost here */
    if (errno != EINTR && errno != EWOULDBLOCK)
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_WRITE_FAIL, errno);

      status = close(rde_rde_cb->clientfd);
      if ( status < 0 )
      {
          m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
      }
      rde_rde_cb->clientReconnCount = 0;
      rde_rde_cb->retry = TRUE;
      rde_rde_cb->clientConnected = FALSE;

      if( rde_rde_client_socket_init(rde_rde_cb)!= RDE_RDE_RC_SUCCESS)
      {
           /* If the socket initialization fails then we need to exit */
             return RDE_RDE_RC_FAILURE;
      }
      else
      {
           return RDE_RDE_RC_SUCCESS;
      } 

     }
     else
     {
         sprintf(log,"RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_REQ sent. Slot Number %d\n",rde_cb->options.slot_number);
         m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
         return RDE_RDE_RC_SUCCESS;
     }
  return RDE_RDE_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:         rde_rde_process_send_slot_number

  DESCRIPTION:            To SEND the slot number to the requested RDE

  ARGUMENTS:

  RETURNS:

    RDE_RDE_RC_SUCCESS:   Successfully SEND the role to the RDE
    RDE_RDE_RC_FAILURE:   Failure to SEND the role to RDE

  NOTES:

*****************************************************************************/

static uns32 rde_rde_process_send_slot_number ()
{
    int rc;
    char msg [RDE_RDE_MSG_SIZE] = {0};
    RDE_CONTROL_BLOCK * rde_cb = rde_get_control_block();
    RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
    char  log[RDE_LOG_MSG_SIZE] = {0};

    sprintf (msg, "%d %d", RDE_RDE_CLIENT_SLOT_ID_EXCHANGE_RESP,rde_cb->options.slot_number);
    if (rde_rde_write_msg (rde_rde_cb->recvFd, msg) != RDE_RDE_MSG_WRT_SUCCESS)
    {
      rc = close(rde_rde_cb->recvFd);
      rde_rde_cb->connRecv = FALSE;
      if ( rc < 0)
      {
          m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_RDE_SOCK_CLOSE_FAIL, errno);
      }

      return RDE_RDE_RC_FAILURE;
    }
    sprintf(log,"Slot Number %d sent to the peer RDE\n",rde_cb->options.slot_number);
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);

    return RDE_RDE_RC_SUCCESS;

}
