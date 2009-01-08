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

  MODULE NAME: rde_rda.c

..............................................................................

  DESCRIPTION:

  This file contains an implementation of the RDE rde_rda_cb Interface
  The rde_rda_cb Interface is used for communication between RDE
  and command line utilities.

  This implementation of the rde_rda_cb` Interface uses a Unix domain sockets
  interface.

******************************************************************************/

#include "rde.h"
#include "rde_cb.h"


/***************************************************************\
 *                                                               *
 *         Prototypes for static functions                       *
 *                                                               *
\***************************************************************/
static uns32 rde_rda_sock_open          (RDE_RDA_CB * rde_rda_cb);
static uns32 rde_rda_sock_init          (RDE_RDA_CB * rde_rda_cb);
static uns32 rde_rda_sock_close         (RDE_RDA_CB * rde_rda_cb);
static uns32 rde_rda_write_msg          (int fd, char *msg);
static uns32 rde_rda_read_msg           (int fd, char *msg, int size);
static uns32 rde_rda_process_set_role   (RDE_RDA_CB  * rde_rda_cb, int index, int role);
static uns32 rde_rda_process_get_role   (RDE_RDA_CB  * rde_rda_cb, int index);
static uns32 rde_rda_process_reg_cb     (RDE_RDA_CB  * rde_rda_cb, int index);
static uns32 rde_rda_process_disconnect (RDE_RDA_CB  * rde_rda_cb, int index);
#if 0
static uns32 rde_rda_process_hb_err     (RDE_RDA_CB  * rde_rda_cb, int index);
static uns32 rde_rda_process_nd_hb_err  (RDE_RDA_CB  * rde_rda_cb, int index, uns32 phy_slot_id);
extern uns32 rde_rda_send_node_reset_to_avm(RDE_RDE_CB  * rde_rde_cb);
extern uns32 rde_rda_send_scb_sw_over(void);
#endif


/*****************************************************************************

  PROCEDURE NAME:       rde_rda_sock_open

  DESCRIPTION:          Open the socket associated with
                        the RDA Interface

  ARGUMENTS:
     rde_rda_cb         Pointer to RDA Socket Config structure

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully opened the socket
    NCSCC_RC_FAILURE:   Failure opening socket

  NOTES:

*****************************************************************************/

static
uns32 rde_rda_sock_open (RDE_RDA_CB    *rde_rda_cb)
{
   uns32 rc= NCSCC_RC_SUCCESS;
   m_RDE_ENTRY("rde_rda_sock_open");


   /***************************************************************\
    *                                                               *
    *         Create the socket descriptor                          *
    *                                                               *
   \***************************************************************/

   rde_rda_cb-> fd = socket (AF_UNIX, SOCK_STREAM, 0) ;

   if (rde_rda_cb-> fd < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_CREATE_FAIL, errno);
      return NCSCC_RC_FAILURE;
   }

   return rc;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_sock_init

  DESCRIPTION:          Initialize the socket associated with
                        the rde_rda_cb Interface

  ARGUMENTS:
     rde_rda_cb         Pointer to RDE  Socket Config structure

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully initialized the socket
    NCSCC_RC_FAILURE:   Failure initializing the socket

  NOTES:

*****************************************************************************/

static
uns32 rde_rda_sock_init (RDE_RDA_CB * rde_rda_cb)
{
   struct stat sockStat;
   int         rc;

   m_RDE_ENTRY("rde_rda_sock_init");

   /***************************************************************\
    *                                                               *
    *         Remove the file if it exists
    *                                                               *
   \***************************************************************/


   rc = stat (rde_rda_cb-> sock_address. sun_path, &sockStat);

   if (rc == 0)   /* File exists */
   {
      rc = remove (rde_rda_cb-> sock_address. sun_path);
      if (rc != 0)
      {
         m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_REMOVE_FAIL, errno);

         return NCSCC_RC_FAILURE;
      }
   }

   /***************************************************************\
    *                                                               *
    *         Bind the socket to the "well-known" server address    *
    *                                                               *
   \***************************************************************/

   rc = bind (rde_rda_cb->fd,
              (struct sockaddr *) & rde_rda_cb-> sock_address,
              sizeof(rde_rda_cb-> sock_address));

   if (rc < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_BIND_FAIL, errno);
      return NCSCC_RC_FAILURE;
   }

   /***************************************************************\
    *                                                               *
    *         Listen on the socket to to accept clients             *
    *                                                               *
   \***************************************************************/

   listen(rde_rda_cb->fd, 5);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_sock_close

  DESCRIPTION:          Close the socket associated with
                        the rde_rda_cb Interface

  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully closed the socket
    NCSCC_RC_FAILURE:   Failure closing the socket

  NOTES:

*****************************************************************************/

static
uns32 rde_rda_sock_close (RDE_RDA_CB * rde_rda_cb)
{
   int rc;

   m_RDE_ENTRY("rde_rda_sock_close");

   /***************************************************************\
    *                                                               *
    *         Close the socket                                      *
    *                                                               *
   \***************************************************************/

   rc = close (rde_rda_cb-> fd);
   if (rc < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_CLOSE_FAIL, errno);

      return NCSCC_RC_FAILURE;
   }

   /***************************************************************\
    *                                                               *
    *         Remove the file                                       *
    *                                                               *
   \***************************************************************/

   rc = remove(rde_rda_cb-> sock_address. sun_path);
   if (rc < 0)
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_REMOVE_FAIL, errno);

      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_write_msg

  DESCRIPTION:    Write a message to a peer on the socket

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/

static
uns32 rde_rda_write_msg (int fd, char *msg)
{
   int         rc         = NCSCC_RC_SUCCESS;
   int         msg_size   = 0;

   /***************************************************************\
    *                                                               *
    *         Read from socket into input buffer                    *
    *                                                               *
   \***************************************************************/

   msg_size = send   (fd, msg, m_NCS_OS_STRLEN (msg) + 1, 0);
   if (msg_size < 0)
   {
      if (errno != EINTR && errno != EWOULDBLOCK)
         /* Non-benign error */
         m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_SEND_FAIL, errno);

      return NCSCC_RC_FAILURE;
   }

   m_RDE_LOG_COND_C (RDE_SEV_DEBUG, RDE_COND_RDA_RESPONSE, msg);

   return rc;
}


/*****************************************************************************

  PROCEDURE NAME:       rde_rda_read_msg

  DESCRIPTION:    Read a complete line from the socket

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/

static
uns32 rde_rda_read_msg (int fd, char *msg, int size)
{
   int         msg_size  = 0;

   /***************************************************************\
    *                                                               *
    *         Read from socket into input buffer                    *
    *                                                               *
   \***************************************************************/
   msg_size = recv (fd, msg, size, 0);
   if (msg_size < 0)
   {
      if (errno != EINTR && errno != EWOULDBLOCK)
         /* Non-benign error */
         m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_RECV_FAIL, errno);

      return NCSCC_RC_FAILURE;
   }

   /*
   ** Is peer closed?
   */
   if (msg_size == 0)
   {
      /*
      ** Yes! disconnect client
      */
      sprintf (msg, "%d", RDE_RDA_DISCONNECT_REQ);
      m_RDE_LOG_COND_C(RDE_SEV_ERROR, RDE_COND_SOCK_RECV_FAIL, "Connection closed by client (orderly shutdown)");
      return NCSCC_RC_SUCCESS;
   }

   m_RDE_LOG_COND_C (RDE_SEV_DEBUG, RDE_COND_RDA_CMD, msg);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_get_role

  DESCRIPTION:          Process the get role message


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to identify the readable client fd

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully got role
    NCSCC_RC_FAILURE:   Failure getiing role

  NOTES:

*****************************************************************************/
static
uns32 rde_rda_process_get_role (RDE_RDA_CB  * rde_rda_cb, int index)
{
    char               msg [64] = {0};
    RDE_CONTROL_BLOCK *rde_cb =  rde_get_control_block();

    sprintf (msg, "%d %d", RDE_RDA_GET_ROLE_RES, rde_cb->ha_role);
    if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;

}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_set_role

  DESCRIPTION:          Process the get role message


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to identify the readable client fd
     role         HA role to seed into RDE 

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully set role
    NCSCC_RC_FAILURE:   Failure setting role

  NOTES:

*****************************************************************************/
static
uns32 rde_rda_process_set_role (RDE_RDA_CB  * rde_rda_cb, int index, int role)
{

    char               msg [64] = {0};

    /*
    ** Seed role into RDE 
    */
    if (rde_rde_set_role (role) != NCSCC_RC_SUCCESS)
        sprintf (msg, "%d", RDE_RDA_SET_ROLE_NACK);
    else
        sprintf (msg, "%d", RDE_RDA_SET_ROLE_ACK);

    if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_reg_cb

  DESCRIPTION:          Process the register callback message


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to identify the readable client fd


  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/
static
uns32 rde_rda_process_reg_cb (RDE_RDA_CB  * rde_rda_cb, int index)
{
    char               msg [64] = {0};

    /*
    ** Asynchronous callback registered by RDA
    */
    rde_rda_cb->clients [index].is_async = TRUE;

    /*
    ** Format ACK
    */
    sprintf (msg, "%d", RDE_RDA_REG_CB_ACK);


    if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}

#if 0
/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_hb_err

  DESCRIPTION:          Process the get role message


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to identify the readable client fd
     role         HA role to seed into RDE 

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully set role
    NCSCC_RC_FAILURE:   Failure setting role

  NOTES:

*****************************************************************************/
static
uns32 rde_rda_process_hb_err (RDE_RDA_CB  * rde_rda_cb, int index)
{
    char               msg [64] = {0};

    if (rde_rde_hb_err () != NCSCC_RC_SUCCESS)
    {
          sprintf (msg, "%d", RDE_RDA_AVD_HB_ERR_NACK);
    }
else
{
          sprintf (msg, "%d", RDE_RDA_AVD_HB_ERR_ACK);
}
    if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_nd_hb_err

  DESCRIPTION:          Process the AVND HB loss message


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to identify the readable client fd
     phy_slot_id  The slot id on which the AvND is not responding.

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully sent message to RDE
    NCSCC_RC_FAILURE:   Failure sending message to RDE

  NOTES:

*****************************************************************************/
static
uns32 rde_rda_process_nd_hb_err (RDE_RDA_CB  *rde_rda_cb, int index, uns32 phy_slot_id)
{
char               msg [64] = {0};
sprintf (msg, "%d", RDE_RDA_AVND_HB_ERR_ACK);
    if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }
    return NCSCC_RC_SUCCESS;
}

#endif

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_disconnect

  DESCRIPTION:          Process the register callback message


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to identify the readable client fd


  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/
static
uns32 rde_rda_process_disconnect (RDE_RDA_CB  * rde_rda_cb, int index)
{
    int     rc     = 0;
    int     iter   = 0;
    int     sockfd = -1;

    /*
    ** Save socket fd
    */
    sockfd = rde_rda_cb->clients [index].fd;

    /*
    ** Delete fd from the client list
    */
    for (iter = index; iter < rde_rda_cb->client_count - 1; iter++)
    {

        rde_rda_cb->clients [iter] = rde_rda_cb->clients [iter + 1];

    }

    rde_rda_cb->client_count--;

    rc = close (sockfd);
    if (rc < 0)
    {
        m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_CLOSE_FAIL, errno);
        return NCSCC_RC_FAILURE;
    }

    return NCSCC_RC_SUCCESS;
}



/*****************************************************************************

  PROCEDURE NAME:       rde_rda_sock_name

  DESCRIPTION:          Return the "name" of this rde_rda_cb interface
                        implementation

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/

const char *rde_rda_sock_name (RDE_RDA_CB * rde_rda_cb)
{
   return RDE_RDA_SOCK_NAME;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_open

  DESCRIPTION:          Initialize the RDA Interface

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:   rde_rda_cb Interface has been successfully initialized
    NCSCC_RC_FAILURE:   Failure initializing rde_rda_cb Interface

  NOTES:

*****************************************************************************/

uns32 rde_rda_open (const char *sock_name, RDE_RDA_CB *rde_rda_cb)
{

   m_RDE_ENTRY("rde_rda_open");

   if (sock_name    == NULL || sock_name[0] == '\0')
   {
      m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_NULL_NAME, 0);
      return NCSCC_RC_FAILURE;
   }

   /***************************************************************\
    *                                                               *
    *         Initialize the  Socket configuration                  *
    *                                                               *
   \***************************************************************/

   m_NCS_OS_STRNCPY(&rde_rda_cb->sock_address.sun_path,  sock_name, sizeof(rde_rda_cb->sock_address));
   rde_rda_cb-> sock_address. sun_family = AF_UNIX ;
   rde_rda_cb-> fd                       = -1      ;

   /***************************************************************\
    *                                                               *
    *         Open socket for communication with rde_rda_cb                *
    *                                                               *
   \***************************************************************/

   if (rde_rda_sock_open(rde_rda_cb) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   /***************************************************************\
    *                                                               *
    *         Initialize socket settings                            *
    *                                                               *
   \***************************************************************/


   if (rde_rda_sock_init(rde_rda_cb) != NCSCC_RC_SUCCESS)
   {
      rde_rda_sock_close(rde_rda_cb);
      return NCSCC_RC_FAILURE;
   }

   m_RDE_LOG_COND_C(RDE_SEV_INFO, RDE_COND_RDA_SOCK_CREATED, sock_name);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_close

  DESCRIPTION:          Close the rde_rda_cb Interface

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:   rde_rda_cb Interface has been successfully destroyed
    NCSCC_RC_FAILURE:   Failure destroying rde_rda_cb Interface

  NOTES:

*****************************************************************************/

uns32 rde_rda_close (RDE_RDA_CB *rde_rda_cb)
{

   m_RDE_ENTRY("rde_rda_close");

   /***************************************************************\
    *                                                               *
    *      Close the socket if it is still in use                   *
    *                                                               *
   \***************************************************************/

   if (rde_rda_sock_close(rde_rda_cb) != NCSCC_RC_SUCCESS)
   {
      return NCSCC_RC_FAILURE;
   }

   rde_rda_cb-> fd = -1;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_process_msg

  DESCRIPTION:          Accept the client on the socket

  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:
    NCSCC_RC_FAILURE:

  NOTES:

*****************************************************************************/
uns32 rde_rda_process_msg (RDE_RDA_CB *rde_rda_cb)
{
   int newsockfd;

   newsockfd = accept(rde_rda_cb->fd, (struct sockaddr *) NULL, NULL);
   if (newsockfd < 0)
   {
       m_RDE_LOG_COND_L(RDE_SEV_ERROR, RDE_COND_SOCK_ACCEPT_FAIL, 0);
       return NCSCC_RC_FAILURE;
   }

   /*
   ** Add the new client fd to client-list
   */
   rde_rda_cb->clients [rde_rda_cb->client_count].is_async = FALSE;
   rde_rda_cb->clients [rde_rda_cb->client_count].fd       = newsockfd;
   rde_rda_cb->client_count++;

   return NCSCC_RC_SUCCESS;

}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_client_process_msg

  DESCRIPTION:          Process the client message on socket


  ARGUMENTS:
     rde_rda_cb   Pointer to RDE  Socket Config structure
     index        Index to idetify the readable client fd

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully closed the socket
    NCSCC_RC_FAILURE:   Failure closing the socket

  NOTES:

*****************************************************************************/
uns32 rde_rda_client_process_msg(RDE_RDA_CB  * rde_rda_cb, int index)
{
    RDE_RDA_CMD_TYPE   cmd_type;
    char               msg [256] = {0};
    uns32              rc        = NCSCC_RC_SUCCESS;
    int                value;
    char              *ptr;

    if (rde_rda_read_msg (rde_rda_cb->clients [index].fd, msg, sizeof (msg)) != NCSCC_RC_SUCCESS)
    {
        return NCSCC_RC_FAILURE;
    }

    /*
    ** Parse the message for cmd type and value
    */
    ptr = strchr (msg, ' ');
    if (ptr == NULL)
    {
        cmd_type = atoi (msg);

    }
    else
    {
        *ptr = '\0';
        cmd_type = atoi (msg);
        value    = atoi (++ptr);
    }

    switch (cmd_type)
    {
        case RDE_RDA_GET_ROLE_REQ:
            rc = rde_rda_process_get_role (rde_rda_cb, index);
            break;

        case RDE_RDA_SET_ROLE_REQ:
            rc = rde_rda_process_set_role (rde_rda_cb, index, value);
            break;

        case RDE_RDA_REG_CB_REQ:
            rc = rde_rda_process_reg_cb (rde_rda_cb, index);
            break;
#if 0
        case RDE_RDA_AVD_HB_ERR_REQ:
            rc = rde_rda_process_hb_err (rde_rda_cb, index);
            break;

        case RDE_RDA_AVND_HB_ERR_REQ:
            rc = rde_rda_process_nd_hb_err (rde_rda_cb, index, value);
            break;
#endif

        case RDE_RDA_DISCONNECT_REQ:
            rc = rde_rda_process_disconnect (rde_rda_cb, index);
            break;
        default:
            ;
    }

    return rc;

}

/*****************************************************************************

  PROCEDURE NAME:       rde_rda_send_role

  DESCRIPTION:          Send HA role to RDA


  ARGUMENTS:
     role               HA role to seed into  RDE

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully set role
    NCSCC_RC_FAILURE:   Failure setting role

  NOTES:

*****************************************************************************/
uns32 rde_rda_send_role (int role)
{
    int                index;
    char               msg [64]   = {0};
    RDE_RDA_CB        *rde_rda_cb = NULL;
    RDE_CONTROL_BLOCK *rde_cb =  rde_get_control_block();
    char  log[RDE_LOG_MSG_SIZE] = {0};

    /** here we are locking as we might be calling this function
     ** from two different threads, so this just make the sent
     ** role serial
     **/
    m_NCS_LOCK(&rde_cb->lock, NCS_LOCK_WRITE);

    rde_rda_cb = &rde_cb->rde_rda_cb;
    sprintf(log,"Sending role %d to RDA",role);
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
    /*
    ** Send role to all async clients
    */
    sprintf (msg, "%d %d", RDE_RDA_HA_ROLE, role);

    for (index = 0; index < rde_rda_cb->client_count; index++)
    {
        if (!rde_rda_cb->clients[index].is_async)
            continue;

        /*
        ** Write message
        */
        if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
        {
            /* We have nothing to do here */
        }

    }

    m_NCS_UNLOCK(&rde_cb->lock, NCS_LOCK_WRITE);
    return NCSCC_RC_SUCCESS;
}
#if 0
/*****************************************************************************

  PROCEDURE NAME:       rde_rda_send_scb_sw_over

  DESCRIPTION:          Send switchover command to RDA


  ARGUMENTS:

  RETURNS:

    NCSCC_RC_SUCCESS:   Successfully sent the cmd
    NCSCC_RC_FAILURE:   Failure sending cmd

  NOTES:
          Currently only AVM should get this command. However, AvM is the only
          client as of today so lets use the below routine to send the
          command using this interface.

          if some other clients register they may choose to ignore this cmd.
          Or we will remove this and migrate to MDS interface.

*****************************************************************************/
uns32 rde_rda_send_scb_sw_over(void)
{
    int                index;
    char               msg [64]   = {0};
    RDE_RDA_CB        *rde_rda_cb = NULL;
    RDE_CONTROL_BLOCK *rde_cb =  rde_get_control_block();

    rde_rda_cb = &rde_cb->rde_rda_cb;

    /*
    ** Send cmd to all async clients
    */
    sprintf (msg, "%d", RDE_RDA_SCB_SWOVER);

    for (index = 0; index < rde_rda_cb->client_count; index++)
    {
        if (!rde_rda_cb->clients[index].is_async)
            continue;

        /*
        ** Write message
        */
        if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
        {
            /* We have nothing to do here */
        }

    }

    return NCSCC_RC_SUCCESS;
}
#endif


#if 0
uns32 rde_rda_send_node_reset_to_avm(RDE_RDE_CB  * rde_rde_cb)
{
    int                index;
    char               msg [64]   = {0};
    char  log[RDE_LOG_MSG_SIZE] = {0};
    RDE_CONTROL_BLOCK *rde_cb =  rde_get_control_block();
    RDE_RDA_CB        *rde_rda_cb = NULL;
    uns32 phy_slot = 0;


    m_NCS_LOCK(&rde_cb->lock, NCS_LOCK_WRITE);

    rde_rda_cb = &rde_cb->rde_rda_cb;
    /* Get the alternate slot which need to be reset */
    phy_slot = rde_rde_cb->peer_slot_number;

    sprintf(log,"Sending node reset cmd to AVM for slot id %d",phy_slot);
    m_RDE_LOG_COND_C(RDE_SEV_NOTICE, RDE_RDE_INFO, log);
/*
** Send this to AVM only as this is only one user presently.
*/
    sprintf (msg, "%d %d", RDE_RDA_NODE_RESET_CMD, phy_slot);

    for (index = 0; index < rde_rda_cb->client_count; index++)
    {
        if (!rde_rda_cb->clients[index].is_async)
            continue;

        /*
        ** Write message
        */
        if (rde_rda_write_msg (rde_rda_cb->clients [index].fd, msg) != NCSCC_RC_SUCCESS)
        {
        /* We have nothing to do here */
        }

    }
    m_NCS_UNLOCK(&rde_cb->lock, NCS_LOCK_WRITE);
    return NCSCC_RC_SUCCESS;

}
#endif

