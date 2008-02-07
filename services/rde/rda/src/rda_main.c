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

  $Header:  $

  MODULE NAME: pcs_rda_main.c

..............................................................................

  DESCRIPTION:  This file implements the RDA to interact with RDE. 

******************************************************************************
*/

/*
** Includes
*/
#include "rda.h"


/*
** Static functions
*/
static uns32 rda_read_msg     (int sockfd, char *msg, int size);
static uns32 rda_write_msg    (int sockfd, char *msg);
static uns32 rda_parse_msg    (const char *pmsg, RDE_RDA_CMD_TYPE *cmd_type, int *value);
static uns32 rda_connect      (int *sockfd);
static uns32 rda_disconnect   (int sockfd);
static uns32 rda_callback_req (int sockfd);
extern RDA_CONTROL_BLOCK *rda_get_control_block (void);

#if (RDA_SPOOF != 0)

/*
** Global data, RDA callback control block
*/
RDA_CALLBACK_CB rda_spoof_callback_cb;
PCS_RDA_ROLE    rda_current_role = PCS_RDA_UNDEFINED;

static uns32 get_init_role(uns32 *init_role);

#endif

/*****************************************************************************

  PROCEDURE NAME:       rda_callback_task

  DESCRIPTION:          Task main routine to perform callback mechanism
                        
                     
  ARGUMENTS:

  RETURNS:

    PCSRDA_RC_SUCCESS:                   

  NOTES:

*****************************************************************************/

