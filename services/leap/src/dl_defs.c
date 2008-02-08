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

  MODULE NAME: DL_DEFS.C

.............................................................................

  DESCRIPTION: Layer 2 Support (for ISIS)

  NOTES:

******************************************************************************
*/


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

            H&J Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_dl.h"
#include "ncssysf_tsk.h"
#include "ncssysf_mem.h"


/* Fix for IR10318 */
#ifdef NCS_IF_NAMESIZE
#undef NCS_IF_NAMESIZE
#define NCS_IF_NAMESIZE IF_NAMESIZE
#endif

/* Fix for IR10317 */
/* Earlier it was included from file ncs_ipprm.h which is
 * included conditionally on flag NCS_IP_SERVICES, which inturn was
 * defined based on NCS_MDS=1 flag. But for independent builds of 
 * LEAP base it needs to be included here.
 */
#include "ncs_scktprm.h"
/* Fix Ends */



#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>

#include "sysf_def.h"
#include "ncs_dlprm.h"

#ifdef  DUMP_USRBUF_IN_MDS_LOG /* Never to be enabled in real code */
#include "mds_inc.h"
#endif

#define L2STACK_BUFFER_SIZE (1500)
#define SYS_L2SOCK_DEF_SEL_TIMEOUT 20

#define NCS_DL_DEFS_TRACE  0
#define NCS_DL_DEFS_LOG  0

#if (NCS_DL_DEFS_TRACE == 1)
#define m_NCS_DL_DEFS_TRACE m_NCS_CONS_PRINTF
#else
#define m_NCS_DL_DEFS_TRACE 
#endif
#if (NCS_DL_DEFS_LOG == 1)
#define m_NCS_DL_DEFS_LOG m_NCS_CONS_PRINTF
#else
#define m_NCS_DL_DEFS_LOG 
#endif

typedef enum {
    NCS_DL_MEM_SUB_ID_L2FILTER_ENTRY,
    NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY,
    NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT,
    NCS_DL_MEM_SUB_ID_CHAR_BUFF
}NCS_DL_MEM_SUB_ID;

static NCS_BOOL gl_dl_res_inited = FALSE;
static uns32   gl_dl_res_usr_cnt = 0;
NCS_LOCK        gl_dl_res_lock;

const NCS_L2SOCKET_HANDLER gl_ncs_l2socket_raw_dispatch [NCS_SOCKET_EVENT_MAX+1][NCS_SOCKET_STATE_MAX+1];

static void
l2socket_list_destroy (NCS_L2SOCKET_LIST *socket_list);

static uns32
dl_svc_802_2_ethhdr_build (NCS_L2SOCKET_ENTRY*    se,
                           NCS_DL_REQUEST_INFO* dlr,
                           struct sockaddr_in* saddr,
                           int*                total_len,
                           USRBUF            **usrbuf);

static void
l2socket_entry_destroy (NCS_L2SOCKET_ENTRY *se);

uns32
l2socket_entry_set_fd (NCS_L2SOCKET_ENTRY *se,
                       NCS_BOOL          readfd,
                       NCS_BOOL          writefd);

