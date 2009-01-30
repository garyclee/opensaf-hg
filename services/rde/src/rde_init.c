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

  MODULE NAME: rde_init.c

..............................................................................

  DESCRIPTION: This file contains the initialization logic for RDE.


******************************************************************************/

#include "rde.h"
#include "rde_cb.h"


/***************************************************************\
*                                                               *
*         Prototypes for static functions                       *
*                                                               *
\***************************************************************/

static uns32 rde_task_create        (RDE_CONTROL_BLOCK * rde_cb);
static uns32 rde_task_main          (RDE_CONTROL_BLOCK * rde_cb);
static uns32 rde_process_port_io    (RDE_CONTROL_BLOCK * rde_cb);
uns32     rde_initialize(void);
/*****************************************************************************

  PROCEDURE NAME:       rde_initialize

  DESCRIPTION:          Initialization logic for RDE
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

uns32 rde_initialize (void)
{
   uns32               rc       = NCSCC_RC_FAILURE;
   RDE_CONTROL_BLOCK  *rde_cb   = rde_get_control_block();
   RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
   NCS_BOOL e;
   char  log[RDE_LOG_MSG_SIZE] = {0};

   /** create a Lock **/
   if (m_NCS_OS_LOCK(&rde_cb->lock, NCS_OS_LOCK_CREATE, 0) 
      != NCSCC_RC_SUCCESS)
   {
      m_RDE_LOG_COND_L (RDE_SEV_ERROR, RDE_COND_LOCK_CREATE_FAILED, errno);
      return NCSCC_RC_FAILURE;
   }else
   {
      m_RDE_LOG_COND_L (RDE_SEV_INFO, RDE_COND_LOCK_CREATE_SUCCESS, 0);
   }
   /***************************************************************\
    *                                                               *
    *         Initialize structures                                 *
    *                                                               *
   \***************************************************************/

   if (m_NCS_OS_SEM (&rde_cb->semaphore, NCS_OS_SEM_CREATE)
       != NCSCC_RC_SUCCESS)
   {
      m_RDE_LOG_COND_L (RDE_SEV_ERROR, RDE_COND_SEM_CREATE_FAILED, errno);
      m_NCS_OS_LOCK(&rde_cb->lock, NCS_OS_LOCK_RELEASE, 0);
      return NCSCC_RC_FAILURE;
   }


   /*
   ** Select timeout
   */
   rde_cb->select_timeout = 2; /* 2 sec */

    rc = rde_rde_parse_config_file();

   if (rc != RDE_RDE_RC_SUCCESS)
   {
   sprintf (log, "Configuration Parameters : hostip = 0x%lX, hostportnum = %d, servip = 0x%lX, servportnum = %d, maxNoClientRetries = %d, ha_role = %d",rde_rde_cb->hostip,rde_rde_cb->hostportnum,rde_rde_cb->servip,rde_rde_cb->servportnum,rde_rde_cb->maxNoClientRetries,rde_cb->ha_role);
   m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
        return NCSCC_RC_FAILURE;
   }

   sprintf (log, "Configuration Parameters : hostip = 0x%lX, hostportnum = %d, servip = 0x%lX, servportnum = %d, maxNoClientRetries = %d, ha_role = %d",rde_rde_cb->hostip,rde_rde_cb->hostportnum,rde_rde_cb->servip,rde_rde_cb->servportnum,rde_rde_cb->maxNoClientRetries,rde_cb->ha_role);
   m_RDE_LOG_COND_C(RDE_SEV_INFO, RDE_RDE_INFO, log);
      /***************************************************************\
      *                                                               *
      *         Open RDA Interface                                    *
      *                                                               *
     \***************************************************************/
    rc = rde_rda_open (RDE_RDA_SOCK_NAME, &rde_cb->rde_rda_cb);
   if (rc != NCSCC_RC_SUCCESS)
    {
        m_NCS_OS_LOCK(&rde_cb->lock, NCS_OS_LOCK_RELEASE, 0);
        m_NCS_OS_SEM (&rde_cb-> semaphore, NCS_OS_SEM_RELEASE);
        sprintf(log,"rde_rda_open  fail");
        m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);
        return NID_RDE_RDA_OPEN_FAILED;
    }