static
uns32 rda_callback_task (RDA_CALLBACK_CB *rda_callback_cb)
{
   char              msg [64]       = {0};
   uns32             rc             = PCSRDA_RC_SUCCESS;
   int               value          = -1;
   int               retry_count    = 0;
   NCS_BOOL          conn_lost      = FALSE;
   RDE_RDA_CMD_TYPE  cmd_type       = 0;
   PCS_RDA_CMD cmd ;

   m_NCS_OS_MEMSET(&cmd, 0, sizeof(PCS_RDA_CMD));

   while (!rda_callback_cb->task_terminate)
   {
        /*
        ** Escalate the issue if the connection with server is not  
        ** established after certain retries.
        */
        if (retry_count >= 100)
        {

            (*rda_callback_cb->callback_ptr) (rda_callback_cb->callback_handle, 
                                              PCS_RDA_UNDEFINED, PCSRDA_RC_FATAL_IPC_CONNECTION_LOST,cmd);
            break;
        }

        /*
        ** Retry if connection with server is lost
        */
        if (conn_lost == TRUE)
        {
            m_NCS_TASK_SLEEP (1000);
            retry_count++;

            /*
            ** Connect
            */
            rc = rda_connect (&rda_callback_cb->sockfd);
            if (rc != PCSRDA_RC_SUCCESS)
            {
                 continue;
            }

            /*
            ** Send callback reg request messgae
            */
            rc = rda_callback_req (rda_callback_cb->sockfd);
            if (rc != PCSRDA_RC_SUCCESS)
            {
                 rda_disconnect (rda_callback_cb->sockfd);
                 continue;
            }

            /*
            ** Connection established
            */
            retry_count = 0;
            conn_lost   = FALSE;
        }

        /*
        ** Recv async role response
        */
        rc = rda_read_msg (rda_callback_cb->sockfd, msg, sizeof (msg));
        if ((rc == PCSRDA_RC_TIMEOUT) || (rc == PCSRDA_RC_FATAL_IPC_CONNECTION_LOST))
        {
            if (rc == PCSRDA_RC_FATAL_IPC_CONNECTION_LOST)
            {
                 close (rda_callback_cb->sockfd);
                 rda_callback_cb->sockfd = -1;
                 conn_lost               = TRUE;
            }
            continue;
        }
        else if (rc != PCSRDA_RC_SUCCESS)
        {
            /* 
            ** Escalate the problem to the application
            */
            (*rda_callback_cb->callback_ptr) (rda_callback_cb->callback_handle, PCS_RDA_UNDEFINED, rc,cmd);
            continue;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if((cmd_type == RDE_RDA_HA_ROLE) || (cmd_type == RDE_RDA_NODE_RESET_CMD) || (value >= 0))
        {
           /* Process the message. */
        }
        else
        {
           continue;
        }

        /*
        ** Invoke callback
        */
        if(cmd_type == RDE_RDA_NODE_RESET_CMD)
        {
          cmd.req_type = PCS_RDA_NODE_RESET_CMD; 
          cmd.info.node_reset_info.slot_id = value;
          cmd.info.node_reset_info.shelf_id = RDE_RDA_SHELF_ID;
 
        } 

        (*rda_callback_cb->callback_ptr) (rda_callback_cb->callback_handle, value, PCSRDA_RC_SUCCESS,cmd);
   }

   return rc;

}


/****************************************************************************
 * Name          : pcs_rda_reg_callback
 *
 * Description   : 
 *                 
 *
 * Arguments     : PCS_RDA_CB_PTR - Callback function pointer
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_reg_callback (uns32 cb_handle, PCS_RDA_CB_PTR rda_cb_ptr, long *task_cb)
{

    uns32  rc = PCSRDA_RC_SUCCESS;

#if (RDA_SPOOF != 0)

    m_NCS_OS_MEMSET(&rda_spoof_callback_cb, 0, sizeof(RDA_CALLBACK_CB));
    rda_spoof_callback_cb.sockfd          = -1;
    rda_spoof_callback_cb.callback_ptr    = rda_cb_ptr;
    rda_spoof_callback_cb.callback_handle = cb_handle;
    rda_spoof_callback_cb.task_terminate  = FALSE;

    *task_cb = (long) &rda_spoof_callback_cb;

#else

    int              sockfd          = -1;
    NCS_BOOL         is_task_spawned = FALSE;
    RDA_CALLBACK_CB *rda_callback_cb = NULL;

    *task_cb = (long) 0;

    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {
        /*
        ** Init leap
        */
        if(ncs_leap_startup(0, 0) != NCSCC_RC_SUCCESS)
        {
          
            rc = PCSRDA_RC_LEAP_INIT_FAILED;
            break;
        }

        /*
        ** Send callback reg request messgae
        */
        rc = rda_callback_req (sockfd);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Allocate callback control block
        */
        rda_callback_cb = (RDA_CALLBACK_CB *) m_NCS_MEM_ALLOC (sizeof (RDA_CALLBACK_CB), 0, 0, 0);
        if (rda_callback_cb == NULL)
        {
            rc = PCSRDA_RC_MEM_ALLOC_FAILED;
            break;
        }

        m_NCS_OS_MEMSET(rda_callback_cb, 0, sizeof(RDA_CALLBACK_CB));
        rda_callback_cb->sockfd          = sockfd;
        rda_callback_cb->callback_ptr    = rda_cb_ptr;
        rda_callback_cb->callback_handle = cb_handle;
        rda_callback_cb->task_terminate  = FALSE;

        /*
        ** Spawn task
        */
        if (m_NCS_TASK_CREATE (
           (NCS_OS_CB) rda_callback_task,
           rda_callback_cb,
           "RDATASK_CALLBACK",
           0,
           NCS_STACKSIZE_HUGE,
           &rda_callback_cb-> task_handle) != NCSCC_RC_SUCCESS)
        {
           
           m_NCS_MEM_FREE (rda_callback_cb, 0, 0, 0);
           rc = PCSRDA_RC_TASK_SPAWN_FAILED;
           break;
        }

        if (m_NCS_TASK_START (rda_callback_cb-> task_handle) != NCSCC_RC_SUCCESS)
        {
           m_NCS_MEM_FREE (rda_callback_cb, 0, 0, 0);
           m_NCS_TASK_RELEASE (rda_callback_cb->task_handle);
           rc = PCSRDA_RC_TASK_SPAWN_FAILED;
           break;
        }

        is_task_spawned = TRUE;
        *task_cb = (long) rda_callback_cb;

    }while (0);

    /*
    ** Disconnect
    */
    if (!is_task_spawned)
    {
       ncs_leap_shutdown();
       rda_disconnect (sockfd);

    }

