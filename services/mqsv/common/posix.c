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


/***************************************************************************
 *
 * uns32 ncs_os_posix_mq(NCS_OS_POSIX_MQ_REQ_INFO *req)
 * 
 *
 * Description:
 *   This routine handles operating system primitives for message-queues.
 *
 * Synopsis:
 *
 * Call Arguments:
 *    req ............... ....action request
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS if successful, otherwise NCSCC_RC_FAILURE.
 *
 * Notes:
 *
 ****************************************************************************/
#include "mqsv.h"


#if 0

#ifdef USE_POSIX_ONLY

uns32 ncs_os_use_posix(NCS_OS_POSIX_MQ_REQ_INFO *req)
{

    return NCSCC_RC_FAILURE;
}

#else
uns32 ncs_os_use_osmq(NCS_OS_POSIX_MQ_REQ_INFO *req)
{

   switch(req->req)
   {
   case NCS_OS_POSIX_MQ_REQ_MSG_SEND:
      {

         NCS_OS_MQ_REQ_INFO os_req;

         os_req.req = NCS_OS_MQ_REQ_MSG_SEND;

         os_req.info.send.i_hdl = req->info.send.mqd;
         os_req.info.send.i_len = req->info.send.i_msg.datalen;
         os_req.info.send.i_msg = req->info.send.i_msg.data;

         if (ncs_os_mq(&os_req) != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;
      }
      break;

   case NCS_OS_POSIX_MQ_REQ_MSG_RECV:
      {

         NCS_OS_MQ_REQ_INFO os_req;

         os_req.req = NCS_OS_MQ_REQ_MSG_RECV;

         os_req.info.recv.i_hdl = req->info.recv.mqd;
         os_req.info.recv.i_msg = req->info.recv.i_msg.data;
         os_req.info.recv.i_max_recv = req->info.recv.i_msg.datalen;

         if (ncs_os_mq(&os_req) != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;

      }
      break;
   case NCS_OS_POSIX_MQ_REQ_OPEN:
      {
         NCS_OS_MQ_REQ_INFO os_req;
         NCS_OS_MQ_KEY key;

         if (req->info.open.iflags & O_CREAT)
         {
             os_req.req = NCS_OS_MQ_REQ_CREATE;
             key = ftok(req->info.open.qname,1);
             os_req.info.create.i_key = &key;
         }
         else
         {
             os_req.req = NCS_OS_MQ_REQ_OPEN;
             key = ftok(req->info.open.qname,1);
             os_req.info.open.i_key = &key;
         }

         if (ncs_os_mq(&os_req) != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;

         if (req->info.open.iflags & O_CREAT)
             req->info.open.o_mqd = os_req.info.create.o_hdl;
         else
             req->info.open.o_mqd = os_req.info.open.o_hdl;
        
      }
      break;
   case NCS_OS_POSIX_MQ_REQ_CLOSE:
      {
         NCS_OS_MQ_REQ_INFO os_req;

         os_req.req = NCS_OS_MQ_REQ_DESTROY;
         os_req.info.destroy.i_hdl = req->info.close.mqd;

         if (ncs_os_mq(&os_req) != NCSCC_RC_SUCCESS)
             return NCSCC_RC_FAILURE;
      }
      break;
   case NCS_OS_POSIX_MQ_REQ_UNLINK:
   default:
      return NCSCC_RC_FAILURE;
   }
   return NCSCC_RC_SUCCESS;
}


#endif



uns32 ncs_os_posix_mq(NCS_OS_POSIX_MQ_REQ_INFO *req)
{


#ifdef USE_POSIX_ONLY
    return ncs_os_use_posix(req);
#else
    return ncs_os_use_osmq(req);
#endif


}

#endif