/* Initialise the rde server */
   rc = rde_rde_open (&rde_cb->rde_rde_cb);
   if (rc != RDE_RDE_RC_SUCCESS)
   {
      m_NCS_OS_LOCK(&rde_cb->lock, NCS_OS_LOCK_RELEASE, 0);
      m_NCS_OS_SEM (&rde_cb-> semaphore, NCS_OS_SEM_RELEASE);
      rde_rda_close (&rde_cb->rde_rda_cb);
      sprintf(log,"rde_rde_open fail");
      m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);
      return NCSCC_RC_FAILURE;
   }

/* Initialise the status of the connections */
   rde_rde_cb->clientReconnCount = 0;
   rde_rde_cb->clientConnected = FALSE;
   rde_rde_cb->connRecv = FALSE;
   rde_rde_cb->count = 0;
   rde_rde_cb->hb_loss = FALSE;
   rde_rde_cb->retry = FALSE;
   rde_rde_cb->nid_ack_sent = FALSE;
   rde_rde_cb->conn_needed = TRUE;

/* Initialise the rde rde client */
   rc = rde_rde_client_socket_init(rde_rde_cb);
   if (rc != RDE_RDE_RC_SUCCESS)
   {
      m_NCS_OS_LOCK(&rde_cb->lock, NCS_OS_LOCK_RELEASE, 0);
      m_NCS_OS_SEM (&rde_cb-> semaphore, NCS_OS_SEM_RELEASE);
      rde_rda_close (&rde_cb->rde_rda_cb);
      rde_rde_close(&rde_cb->rde_rde_cb);
      sprintf(log,"rde_rde_client_socket_init fail");
      m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);
      return NCSCC_RC_FAILURE;
   }
/* check if role is established already */
   e = rde_rde_get_set_established(FALSE);

   if(!e )
   {
/* connect to other rde */
       rde_rde_clCheckConnect(rde_rde_cb);
       rde_count_testing();
       rc =  NCSCC_RC_SUCCESS;
   }


   /***************************************************************\
    *                                                               *
    *         Open AMF Interface                                    *
    *                                                               *
   \***************************************************************/
   rc = rde_amf_open (&rde_cb->rde_amf_cb);
   if (rc != NCSCC_RC_SUCCESS)
   {
      m_NCS_OS_LOCK(&rde_cb->lock, NCS_OS_LOCK_RELEASE, 0);
      m_NCS_OS_SEM (&rde_cb-> semaphore, NCS_OS_SEM_RELEASE);
      rde_rda_close (&rde_cb->rde_rda_cb);
      rde_rde_close(&rde_cb->rde_rde_cb);
      sprintf(log,"rde_amf_open fail");
      m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);

       return NID_RDE_AMF_OPEN_FAILED;

   }

   /***************************************************************\
    *                                                               *
    *         Spawn task to manage communication                    *
    *                                                               *
   \***************************************************************/
   if (rde_task_create(rde_cb) != NCSCC_RC_SUCCESS)
   {
       m_RDE_LOG_COND_L (RDE_SEV_ERROR, RDE_COND_TASK_CREATE_FAILED, errno);
       m_NCS_OS_SEM  (&rde_cb-> semaphore, NCS_OS_SEM_GIVE);
       m_NCS_OS_SEM  (&rde_cb-> semaphore, NCS_OS_SEM_RELEASE);
       rde_rda_close (&rde_cb->rde_rda_cb);
       rde_rde_close(&rde_cb->rde_rde_cb);
       rde_amf_close (&rde_cb->rde_amf_cb);
       sprintf(log,"rde_task_create fail");
       m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_RDE_ERROR, log);
       return NCSCC_RC_FAILURE;
   }

   return rc;
}