#endif

    /*
    ** Done
    */
    return rc;
}

/****************************************************************************
 * Name          : pcs_rda_unreg_callback
 *
 * Description   : 
 *                 
 *
 * Arguments     : PCS_RDA_CB_PTR - Callback function pointer
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_unreg_callback (long task_cb)
{   
    uns32            rc              = PCSRDA_RC_SUCCESS;
    RDA_CALLBACK_CB *rda_callback_cb = NULL;
    rda_callback_cb                  = (RDA_CALLBACK_CB *) task_cb;
    rda_callback_cb ->task_terminate = TRUE;

#if (RDA_SPOOF != 0)

    m_NCS_OS_MEMSET(&rda_spoof_callback_cb, 0, sizeof(RDA_CALLBACK_CB));

#else

    /*
    ** Stop task
    */
    m_NCS_TASK_STOP (rda_callback_cb->task_handle);

    /*
    ** Disconnect
    */
    rda_disconnect (rda_callback_cb->sockfd);

    /*
    ** Release task
    */
    m_NCS_TASK_RELEASE (rda_callback_cb->task_handle);

    /*
    ** Free memory
    */
    m_NCS_MEM_FREE (rda_callback_cb, 0, 0, 0);

    /*
    ** Destroy leap
    */
    ncs_leap_shutdown();

#endif

    /*
    ** Done
    */
    return rc;

}

/****************************************************************************
 * Name          : pcs_rda_set_role
 *
 * Description   : 
 *                 
 *
 * Arguments     : role - HA role to set
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_set_role     (PCS_RDA_ROLE role)
{
    uns32   rc = PCSRDA_RC_SUCCESS;
PCS_RDA_CMD cmd ;
   m_NCS_OS_MEMSET(&cmd, 0, sizeof(PCS_RDA_CMD));

#if (RDA_SPOOF != 0)

    if (rda_current_role == role)
        return rc;

    /*
    ** Set role
    */
    rda_current_role = role;

    /* 
    ** If the set role is Quiesced, nothing to be done, just return
    */
    if (role == PCS_RDA_QUIESCED)
        return rc;

    /*
    ** Intimate registered client about role change
    */
    if (rda_spoof_callback_cb.callback_ptr != NULL)
        (*rda_spoof_callback_cb.callback_ptr) (rda_spoof_callback_cb.callback_handle, role, PCSRDA_RC_SUCCESS,cmd);

#else

    int     sockfd;
    char    msg [64]          = {0};
    int     value             = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;


    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {
        /*
        ** Send set-role request messgae
        */
        sprintf (msg, "%d %d", RDE_RDA_SET_ROLE_REQ, role);
        rc = rda_write_msg (sockfd, msg);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Recv set-role ack
        */
        rc = rda_read_msg (sockfd, msg, sizeof (msg));
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if (cmd_type != RDE_RDA_SET_ROLE_ACK)
        {
            rc = PCSRDA_RC_ROLE_SET_FAILED;
            break;
        }

    }while (0);

    /*
    ** Disconnect
    */
    rda_disconnect (sockfd);

#endif

    /*
    ** Done
    */
    return rc;

}

/****************************************************************************
 * Name          : pcs_rda_get_role
 *
 * Description   : 
 *                 
 *
 * Arguments     : PCS_RDA_ROLE - pointer to variable to return init role
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_get_role (PCS_RDA_ROLE * role)
{
    uns32   rc = PCSRDA_RC_SUCCESS;

#if (RDA_SPOOF != 0)

    /*
    ** Determine the role from slot id
    */
    if (rda_current_role == PCS_RDA_UNDEFINED)
    {
        uns32 init_role = -1;

        if (get_init_role (&init_role) != 0)
        {
            return PCSRDA_RC_CALLBACK_REG_FAILED;

        }

        if (init_role == 1)
        {
            rda_current_role = PCS_RDA_ACTIVE;

        }
        else
        {
            rda_current_role = PCS_RDA_STANDBY;

        }
    }

    *role = rda_current_role;