uns32
l2socket_entry_set_fd (NCS_L2SOCKET_ENTRY *se,
                       NCS_BOOL          readfd,
                       NCS_BOOL          writefd)
{

   NCS_L2SOCKET_LIST    *socket_list;
   NCS_L2SOCKET_CONTEXT *socket_context;

   if ((socket_context = se->socket_context) == NULL)
      return NCSCC_RC_FAILURE;

   socket_list = &socket_context->ReceiveQueue;

   if(m_NCS_LOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
     return NCSCC_RC_FAILURE;
   }

   if (readfd == TRUE)
   {
      if (!(FD_ISSET (se->client_socket, &socket_list->readfds)))
         FD_SET (se->client_socket, &socket_list->readfds);
   }
   else
   {
      if (FD_ISSET (se->client_socket, &socket_list->readfds))
         FD_CLR (se->client_socket, &socket_list->readfds);

   }
   if (writefd == TRUE)
   {
      if (!(FD_ISSET (se->client_socket, &socket_list->writefds)))
         FD_SET (se->client_socket, &socket_list->writefds);
   }
   else
   {
      if (FD_ISSET (se->client_socket, &socket_list->writefds))
         FD_CLR (se->client_socket, &socket_list->writefds);
   }

   if(m_NCS_UNLOCK (&socket_list->lock, NCS_LOCK_WRITE) ==  NCSCC_RC_FAILURE)
   {
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;

}

static void
l2socket_entry_remove (NCS_L2SOCKET_CONTEXT *socket_context,
             SOCKET             sock)
{

   NCS_L2SOCKET_LIST    *socket_list;
   NCS_L2SOCKET_ENTRY   *socket_list_walker;

   socket_list = &socket_context->ReceiveQueue;

   if(m_NCS_LOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
     return;
   }

   /* walk through the list until the socket is found. If it isn't
      found, we don't consider it an error */
   socket_list_walker = socket_list->first;

   while (socket_list_walker != NULL)
   {
      /* found the socket, mark it for removal */
      if (socket_list_walker->client_socket == sock)
      {
         socket_list_walker->dl_data_indication = NULL;
         socket_list_walker->dl_ctrl_indication = NULL;

         socket_list_walker->state = NCS_SOCKET_STATE_CLOSING;
         socket_list->socket_entries_freed++;
         socket_list->num_in_list--;
     m_NCS_SEL_OBJ_IND(socket_context->fast_open_sel_obj); /*IR00059000 fix*/
      }
      /* move the walk pointer. */
      socket_list_walker = socket_list_walker->next;
   }

  if(m_NCS_UNLOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
  {
    return;
  }

}


static NCS_L2FILTER_ENTRY*
l2filter_entry_exists(NCS_L2SOCKET_CONTEXT * socket_context, uns16 protocol, uns8 * addr,uns32 if_index)
{
    NCS_PATRICIA_NODE * pNode;
    NCS_L2FILTER_KEY tmp;

    tmp.protocol = protocol;
    tmp.if_index = if_index;
    m_NCS_OS_MEMCPY(tmp.addr, addr, 6);

    if (m_NCS_LOCK(&socket_context->lock, NCS_LOCK_READ) == NCSCC_RC_FAILURE)
        return NULL;

    pNode = ncs_patricia_tree_get(&socket_context->socket_tree, (const uns8 *)&tmp);

    if (m_NCS_UNLOCK(&socket_context->lock, NCS_LOCK_READ) == NCSCC_RC_FAILURE)
        return NULL;

    return (NCS_L2FILTER_ENTRY*)pNode;
}


static NCS_L2FILTER_ENTRY*
l2filter_entry_create(NCS_L2SOCKET_CONTEXT * socket_context,
                      NCS_L2SOCKET_ENTRY * se, uns16 protocol,
                      uns8 * addr, NCSCONTEXT user_connection_handle,uns32 if_index)
{
    NCS_L2FILTER_ENTRY * fe;

    if ((fe = (NCS_L2FILTER_ENTRY *)m_NCS_MEM_ALLOC(sizeof(NCS_L2FILTER_ENTRY),
                                           NCS_MEM_REGION_PERSISTENT,
                                           NCS_SERVICE_ID_L2SOCKET, 
                                           NCS_DL_MEM_SUB_ID_L2FILTER_ENTRY)) == NULL)
                                           return (NCS_L2FILTER_ENTRY*)NULL;

    fe->key.protocol = protocol;
    fe->key.if_index = if_index; /* IR00058904 */
    m_NCS_OS_MEMCPY(fe->key.addr, addr, 6);
    fe->node.key_info = (uns8 *)&fe->key;
    fe->user_connection_handle = user_connection_handle;
    fe->se = se;

    if (m_NCS_LOCK(&socket_context->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
        return NULL;

    if (ncs_patricia_tree_add(&socket_context->socket_tree, &fe->node)
        != NCSCC_RC_SUCCESS)
    {
        m_NCS_UNLOCK(&socket_context->lock, NCS_LOCK_WRITE);
        m_NCS_MEM_FREE(fe, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_L2SOCKET, 
            NCS_DL_MEM_SUB_ID_L2FILTER_ENTRY);
        return NULL;
    }

    m_NCS_UNLOCK(&socket_context->lock, NCS_LOCK_WRITE);

    se->refcount++;
    return fe;
}


static void
l2filter_entry_destroy(NCS_L2FILTER_ENTRY * fe)
{
    NCS_L2SOCKET_ENTRY * se = fe->se;
    NCS_L2SOCKET_CONTEXT * sc = se->socket_context;

    m_NCS_LOCK(&sc->lock, NCS_LOCK_WRITE);
    ncs_patricia_tree_del(&sc->socket_tree, (NCS_PATRICIA_NODE *)fe);
    m_NCS_UNLOCK(&sc->lock, NCS_LOCK_WRITE);

    m_NCS_MEM_FREE(fe, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_L2SOCKET, 
        NCS_DL_MEM_SUB_ID_L2FILTER_ENTRY);
    se->refcount--;
}


static void
l2filter_entry_update_se(NCS_L2SOCKET_ENTRY *se)
{
    NCS_L2FILTER_KEY tmp;
    NCS_L2SOCKET_CONTEXT * sc = se->socket_context;
    NCS_L2FILTER_ENTRY *fe;

    tmp.protocol = se->protocol;
    tmp.if_index = se->if_index;    /* IR00058904 */
    m_NCS_OS_MEMSET(tmp.addr, 0,  6);

    m_NCS_LOCK(&sc->lock, NCS_LOCK_WRITE);
    fe = (NCS_L2FILTER_ENTRY *)ncs_patricia_tree_getnext(&sc->socket_tree, (const uns8 *)&tmp);
    m_NCS_UNLOCK(&sc->lock, NCS_LOCK_WRITE);

    if (fe == NULL)
        return;

    if (fe->key.protocol != se->protocol)
        return;
    if (fe->key.if_index != se->if_index) /* IR00058904 */
    return;
    se->user_connection_handle = fe->user_connection_handle;
    return;
}


static NCS_L2SOCKET_ENTRY*
l2socket_entry_exists(NCS_L2SOCKET_CONTEXT * socket_context, uns16 protocol ,uns32 if_index, char *ifname)
{
   NCS_L2SOCKET_LIST    *socket_list;
   NCS_L2SOCKET_ENTRY   *socket_list_walker;

   if (socket_context == NULL)
       return NULL;

   socket_list = &socket_context->ReceiveQueue;

   if(m_NCS_LOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
     return NULL;

   /* walk through the list until the socket is found. If it isn't
      found, we don't consider it an error */
   socket_list_walker = socket_list->first;

   while (socket_list_walker != NULL)
   {
       if ((socket_list_walker->protocol == protocol) && 
           ( ((if_index != 0 ) && (socket_list_walker->if_index == if_index)) ||
             ((if_index == 0 ) && (memcmp(socket_list_walker->if_name, ifname, sizeof(socket_list_walker->if_name)) == 0))
           )&&
           (socket_list_walker->state != NCS_SOCKET_STATE_CLOSING))
           break;

       socket_list_walker = socket_list_walker->next;
   }

   if(m_NCS_UNLOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
       return NULL;

   return socket_list_walker;
}


static NCS_L2SOCKET_ENTRY*
l2socket_entry_create(NCS_DL_REQUEST_INFO *dlr)
{

   NCS_L2SOCKET_ENTRY          *se;
   NCS_L2SOCKET_CONTEXT        *socket_context;

   if ((se = (NCS_L2SOCKET_ENTRY *)m_NCS_MEM_ALLOC(sizeof(NCS_L2SOCKET_ENTRY),
                                           NCS_MEM_REGION_PERSISTENT,
                                           NCS_SERVICE_ID_L2SOCKET, 
                                           NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY)) == NULL)
      return (NULL);


   m_NCS_OS_MEMSET (se, '\0', sizeof(NCS_L2SOCKET_ENTRY));

   if ((m_NCS_LOCK_INIT (&se->lock)) == NCSCC_RC_FAILURE)
   {
      m_NCS_MEM_FREE(se,NCS_MEM_REGION_PERSISTENT,NCS_SERVICE_ID_L2SOCKET,
          NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY);
      return NULL;
   }

   se->hm_hdl = 0;

   switch (dlr->i_request)
   {

   case NCS_DL_CTRL_REQUEST_OPEN:
      if ((socket_context =
         *(NCSCONTEXT*)(dlr->info.ctrl.open.i_dl_client_handle.data))
         == NULL)
      {
        m_NCS_LOCK_DESTROY (&se->lock);
        m_NCS_MEM_FREE(se, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_L2SOCKET,
            NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY);
        return (NULL);
      }

      se->socket_context         = socket_context;
      se->dl_type                = dlr->info.ctrl.open.i_dl_type;
      se->protocol               = dlr->info.ctrl.open.i_dl_protocol;
      se->user_handle            = dlr->i_user_handle;
      se->user_connection_handle
                             = dlr->info.ctrl.open.i_user_connection_handle;
      se->local_addr             = dlr->info.ctrl.open.i_local_addr;
      se->remote_addr            = dlr->info.ctrl.open.i_remote_addr;

      se->dl_data_indication     = dlr->info.ctrl.open.i_dl_data_indication;
      se->dl_ctrl_indication     = dlr->info.ctrl.open.i_dl_ctrl_indication;
      se->bindtodevice           = dlr->info.ctrl.open.i_bindtodevice;
      se->refcount               = 0;

    break;

   default:
      m_NCS_LOCK_DESTROY (&se->lock);
      m_NCS_MEM_FREE(se, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_L2SOCKET,
          NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY);
      return (NULL);
   }

   switch (se->dl_type)
   {

   case NCS_L2_SOCK_TYPE_RAW:
      se->dispatch       = &gl_ncs_l2socket_raw_dispatch[0][0];
      break;
   default:
      m_NCS_LOCK_DESTROY (&se->lock);
      m_NCS_MEM_FREE(se, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_L2SOCKET, 
          NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY);
      return (NULL);
   }
   return se;
}

static uns32
l2socket_list_clean(NCS_L2SOCKET_LIST *socket_list)
{
   NCS_L2SOCKET_ENTRY *socket_list_walker;
   NCS_L2SOCKET_ENTRY *temp_socket_entry;


   if (socket_list == NULL)
      return NCSCC_RC_FAILURE;

   if(m_NCS_LOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
       return NCSCC_RC_FAILURE;
   }

   /* walk through the socket list looking for remove flags */
   socket_list_walker = socket_list->first;

   while (socket_list_walker != NULL)
   {
      if (socket_list_walker->state == NCS_SOCKET_STATE_CLOSING)
      {

         if(socket_list_walker->client_socket != 0)
         {
         /* close the socket */
           m_NCSSOCK_CLOSE(socket_list_walker->client_socket);
           socket_list_walker->client_socket = 0;
         }

         socket_list_walker->client_socket = 0;

         /* check to see if this is the first socket in the list */
         if (socket_list_walker->prev == NULL)
         {
            /* point the list's first to this node's next */
            socket_list->first = socket_list_walker->next;

            /* If the new first isn't NULL, set it's previous to NULL */
            if (socket_list->first!=NULL)
               socket_list->first->prev = NULL;
         }
         else /* point the prev node past this node to next */
            socket_list_walker->prev->next = socket_list_walker->next;


         /* check to see if this is the last socket in the list */
         if (socket_list_walker->next == NULL)
         {
            /* point the list's last to this node's prev */
            socket_list->last = socket_list_walker->prev;

            /* If the new last isn't NULL, set it's next to NULL */
            if (socket_list->last!=NULL)
               socket_list->last->next = NULL;
         }
         else /* point the next node past this node to prev */
            socket_list_walker->next->prev = socket_list_walker->prev;


         /* move the walk pointer and free the deleted descriptor */
         temp_socket_entry   = socket_list_walker;
         socket_list_walker  = temp_socket_entry->next;
         l2socket_entry_destroy (temp_socket_entry);
         socket_list->socket_entries_freed--;
      }
      else
      {
         /* move the walk pointer. */
         socket_list_walker = socket_list_walker->next;
      }
   }

   if(m_NCS_UNLOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
     /* LOG_ISIS */
   }
   return NCSCC_RC_SUCCESS;
}



static uns32
l2socket_entry_dispatch (NCS_L2SOCKET_ENTRY               *se,
                         NCS_SOCKET_EVENT                 event,
                         NCSCONTEXT                       arg)
{

   NCS_L2SOCKET_HANDLER    socket_handler;
   uns32                  rc;

   /* error */
   if (se == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   /* 1. Every se should be put into the list to prevent memory leak */
   /* 2. Don't call socket_entry_destroy() in handlers, mark it for deletion. */
   /*    socket_list_clean() will delete se entries periodically */
   /* 3. Handler should not be called on se whose state is NCS_SOCKET_STATE_CLOSING */
   if (se->state == NCS_SOCKET_STATE_CLOSING)
      return NCSCC_RC_SUCCESS;

  
   if(event != NCS_SOCKET_EVENT_CLOSE)
   {
     if(m_NCS_LOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
     {
#if 0
       m_SYSF_IP_LOG_ERROR("socket_entry_dispatch:failed to lock se", se);
#endif
       return NCSCC_RC_FAILURE;
     }
   }


   if((event > NCS_SOCKET_EVENT_MAX) || (se->state > NCS_SOCKET_STATE_MAX))
   {
     if(event != NCS_SOCKET_EVENT_CLOSE)
     {
       if(m_NCS_UNLOCK(&se->lock,NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
       {
#if 0
         m_SYSF_IP_LOG_ERROR("socket_entry_dispatch:failed to unlock se", se);
#endif
       }
     }
     return NCSCC_RC_FAILURE;
   }

   socket_handler = *(se->dispatch + (event * (NCS_SOCKET_STATE_MAX+1)) + se->state);
   rc = socket_handler (se, arg);

   if(event != NCS_SOCKET_EVENT_CLOSE)
   {
     if(m_NCS_UNLOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
     {
       /* LOG_ISIS */
     }
   }

   return rc;

}


static void
l2socket_list_task(void *ctxt)
{
   NCS_L2SOCKET_LIST        *socket_list;
   NCS_L2SOCKET_ENTRY       *se;
   NCS_L2SOCKET_CONTEXT     *socket_context;
   struct timeval         timeout;
   int                    ret_val;
   fd_set                 readfds;
   fd_set                 writefds;
   fd_set                 exceptfds;
   NCS_DL_ERROR            dl_error;
   int fdwidth = FD_SETSIZE;


   if ((socket_context = (NCS_L2SOCKET_CONTEXT *) ctxt) == NULL)
      return;

   socket_list = &socket_context->ReceiveQueue;

   while (socket_context->stop_flag == FALSE)
   {
      if (0 == socket_context->ReceiveQueue.num_in_list)
      {
         m_NCS_TASK_SLEEP(10);
      }
      else /* there are sockets to process */
      {
         m_NCS_MEMCPY ((char*) &writefds,
                      (char*) &socket_context->ReceiveQueue.writefds,
                      sizeof (fd_set));

         m_NCS_MEMCPY ((char*) &exceptfds,
                      (char*) &socket_context->ReceiveQueue.writefds,
                       sizeof (fd_set));

         m_NCS_MEMCPY ((char*) &readfds,
                      (char*) &socket_context->ReceiveQueue.readfds,
                      sizeof (fd_set));
     /* IR00059000 fix */
         FD_SET(m_GET_FD_FROM_SEL_OBJ(socket_context->fast_open_sel_obj), &readfds);

         timeout.tv_sec = socket_context->select_timeout;
         timeout.tv_usec = 0;

         /* Select has NOT timed out */
         socket_context->socket_timeout_flag = 0;


         se = socket_list->first;

         /* Check whether the socket is valid */
         if(se == NULL)
         {
            ret_val = m_NCSSOCK_ERROR;
            socket_context->stop_flag = TRUE;
            break;
         }
         else if(0 == se->client_socket || NCSSOCK_INVALID == se->client_socket || NCS_SOCKET_STATE_CLOSING == se->state)
         {
            ret_val = m_NCSSOCK_ERROR;
         }
         else
         {

            ret_val = m_NCSSOCK_SELECT (fdwidth, &readfds,
                                       &writefds, &exceptfds, &timeout);
            /* LOG_ISIS */
         }

         if(ret_val == NCSSOCK_ERROR)
         {
            dl_error = m_NCSSOCK_ERROR;

            /* LOG_ISIS */
            /** sleep for 20 ms on select error **/
            m_NCS_TASK_SLEEP(10);
         }
         else /* select did not return an error */
         {
            /* Select has timed out */
            socket_context->socket_timeout_flag = 1;
            if (ret_val == 0)
            {
            /* LOG_ISIS */
            }
            else /* there is activity on at least one socket */
            {

         /* IR00059000 fix */
        if(FD_ISSET(m_GET_FD_FROM_SEL_OBJ(socket_context->fast_open_sel_obj),&readfds))
                {
                  m_NCS_SEL_OBJ_RMV_IND(socket_context->fast_open_sel_obj,TRUE,FALSE);
                 }

               /** walk through this socket list until the end is reached **/
               se = socket_list->first;

               while (se != NULL)
               {
                  /* If this socket is set in the select mask invoke the fsm */
                  if (m_NCSSOCK_FD_ISSET (se->client_socket, &writefds))
                  {
                     int err = 0;
                     int sz = sizeof(err);
                     /* Peek into the socket for errors */
                     getsockopt(se->client_socket, SOL_SOCKET, SO_ERROR,
                                (char*)&err, &sz);
                     l2socket_entry_dispatch (se,
                                            ((err == 0) ?
                                             NCS_SOCKET_EVENT_WRITE_INDICATION :
                                             NCS_SOCKET_EVENT_ERROR_INDICATION),
                                             NULL);
                  }

                  if (m_NCSSOCK_FD_ISSET (se->client_socket, &readfds))
                  {
                     l2socket_entry_dispatch (se,
                                        NCS_SOCKET_EVENT_READ_INDICATION,NULL);
                  }

                  if (m_NCSSOCK_FD_ISSET (se->client_socket, &exceptfds))
                  {
                     int err=0, sz=sizeof(err);
                     getsockopt(se->client_socket, SOL_SOCKET,
                                SO_ERROR, (char *) &err, &sz);

                     l2socket_entry_dispatch (se,
                                      NCS_SOCKET_EVENT_ERROR_INDICATION,NULL);
                  }

                  /* move the walk pointer */
                  se = se->next;
               } /* end of while (se != NULL) */
            } /* end of else; there's activity on a socket */
         } /* end of else; select did not return an error */
      } /* end of else; there are sockets to process */


#ifdef __NCSINC_OSE__
     m_NCS_TASK_SLEEP(200);
#endif

      if ( 0 !=  socket_list->socket_entries_freed )
         l2socket_list_clean( socket_list );

   } /* end of while (socket_context->stop_flag == FALSE) */

   socket_context->stop_flag = FALSE;
}

static void l2socket_entry_destroy (NCS_L2SOCKET_ENTRY *se)
{
      m_NCS_LOCK_DESTROY (&se->lock);
      m_NCS_MEM_FREE(se, NCS_MEM_REGION_PERSISTENT, NCS_SERVICE_ID_L2SOCKET,
          NCS_DL_MEM_SUB_ID_L2SOCKET_ENTRY);

}

static void
l2socket_list_destroy (NCS_L2SOCKET_LIST *socket_list)
{
   NCS_L2SOCKET_ENTRY *socket_list_walker;
   NCS_L2SOCKET_ENTRY *socket_entry_to_free;

   if (NULL != socket_list)
   {
      socket_list_walker = socket_list->first;
      while (NULL != socket_list_walker)
      {
         /* save the cuurent descriptor */
         socket_entry_to_free = socket_list_walker;

         /* move the walk pointer */
         socket_list_walker = socket_list_walker->next;

         /* now it's safe to free it */
         l2socket_entry_destroy (socket_entry_to_free);
      }

      m_NCS_LOCK_DESTROY (&socket_list->lock);
   }

}

static void
l2socket_tree_destroy (NCS_PATRICIA_TREE * socket_tree)
{
    NCS_PATRICIA_NODE * pnode = NULL;

    pnode = ncs_patricia_tree_getnext(socket_tree, NULL);

    while (pnode != NULL)
    {
        /* IR00061773 */ 
        ncshm_destroy_hdl(NCS_SERVICE_ID_L2SOCKET,(uns32)(((NCS_L2FILTER_ENTRY *)pnode)->hm_hdl));
        ncs_patricia_tree_del(socket_tree, pnode);
        /* IR00061611 */ 
        m_NCS_MEM_FREE(pnode, NCS_MEM_REGION_PERSISTENT,
                       NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_L2FILTER_ENTRY);
        pnode = ncs_patricia_tree_getnext(socket_tree, NULL);
    }

    ncs_patricia_tree_destroy(socket_tree);
}


static uns32
l2socket_layer_destroy (NCSCONTEXT dl_client_handle)
{

   NCS_L2SOCKET_CONTEXT *socket_context;

   if ((socket_context = dl_client_handle) == NULL)
      return NCSCC_RC_FAILURE;
      
      /* IR00059000 fix */
     m_NCS_SEL_OBJ_DESTROY(socket_context->fast_open_sel_obj);
    
   /* Only when there are valid sockets with the socket_context, the 
      l2socket_list_task would be alive. When the active sockets are released,
      the task exits normally. Fix for BugID 14733. */
   if(socket_context->ReceiveQueue.first != NULL)
   {
       socket_context->stop_flag = TRUE; /* tell listen socket task to stop */
       
       while (socket_context->stop_flag == TRUE) /* wait for it to quiet down */
           m_NCS_TASK_SLEEP(10);
   }

   /* If control reaches here, socket_list_task() has cleared */
   /* the socket_context->stop_flag and exited by reaching */
   /* the end of its body. Can't stop a dead thread. -- HF */
   /* m_NCS_TASK_STOP(socket_context->task_handle); */

   m_NCS_TASK_RELEASE(socket_context->task_handle);

   l2socket_list_destroy (&socket_context->ReceiveQueue);

   l2socket_tree_destroy (&socket_context->socket_tree);
   m_NCS_MEM_FREE(socket_context,
                 NCS_MEM_REGION_PERSISTENT,
                 NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT);

   m_NCSSOCK_DESTROY;

   /* LOG_ISIS */

   return NCSCC_RC_SUCCESS;
}





static uns32
l2socket_list_create (NCS_L2SOCKET_LIST *socket_list)
{
   socket_list->first = NULL;
   socket_list->last  = NULL;
   socket_list->socket_entries_freed = 0;
   socket_list->num_in_list = 0;

   m_NCSSOCK_FD_ZERO (&socket_list->readfds);
   m_NCSSOCK_FD_ZERO (&socket_list->writefds);

   if (m_NCS_LOCK_INIT (&socket_list->lock)==NCSCC_RC_FAILURE)
   {
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;

}

static NCSCONTEXT
l2socket_layer_create (int select_timeout)
{

   NCS_L2SOCKET_CONTEXT *socket_context;
   
   /* Allocate a socket context */
   if ((socket_context = 
        (NCS_L2SOCKET_CONTEXT *) m_NCS_MEM_ALLOC(sizeof(NCS_L2SOCKET_CONTEXT),
                                           NCS_MEM_REGION_PERSISTENT, 
                                           NCS_SERVICE_ID_L2SOCKET,
                                           NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT))
                                                                       == NULL)
      return NULL;


   m_NCS_OS_MEMSET(socket_context, 0, sizeof(*socket_context));
   m_NCS_LOCK_INIT(&socket_context->lock);

   socket_context->tree_params.key_size =  sizeof(NCS_L2FILTER_KEY);   /* IR00058904 */
   if (ncs_patricia_tree_init(&socket_context->socket_tree, &socket_context->tree_params) != NCSCC_RC_SUCCESS)
   {
      m_NCS_MEM_FREE(socket_context, NCS_MEM_REGION_PERSISTENT,
                    NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT);
      m_NCSSOCK_DESTROY;
      return NULL;
   }

   /* Init the Receive queues in L2 socket */
   if (NCSCC_RC_FAILURE == l2socket_list_create (&socket_context->ReceiveQueue))
   {
      l2socket_tree_destroy(&socket_context->socket_tree);
      m_NCS_MEM_FREE(socket_context, NCS_MEM_REGION_PERSISTENT,
                    NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT);
      m_NCSSOCK_DESTROY;
      return NULL;
   }

   socket_context->socket_timeout_flag = 0; /* Forces a select timeout before a socket close */
   socket_context->max_connections = 10;    /* maximum backlog of pending connection requests
                                               used by a listening socket */
   socket_context->select_timeout = select_timeout;
   socket_context->stop_flag      = FALSE;

   if ( NCSCC_RC_FAILURE ==  m_NCS_TASK_CREATE((NCS_OS_CB) l2socket_list_task,
                                             (void *) socket_context,
                                             "NCSDL",
                                             NCS_TASK_PRIORITY_1,
                                             NCS_STACKSIZE_HUGE,
                                             (void **) &(socket_context->task_handle)))
   {
      l2socket_list_destroy (&socket_context->ReceiveQueue);
      l2socket_tree_destroy (&socket_context->socket_tree);
      m_NCS_MEM_FREE(socket_context, NCS_MEM_REGION_PERSISTENT,
                    NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT);
      m_NCSSOCK_DESTROY;
      return NULL;
   }
   
   m_NCS_TASK_START(socket_context->task_handle);
 
     /* IR00059000 fix */
   if (m_NCS_SEL_OBJ_CREATE(&socket_context->fast_open_sel_obj)!= NCSCC_RC_SUCCESS)
   {
      m_NCS_TASK_RELEASE(socket_context->task_handle);
      l2socket_list_destroy (&socket_context->ReceiveQueue);
      l2socket_tree_destroy (&socket_context->socket_tree);
      m_NCS_MEM_FREE(socket_context, NCS_MEM_REGION_PERSISTENT,
                    NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_L2SOCKET_CONTEXT);
      m_NCSSOCK_DESTROY;
      return NULL;
   }
   return (NCSCONTEXT) socket_context;
}

uns32
dl_svc_802_2_ethhdr_build (NCS_L2SOCKET_ENTRY*    se,
                           NCS_DL_REQUEST_INFO* dlr,
                           struct sockaddr_in* saddr,
                           int*                total_len,
                           USRBUF            **usrbuf)
{
   USRBUF *pHdr;
   uns8   *p8;
   struct ifreq ifr;
   uns16  data_len;
   uns16  eth_hdr_len;


   /* Store payload-data length */
   data_len = m_MMGR_LINK_DATA_LEN (dlr->info.data.send_data.i_usrbuf);

   /* Determine Ethernet-header length */
   eth_hdr_len = (se->protocol == ETH_P_802_2) ? 17 : 14;

#if 0 /* PM : 1/Feb/2005 :  Replacing MMGR_ALLOC_BUF with RESERVE_AT_START */
   if ((pHdr = m_MMGR_ALLOC_BUFR (0)) == NULL) 
#else
   if ((p8 = m_MMGR_RESERVE_AT_START(
                 &dlr->info.data.send_data.i_usrbuf, 
                 eth_hdr_len, uns8*)) == NULL) 
#endif
   {
      /* LOG_ISIS */
      return NCSCC_RC_FAILURE;
   }

   pHdr = dlr->info.data.send_data.i_usrbuf;

#if 0 /* PM : 1/Feb/2005 */
   ubuf = pHdr;
   if ((p8 = m_MMGR_DATA(ubuf, char *)) == NULL)
   {
      m_MMGR_FREE_BUFR_LIST(pHdr);
      return NCSCC_RC_FAILURE;
   }
#endif

   /* Fill the IEEE 802.2 ethernet Header (802.3 without SNAP is 802.2)*/
   m_NCS_OS_MEMCPY (p8, dlr->info.data.send_data.i_remote_addr.data.eth, 6);

   p8 += 6;


   /* set the source hw address */
   m_NCS_MEMSET(&ifr,0,sizeof(ifr));

   /********* Fix for 10318 ************/
   /* Directly get if_name stored in socket entry while opening socket */
   m_NCS_MEMCPY (ifr.ifr_name, se->if_name, sizeof(ifr.ifr_name));   
   /************* Fix ends ***************/

   if(m_NCS_TS_SOCK_IOCTL(se->client_socket,SIOCGIFHWADDR,&ifr) < NCS_TS_SOCK_ERROR)
   {
      perror("SIOGIFHWADDR");
      return NCSCC_RC_FAILURE;
   }

  /* Fill the source Address */
   m_NCS_OS_MEMCPY (p8, ifr.ifr_hwaddr.sa_data, 6);

   p8 += 6;

   /* Get the payload Length */

   if (se->protocol == ETH_P_802_2)
   {
       /* Fill in the Length */
       *((uns16 *)p8) = htons(data_len + 3); /* data_len + LLC (DSAP+SSAP+CTRL) */

       p8 += 2;

       *p8++ = 0xFE;  /* OSI PACKETS */
       *p8++ = 0xFE;  /* OSI PACKETS */
       *p8   = 0x03;

       /* Give the caller the total len */
       *total_len = data_len + 17;
   }
   else
   {
       *((uns16 *)p8) = htons(se->protocol);
       /* Give the caller the total len */
       *total_len = data_len + 14;
   }

   /* Append the Network layer data */
#if 0 /* PM : 1/Feb/2005 */
   m_MMGR_APPEND_DATA (pHdr, dlr->info.data.send_data.i_usrbuf);
   dlr->info.data.send_data.i_usrbuf = pHdr;
#endif
   *usrbuf = pHdr;

   return NCSCC_RC_SUCCESS;
}

static uns32
ncsl2sock_event_raw_send (NCS_L2SOCKET_ENTRY               *se,
                         NCSCONTEXT                      arg)
{
   struct ncs_dl_request_info_tag* dlr = (struct ncs_dl_request_info_tag*)arg;
   struct sockaddr_in             saddr;
   int                            total_len   = 0;
   USRBUF                        *usrbuf;
   NCS_L2_ADDR                   dst_addr;
   char                           buffer[L2STACK_BUFFER_SIZE];
   char                          *pBigBuff;
   char                          *bufp;
   uns32                         if_index;
   NCSCONTEXT                     dl_handle;
   int                            bytes_sent  = 0;
   struct sockaddr_ll             send_addr;
   uns32 total_bytes;

   dst_addr.dl_type = dlr->info.data.send_data.i_remote_addr.dl_type;
   m_NCS_OS_MEMCPY (dst_addr.data.eth , dlr->info.data.send_data.i_remote_addr.data.eth, 6);

   if_index  = se->if_index;
   dl_handle = dlr->info.data.send_data.i_dl_handle;

   if (dl_svc_802_2_ethhdr_build (se, dlr, &saddr, &total_len, &usrbuf) == NCSCC_RC_FAILURE)
   {
      return NCSCC_RC_FAILURE;
   }


   if (total_len <= L2STACK_BUFFER_SIZE)
   {
      pBigBuff = buffer;
   }
   else
   {
      /* Must malloc! */
      if ((pBigBuff = (char *)m_NCS_MEM_ALLOC((unsigned int)total_len , NCS_MEM_REGION_TRANSIENT, 
                                              NCS_SERVICE_ID_L2SOCKET, 
                                              NCS_DL_MEM_SUB_ID_CHAR_BUFF)) == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
   }

#ifdef DUMP_USRBUF_IN_MDS_LOG /* PM : 31/Jan/2005 */
        {
              uns32  ubuf_num;
              USRBUF *print_usrmsg;

              /* Explicit log trace-level check to avoid expensive logging code */
                  /* TRACE logging is enabled. Some more expensive trace logging */

                  for (ubuf_num = 0, print_usrmsg = usrbuf;
                  print_usrmsg;
                  ubuf_num++, print_usrmsg = print_usrmsg->link)
                  {
                      m_LOG_MDS_TRACE("DL_DEFS.C:WRITE_FRAG:UBUF_NUM=%d:START=%d:COUNT=%d:UB_REF_COUNT=%d\n",
                     ubuf_num, print_usrmsg->start, print_usrmsg->count, print_usrmsg->payload->RefCnt );
                  }
        }
#endif

   if ((bufp = m_MMGR_DATA_AT_START (usrbuf, (unsigned int)total_len , pBigBuff)) == NULL)
   {
        bytes_sent = NCSSOCK_ERROR;
   }

   m_NCS_MEMSET(&send_addr,0,sizeof(send_addr));
   send_addr.sll_family = AF_PACKET;
   send_addr.sll_protocol = htons(se->protocol);
   send_addr.sll_ifindex = if_index;

   total_bytes = sendto(se->client_socket, bufp, total_len, 
                        MSG_DONTWAIT,(struct sockaddr*)&send_addr,sizeof(send_addr));
#if 0
   total_bytes = sendto(se->client_socket, bufp, total_len - 17, 
                        0,(struct sockaddr*)&send_addr,sizeof(send_addr));
#endif
   if(total_bytes < 1)
   {
      perror("sendto()");
      return NCSCC_RC_FAILURE;
   }

   if (total_len >= L2STACK_BUFFER_SIZE)
   {
      m_NCS_MEM_FREE(pBigBuff, NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_L2SOCKET, 
          NCS_DL_MEM_SUB_ID_CHAR_BUFF);
   }

   m_MMGR_FREE_BUFR_LIST (usrbuf);

   return NCSCC_RC_SUCCESS;
      
}


uns32 
ncs_dl_bind_l2_layer (struct ncs_dl_request_info_tag *dlr)
{
 
   /* IR00060578 */     
   if (ncs_is_root()== FALSE) /* IR00059585 */
   {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   /* Create Socket layer */
   if ((*(NCSCONTEXT*)(dlr->info.ctrl.bind.o_dl_client_handle.data) =
           l2socket_layer_create (SYS_L2SOCK_DEF_SEL_TIMEOUT)) == NULL)
         return NCSCC_RC_FAILURE;

   dlr->info.ctrl.bind.o_dl_client_handle.len = sizeof (void*);

   if(gl_dl_res_inited == FALSE)
   {
      m_NCS_LOCK_INIT (&gl_dl_res_lock);
      gl_dl_res_inited = TRUE;
   }
   
   gl_dl_res_usr_cnt++;

   return NCSCC_RC_SUCCESS;
}


uns32 
ncs_dl_unbind_l2_layer (struct ncs_dl_request_info_tag *dlr)
{
   uns32 err;
   /* LOG_ISIS */

   err = l2socket_layer_destroy (*(NCSCONTEXT*)(dlr->info.ctrl.unbind.i_dl_client_handle.data));

   gl_dl_res_usr_cnt--;
   if((gl_dl_res_inited == TRUE) && (gl_dl_res_usr_cnt == 0)) 
   {
      m_NCS_LOCK_DESTROY (&gl_dl_res_lock);
      gl_dl_res_inited = FALSE;
   }
   return NCSCC_RC_SUCCESS;
}


uns32 
ncs_dl_open_l2_layer (struct ncs_dl_request_info_tag *dlr)
{
  NCS_L2SOCKET_ENTRY *se;
  uns8 dl_pool_id = 0;
  NCS_L2FILTER_ENTRY *fe;
  NCS_L2SOCKET_CONTEXT * socket_context;
  uns16 protocol = dlr->info.ctrl.open.i_dl_protocol;
  uns8 * addr    = dlr->info.ctrl.open.i_remote_addr.data.eth;
  uns32 if_index = dlr->info.ctrl.open.i_if_index;     /* IR00058904 */
  char* if_name = dlr->info.ctrl.open.i_if_name;       /* IR00058904 */
  NCS_L2_ADDR zero_addr; 
  NCS_BOOL    remote_addr_is_zero; 

  socket_context = (NCS_L2SOCKET_CONTEXT *)(*(NCSCONTEXT*)(dlr->info.ctrl.open.i_dl_client_handle.data));

  se = l2socket_entry_exists(socket_context, protocol,if_index, if_name); /* IR00058904 */
  
  /* IR58824: 
     -  For a shared socket, an open with zero-addr not allowed.
     -  For an unshared socket, an open with zero-addr is only allowed
        and that too only once.
  */
   /* Check if remote addr is zero */
  memset(&zero_addr, 0, sizeof(zero_addr));
  if (memcmp(&zero_addr, &dlr->info.ctrl.open.i_remote_addr, sizeof(zero_addr)) == 0)
     remote_addr_is_zero = TRUE;
  else
     remote_addr_is_zero = FALSE;
  
  if (se != NULL)
  {
     if_index = se->if_index; /* Since, it may be zero, in user ifnames */

     if ((se->shared == FALSE) ||(remote_addr_is_zero))
     {
         /* Either the socket is not shared or this is an
            second attempt open with a zero-addr. */
         return NCSCC_RC_FAILURE; 
     }

     /* This is a shared socket. Check that an open with this
        same address has not been done before. */
     fe = l2filter_entry_exists(socket_context, protocol, addr,if_index);
     if (fe != NULL)
          return NCSCC_RC_FAILURE;
  }
  else
  {
      if ((se = l2socket_entry_create (dlr)) == NULL)
          return NCSCC_RC_FAILURE;


      if (remote_addr_is_zero)
         se->shared = FALSE;      /* IR58824:  */
      else
         se->shared = TRUE;      /* IR58824:  */

      /* LOG_ISIS */

      if (l2socket_entry_dispatch (se, NCS_SOCKET_EVENT_OPEN, dlr) != NCSCC_RC_SUCCESS)
      {
          dlr->info.ctrl.open.o_dl_handle = (NCSCONTEXT)NULL;

          l2socket_entry_destroy (se); 
          return NCSCC_RC_FAILURE;
      }
      if_index = se->if_index; /* If the client provided an ifname, then now if_index would be updated */
  }

  fe = l2filter_entry_create(socket_context, se, protocol, addr,
                             dlr->info.ctrl.open.i_user_connection_handle, if_index);
  if (fe == NULL)
  {
      if (se->refcount == 0)
          l2socket_entry_destroy(se);

      return NCSCC_RC_FAILURE;
  }

  /* Give the user an HM handle instead of a socket_entry pointer */

  dlr->info.ctrl.open.o_dl_handle = NCS_UNS32_TO_PTR_CAST(ncshm_create_hdl(dl_pool_id,
                                                               NCS_SERVICE_ID_L2SOCKET,
                                                                fe));
  fe->hm_hdl = NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.open.o_dl_handle);

  return NCSCC_RC_SUCCESS;
}


uns32 
ncs_dl_close_l2_layer (struct ncs_dl_request_info_tag *dlr)
{
    NCS_L2FILTER_ENTRY *fe;
   NCS_L2SOCKET_ENTRY *se;

   fe = (NCS_L2FILTER_ENTRY*)NCS_UNS32_TO_PTR_CAST(ncshm_take_hdl(NCS_SERVICE_ID_L2SOCKET,NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.close.i_dl_handle)));

      if(fe == NULL)
        return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); 

      se = fe->se;
      ncshm_give_hdl(NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.close.i_dl_handle));

      ncshm_destroy_hdl(NCS_SERVICE_ID_L2SOCKET,NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.close.i_dl_handle));
      l2filter_entry_destroy(fe);

      if (se->refcount > 0)
      {
          /* se->user_connection_handle needs to be updated in se
             if any logical socket is destroyed */
          l2filter_entry_update_se(se);
          return NCSCC_RC_SUCCESS;
      }

      /* LOG_ISIS */
      return l2socket_entry_dispatch (se, NCS_SOCKET_EVENT_CLOSE, dlr);
}


uns32 
ncs_dl_send_data_to_l2_layer (struct ncs_dl_request_info_tag *dlr)
{

   NCS_L2FILTER_ENTRY *fe;
   uns32             rc;

   fe = (NCS_L2FILTER_ENTRY*)NCS_UNS32_TO_PTR_CAST(ncshm_take_hdl(NCS_SERVICE_ID_L2SOCKET,NCS_PTR_TO_UNS32_CAST(dlr->info.data.send_data.i_dl_handle)));

   if(fe == NULL)
      return NCSCC_RC_FAILURE; 

    /* LOG_ISIS */

    rc = l2socket_entry_dispatch (fe->se, NCS_SOCKET_EVENT_SEND_DATA, dlr);

    ncshm_give_hdl(NCS_PTR_TO_UNS32_CAST(dlr->info.data.send_data.i_dl_handle));

    return rc;
}

uns32
ncs_dl_mcast_join (struct ncs_dl_request_info_tag *dlr)
{
   NCS_L2FILTER_ENTRY *fe;
   uns32             rc;

   /* Convert the HM handle to the socket_entry ptr and go on...*/
   fe = (NCS_L2FILTER_ENTRY*)NCS_UNS32_TO_PTR_CAST(ncshm_take_hdl(NCS_SERVICE_ID_L2SOCKET,NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.multicastjoin.i_dl_handle)));

   if(fe == NULL)
     return NCSCC_RC_FAILURE; 

   /* LOG_ISIS */
   rc = l2socket_entry_dispatch (fe->se, NCS_SOCKET_EVENT_MULTICAST_JOIN, dlr);

   ncshm_give_hdl(NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.multicastjoin.i_dl_handle));

   return rc;

}

uns32
ncs_dl_mcast_leave (struct ncs_dl_request_info_tag *dlr)
{
   NCS_L2FILTER_ENTRY *fe;
   uns32             rc;

   fe = (NCS_L2FILTER_ENTRY*)ncshm_take_hdl(NCS_SERVICE_ID_L2SOCKET,
                                          NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.multicastleave.i_dl_handle));
   if(fe == NULL)
      return NCSCC_RC_FAILURE; 

   /* LOG_ISIS */

   rc = l2socket_entry_dispatch (fe->se, NCS_SOCKET_EVENT_MULTICAST_LEAVE, dlr);

   ncshm_give_hdl(NCS_PTR_TO_UNS32_CAST(dlr->info.ctrl.multicastleave.i_dl_handle));

   return rc;

}


static uns32
l2socket_entry_insert (NCS_L2SOCKET_ENTRY *se)
{

   NCS_L2SOCKET_LIST    *socket_list;
   NCS_L2SOCKET_CONTEXT *socket_context;

   if ((socket_context = se->socket_context) == NULL)
      return NCSCC_RC_FAILURE;

   socket_list = &socket_context->ReceiveQueue;

   if(m_NCS_LOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
     /* LOG_ISIS */
     return NCSCC_RC_FAILURE;
   }

  /* put the socket into the normal list */
  if (socket_list->first == NULL)
  {
     se->next           = NULL;
     se->prev           = NULL;
     socket_list->first = se;
     socket_list->last  = se;
  }
  else
  {
     /* always add at the end of the list */
     se->next                = NULL;
     se->prev                = socket_list->last;
     socket_list->last->next = se;
     socket_list->last       = se;
  }

  l2socket_entry_set_fd (se, TRUE, FALSE);
  socket_list->num_in_list++;

  m_NCS_SEL_OBJ_IND(socket_context->fast_open_sel_obj); /*fix IR00059000 */

  if(m_NCS_UNLOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
  {
#if 0
    m_SYSF_IP_LOG_INFO("socket_entry_insert:failed to unlock sl", socket_list);
#endif
    return NCSCC_RC_SUCCESS;
  }

   /* LOG_ISIS */

  return NCSCC_RC_SUCCESS;
}



static uns32
ncsl2sock_event_raw_open (NCS_L2SOCKET_ENTRY               *se,
                         NCSCONTEXT                      arg)
{
   struct ncs_dl_request_info_tag* dlr = (struct ncs_dl_request_info_tag*)arg ;
   NCS_DL_ERROR       dl_error;
   char alll1IS[6], alll2IS[6], allIS[6];
   struct ifreq  ifr;
   struct sockaddr_ll serverAddr;

   /* alll1IS */
   alll1IS[0] = 0x01;
   alll1IS[1] = 0x80;
   alll1IS[2] = 0xC2;
   alll1IS[3] = 0x00;
   alll1IS[4] = 0x00;
   alll1IS[5] = 0x14;

   /* alll2IS */
   alll2IS[0] = 0x01;
   alll2IS[1] = 0x80;
   alll2IS[2] = 0xC2;
   alll2IS[3] = 0x00;
   alll2IS[4] = 0x00;
   alll2IS[5] = 0x15;

   /* allIS */
   allIS[0] = 0x09;
   allIS[1] = 0x00;
   allIS[2] = 0x2B;
   allIS[3] = 0x00;
   allIS[4] = 0x00;
   allIS[5] = 0x05;

   se->client_socket = m_NCSSOCK_SOCKET (PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

   if (se->client_socket == NCSSOCK_INVALID)
   {
      dl_error = m_NCSSOCK_ERROR;
      /* LOG_ISIS */
      se->state = NCS_SOCKET_STATE_CLOSING;

      return NCSCC_RC_FAILURE;
   }

   /* Fix for IR10318, Set both ifname and ifindex in socket entry*/
   /* Get the if name */
   if (dlr->info.ctrl.open.i_if_index != 0)
   {
       se->if_index = dlr->info.ctrl.open.i_if_index;
       m_NCS_OS_MEMSET(&ifr, '\0', sizeof ifr);
       ifr.ifr_ifindex = se->if_index;

       if (m_NCS_TS_SOCK_IOCTL(se->client_socket,
          SIOCGIFNAME,
          &ifr)==NCS_TS_SOCK_ERROR)
       {
           dl_error = m_NCSSOCK_ERROR;
           /* LOG_ISIS */
           se->state = NCS_SOCKET_STATE_CLOSING;
           m_NCSSOCK_CLOSE(se->client_socket);

          return NCSCC_RC_FAILURE;
       }
       m_NCS_MEMCPY(se->if_name, ifr.ifr_name, sizeof(se->if_name));
   }
   else /* Get if index */
   {
       /* get interface index number */
       strncpy(se->if_name, dlr->info.ctrl.open.i_if_name, sizeof(se->if_name)); /* IR00058904 */
       m_NCS_OS_MEMSET(&ifr, '\0', sizeof ifr);
       m_NCS_MEMCPY (ifr.ifr_name, se->if_name, sizeof(se->if_name));

       if (m_NCS_TS_SOCK_IOCTL(se->client_socket,
         SIOCGIFINDEX,
         &ifr)==NCS_TS_SOCK_ERROR)
       {
          dl_error = m_NCSSOCK_ERROR;
         /* LOG_ISIS */
         se->state = NCS_SOCKET_STATE_CLOSING;
         m_NCSSOCK_CLOSE(se->client_socket);

         return NCSCC_RC_FAILURE;
       }
       se->if_index = ifr.ifr_ifindex;
   }

       /* IR00011857 */
       m_NCS_OS_MEMSET(&ifr,'\0',sizeof(ifr));

       m_NCS_MEMCPY(ifr.ifr_name,se->if_name,sizeof(se->if_name));

       /* Get the IFFlags of the corresponding interface */
       if (m_NCSSOCK_IOCTL(se->client_socket, SIOCGIFFLAGS, &ifr) < 0)
       {
            dl_error = m_NCSSOCK_ERROR;
           /* LOG_ISIS */
           se->state = NCS_SOCKET_STATE_CLOSING;
           m_NCSSOCK_CLOSE(se->client_socket);

           return NCSCC_RC_FAILURE;

       }

      if (!(ifr.ifr_flags &(IFF_UP|IFF_RUNNING)))
      {
         /* When  Interface is DOWN just return NCSCC_RC_FAILURE*/
          dl_error = m_NCSSOCK_ERROR;
         /* LOG_ISIS */
         se->state = NCS_SOCKET_STATE_CLOSING;
         m_NCSSOCK_CLOSE(se->client_socket);

         return NCSCC_RC_FAILURE;
      }
      else
      {
         /* When  Interface is UP */
      }

   
   /* Bind the socket to device, so that we receive pkts from only this  */
   if (m_NCS_TS_SOCK_SETSOCKOPT(se->client_socket,
      SOL_SOCKET,
      SO_BINDTODEVICE,
      se->if_name,
      IFNAMSIZ))
   {
            dl_error = m_NCSSOCK_ERROR;
           /* LOG_ISIS */
           se->state = NCS_SOCKET_STATE_CLOSING;
           m_NCSSOCK_CLOSE(se->client_socket);
           return NCSCC_RC_FAILURE;

   }

   /* Bind the socket to particular interface. It is essential
     *  since if you don't bind, you'll be receiving all the
     *  packets that are destined to other interfaces in the
     *  system.
     */
   m_NCS_OS_MEMSET(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sll_family   = AF_PACKET;
    serverAddr.sll_protocol = htons(se->protocol);
    serverAddr.sll_ifindex  = se->if_index;

    if (m_NCSSOCK_BIND (se->client_socket,
          (struct sockaddr *) &serverAddr,sizeof(serverAddr))==NCSSOCK_ERROR)
   {
      /* Log the reason */
      dl_error = m_NCSSOCK_ERROR;
      /* LOG_ISIS */
      se->state = NCS_SOCKET_STATE_CLOSING;
      m_NCSSOCK_CLOSE(se->client_socket);

      return NCSCC_RC_FAILURE;
 
   }


#if 0
   memcpy (ifr.ifr_hwaddr.sa_data, alll1IS, 6);

   /* Add ALLL1IS */
   if (m_NCSSOCK_IOCTL (se->client_socket,SIOCADDMULTI, &ifr) == NCSSOCK_ERROR)
   {
      dl_error = m_NCSSOCK_ERROR;
      /* LOG_ISIS */
      se->state = NCS_SOCKET_STATE_CLOSING;
      m_NCSSOCK_CLOSE(se->client_socket);

      return NCSCC_RC_FAILURE;
   }


   memcpy (ifr.ifr_hwaddr.sa_data, alll2IS, 6);

   /* Add ALLL2IS */
   if (m_NCSSOCK_IOCTL (se->client_socket,SIOCADDMULTI, &ifr) == NCSSOCK_ERROR)
   {
      dl_error = m_NCSSOCK_ERROR;
      /* LOG_ISIS */
      se->state = NCS_SOCKET_STATE_CLOSING;
      m_NCSSOCK_CLOSE(se->client_socket);

      return NCSCC_RC_FAILURE;
   }

   memcpy (ifr.ifr_hwaddr.sa_data, allIS, 6);

   /* Add ALLL1IS */
   if (m_NCSSOCK_IOCTL (se->client_socket,SIOCADDMULTI, &ifr) == NCSSOCK_ERROR)
   {
      dl_error = m_NCSSOCK_ERROR;
      /* LOG_ISIS */
      se->state = NCS_SOCKET_STATE_CLOSING;
      m_NCSSOCK_CLOSE(se->client_socket);

      return NCSCC_RC_FAILURE;
   }
#endif
   se->state = NCS_SOCKET_STATE_ACTIVE;

   if (l2socket_entry_insert (se) == NCSCC_RC_FAILURE)
   {
      /* LOG_ISIS */
      m_NCSSOCK_CLOSE(se->client_socket);
      /* socket_entry_destroy (se); */
      se->state = NCS_SOCKET_STATE_CLOSING;
      return NCSCC_RC_FAILURE;
   }


   dlr->info.ctrl.open.o_dl_handle = (NCSCONTEXT) se;


   /* LOG_ISIS */

   return NCSCC_RC_SUCCESS;

}



static uns32 ncsl2sock_event_gen_close (NCS_L2SOCKET_ENTRY               *se,
                                       NCSCONTEXT                      arg)
{
   NCS_L2SOCKET_CONTEXT   *socket_context;
   NCS_L2SOCKET_LIST      *socket_list;

   if ((socket_context = se->socket_context) == NULL)
      return NCSCC_RC_FAILURE;

   socket_list = &socket_context->ReceiveQueue;

   if(m_NCS_LOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
     /* LOG_ISIS */
     return NCSCC_RC_FAILURE;
   }
   if(m_NCS_LOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
     /* LOG_ISIS */
     if(m_NCS_UNLOCK(&se->lock,NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
     {
       /* LOG_ISIS */
     }
     return NCSCC_RC_FAILURE;
   }

   se->state = NCS_SOCKET_STATE_CLOSING;  /* mark for deletion */

   l2socket_entry_set_fd (se, FALSE, FALSE);  /* shut off socket i/o */


   m_NCS_TASK_SLEEP(10);


   l2socket_entry_remove (se->socket_context, se->client_socket);

   se->dl_data_indication = NULL;
   se->dl_ctrl_indication = NULL;

#if 0
   m_SYSF_IP_LOG_INFO("ncssock_event_gen_close - nonsharing entry", se);
#endif

   /* the NCSDL task returned from select(), but it's blocked on socket_list->lock */
   /* Turn off read and write fd descriptor for read/write fdsets */
   /* Hmmm...Maybe do this as part of dispatch */
   m_NCS_TASK_SLEEP(10);
   l2socket_entry_set_fd (se, FALSE, FALSE);  /* shutting off the i/o */

   if(m_NCS_UNLOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
#if 0
     m_SYSF_IP_LOG_ERROR("ncssock_event_gen_close:failed to unlock se", se);
#endif
   }

   if(m_NCS_UNLOCK (&socket_list->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
   {
#if 0
     m_SYSF_IP_LOG_ERROR("ncssock_event_gen_close:failed to unlock sl", socket_list);
#endif
   }


   return NCSCC_RC_SUCCESS;
}


static uns32
ReadRawPacket(NCS_L2SOCKET_ENTRY    *se,
              struct sockaddr_in *p_saddr,
              USRBUF            **p_dlbuf,
              uns32              *dl_error)

{
   unsigned long            PktLen;
   unsigned long            buf_len;
   unsigned int             len;
   USRBUF                  *bufn;
   char                    *buf;
   int                      saddrlen = sizeof (struct sockaddr_in);
   SOCKET                   rcv_socket = se->client_socket;
   char             *pBigBuff;
   unsigned int      PktLenRemaining;
   char             *pThisCopy;
   struct ifreq request;
   

   *dl_error = NCSSOCK_ERROR_TYPES_NO_ERROR;

   /** We have a UDP or Raw socket that select indicated was readable.
    **/

   if (m_NCSSOCK_IOCTL (rcv_socket, m_NCSSOCK_FIONREAD, &PktLen) == NCSSOCK_ERROR)
   {
      /* LOG_ISIS */
      *p_dlbuf = NULL;
      return 0;
   }


   if (PktLen == 0)
   {
      /* LOG_ISIS */
      *p_dlbuf = NULL;

        /* IR00011857 */
      m_NCS_OS_MEMSET(&request,0,sizeof(request));
       
       /* Get the IFFlags of the corresponding interface */
      m_NCS_MEMCPY(request.ifr_name,se->if_name,sizeof(se->if_name));    

      if (m_NCSSOCK_IOCTL(rcv_socket, SIOCGIFFLAGS, &request) < 0)
      {
          return 0;
      }
    
      if ((request.ifr_flags & (IFF_UP|IFF_RUNNING)))
      {
         /* When  Interface is DOWN just return NCSSOCK_ERROR */ 
      }
      else
      {
         /* When  Interface is UP just return 0*/
         *dl_error = NCSSOCK_ERROR_TYPES_CONN_DOWN;
         return m_LEAP_DBG_SINK(NCSSOCK_ERROR); /* IR00011857 */
      }

      return 0;
   }

   if ((*p_dlbuf = m_MMGR_ALLOC_BUFR(PktLen)) == NULL)
   {
      /* Buffer Alloc failed */
      return 0;
   }

   bufn    = *p_dlbuf;
   buf_len = PAYLOAD_BUF_SIZE;

   if (PktLen <= buf_len)
   {
      /* The entire packet can fit in one USRBUF.  We'll use the system
       * call recvfrom() to deliver the data to the USRBUF.
       */

      buf = m_MMGR_RESERVE_AT_END(&bufn, (unsigned int)PktLen, char*);

      len = m_NCSSOCK_RECVFROM (rcv_socket, buf, PktLen, 0, (struct sockaddr*)p_saddr, &saddrlen);
   }
   else
   {

      /* The system DOES NOT support direct read from socket into a
       * linked list of USRBUFs.  This is too bad, as we need to
       * do an extra malloc and an extra data copy.
       */

      if ((pBigBuff = (char *)m_NCS_MEM_ALLOC((uns32)PktLen, NCS_MEM_REGION_TRANSIENT,
            NCS_SERVICE_ID_L2SOCKET, NCS_DL_MEM_SUB_ID_CHAR_BUFF)) == NULL)
      {
         m_MMGR_FREE_BUFR_LIST(*p_dlbuf);
         *p_dlbuf = NULL;
         /* buf alloc failed */
         return 0;
      }

      len = m_NCSSOCK_RECVFROM (rcv_socket, pBigBuff, PktLen, 0, (struct sockaddr*)p_saddr, &saddrlen);

      PktLenRemaining = PktLen;
      pThisCopy = pBigBuff;

      do
      {
         if (PktLenRemaining < buf_len)  /* Less than a single USRBUF? */
            buf_len = PktLenRemaining;

         buf = m_MMGR_RESERVE_AT_END(&bufn, (unsigned int)buf_len, char*);

         if (buf == NULL)
         {
            m_MMGR_FREE_BUFR_LIST(*p_dlbuf);
            *p_dlbuf = NULL;
            m_NCS_MEM_FREE(pBigBuff, NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_L2SOCKET,
                NCS_DL_MEM_SUB_ID_CHAR_BUFF);
            /* usrbuf failure */
            return 0;
         }

         /* Copy to Usrbuf. */
         m_NCS_MEMCPY(buf, pThisCopy, (size_t)buf_len);

         PktLenRemaining -= buf_len;
         pThisCopy += buf_len;
      } while (PktLenRemaining > 0);

      /* USRBUFs all set up.  Free BigBuff. */
      m_NCS_MEM_FREE(pBigBuff, NCS_MEM_REGION_TRANSIENT, NCS_SERVICE_ID_L2SOCKET,
          NCS_DL_MEM_SUB_ID_CHAR_BUFF);

   }

   /* Once we get here, the USRBUF contains the packet, unless an error
    * occurred reading from the socket.
    */
   switch (len)
   {
   case 0:
      m_MMGR_FREE_BUFR_LIST(*p_dlbuf);
      *p_dlbuf = NULL;
      break;

   case NCSSOCK_ERROR:
      *dl_error = m_NCSSOCK_ERROR;
      break;  /* handled in caller's stack. */

   default:
      /** Any other value is a length received. **/

      if (PktLen > len)
      {
         m_MMGR_REMOVE_FROM_END(*p_dlbuf, (unsigned int)(PktLen-len));
      }
      break;
   }

   /* LOG_ISIS */

   return len;

}


static uns32
ncsl2sock_event_raw_indication (NCS_L2SOCKET_ENTRY               *se_in,
                               NCSCONTEXT                      arg)
{
   SYSF_DL_DISPATCH_INFO*   ddi = (SYSF_DL_DISPATCH_INFO*)arg ;
   NCS_L2SOCKET_ENTRY*       se;
   NCS_L2FILTER_ENTRY*       fe = NULL;
   NCS_L2FILTER_KEY          key;
   NCS_L2SOCKET_CONTEXT *    sc;
   NCS_DL_INDICATION_INFO  dli;
   unsigned int           len;
   USRBUF                *dlbuf      = NULL;
   uns8                   eth_hdr[17];
   uns32                  header_len;
   uns32                  data_len;
   uns8                  *p8;
   uns8                   dsap, ssap, ctrl;
   struct sockaddr_in     saddr;
   NCS_DL_ERROR            dl_error;
   NCS_DL_INDICATION       loc_dl_ind = NULL;

   USE(ddi);

#if (NCS_DL_DEFS_TRACE == 1)
   m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x)\n", (uns32)se_in);
#endif

   /** We have a RAW socket that select indicated was readable.
    **/
   /* The below m_NCS_OS_MEMSET call is a temporary fix
      to allow MDS to function on Layer 2 on a single
      Linux Box. (Phani's temporay MDS fix)
   */
   m_NCS_OS_MEMSET(&dli, 0, sizeof(dli));
   if(se_in->state == NCS_SOCKET_STATE_CLOSING)
   {
#if (NCS_DL_DEFS_LOG == 1)
      m_NCS_DL_DEFS_LOG("ncsl2sock_event_raw_indication(0x%08x):Success(sock-closing)", (uns32)se_in);
#endif
      return NCSCC_RC_SUCCESS;
   }

   se = se_in;

   len = ReadRawPacket(se_in, &saddr, &dlbuf, &dl_error);

   /* Once we get here, the USRBUF contains the packet, unless an error
    * occurred reading from the socket.
    */
   switch (len)
   {
   /* A length of 0 indicates that there is a potential fatal problem with the socket
   */
   case 0:
   case NCSSOCK_ERROR:
#if (NCS_DL_DEFS_LOG == 1)
      m_NCS_DL_DEFS_LOG("ncsl2sock_event_raw_indication(0x%08x):Failure(len=NCSSOCK_ERROR)", (uns32)se_in);
#endif
      l2socket_entry_set_fd (se, FALSE, FALSE);
      se->state = NCS_SOCKET_STATE_DISCONNECT;
      if(dlbuf != NULL)
        m_MMGR_FREE_BUFR_LIST(dlbuf);
      /* dl_error = m_NCSSOCK_ERROR;  This is set within ReadRawPacket */


      dli.i_indication                  = NCS_DL_CTRL_INDICATION_ERROR;
      dli.i_dl_handle                   = NCS_UNS32_TO_PTR_CAST(se->hm_hdl);
      dli.i_user_handle                 = se->user_handle;
      dli.i_user_connection_handle      = se->user_connection_handle;
      dli.info.ctrl.error.i_error       = dl_error;
      dli.info.ctrl.error.i_local_addr  = se->local_addr;
      dli.info.ctrl.error.i_remote_addr = se->remote_addr;
      dli.info.ctrl.error.i_if_index    = se->if_index;
      loc_dl_ind                        = se->dl_ctrl_indication;

      if(loc_dl_ind != NULL)
      {
        NCS_BOOL is_unlocked = TRUE;
        if (se == se_in)
        {
          if(m_NCS_UNLOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
          {
            is_unlocked = FALSE;
          }
          else
          {
            is_unlocked = TRUE;
          }
        }

        /* Indicate the Error to the client */
        (loc_dl_ind)(&dli);

        if ((se == se_in) && (is_unlocked == TRUE))
        {
          if(m_NCS_LOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
          {
              /* Failed to Lock again */
          }
        }
      }

      return NCSCC_RC_FAILURE;

   default:
#if (NCS_DL_DEFS_TRACE == 1)
      m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:LEN = %d\n", (uns32)se_in, len);
      {
         uns8 *buf = (char *)malloc(len), word_buf[4];
         uns8 *msg;
         msg = (uns8*) m_MMGR_DATA_AT_START(dlbuf, len, buf);
         int word_cursor, byte_cursor, num;
         for (word_cursor = 0; word_cursor < 1 + (len-1)/4; word_cursor+=4)
         {
            word_buf[0] = word_buf[1] = word_buf[2] = word_buf[3] = 0;
            for (byte_cursor = word_cursor*4, num = 0;
            (byte_cursor < (word_cursor*4+4)) && (byte_cursor < len);
            byte_cursor++, num++)
            {
               word_buf[num] = msg[byte_cursor];
            }
            m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DUMP@%d:0x%02x:%02x:%02x:%02x\n",
               (uns32)se_in, word_cursor, word_buf[0], word_buf[1], word_buf[2], word_buf[3]);
         }
         free(buf);
      }
#endif
       /* Extract the Ethernet header Info */
      p8 = (uns8*) m_MMGR_DATA_AT_START(dlbuf, 17, eth_hdr);

      m_NCS_OS_MEMCPY (dli.info.data.recv_data.i_local_addr.data.eth ,p8, 6);
      p8 += 6;

      m_NCS_OS_MEMCPY (dli.info.data.recv_data.i_remote_addr.data.eth,p8,6);
      p8 += 6;

      data_len = m_NCS_OS_NTOHS_P(p8);
      p8 += 2;

#if (NCS_DL_DEFS_TRACE == 1)
      m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:HDR:"
         "LOCAL_MAC=0x%02x:%02x:%02x:%02x:%02x:%02x:REMOTE_MAC=0x%02x:%02x:%02x:%02x:%02x:%02x:LEN=0x%04x\n",
         (uns32)se_in,
         ((uns8*)&dli.info.data.recv_data.i_local_addr.data.eth)[0],
         ((uns8*)&dli.info.data.recv_data.i_local_addr.data.eth)[1],
         ((uns8*)&dli.info.data.recv_data.i_local_addr.data.eth)[2],
         ((uns8*)&dli.info.data.recv_data.i_local_addr.data.eth)[3],
         ((uns8*)&dli.info.data.recv_data.i_local_addr.data.eth)[4],
         ((uns8*)&dli.info.data.recv_data.i_local_addr.data.eth)[5],
         ((uns8*)&dli.info.data.recv_data.i_remote_addr.data.eth)[0],
         ((uns8*)&dli.info.data.recv_data.i_remote_addr.data.eth)[1],
         ((uns8*)&dli.info.data.recv_data.i_remote_addr.data.eth)[2],
         ((uns8*)&dli.info.data.recv_data.i_remote_addr.data.eth)[3],
         ((uns8*)&dli.info.data.recv_data.i_remote_addr.data.eth)[4],
         ((uns8*)&dli.info.data.recv_data.i_remote_addr.data.eth)[5],
         data_len);
#endif

      /* GET the DSAP, SSAP, Ctrl --- IEEE 802.2 Header -- Logical Link Control */
      if (data_len <= 0x5ee)
      {

      dsap = *p8++;
      ssap = *p8++;
      ctrl = *p8++;

#if (NCS_DL_DEFS_TRACE == 1)
      m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DESC:DSAP=0x%02x,SSAP=0x%02x,CTRL=0x%02x\n",
               (uns32)se_in, dsap,ssap,ctrl);
#endif

      if (dsap != 0xFE )
      {
         /* NOT AN ISIS Packet */
#if (NCS_DL_DEFS_TRACE == 1)
         m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DROP(DSAP not for ISIS)\n", (uns32)se_in);
#endif
         m_MMGR_FREE_BUFR_LIST(dlbuf);
         return NCSCC_RC_SUCCESS;
      }

      if (ssap != 0xFE )
      {
#if (NCS_DL_DEFS_TRACE == 1)
         m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DROP(SSAP not for ISIS)\n", (uns32)se_in);
#endif
         /* NOT AN ISIS Packet */
         m_MMGR_FREE_BUFR_LIST(dlbuf);
         return NCSCC_RC_SUCCESS;
      }

      if (ctrl != 0x03)
      {
#if (NCS_DL_DEFS_TRACE == 1)
         m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DROP(CTRL not for ISIS)\n", (uns32)se_in);
#endif
         /* NOT AN ISIS Packet */
         m_MMGR_FREE_BUFR_LIST(dlbuf);
         return NCSCC_RC_SUCCESS;
      }

      header_len = 17;
#if (NCS_DL_DEFS_TRACE == 1)
      m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:ACCEPT for ISIS\n", (uns32)se_in);
#endif
    }
      else
    {
      header_len = 14;
    }
#ifdef DUMP_USRBUF_IN_MDS_LOG /* PM : 31/Jan/2005 */
        {
              uns32  ubuf_num;
              USRBUF *print_usrmsg;

              /* Explicit log trace-level check to avoid expensive logging code */
                  /* TRACE logging is enabled. Some more expensive trace logging */

                  for (ubuf_num = 0, print_usrmsg = dlbuf;
                  print_usrmsg;
                  ubuf_num++, print_usrmsg = print_usrmsg->link)
                  {
                      m_LOG_MDS_TRACE("DL_DEFS.C:READ_FRAG:UBUF_NUM=%d:START=%d:COUNT=%d:UB_REF_COUNT=%d\n",
                     ubuf_num, print_usrmsg->start, print_usrmsg->count, print_usrmsg->payload->RefCnt );
                  }
        }
#endif

       /** Strip off the Ethernet Header...
       **/
      m_MMGR_REMOVE_FROM_START(&dlbuf, header_len);

      if (se->shared == FALSE) /* See IR58824 */
      {
          /* There is no need to search for an L2-address filter,
             because this socket is not shared */
          dli.i_dl_handle                       = NCS_UNS32_TO_PTR_CAST( se->hm_hdl);
          dli.i_user_connection_handle          = se->user_connection_handle;
      }
      else
      {
          /* Since this is a shared socket we need to find a filter corresponding
             to remote-l2-addr.
          */
          key.protocol                          = se->protocol;
          key.if_index              = se->if_index; /*IR00058904 */



          m_NCS_OS_MEMCPY(key.addr, dli.info.data.recv_data.i_remote_addr.data.eth, 6);
          sc = se->socket_context;

          m_NCS_LOCK(&sc->lock, NCS_LOCK_READ);
          fe = (NCS_L2FILTER_ENTRY *)ncs_patricia_tree_get(&sc->socket_tree, (const uns8*)&key);
          m_NCS_UNLOCK(&sc->lock, NCS_LOCK_READ);


          if (fe == NULL) /* See IR 58812 */
          {
              if(dlbuf != NULL)
                  m_MMGR_FREE_BUFR_LIST(dlbuf);
              return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
          }

          dli.i_dl_handle                       = NCS_UNS32_TO_PTR_CAST(fe->hm_hdl);
          dli.i_user_connection_handle          = fe->user_connection_handle;
      }

      dli.i_indication                      = NCS_DL_DATA_INDICATION_RECV_DATA;
      dli.info.data.recv_data.i_usrbuf      = dlbuf;
      loc_dl_ind                            = se->dl_data_indication;
      dli.i_user_handle                     = se->user_handle;
      dli.info.data.recv_data.i_if_index    = se->if_index;


      if(loc_dl_ind != NULL)
      {
        NCS_BOOL is_unlocked = TRUE;
        if(se == se_in)
        {
          if(m_NCS_UNLOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
          {
            /* Failed to unlock */
            is_unlocked = FALSE;
          }
          else
          {
            is_unlocked = TRUE;
          }
        }
        if((loc_dl_ind)(&dli) != NCSCC_RC_SUCCESS)
        {
#if (NCS_DL_DEFS_TRACE == 1)
           m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DELIVERY FAILURE\n", (uns32)se_in);
#endif
          if(dlbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(dlbuf);
        }
        else
        {
#if (NCS_DL_DEFS_TRACE == 1)
           m_NCS_DL_DEFS_TRACE("ncsl2sock_event_raw_indication(0x%08x):MSG:DELIVERY SUCCESS\n", (uns32)se_in);
#endif
        }
        if ((se == se_in) && (is_unlocked == TRUE))
        {
          if(m_NCS_LOCK (&se->lock, NCS_LOCK_WRITE) == NCSCC_RC_FAILURE)
          {
            /* Failed to lock again */
          }
        }
      }
      else
      {
#if (NCS_DL_DEFS_LOG == 1)
        m_NCS_DL_DEFS_LOG("ncsl2sock_event_raw_indication(0x%08x):MSG:DELIVERY FAILURE\n", (uns32)se_in);
#endif
        if(dlbuf != NULL)
          m_MMGR_FREE_BUFR_LIST(dlbuf);
      }
      break;
   }

   return NCSCC_RC_SUCCESS;

}

static uns32
ncsl2sock_event_null (NCS_L2SOCKET_ENTRY *se,
           NCSCONTEXT       arg)
{
  USE (se);
  USE (arg);
  return NCSCC_RC_SUCCESS;
}

static uns32
ncsl2sock_event_failure (NCS_L2SOCKET_ENTRY *se,
           NCSCONTEXT       arg)
{
  USE (se);
  USE (arg);
  return NCSCC_RC_FAILURE;
}

uns32
ncs_dl_getl2_eth_addr(uns32 *if_index, char if_name[NCS_IF_NAMESIZE], uns8 mac_addr[6])
{
  SOCKET sock;
  struct ifreq ifr;
  
  sock = m_NCSSOCK_SOCKET(PF_PACKET, SOCK_RAW, htons(ETH_P_802_2));
  if (sock == NCSSOCK_INVALID)
   return NCSCC_RC_FAILURE;

  m_NCS_OS_MEMSET(&ifr, 0, sizeof(ifr));

  /* Fix for IR10315 and IR10318 */
    /*
    if anyone of if_index or if_name is given then mac_address is returned back, 
    also if suppose if_index is given and if_name is not null if_name will also
    be returned back and vice versa.
    */
  if ((if_index != NULL) && (*if_index != 0))
  {
      ifr.ifr_ifindex = *if_index;

      if (m_NCS_TS_SOCK_IOCTL(sock,
         SIOCGIFNAME,
         &ifr)==NCS_TS_SOCK_ERROR)
      {
         return NCSCC_RC_FAILURE;
      }
      if (if_name != NULL)
      {
         m_NCS_MEMCPY (if_name, ifr.ifr_name, sizeof(ifr.ifr_name));
      }
  }
  else
  {
      if (if_name == NULL)
      {
         return NCSCC_RC_FAILURE;
      }
      else
      {
         m_NCS_MEMCPY (ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
         if (m_NCS_TS_SOCK_IOCTL(sock,
           SIOCGIFINDEX,
           &ifr)==NCS_TS_SOCK_ERROR)
         {
           return NCSCC_RC_FAILURE;
         }
         if (if_index != NULL)
         {
            *if_index = ifr.ifr_ifindex;
         }
         m_NCS_MEMCPY (ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
      }
  }

  if (m_NCS_TS_SOCK_IOCTL(sock, SIOCGIFHWADDR, &ifr) == NCS_TS_SOCK_ERROR)
  {
   return NCSCC_RC_FAILURE;
  }

  /* Copy the Hardware address */
  m_NCS_OS_MEMCPY(mac_addr, ifr.ifr_hwaddr.sa_data, 6);
  
  return NCSCC_RC_SUCCESS;
}

static uns32
ncsl2sock_event_raw_mcast_join (NCS_L2SOCKET_ENTRY  *se,
                         NCSCONTEXT       arg)
{
   struct ncs_dl_request_info_tag* dlr = (struct ncs_dl_request_info_tag*)arg ;
   struct ifreq  ifr;
   
   m_NCS_OS_MEMSET(&ifr,0,sizeof(ifr));

   m_NCS_MEMCPY(ifr.ifr_name,se->if_name,sizeof(ifr.ifr_name));

   ifr.ifr_ifru.ifru_addr.sa_family= AF_UNSPEC;

   /* Copy Multicast Address */
   m_NCS_OS_MEMCPY(ifr.ifr_ifru.ifru_addr.sa_data,dlr->info.ctrl.multicastjoin.i_multicast_addr.data.eth, 6);

   if (m_NCSSOCK_IOCTL (se->client_socket,SIOCADDMULTI, &ifr) == NCSSOCK_ERROR)
   {
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE); 
   }

   return NCSCC_RC_SUCCESS;
}



static uns32
ncsl2sock_event_raw_mcast_leave (NCS_L2SOCKET_ENTRY  *se,
                         NCSCONTEXT       arg)
{
   struct ncs_dl_request_info_tag* dlr = (struct ncs_dl_request_info_tag*)arg ;
   struct ifreq  ifr;
   
   m_NCS_OS_MEMSET(&ifr,0,sizeof(ifr));

   m_NCS_MEMCPY(ifr.ifr_name,se->if_name,sizeof(ifr.ifr_name));

   ifr.ifr_ifru.ifru_addr.sa_family= AF_UNSPEC;

   /* Copy Multicast Address */
   m_NCS_OS_MEMCPY(ifr.ifr_ifru.ifru_addr.sa_data,dlr->info.ctrl.multicastleave.i_multicast_addr.data.eth, 6);

   if (m_NCSSOCK_IOCTL (se->client_socket,SIOCDELMULTI, &ifr) == NCSSOCK_ERROR)
   {
        return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;

}

const NCS_L2SOCKET_HANDLER gl_ncs_l2socket_raw_dispatch [NCS_SOCKET_EVENT_MAX+1][NCS_SOCKET_STATE_MAX+1] =
{
   {  /** NCS_SOCKET_EVENT_OPEN **/
      ncsl2sock_event_raw_open,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_OPEN_ESTABLISH **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_CLOSE **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_gen_close,
      ncsl2sock_event_gen_close,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_SET_ROUTER_ALERT **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_MULTICAST_JOIN **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      (NCS_L2SOCKET_HANDLER)ncsl2sock_event_raw_mcast_join,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_MULTICAST_LEAVE **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      (NCS_L2SOCKET_HANDLER)ncsl2sock_event_raw_mcast_leave,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_SET_TTL **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_SET_TOS **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_SEND **/
      ncsl2sock_event_failure,
      ncsl2sock_event_failure,
      ncsl2sock_event_failure,
      ncsl2sock_event_raw_send,
      ncsl2sock_event_failure,
      ncsl2sock_event_failure
   },
   {  /** NCS_SOCKET_EVENT_READ_INDICATION **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_raw_indication,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_WRITE_INDICATION **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   },
   {  /** NCS_SOCKET_EVENT_ERROR_INDICATION **/
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null,
      ncsl2sock_event_null
   }

};