/*****************************************************************************

  PROCEDURE NAME:       rde_task_create

  DESCRIPTION:          Start a task for selecting on amf, rda and rde fds
                     
  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/

static
uns32 rde_task_create (RDE_CONTROL_BLOCK * rde_cb)
{
   m_RDE_ENTRY("rde_task_create");

   /***************************************************************\
    *                                                               *
    *         Create the fds interface task                         *
    *                                                               *
   \***************************************************************/

   if (m_NCS_TASK_CREATE (
          (NCS_OS_CB) rde_task_main,
          rde_cb,
          "RDETASK",
          0,
          NCS_STACKSIZE_HUGE,
          &rde_cb-> task_handle) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }


   /***************************************************************\
    *                                                               *
    *         Start the task                                        *
    *                                                               *
   \***************************************************************/

   if (m_NCS_TASK_START (rde_cb-> task_handle) != NCSCC_RC_SUCCESS)
   {
      m_NCS_TASK_RELEASE(rde_cb-> task_handle);
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:       rde_task_main

  DESCRIPTION:          Task main routine: single process concurrent server


  ARGUMENTS:            RDE control block pointer

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/

static uns32 rde_task_main (RDE_CONTROL_BLOCK * rde_cb)
{
   uns32         rc    = NCSCC_RC_SUCCESS;
   NCS_BOOL e = FALSE;
   char  log[RDE_LOG_MSG_SIZE] = {0};

  RDE_RDE_CB * rde_rde_cb = &rde_cb->rde_rde_cb;
   m_RDE_ENTRY("rde_task_main");

   m_RDE_LOG_HDLN (RDE_HDLN_TASK_STARTED_CLI);

   /***************************************************************\
    *                                                               *
    *         Task main loop: Read and process                      *
    *                                                               *
   \***************************************************************/

   while (! rde_cb->task_terminate)
   {
      rde_process_port_io (rde_cb);

/* If not connected to other rde then connect */
       e = rde_rde_get_set_established(FALSE);
       if(!e)
       {
                rc = rde_rde_clCheckConnect(rde_rde_cb);
                rc = rde_count_testing();
       }
       if ((rde_rde_cb->retry == TRUE) && (rde_cb->ha_role != PCS_RDA_ACTIVE))
       {
        /* Once the connection has been established and then gone down
            Try reconnecting for indefinite. */  
           m_NCS_OS_MEMSET(log,0,RDE_LOG_MSG_SIZE);
           sprintf(log,"Role = %d, RDE Got disconnected",rde_cb->ha_role);
           m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
           rc = rde_rde_clCheckConnect(rde_rde_cb);
           
       }

   }

   /***************************************************************\
    *                                                               *
    *         Exit the task                                         *
    *                                                               *
   \***************************************************************/

   rde_cb-> task_handle = NULL;

   m_RDE_LOG_HDLN (RDE_HDLN_TASK_STOPPED_CLI);

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_process_port_io

  DESCRIPTION:          Process characters received from the  Port

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/


static uns32 rde_process_port_io (RDE_CONTROL_BLOCK * rde_cb)
{
   int                large_fd    = 0;
   fd_set             readfds;
   struct timeval     tv;
   uns32              rc          = NCSCC_RC_SUCCESS;

   RDE_AMF_CB        *rde_amf_cb  = NULL;
   RDE_RDE_CB        *rde_rde_cb  = NULL;
   RDE_RDA_CB        *rde_rda_cb  = NULL;
   int                index;
   char  log[RDE_LOG_MSG_SIZE] = {0};

   m_RDE_ENTRY("rde_process_port_io");

   FD_ZERO(&readfds);

   /*
   ** Set timeout value
   */
   tv.tv_sec  = rde_cb->select_timeout;
   tv.tv_usec = 0;

   /*
   ** Get the RDE AMF and RDA control block pointers
   */
   rde_amf_cb   = &rde_cb->rde_amf_cb;
   rde_rde_cb   = &rde_cb->rde_rde_cb;


   rde_rda_cb   = &rde_cb->rde_rda_cb;

   /*
   ** Set RDA interface socket fd
   */
   FD_SET (rde_rda_cb->fd, &readfds);
   large_fd = rde_rda_cb->fd;

   /*
   ** Set RDA client fds
   */
   for (index = 0; index < rde_rda_cb->client_count; index++)
   {
       /* Set the fd and check if it is larger */
       FD_SET (rde_rda_cb->clients[index].fd, &readfds);
       if (rde_rda_cb->clients[index].fd > large_fd)
           large_fd = rde_rda_cb->clients[index].fd;
   }

   /*
   ** Set AMF fd
   ** Is it amf fd?
   */
   if (rde_amf_cb-> is_amf_up)
   {
       /* Yes! Set the fd and check if it is larger */
       FD_SET (rde_amf_cb->amf_fd, &readfds);
       if (rde_amf_cb->amf_fd > large_fd)
           large_fd = rde_amf_cb->amf_fd;
   }
   else
   {
       /* No! Set the fd and check if it is larger */
       FD_SET (rde_amf_cb->pipe_fd, &readfds);
       if (rde_amf_cb->pipe_fd > large_fd)
           large_fd = rde_amf_cb->pipe_fd;
   }

   FD_SET (rde_rde_cb->fd, &readfds);
   if (rde_rde_cb->fd > large_fd)
       large_fd = rde_rde_cb->fd;

   /* If the other rde has connected,set RDE client fd */
   if(rde_rde_cb->connRecv)
   {
   /* Set the fd and check if it is larger */
       FD_SET (rde_rde_cb->recvFd, &readfds);
       if (rde_rde_cb->recvFd > large_fd)
           large_fd = rde_rde_cb->recvFd;
   }

   /* Set RDE RDE Client Interface Socket fd */

   /*Is it connected to the other RDE  */

   if(rde_rde_cb->clientConnected)
   {
   /* set the client fd */
      FD_SET (rde_rde_cb->clientfd, &readfds);
      if (rde_rde_cb->clientfd > large_fd)
         large_fd = rde_rde_cb->clientfd;
   }
   rc = select(large_fd + 1, &readfds, NULL, NULL, &tv);

   if (rc < 0) /* Error */
   {
       if (errno != EINTR)
           m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SELECT_ERROR, errno);
       sprintf(log,"RDE_COND_SELECT_ERROR: %d",rc);
       m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_ERROR, log);

       return NCSCC_RC_FAILURE;
   }

   if (rc == 0) /* Timeout */
   {
       return NCSCC_RC_FAILURE;
   }

   /*
   ** Is AMF fd readable?
   */
   if (rde_amf_cb->is_amf_up)
   {
      if (FD_ISSET (rde_amf_cb->amf_fd, &readfds))
      {
           /* AMF socket is readable */
           rc = rde_amf_process_msg (rde_amf_cb);
      }
   }
   else
   {
       if (FD_ISSET (rde_amf_cb->pipe_fd, &readfds))
       {
           /* Pipe is readable */
           if ((rc = rde_amf_pipe_process_msg (rde_amf_cb)) != NCSCC_RC_SUCCESS)
           {
           }
                
       }
   }

   /*
   ** Is RDA interface fd readable?
   */
   if (FD_ISSET (rde_rda_cb->fd, &readfds))
   {
       /* RDA interface socket is readable */
       rc = rde_rda_process_msg (rde_rda_cb);
   }

   /*
   ** Is any RDA client fd readable?
   */
   for (index = 0; index < rde_rda_cb->client_count; index++)
   {
       if (FD_ISSET (rde_rda_cb->clients[index].fd, &readfds))
       {
           /* RDA Client fd is readable */
           rc = rde_rda_client_process_msg (rde_rda_cb, index);
       }

   }

/* check if the other rde is attempting a connection */
   if (FD_ISSET (rde_rde_cb->fd,&readfds))
   {
        /* RDE is readable */
        rde_rde_process_msg (rde_rde_cb);
       /* if this RDE is not connected to the other RDE then connect */
        if (!rde_rde_cb->clientConnected)
        {
		   if(rde_rde_cb->conn_needed == TRUE)
		   {
 		      /* Connect only if it is needed. */
              if (rde_rde_connect(rde_rde_cb)!=RDE_RDE_RC_SUCCESS)
              {
                  return NCSCC_RC_FAILURE;
              }
		   }
        }
        /* we will not send a request for role here because that will be taken care by clCheckConnect */
        rc = NCSCC_RC_SUCCESS;
   }

   if(rde_rde_cb->connRecv)
   {
   /* Is there an activity from the other RDE on the recv fd*/
        if (FD_ISSET (rde_rde_cb->recvFd, &readfds))
        {
              rde_rde_client_process_msg (rde_rde_cb);
               rc = NCSCC_RC_SUCCESS;
        }
   }
    /* if the rde has connected to the other rde */
   if(rde_rde_cb->clientConnected)
   {
   /* If the clientfd is active */
        if (FD_ISSET (rde_rde_cb->clientfd ,&readfds))
        {
                if(rde_rde_client_read_role ()!=RDE_RDE_RC_SUCCESS)
                {
                        /* unable to initialize a new socket in case of failure */
                     rc = NCSCC_RC_FAILURE;
         return rc;
                }
                  rc = NCSCC_RC_SUCCESS;
        }
   }

   return rc;
}