#else

    int     sockfd;
    char    msg [64]          = {0};
    int     value             = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;

    *role = 0;

    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {

        /*
        ** Send get-role request messgae
        */
        sprintf (msg, "%d", RDE_RDA_GET_ROLE_REQ);
        rc = rda_write_msg (sockfd, msg);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Recv get-role response
        */
        rc = rda_read_msg (sockfd, msg, sizeof (msg));
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if ((cmd_type != RDE_RDA_GET_ROLE_RES) || (value < 0))
        {
            rc = PCSRDA_RC_ROLE_GET_FAILED;
            break;
        }

        /*
        ** We have a role
        */
        *role = value;

    }while (0);

    /*
    ** Disconnect
    */
    rda_disconnect (sockfd);

#endif

    /*
    ** Done
    */
    return rc;

}

/****************************************************************************
 * Name          : pcs_rda_avd_hb_err
 *
 * Description   : 
 *                 
 *
 * Arguments     : 
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_avd_hb_err (void)
{

    uns32   rc = PCSRDA_RC_SUCCESS;
PCS_RDA_CMD cmd ;

   m_NCS_OS_MEMSET(&cmd, 0, sizeof(PCS_RDA_CMD));
#if (RDA_SPOOF != 0)

    /*
    ** If standby change role to active
    */
    if( (rda_current_role == PCS_RDA_STANDBY) ||
        (rda_current_role == PCS_RDA_QUIESCED) )
    {
        rda_current_role = PCS_RDA_ACTIVE;

        /*
        ** Intimate registered client about role change
        */
        if (rda_spoof_callback_cb.callback_ptr != NULL)
            (*rda_spoof_callback_cb.callback_ptr) (rda_spoof_callback_cb.callback_handle, rda_current_role, PCSRDA_RC_SUCCESS,cmd);

    }

#else

    int     sockfd;
    char    msg [64]          = {0};
    int     value             = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;

    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {
        /*
        ** Send heart beat error messgae
        */
        sprintf (msg, "%d", RDE_RDA_AVD_HB_ERR_REQ);
        rc = rda_write_msg (sockfd, msg);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Recv heart beat error response
        */
        rc = rda_read_msg (sockfd, msg, sizeof (msg));
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if (cmd_type != RDE_RDA_AVD_HB_ERR_ACK)
        {
            rc = PCSRDA_RC_AVD_HB_ERR_FAILED;
            break;
        }

    }while (0);

    /*
    ** Disconnect
    */
    rda_disconnect (sockfd);

#endif

    /*
    ** Done
    */
    return rc;

}

/****************************************************************************
 * Name          : pcs_rda_avnd_hb_err
 *
 * Description   : 
 *                 
 *
 * Arguments     : 
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_avnd_hb_err (uns32 phy_slot_id)
{

    uns32   rc = PCSRDA_RC_SUCCESS;
PCS_RDA_CMD cmd;
   m_NCS_OS_MEMSET(&cmd, 0, sizeof(PCS_RDA_CMD));

#if (RDA_SPOOF != 0)

   /*
   ** Intimate registered client about this event
   */
   if (rda_spoof_callback_cb.callback_ptr != NULL)
      (*rda_spoof_callback_cb.callback_ptr) (rda_spoof_callback_cb.callback_handle, rda_current_role, PCSRDA_RC_SUCCESS,cmd);

#else

    int     sockfd;
    char    msg [64]          = {0};
    int     value             = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;

    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {
        /*
        ** Send heart beat error messgae
        */
        sprintf (msg, "%d %d", RDE_RDA_AVND_HB_ERR_REQ, phy_slot_id);
        rc = rda_write_msg (sockfd, msg);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Recv heart beat error response
        */
        rc = rda_read_msg (sockfd, msg, sizeof (msg));
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if (cmd_type != RDE_RDA_AVND_HB_ERR_ACK)
        {
            rc = PCSRDA_RC_AVND_HB_ERR_FAILED;
            break;
        }

    }while (0);

    /*
    ** Disconnect
    */
    rda_disconnect (sockfd);

