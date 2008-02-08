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

 MODULE NAME:  SHIMMAB.C

$Header: /ncs/software/leap/base/products/cli/common/shimmab.c 9     6/19/01 3:20p Agranos $
..............................................................................

  DESCRIPTION:

  Source file for external pubished API of SHIM Layer that mimic MAB
******************************************************************************
*/

#include "shimmab.h"

#if ((NCS_MAB != 1) &&  (NCS_CLI == 1))

#define NCS_SHIMMAB_TASK_PRIO       NCS_TASK_PRIORITY_4

/*Global variables */
static SYSF_MBX  gl_ShimMabMbx;
static NCSCONTEXT gl_ShimMabTask;

/*****************************************************************************
  PROCEDURE NAME  :  ncsshim_create
  DESCRIPTION     :  Creates the shim layer that is used when MAB is not present
  PARAMETERS      :  demo_mbx : Pointer to SYSF_MBX for communication with Demo task.
  RETURNS         :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  NOTES           :  This function conforms to the NCS_OS_CB function prototype.
*****************************************************************************/
uns32 ncsshim_create(void)
{
    /*Create Mailbox */
    if (NCSCC_RC_SUCCESS != m_NCS_IPC_CREATE (&gl_ShimMabMbx))
        return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);

    if(NCSCC_RC_SUCCESS != m_NCS_TASK_CREATE((NCS_OS_CB)shim_mbx_handler,
                         &gl_ShimMabMbx,
                         NCS_CLI_SHIM_TASKNAME,
                         NCS_CLI_SHIM_PRIORITY,
                         NCS_CLI_SHIM_STACKSIZE,
                         &gl_ShimMabTask))
   {
      m_NCS_IPC_RELEASE(&gl_ShimMabMbx, NULL);
      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   }

   /** Attach to the mailbox, ie, bump the usecount.**/
   m_NCS_IPC_ATTACH(&gl_ShimMabMbx);

   /** Now put the thread into active state**/
   if (NCSCC_RC_SUCCESS != m_NCS_TASK_START (gl_ShimMabTask))
   {
      m_NCS_TASK_RELEASE(gl_ShimMabTask);
      m_NCS_IPC_RELEASE(&gl_ShimMabMbx, NULL);

      return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  PROCEDURE NAME  :  ncsshim_mib_request
  DESCRIPTION     :  Hign level entry point into SHIM Layer for MIB requests 
                     if MAB not in use
  ARGUMENTS       :  mib_arg:  Arguments for request
  RETURNS         :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  NOTES           :
*****************************************************************************/
uns32 ncsshim_mib_request(struct ncsmib_arg *mib_args)
{
    SHIM_MSG *shim_msg = m_MMGR_ALLOC_SHIM_MSG;

    m_NCS_OS_MEMSET(shim_msg, 0, sizeof(SHIM_MSG));
    shim_msg->event = mib_args->i_mib_key->svc;

    if((shim_msg->mib_arg = ncsmib_memcopy(mib_args)) == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    
    /* USRBUF, if any, would have been DITTOed in the ncsmib_memcopy() call 
       done above. While the DITTOED USRBUF is dispatched by SHIM to the 
       subsystem which needs to handle the MIB request, the original USRBUF
       is freed here. 

       This behaviour is consistent with processing done by MAC. 
    */
    switch (mib_args->i_op)
    {
    case NCSMIB_OP_REQ_SETROW  :
    case NCSMIB_OP_REQ_TESTROW :
       {
          m_MMGR_FREE_BUFR_LIST(mib_args->req.info.setrow_req.i_usrbuf);          
          mib_args->req.info.setrow_req.i_usrbuf = NULL;
          break;
       }
    default:
       break;
    }

    /** Send it to the SHIM Task...
    **/
    if(NCSCC_RC_SUCCESS != m_NCS_IPC_SEND(&gl_ShimMabMbx,
                                        shim_msg,
                                        NCS_IPC_PRIORITY_NORMAL))
    {
       /* Free the allocated memory by ncsmib_memcopy() routine */
       ncsmib_memfree(shim_msg->mib_arg);         
       
       m_MMGR_FREE_SHIM_MSG(shim_msg);
       return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
    }

    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME  :  shim_mbx_handler
  DESCRIPTION     :  Main entry Point
  PARAMETERS      :  mbx : Pointer to SYSF_MBX for communication Subsystem .
  RETURNS         :  Nothing
  NOTES           :  This function conforms to the NCS_OS_CB function prototype.
*****************************************************************************/
void
shim_mbx_handler(void *mbx)
{   
   SHIM_MSG *msg = 0;
   
   /*** notifications from the  subsystem CEF **/
   while(NULL != (msg=(SHIM_MSG *)m_NCS_IPC_RECEIVE(mbx, msg)))
   {
      switch(msg->event)
      {
      case NCS_SERVICE_ID_DUMMY:
#if(NCS_CLI == 1)
         ncsgbl_mib_request(msg->mib_arg);
         break;
#endif
         
      default:
         break;
      }
      
      if(msg)
      {    
         ncsmib_arg_free_resources (msg->mib_arg, TRUE);
         m_MMGR_FREE_SHIM_MSG(msg);
         msg = 0;
      }
   }  
}

/*****************************************************************************

  Function NAME:  ncsshim_destroy
  DESCRIPTION:    Destroy the shim layer that is used when MAB is not present
  PARAMETERS   :  demo_mbx:   Pointer to SYSF_MBX for communication with Demo task.
  RETURNS      :  NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
  NOTES        :  This function conforms to the NCS_OS_CB function prototype.
*****************************************************************************/
uns32
ncsshim_destroy(void)
{
    /** Detach to the mailbox.
    **/
    if(NCSCC_RC_SUCCESS != m_NCS_IPC_DETACH(&gl_ShimMabMbx, NULL, NULL))
        return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);

   /** Release IPC mechanism for the SHIM...
    **/
    if(NCSCC_RC_SUCCESS != m_NCS_IPC_RELEASE(&gl_ShimMabMbx, NULL))
        return m_CLI_DBG_SINK(NCSCC_RC_FAILURE);
   return NCSCC_RC_SUCCESS;
}

#else
extern int dummy;
#endif /* (if NCS_CLI == 1) */