#endif

    /*
    ** Done
    */
    return rc;

}

/****************************************************************************
 * Name          : pcs_rda_avd_hb_restore
 *
 * Description   : 
 *                 
 *
 * Arguments     : 
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_avd_hb_restore (void)
{

    uns32   rc = PCSRDA_RC_SUCCESS;

#if (RDA_SPOOF != 0)

    /*
    ** For spoof implementation this is a no-op.
    */
    if (rda_spoof_callback_cb.callback_ptr != NULL)
         (*rda_spoof_callback_cb.callback_ptr) (rda_spoof_callback_cb.callback_handle, rda_current_role, PCSRDA_RC_SUCCESS);

#else

    int     sockfd;
    char    msg [64]          = {0};
    int     value             = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;

    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {
        /*
        ** Send heart beat error messgae
        */
        sprintf (msg, "%d", RDE_RDA_AVD_HB_RESTORE_REQ);
        rc = rda_write_msg (sockfd, msg);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Recv heart beat error response
        */
        rc = rda_read_msg (sockfd, msg, sizeof (msg));
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if (cmd_type != RDE_RDA_AVD_HB_RESTORE_ACK)
        {
            rc = PCSRDA_RC_AVD_HB_RESTORE_FAILED;
            break;
        }

    }while (0);

    /*
    ** Disconnect
    */
    rda_disconnect (sockfd);

#endif

    /*
    ** Done
    */
    return rc;

}

/****************************************************************************
 * Name          : pcs_rda_avnd_hb_restore
 *
 * Description   : 
 *                 
 *
 * Arguments     : 
 *
 * Return Values : 
 *
 * Notes         : None
 *****************************************************************************/
int pcs_rda_avnd_hb_restore (uns32 phy_slot_id)
{

    uns32   rc = PCSRDA_RC_SUCCESS;

#if (RDA_SPOOF != 0)

   /*
   ** Intimate registered client about this event
   */
   if (rda_spoof_callback_cb.callback_ptr != NULL)
      (*rda_spoof_callback_cb.callback_ptr) (rda_spoof_callback_cb.callback_handle, rda_current_role, PCSRDA_RC_SUCCESS);

#else

    int     sockfd;
    char    msg [64]          = {0};
    int     value             = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;

    /*
    ** Connect
    */
    rc = rda_connect (&sockfd);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    do
    {
        /*
        ** Send heart beat error messgae
        */
        sprintf (msg, "%d %d", RDE_RDA_AVND_HB_RESTORE_REQ, phy_slot_id);
        rc = rda_write_msg (sockfd, msg);
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        /*
        ** Recv heart beat error response
        */
        rc = rda_read_msg (sockfd, msg, sizeof (msg));
        if (rc != PCSRDA_RC_SUCCESS)
        {
            break;
        }

        rda_parse_msg (msg, &cmd_type, &value);
        if (cmd_type != RDE_RDA_AVND_HB_RESTORE_ACK)
        {
            rc = PCSRDA_RC_AVND_HB_RESTORE_FAILED;
            break;
        }

    }while (0);

    /*
    ** Disconnect
    */
    rda_disconnect (sockfd);

#endif

    /*
    ** Done
    */
    return rc;
}

/*****************************************************************************

  PROCEDURE NAME:       rda_get_control_block

  DESCRIPTION:          Singleton implementation for RDE Context
                     
  ARGUMENTS:

  RETURNS:

  NOTES:

*****************************************************************************/

RDA_CONTROL_BLOCK *rda_get_control_block (void)
{
   static NCS_BOOL          initialized = FALSE;
   static RDA_CONTROL_BLOCK rda_cb;

   /*
   ** Initialize CB
   */
   if (!initialized)
   {
       initialized = TRUE;
       m_NCS_OS_MEMSET( &rda_cb, 0, sizeof(rda_cb));

       /* Init necessary members */
       m_NCS_OS_STRNCPY(&rda_cb.sock_address.sun_path, RDE_RDA_SOCK_NAME, sizeof(rda_cb.sock_address));
       rda_cb.sock_address.sun_family = AF_UNIX ;
   }

   return &rda_cb;
}


/*****************************************************************************

  PROCEDURE NAME:       rda_connect

  DESCRIPTION:          Open the socket associated with 
                        the RDA Interface 
                     
  ARGUMENTS:


  RETURNS:

    PCSRDA_RC_SUCCESS:   

  NOTES:

*****************************************************************************/

static
uns32 rda_connect (int *sockfd)
{
    RDA_CONTROL_BLOCK *rda_cb = rda_get_control_block ();
   /*
   ** Create the socket descriptor
   */
   *sockfd = socket (AF_UNIX, SOCK_STREAM, 0) ;
   if (sockfd < 0)
   {
       return PCSRDA_RC_IPC_CREATE_FAILED;
   }

   /*
   ** Connect to the server.
   */
   if (connect(*sockfd, (struct sockaddr *) &rda_cb->sock_address, sizeof (rda_cb->sock_address)) < 0)
   {
       close (*sockfd);
       return PCSRDA_RC_IPC_CONNECT_FAILED;
   }    
        
   return PCSRDA_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rda_disconnect

  DESCRIPTION:          Close the socket associated with 
                        the RDA Interface 
                     
  ARGUMENTS:


  RETURNS:

    PCSRDA_RC_SUCCESS:   

  NOTES:

*****************************************************************************/

static
uns32 rda_disconnect (int sockfd)
{
    char               msg [64] = {0};
    
    /*
    ** Format message
    */
    sprintf (msg, "%d", RDE_RDA_DISCONNECT_REQ);

    if (rda_write_msg (sockfd, msg) != PCSRDA_RC_SUCCESS)
    {
        /* Nothing to do here */
    }

    /*
    ** Close
    */
    close (sockfd);

   return PCSRDA_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rda_callback_req

  DESCRIPTION:          Sends callback request to RDE

  ARGUMENTS:


  RETURNS:

    PCSRDA_RC_SUCCESS:

  NOTES:

*****************************************************************************/

static
uns32 rda_callback_req (int sockfd)
{
    char             msg [64] = {0};
    uns32            rc       = PCSRDA_RC_SUCCESS;
    int              value    = -1;
    RDE_RDA_CMD_TYPE cmd_type = 0;

    /*
    ** Send callback reg request messgae
    */
    sprintf (msg, "%d", RDE_RDA_REG_CB_REQ);
    rc = rda_write_msg (sockfd, msg);
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    /*
    ** Recv callback reg response
    */
    rc = rda_read_msg (sockfd, msg, sizeof (msg));
    if (rc != PCSRDA_RC_SUCCESS)
    {
        return rc;
    }

    rda_parse_msg (msg, &cmd_type, &value);
    if (cmd_type != RDE_RDA_REG_CB_ACK)
    {
        return PCSRDA_RC_CALLBACK_REG_FAILED;
    }

    return PCSRDA_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:       rda_write_msg

  DESCRIPTION:    Write a message to a peer on the socket
                     
  ARGUMENTS:

  RETURNS:

    PCSRDA_RC_SUCCESS:                   

  NOTES:

*****************************************************************************/

static
uns32 rda_write_msg (int sockfd, char *msg)
{
   int  msg_size   = 0;

   /*
   ** Read from socket into input buffer
   */

   msg_size = send (sockfd, msg, m_NCS_OS_STRLEN (msg) + 1, 0);
   if (msg_size < 0)
   {
        return PCSRDA_RC_IPC_SEND_FAILED;
   }

   return PCSRDA_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:       rda_read_msg

  DESCRIPTION:    Read a complete line from the socket
                     
  ARGUMENTS:

  RETURNS:

    PCSRDA_RC_SUCCESS:                   

  NOTES:

*****************************************************************************/

static
uns32 rda_read_msg (int sockfd, char *msg, int size)
{
   int              rc        = PCSRDA_RC_SUCCESS;
   int              msg_size  = 0;
   int              retry_cnt = 0;
   fd_set           readfds;
   struct timeval   tv;

RDE_SELECT:
   FD_ZERO(&readfds);

   /* 
    * Set timeout value
    */
   tv.tv_sec  = 30; 
   tv.tv_usec = 0;

   /*
    * Set RDA interface socket fd
    */
   FD_SET (sockfd, &readfds);

   rc = select(sockfd + 1, &readfds, NULL, NULL, &tv);
   if (rc < 0)
   {
        if (errno == 4) /* EINTR */
        {
            printf ("select: PCSRDA_RC_IPC_RECV_FAILED: rc=%d-%s\n", errno, strerror (errno));
            if (retry_cnt < 5)
            {
                retry_cnt++;
                goto RDE_SELECT;
            }
        }
       return PCSRDA_RC_IPC_RECV_FAILED;
   }

   if (rc == 0)
   {
       /*
       ** Timed out
       */
       return PCSRDA_RC_TIMEOUT;
   }

   /*
   ** Read from socket into input buffer
   */
   msg_size = recv (sockfd, msg, size, 0);
   if (msg_size < 0)
   {
       printf ("recv: PCSRDA_RC_IPC_RECV_FAILED: rc=%d-%s\n", errno, strerror (errno));
       return PCSRDA_RC_IPC_RECV_FAILED;
   }
   
   /*
   ** Is connection shutdown by server?
   */
   if (msg_size == 0)
   {
        /*
        ** Yes
        */
        return PCSRDA_RC_FATAL_IPC_CONNECTION_LOST;
   }

   return PCSRDA_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:       rda_parse_msg

  DESCRIPTION:    
                     
  ARGUMENTS:

  RETURNS:

    PCSRDA_RC_SUCCESS:                   

  NOTES:

*****************************************************************************/
static
uns32 rda_parse_msg(const char *pmsg, RDE_RDA_CMD_TYPE *cmd_type, int *value)
{
    char               msg [64] = {0};
    char              *ptr;

    m_NCS_STRCPY (msg, pmsg);
    *value    = -1;
    *cmd_type = RDE_RDA_UNKNOWN;

    /*
    ** Parse the message for cmd type and value
    */
    ptr = strchr (msg, ' ');
    if (ptr == NULL)
    {
        *cmd_type = atoi (msg);

    }
    else
    {
        *ptr      = '\0';
        *cmd_type = atoi (msg);
        *value    = atoi (++ptr);
    }

    return PCSRDA_RC_SUCCESS;

}

/**************************************************/

#if (RDA_SPOOF != 0)

static char afs_config_root[256];

static uns32 file_get_word(FILE **fp, char *o_chword)
{
   int temp_char;
   unsigned int temp_ctr=0;
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
      return(EOF);
   }
   if (temp_char == '\n')
   {
      return(1);
   }
   if(o_chword[0] == 0x0)
      goto try_again;
   return(0);
}

static uns32 get_init_role(uns32 *init_role)
{
   FILE *fp;
   char get_word[256];
   uns32 res = 0;
   uns32 d_len;
   
   sprintf(afs_config_root, "%s", "/etc/opt/opensaf");
   /*sprintf(afs_config_root, "%s", ".");*/
   d_len = strlen(afs_config_root);
   /* Hack afs_config_root to construct path */
   sprintf(afs_config_root + d_len, "%s", "/init_role");
   fp = fopen(afs_config_root,"r");
   /* Reverse hack afs_config_root to original value*/
   afs_config_root[d_len] = 0;

   if (fp == NULL)
   {
      printf("\nRDA: Couldn't open %s/init_role \n", afs_config_root);
      return 1;
   }
   do
   {   
      file_get_word(&fp,get_word);      
      if ((*init_role = atoi(get_word)) == -1)
      {
         res = 1;
         break;
      }
      fclose(fp);
   } while(0);

   return(res);
}

#endif
