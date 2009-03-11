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



 
..............................................................................

  DESCRIPTION:

  This module contains procedures related to Universal MIB operations

..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncsmib_init                   - initial creator puts an NCSMIB_ARG into start state

  ncsmib_sync_request           - PUBLIC :model a MIB request as synchronous; set timer
  mib_sync_response            - PRIVATE:callback function to complete sync paradigm

  ncsmib_timed_request          - PUBLIC :associate timer with asynchronous MIB request
  mib_timed_store              - PRIVATE:store a timer block waiting for response
  mib_timed_find               - PRIVATE:find stored timer block
  mib_timed_remove             - PRIVATE:remove store timer block
  mib_timed_expiry             - PRIVATE:timout function when we waited long enough
  mib_timed_response           - PRIVATE:if here, the answer beat the clock usually.

  ncsmib_make_req_looklike_rsp  - PUBLIC:put rsp data on a copy of original request.
  ncs_key_memcopy               - PUBLIC:make a HEAP copy of a NCS_KEY
  ncsmib_inst_memcopy           - PUBLIC:make a HEAP copy of a MIB instance array.
  ncsmib_oct_memcopy            - PUBLIC:make a HEAP copy of a MIB OctetString
  ncsmib_memcopy                - PUBLIC:make a HEAP copy of a NCSMIB_ARG 
  ncsmib_memfree                - PUBLIC:free a pure HEAP instanse of an NCSMIB_ARG

  ncsmib_encode                 - PUBLIC: put an NCSMIB_ARG in a USRBUF
  ncsmib_decode                 - PUBLIC: get an NCSMIB_ARG out of a USRBUF

******************************************************************************/

/** Get compile time options...**/
#include "ncs_opt.h"

/** Get general definitions...**/
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncsmib_mem.h"
#include "ncssysf_def.h"
#include "ncssysf_mem.h"
#include "ncssysf_sem.h"
#include "ncssysf_tmr.h"
#include "ncssysfpool.h"
#include "ncsencdec_pub.h"
#include "usrbuf.h"
#include "ncs_ubaid.h"
#include "ncs_trap.h"

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_init

  DESCRIPTION:
     Put this NCSMIB_ARG structure in start state. Do not have any valid data
     or pointers hanging off the passed structure, as this function will 
     overwrite such stuff with null.

  RETURNS:
     nothing

*****************************************************************************/

uns32 gl_xch_id = 0;               /* locally generates unique Exchange IDs */

void ncsmib_init(NCSMIB_ARG* arg)
{
   arg->i_rsp_fnc = 0;
   arg->i_op      = 0;
   ncsstack_init(&arg->stack, NCSMIB_STACK_SIZE);

   m_NCS_MEMSET(&arg->req, 0, sizeof(NCSMIB_REQ) );
   m_NCS_MEMSET(&arg->rsp, 0, sizeof(NCSMIB_RSP) );

   arg->i_mib_key = 0;
   arg->i_usr_key = 0;
   arg->i_xch_id  = gl_xch_id++;
   arg->i_tbl_id  = 0;
   arg->i_policy  = NCSMIB_POLICY_OTHER;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_tm_create

  DESCRIPTION:
     Creates the transaction manager.

  RETURNS:
    SUCCESS     - Successfully created the transaction manager.
    FAILURE     - Something went wrong.

*****************************************************************************/

static NCS_LOCK tm_lock;           /* transaction manager lock */
static uns32   gl_tm_xch_id;      /* transaction  */
static uns32   gl_tm_created = 0; /* create counter */

uns32 ncsmib_tm_create(void)
{
  gl_tm_created++;
  if(gl_tm_created > 1)
    return NCSCC_RC_SUCCESS;

   gl_tm_xch_id = 1;
   m_NCS_LOCK_INIT_V2(&tm_lock,NCS_SERVICE_ID_MAB,111);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_tm_destroy

  DESCRIPTION:
     Destroys the transaction manager.

  RETURNS:
    SUCCESS     - Successfully destroyed the transaction manager.
    FAILURE     - Something went wrong.

*****************************************************************************/

uns32 ncsmib_tm_destroy(void)
{
  gl_tm_created--;
  if(gl_tm_created > 0)
    return NCSCC_RC_SUCCESS;

   m_NCS_LOCK_DESTROY_V2(&tm_lock, NCS_SERVICE_ID_MAB,111);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 *****************************************************************************

  Synchronized NCSMIB services

    The functions that contribute to its working....

 *****************************************************************************
 *****************************************************************************/

/*****************************************************************************


  PROCEDURE NAME:    ncsmib_sync_request

  DESCRIPTION:
    Set up a mailbox on the stack and initialize it. Push a MIB_SYNC stack
    element on the stack and invoke ncsmib_timed_request.

  ARGUMENTS:
      arg - Legally populated NCSMIB_ARG for Request operation.
      req - A function pointer that conforms to the 'NetPlane Common MIB
            Access API Prototype'.
      periond_10ms - How long code will wait for system answer befor 
            declaring the transaction as FAILURE.
      ma  - IFF
              (ma != NULL) Alloc OCTET-STRING answer in passed stack space
                           a la ncsmem_aid_alloc().
              (ma == NULL), get memory from HEAP via OS Services mem macro

            The issue here is, where does the invoker want the memory to
            come from? Strategy is driven by the duration of answer or goals
            of invoker. 

  RETURNS:    
    SUCCESS     - Successful pop, with NCSMIB_ARG populated with answer.
    FAILURE     - Time expired, or illegal arguments.


*****************************************************************************/

uns32 ncsmib_sync_request(NCSMIB_ARG*    arg, 
                         NCSMIB_REQ_FNC req, 
                         uns32         period_10ms,
                         NCSMEM_AID*    ma)
{
   NCS_SE*                 se;
   NCS_SE_MIB_SYNC*        mib_sync;
   uns32                  sem_hdl;
   void*                  p_sem_hdl = &sem_hdl;

   /* validate the input parameters */ 
   if (arg == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (req == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (ma == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
      
   /* Get the space for a stack element */
   if ((se = ncsstack_push( &arg->stack, 
                           NCS_SE_TYPE_MIB_SYNC, 
                           (uns8)sizeof(NCS_SE_MIB_SYNC))) == NULL)
    {
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    }

   /* Create a semaphor to block this thread */

   m_NCS_SEM_CREATE(&p_sem_hdl);

   /* frame the stack element to the correct subtype */

   mib_sync = (NCS_SE_MIB_SYNC*) se;

   /* Load the stack element with the right stuff */

   mib_sync->usr_rsp_fnc  = arg->i_rsp_fnc;
   mib_sync->stack_sem    = p_sem_hdl;
   mib_sync->stack_arg    = arg;
   mib_sync->maid         = ma;

   /* update the NCSMIB_ARG with the right callback function */

   arg->i_rsp_fnc        = mib_sync_response;

   /* call the watchdog timer portion for this transaction */

   if (ncsmib_timed_request(arg, req, period_10ms) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   /* Wait here till something comes back with SUCCESS or failure */

   m_NCS_SEM_TAKE(p_sem_hdl);

   /* The answer is now planted on this stack's 'arg' value */

   m_NCS_SEM_RELEASE(p_sem_hdl);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    mib_sync_response

  DESCRIPTION:
    This is the callback so that the thread waiting for a response has a 
    place to answer.


  RETURNS:
    SUCCESS     - Successful pop
    FAILURE     - Stack underflow

*****************************************************************************/

uns32 mib_sync_response(NCSMIB_ARG* arg)
{
   NCS_SE*            se;
   NCS_SE_MIB_SYNC*   mib_sync;

   if ((se = ncsstack_pop(&arg->stack)) == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);  /* really bad; Timer will unlock stack */

   if (se->type != NCS_SE_TYPE_MIB_SYNC)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);  /* really bad; Timer will unlock stack */

   mib_sync = (NCS_SE_MIB_SYNC*)se;

   arg->i_rsp_fnc = mib_sync->usr_rsp_fnc;

   /* ncsmib_make_req_looklike_rsp() checks to see if two args are same instance */

   if (ncsmib_make_req_looklike_rsp( mib_sync->stack_arg, 
                                    arg, mib_sync->maid ) != NCSCC_RC_SUCCESS){
      m_NCS_SEM_GIVE(mib_sync->stack_sem);
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   m_NCS_SEM_GIVE(mib_sync->stack_sem);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
 *****************************************************************************

  Timed (but asynchronous) NCSMIB services

    - Local structures used only in this file.
    - The functions that contribute to its working....

 *****************************************************************************
 *****************************************************************************/

/* The transaction blocks created by timed MIB requests */

typedef struct mib_timed
{
   struct mib_timed* next;
   tmr_t             tmr_id;
   uns32             tm_xch_id;
   NCSMIB_RSP_FNC     usr_rsp_fnc;
   NCSMIB_ARG*        usr_mib_cpy;

} MIB_TIMED;

/* To manage outstanding timed MIB requests ............*/

MIB_TIMED  gl_mib_timed_struct = {0};
MIB_TIMED* gl_mib_timed = &gl_mib_timed_struct;
NCS_LOCK    gl_mib_timed_lock;

/*****************************************************************************

  PROCEDURE NAME:   mib_timed_store

  DESCRIPTION:
    put a MIB_TIMED structure in the list.

  RETURNS:
    None

*****************************************************************************/

static void mib_timed_store(MIB_TIMED* to_tmr)
{
   m_NCS_LOCK_V2(&tm_lock,NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111);                                             

   to_tmr->next       = gl_mib_timed->next;
   gl_mib_timed->next = to_tmr;

   m_NCS_UNLOCK_V2(&tm_lock, NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111);
}

/*****************************************************************************

  PROCEDURE NAME:   mib_timed_remove

  DESCRIPTION:
    find and splice out a MIB_TIMED structure from list.

  RETURNS:
    NULL : Not found
    PTR  : the answer

*****************************************************************************/

static MIB_TIMED* mib_timed_remove(uns32 key)
{
   MIB_TIMED* walker; 
   MIB_TIMED* ret;

   m_NCS_LOCK_V2(&tm_lock,NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111); 

   walker = gl_mib_timed;


   while (walker->next != NULL)
   {
      if (walker->next->tm_xch_id == key)
      {
         ret = walker->next;
         walker->next = walker->next->next;
         m_NCS_UNLOCK_V2(&tm_lock, NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111);
         return ret;
      }
      walker = walker->next;
   }
   m_NCS_UNLOCK_V2(&tm_lock, NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111);
   return NULL;
}

/*****************************************************************************

  PROCEDURE NAME:   mib_timed_find

  DESCRIPTION:
    find a MIB_TIMED structure from list, but don't splice it out.

  RETURNS:
    NULL : Not found
    PTR  : the answer

*****************************************************************************/

static MIB_TIMED* mib_timed_find(uns32 key)
{
   MIB_TIMED* walker;

   m_NCS_LOCK_V2(&tm_lock,NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111); 

   walker = gl_mib_timed;


   while (walker->next != NULL)
   {
      if (walker->next->tm_xch_id == key)
      {
         m_NCS_UNLOCK_V2(&tm_lock, NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111);
         return walker->next;
      }
      walker = walker->next;
   }
   m_NCS_UNLOCK_V2(&tm_lock, NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111);
   return NULL;
}

/*****************************************************************************

 PROCEDURE NAME:   ncsmib_timed_request

 DESCRIPTION:
   This function sets a timer so that if the request gets lost in the system,
   the system will be notified and resources can be reclaimed.

 RETURNS:
   SUCCESS     - Successful pop
   FAILURE     - Stack underflow

*****************************************************************************/

uns32 ncsmib_timed_request(NCSMIB_ARG* arg, NCSMIB_REQ_FNC req, uns32 period_10ms)
{
   NCS_SE*               se;
   MIB_TIMED*           to_tmr;
   MIB_TIMED*           here;
   NCS_SE_MIB_TIMED*     mib_timed;
   NCSMIB_ARG*           cpy;
   uns32                tm_xch_id;
   uns32                status; 

   /* validate the paramaters */ 
   if (arg == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (req == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   /* Make a copy before we tamper with the state of the stack */

   if ((cpy = ncsmib_memcopy(arg)) == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   /* Secure stack space for timer information */

   if ((se = ncsstack_push( &arg->stack, 
                           NCS_SE_TYPE_MIB_TIMED, 
                           (uns8)sizeof(NCS_SE_MIB_TIMED))) == NULL)
   {
      ncsmib_memfree(cpy);
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   /* allocate timer transaction space */

   if ((to_tmr = m_MMGR_ALLOC_MIB_TIMED) == NULL)
   {
      ncsmib_memfree(cpy);
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   /* Fill in data going to timer services */

   to_tmr->tmr_id      = 0;
   to_tmr->usr_rsp_fnc = arg->i_rsp_fnc;
   to_tmr->usr_mib_cpy = cpy;
   to_tmr->tm_xch_id   = gl_tm_xch_id;

   gl_tm_xch_id++;

   /* Prepare and then load NCSMIB_ARG stack area */

   mib_timed = (NCS_SE_MIB_TIMED*) se;

   mib_timed->tm_xch_id = to_tmr->tm_xch_id;
   tm_xch_id = to_tmr->tm_xch_id;


   arg->i_rsp_fnc = mib_timed_response;

   /* Store the timer stuff */

   mib_timed_store(to_tmr);

   /* Now advance the request to the proper MIB request service */

   status = req(arg);
   if (status != NCSCC_RC_SUCCESS)
   {
      arg->i_rsp_fnc = to_tmr->usr_rsp_fnc; 
      /* ncsmib_memfree(cpy);*/
      mib_timed_remove((uns32)mib_timed->tm_xch_id);
      return m_LEAP_DBG_SINK(status);
   }
       

   /* See if the answer has happened on the stack.. We lock, since */
   /* timed_expiry() and/or timed_response() function both want to */
   /* destroy 'to_tmr'. If the callback has already happened using */
   /* this same call thread, then 'here' (below) will come back    */
   /* NULL, and we will bail out.. Otherwise, the lock guarentees  */
   /* that 'to_tmr' exists and that the timer is indeed started.   */

   m_NCS_LOCK_V2(&tm_lock,NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111); 

   if ((here = mib_timed_find(tm_xch_id)) == NULL)
     {
     /*ncsmib_memfree(cpy);*/
     m_NCS_UNLOCK_V2(&tm_lock,NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111); 
     return NCSCC_RC_SUCCESS;
     }

   /* Well, we did not get the callback yet, so start timer services */

   m_NCS_TMR_CREATE ( to_tmr->tmr_id,
                     period_10ms,
                     mib_timed_expiry,
                     (void*)to_tmr->tm_xch_id);

   m_NCS_TMR_START  ( to_tmr->tmr_id,
                     period_10ms,
                     mib_timed_expiry,
                     NCS_INT32_TO_PTR_CAST(to_tmr->tm_xch_id));

   m_NCS_UNLOCK_V2(&tm_lock,NCS_LOCK_WRITE,NCS_SERVICE_ID_MAB,111); 
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:   mib_timed_expiry

  DESCRIPTION:
    This function is invoked if a MIB transaction has not come back and
    cancelled the timer. That is, this function means the MIB transaction 
    has failed.

  RETURNS:
    SUCCESS     - Successful pop
    FAILURE     - Stack underflow

*****************************************************************************/

void mib_timed_expiry(void* opaque_to_tmr)
{
   MIB_TIMED*   to_tmr;
   NCSMIB_ARG*   mib_arg;

   if ((to_tmr = mib_timed_remove(NCS_PTR_TO_UNS32_CAST(opaque_to_tmr))) == NULL)
   {
      /* According to me (SMM), if we are here, to_tmr must be there!! */
      /* If you get the debug statement, there is indeed a BUG!!       */

      m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return;
   }

   mib_arg               = to_tmr->usr_mib_cpy;

   mib_arg->i_op         = m_NCSMIB_REQ_TO_RSP(mib_arg->i_op);
   mib_arg->i_rsp_fnc    = to_tmr->usr_rsp_fnc;
   mib_arg->rsp.i_status = NCSCC_RC_REQ_TIMOUT;

   m_NCS_MEMSET(&mib_arg->rsp.info,0,sizeof(mib_arg->rsp.info));


   to_tmr->usr_rsp_fnc(mib_arg);

   mib_arg->i_op         = m_NCSMIB_RSP_TO_REQ(mib_arg->i_op);
   ncsmib_memfree(mib_arg); /* Invoker owns it, and this is pure HEAP */


   m_NCS_TMR_DESTROY(to_tmr->tmr_id);

   m_MMGR_FREE_MIB_TIMED(to_tmr);

   return;
}

/*****************************************************************************

  PROCEDURE NAME:   mib_timed_response

  DESCRIPTION:
    This function is invoked when a timed transaction responds before the
    timer expires.

  RETURNS:
    SUCCESS     - Successful pop
    FAILURE     - Stack underflow

*****************************************************************************/

uns32 mib_timed_response(NCSMIB_ARG* arg)
{
   NCS_SE*            se;
   MIB_TIMED*        to_tmr;
   NCS_SE_MIB_TIMED*  mib_timed;

   if ((se = ncsstack_pop(&arg->stack)) == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);  /* really bad; Timer will unlock stack */

   if (se->type != NCS_SE_TYPE_MIB_TIMED)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);  /* really bad; Timer will unlock stack */

   mib_timed = (NCS_SE_MIB_TIMED*)se;

   /* Given below condition implies it is a partial response for the CLI request
      made. Hence don't delete the mib_time node from the list */
   if ((arg->i_op == NCSMIB_OP_RSP_CLI) && 
       (arg->rsp.info.cli_rsp.o_partial == TRUE))
   {
      if ((to_tmr = mib_timed_find(mib_timed->tm_xch_id)) == NULL)
         return NCSCC_RC_SUCCESS;        /* timer here first.. Transaction is dead */

      arg->i_rsp_fnc = to_tmr->usr_rsp_fnc;
      to_tmr->usr_rsp_fnc(arg);          /* do the callback now */
   }
   else
   {
      if ((to_tmr = mib_timed_remove(mib_timed->tm_xch_id)) == NULL)
         return NCSCC_RC_SUCCESS;                /* timer here first.. Transaction is dead */

      if (to_tmr->tmr_id != 0)
      {
         m_NCS_TMR_STOP   ( to_tmr->tmr_id );
         m_NCS_TMR_DESTROY( to_tmr->tmr_id );
      }

      arg->i_rsp_fnc    = to_tmr->usr_rsp_fnc;

      to_tmr->usr_rsp_fnc(arg);           /* do the callback now */

      ncsmib_memfree(to_tmr->usr_mib_cpy); /* we didn't need this copy after all */

      m_MMGR_FREE_MIB_TIMED(to_tmr);      /* give back to heap */
   }

   return NCSCC_RC_SUCCESS; 
}


/*****************************************************************************
 *****************************************************************************

  NCSMIB_ARG utilities that may assist in various contexts.

    - The functions that contribute to its working....

 *****************************************************************************
 *****************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:    ncsmib_make_req_looklike_rsp

  DESCRIPTION:
    Do some sanity checking to confirm that the nature of the response
    maps to the type of request. Assuming that goes well, plant the
    response data on the destination using the space provided by NCSMEM_AID.




  RETURNS:
    SUCCESS     - Successful pop
    FAILURE     - Stack underflow

*****************************************************************************/

uns32 ncsmib_make_req_looklike_rsp( NCSMIB_ARG* req, 
                                   NCSMIB_ARG* rsp,
                                   NCSMEM_AID* ma)
  {
  NCSMIB_OP req_as_rsp_op = m_NCSMIB_REQ_TO_RSP(req->i_op);
  
  if (req != rsp) /* if the are the same, doing these checks are flawed */
    {
    if (req_as_rsp_op != rsp->i_op)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
    
    req->i_op = rsp->i_op;
    memcpy(&req->rsp,&rsp->rsp,sizeof(req->rsp));/* do the first order copy, w/no reguard to memory */
    
    /* reuse the arg.i_idx value */
    if (req->i_idx.i_inst_len != rsp->i_idx.i_inst_len)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
    req->i_idx.i_inst_ids = (const uns32*)ncsmem_aid_cpy(ma,(const uns8*)rsp->i_idx.i_inst_ids,
                                                        req->i_idx.i_inst_len * sizeof(uns32));
    }

   /* If the response came back with some sort of error, then the analysis is done */

   if (rsp->rsp.i_status != NCSCC_RC_SUCCESS)
   {
     if(rsp->rsp.add_info_len > 0)
       if(rsp->rsp.add_info != NULL)
         if((req->rsp.add_info = (uns8 *)ncsmem_aid_cpy(ma,(const uns8 *)rsp->rsp.add_info, rsp->rsp.add_info_len)) == NULL)
             return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
     return NCSCC_RC_SUCCESS;
   }

   switch (rsp->i_op)
   {
   /*****************************************************************************
    Just the Response Cases
   *****************************************************************************/
   case NCSMIB_OP_RSP_GET  :
      {
         NCSMIB_GET_RSP* src = &rsp->rsp.info.get_rsp;
         NCSMIB_GET_RSP* dst = &req->rsp.info.get_rsp;

         if (src->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            dst->i_param_val.info.i_oct = ncsmem_aid_cpy(ma, src->i_param_val.info.i_oct,
                                                        src->i_param_val.i_length);
         break;
      }
   case NCSMIB_OP_RSP_SET  :
      {
         NCSMIB_SET_RSP* src = &rsp->rsp.info.get_rsp;
         NCSMIB_SET_RSP* dst = &req->rsp.info.set_rsp;

         if (src->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            dst->i_param_val.info.i_oct = ncsmem_aid_cpy(ma, src->i_param_val.info.i_oct,
                                                        src->i_param_val.i_length);
         break;
      }
   case NCSMIB_OP_RSP_TEST :
      {
         NCSMIB_TEST_RSP* src = &rsp->rsp.info.get_rsp;
         NCSMIB_TEST_RSP* dst = &req->rsp.info.set_rsp;


         if (src->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            dst->i_param_val.info.i_oct = ncsmem_aid_cpy(ma, src->i_param_val.info.i_oct,
                                                        src->i_param_val.i_length);
         break;
      }

   case NCSMIB_OP_RSP_NEXT :
      {
         NCSMIB_NEXT_RSP* src  = &rsp->rsp.info.next_rsp;
         NCSMIB_NEXT_RSP* dst  = &req->rsp.info.next_rsp;

         dst->i_next.i_inst_ids = (uns32*)ncsmem_aid_cpy(ma,(uns8*)src->i_next.i_inst_ids,
                                                        src->i_next.i_inst_len * sizeof(uns32));

         if (src->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            dst->i_param_val.info.i_oct = ncsmem_aid_cpy(ma, src->i_param_val.info.i_oct,
                                                        src->i_param_val.i_length);
         break;
      }

   case NCSMIB_OP_RSP_GETROW :
   case NCSMIB_OP_RSP_SETROW :
   case NCSMIB_OP_RSP_TESTROW:
      {
         NCSMIB_GETROW_RSP* src = &rsp->rsp.info.getrow_rsp;
         NCSMIB_GETROW_RSP* dst = &req->rsp.info.getrow_rsp;

         if (src->i_usrbuf != NULL)
         {
            dst->i_usrbuf = src->i_usrbuf;
            if (req != rsp) /* if the are the same, doing these are flawed */
            {
               src->i_usrbuf = NULL;
            }
         }

         break;
      }

   case NCSMIB_OP_RSP_SETALLROWS:
      {
         NCSMIB_SETALLROWS_RSP* src = &rsp->rsp.info.setallrows_rsp;
         NCSMIB_SETALLROWS_RSP* dst = &req->rsp.info.setallrows_rsp;

         if (src->i_usrbuf != NULL)
         {
            dst->i_usrbuf = src->i_usrbuf;
            if (req != rsp) /* if the are the same, doing these are flawed */
            {
               src->i_usrbuf = NULL;
            }
         }

         break;
      }

   case NCSMIB_OP_RSP_REMOVEROWS:
      {
         NCSMIB_REMOVEROWS_RSP* src = &rsp->rsp.info.removerows_rsp;
         NCSMIB_REMOVEROWS_RSP* dst = &req->rsp.info.removerows_rsp;

         if (src->i_usrbuf != NULL)
         {
            dst->i_usrbuf = src->i_usrbuf;
            if (req != rsp) /* if the are the same, doing these are flawed */
            {
               src->i_usrbuf = NULL;
            }
         }
         break;
      }

   case NCSMIB_OP_RSP_NEXTROW :
      {
         NCSMIB_NEXTROW_RSP* src = &rsp->rsp.info.nextrow_rsp;
         NCSMIB_NEXTROW_RSP* dst = &req->rsp.info.nextrow_rsp;

         dst->i_next.i_inst_ids = (uns32*)ncsmem_aid_cpy(ma,(uns8*)src->i_next.i_inst_ids,
                                                        src->i_next.i_inst_len * sizeof(uns32));
         if (src->i_usrbuf != NULL)
         {
            dst->i_usrbuf = src->i_usrbuf;
            if (req != rsp) /* if the are the same, doing these are flawed */
            {
               src->i_usrbuf = NULL;
            }
         }
         break;
      }

   case NCSMIB_OP_RSP_MOVEROW:
     {
        NCSMIB_MOVEROW_RSP* src = &rsp->rsp.info.moverow_rsp;
        NCSMIB_MOVEROW_RSP* dst = &req->rsp.info.moverow_rsp;

        dst->i_move_to = src->i_move_to;
        if (src->i_usrbuf != NULL)
        {
           dst->i_usrbuf = src->i_usrbuf;
           if (req != rsp) /* if the are the same, doing these are flawed */
           {
              src->i_usrbuf = NULL;
           }
        }
        break;
     }
     
   case NCSMIB_OP_RSP_CLI:
   case NCSMIB_OP_RSP_CLI_DONE:
     {
        NCSMIB_CLI_RSP *src = &rsp->rsp.info.cli_rsp;
        NCSMIB_CLI_RSP *dst = &req->rsp.info.cli_rsp;

        if (src->o_answer != NULL)
        {
           dst->o_answer = src->o_answer;
           if (req != rsp) /* if the are the same, doing these are flawed */
           {
              src->o_answer = NULL;
           }
        }
        break;
     }

   case NCSMIB_OP_RSP_GET_INFO:
     {
        break;
     }

   default:
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   if(rsp->rsp.add_info_len > 0)
       if(rsp->rsp.add_info != NULL)
          if((req->rsp.add_info = (uns8 *)ncsmem_aid_cpy(ma,(const uns8 *)rsp->rsp.add_info, rsp->rsp.add_info_len)) == NULL)
             return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (ma->status != NCSCC_RC_SUCCESS) /* NCSMEM_AID kept track of its status */
      return NCSCC_RC_FAILURE;

   return NCSCC_RC_SUCCESS;
}
/*****************************************************************************/

uns32* ncsmib_inst_memcopy(uns32 len, const uns32* ref)
{
   uns32* cpy = NULL;
   if(len != 0)
     {
     if((cpy = m_MMGR_ALLOC_MIB_INST_IDS(len)) == NULL)
     {
       m_LEAP_DBG_SINK(0);
       return (uns32*)NULL;
     }
     memcpy((uns32*)cpy,ref,len * sizeof(uns32));
     return cpy;
     }
   else
     return NULL;
}

/*****************************************************************************/

uns8* ncsmib_oct_memcopy(uns32 len, const uns8*  ref)
{
   uns8*  cpy = 0;

   if (len != 0)
   {
      if ((cpy = m_MMGR_ALLOC_MIB_OCT(len)) == NULL)
      {
         m_LEAP_DBG_SINK(0);
         return (uns8*)NULL;
      }
      memcpy( (uns8*)cpy, ref, (size_t)len);
      return cpy;
   }
   else
      return NULL;
}

/*****************************************************************************/

NCSMIB_ARG* ncsmib_memcopy( NCSMIB_ARG*    arg)
{
   NCSMIB_ARG* cpy;

   if ((cpy = m_MMGR_ALLOC_NCSMIB_ARG) == NULL)
   {
      m_LEAP_DBG_SINK(0);
      return (NCSMIB_ARG*)NULL;
   }

   /* NOTE!! When we do the copy, we believe the values of the fields copied below */
   /* That is, messing with this memcpy() line messes with algorythms below........*/

   memcpy(cpy,arg,sizeof(NCSMIB_ARG));

   if (arg->i_idx.i_inst_ids != NULL)
      cpy->i_idx.i_inst_ids = ncsmib_inst_memcopy(cpy->i_idx.i_inst_len,arg->i_idx.i_inst_ids);

   switch (arg->i_op)
   {
   /*****************************************************************************
    The Response Cases
    *****************************************************************************/
   case NCSMIB_OP_RSP_GET  :   
   case NCSMIB_OP_RSP_SET  :
   case NCSMIB_OP_RSP_TEST :
      {
         NCSMIB_GET_RSP* rsp = &cpy->rsp.info.get_rsp;
         if (rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            rsp->i_param_val.info.i_oct = ncsmib_oct_memcopy(rsp->i_param_val.i_length,
                                                            rsp->i_param_val.info.i_oct);
         break;
      }
   case NCSMIB_OP_RSP_GET_INFO:
      {

         break;
      }

   case NCSMIB_OP_RSP_NEXT :
      {
         NCSMIB_NEXT_RSP* rsp  = &cpy->rsp.info.next_rsp;
         rsp->i_next.i_inst_ids = ncsmib_inst_memcopy(rsp->i_next.i_inst_len,
                                                     rsp->i_next.i_inst_ids);

         if (rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            rsp->i_param_val.info.i_oct = ncsmib_oct_memcopy(rsp->i_param_val.i_length,
                                                            rsp->i_param_val.info.i_oct);

         break;
      }

   case NCSMIB_OP_RSP_GETROW :
   case NCSMIB_OP_RSP_SETROW :
   case NCSMIB_OP_RSP_TESTROW:
   case NCSMIB_OP_RSP_SETALLROWS:
   case NCSMIB_OP_RSP_REMOVEROWS:
      {
         NCSMIB_GETROW_RSP* rsp = &cpy->rsp.info.getrow_rsp;
         if (rsp->i_usrbuf != NULL)
            rsp->i_usrbuf   = m_MMGR_DITTO_BUFR(rsp->i_usrbuf);
         break;
      }

   case NCSMIB_OP_RSP_NEXTROW :
      {
         NCSMIB_NEXTROW_RSP* rsp = &cpy->rsp.info.nextrow_rsp;
         if (rsp->i_usrbuf != NULL)
            rsp->i_usrbuf = m_MMGR_DITTO_BUFR(rsp->i_usrbuf);

         rsp->i_next.i_inst_ids = ncsmib_inst_memcopy(rsp->i_next.i_inst_len,
                                                     rsp->i_next.i_inst_ids);
         break;
      }

   case NCSMIB_OP_RSP_MOVEROW :
      {
         NCSMIB_MOVEROW_RSP* rsp = &cpy->rsp.info.moverow_rsp;
         if (rsp->i_usrbuf != NULL)
            rsp->i_usrbuf = m_MMGR_DITTO_BUFR(rsp->i_usrbuf);
      }
     break;

   case NCSMIB_OP_RSP_CLI :
   case NCSMIB_OP_RSP_CLI_DONE :
      {
         NCSMIB_CLI_RSP* rsp = &cpy->rsp.info.cli_rsp;
         if (rsp->o_answer != NULL)
            rsp->o_answer = m_MMGR_DITTO_BUFR(rsp->o_answer);
      }
     break;

      /*****************************************************************************
       The Request Cases
       *****************************************************************************/

   case NCSMIB_OP_REQ_GET     :   
   case NCSMIB_OP_REQ_NEXT    :
   case NCSMIB_OP_REQ_GETROW  :
   case NCSMIB_OP_REQ_NEXTROW :
      break;

   case NCSMIB_OP_REQ_SET  :
   case NCSMIB_OP_REQ_TEST :
      {
         NCSMIB_SET_REQ* req = &cpy->req.info.set_req;

         if (req->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
            req->i_param_val.info.i_oct = ncsmib_oct_memcopy(req->i_param_val.i_length,
                                                            req->i_param_val.info.i_oct);
         break;
      }

   case NCSMIB_OP_REQ_SETROW  :
   case NCSMIB_OP_REQ_TESTROW :
   case NCSMIB_OP_REQ_SETALLROWS:
   case NCSMIB_OP_REQ_REMOVEROWS:
      {
         NCSMIB_SETROW_REQ* req = &cpy->req.info.setrow_req;
         if (req->i_usrbuf != NULL)
            req->i_usrbuf = m_MMGR_DITTO_BUFR(req->i_usrbuf);

         break;
      }

   case NCSMIB_OP_REQ_MOVEROW :
     {
        NCSMIB_MOVEROW_REQ* req = &cpy->req.info.moverow_req;
        if(req->i_usrbuf != NULL)
          req->i_usrbuf = m_MMGR_DITTO_BUFR(req->i_usrbuf);

        break;
     }
     
   case NCSMIB_OP_REQ_CLI :
      {
         NCSMIB_CLI_REQ* req = &cpy->req.info.cli_req;
         if (req->i_usrbuf != NULL)
            req->i_usrbuf = m_MMGR_DITTO_BUFR(req->i_usrbuf);
      }
     break;
     
   case NCSMIB_OP_REQ_GET_INFO :
      break;

   default:
      {
         m_MMGR_FREE_NCSMIB_ARG(cpy);

         m_LEAP_DBG_SINK(0);
         return (NCSMIB_ARG*)NULL;
      }
   }
   switch (arg->i_op)
   {
       case NCSMIB_OP_RSP_GET  :   
       case NCSMIB_OP_RSP_SET  :
       case NCSMIB_OP_RSP_TEST :
       case NCSMIB_OP_RSP_GET_INFO:
       case NCSMIB_OP_RSP_NEXT :
       case NCSMIB_OP_RSP_GETROW :
       case NCSMIB_OP_RSP_SETROW :
       case NCSMIB_OP_RSP_TESTROW:
       case NCSMIB_OP_RSP_SETALLROWS:
       case NCSMIB_OP_RSP_REMOVEROWS:
       case NCSMIB_OP_RSP_NEXTROW :
       case NCSMIB_OP_RSP_MOVEROW :
       case NCSMIB_OP_RSP_CLI :
       case NCSMIB_OP_RSP_CLI_DONE :
            if(arg->rsp.add_info != NULL)
               cpy->rsp.add_info = ncsmib_oct_memcopy(cpy->rsp.add_info_len, cpy->rsp.add_info);
            break;
       default:
            break;
   }
   return cpy;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_memfree

  DESCRIPTION:
    Free a heap instance of an NCSMIB_ARG. 

    WARNING: Invoker must be sure only heap memory is hanging off of this
    struct.

  RETURNS:
    Nothing..

*****************************************************************************/

void ncsmib_memfree( NCSMIB_ARG*    arg)
{
   if (arg == NULL)
   {
      m_LEAP_DBG_SINK_VOID(0);
      return;
   }

   if (arg->i_idx.i_inst_ids != NULL)
      m_MMGR_FREE_MIB_INST_IDS(arg->i_idx.i_inst_ids);

   switch (arg->i_op)
   {
   /*****************************************************************************
    The Response Cases
    *****************************************************************************/
   case NCSMIB_OP_RSP_GET  :
   case NCSMIB_OP_RSP_SET  :
   case NCSMIB_OP_RSP_TEST :
      {
         NCSMIB_GET_RSP* rsp = &arg->rsp.info.get_rsp;

         if (rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
           {
           if ((rsp->i_param_val.i_length != 0) && (rsp->i_param_val.info.i_oct != NULL))
             m_MMGR_FREE_MIB_OCT(rsp->i_param_val.info.i_oct);
           }
         break;
      }

   case NCSMIB_OP_RSP_NEXT :
      {
         NCSMIB_NEXT_RSP* rsp  = &arg->rsp.info.next_rsp;
         if (rsp->i_next.i_inst_ids != NULL)
             m_MMGR_FREE_MIB_INST_IDS(rsp->i_next.i_inst_ids);

         if (rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
           {
           if ((rsp->i_param_val.i_length != 0) && (rsp->i_param_val.info.i_oct != NULL))
             m_MMGR_FREE_MIB_OCT(rsp->i_param_val.info.i_oct);
           }
         break;
      }

   case NCSMIB_OP_RSP_GETROW :
   case NCSMIB_OP_RSP_SETROW :
   case NCSMIB_OP_RSP_TESTROW:
   case NCSMIB_OP_RSP_SETALLROWS:
   case NCSMIB_OP_RSP_REMOVEROWS:
      {
         NCSMIB_GETROW_RSP* rsp = &arg->rsp.info.getrow_rsp;
         if(rsp->i_usrbuf != NULL)
           m_MMGR_FREE_BUFR_LIST(rsp->i_usrbuf);
         break;
      }

   case NCSMIB_OP_RSP_NEXTROW :
      {
         NCSMIB_NEXTROW_RSP* rsp  = &arg->rsp.info.nextrow_rsp;
         if(rsp->i_usrbuf != NULL)
           m_MMGR_FREE_BUFR_LIST(rsp->i_usrbuf);

         if (rsp->i_next.i_inst_ids != NULL)
           m_MMGR_FREE_MIB_INST_IDS(rsp->i_next.i_inst_ids); 
         break;
      }

   case NCSMIB_OP_RSP_MOVEROW :
      {
         NCSMIB_MOVEROW_RSP* rsp  = &arg->rsp.info.moverow_rsp;
         if(rsp->i_usrbuf != NULL)
           m_MMGR_FREE_BUFR_LIST(rsp->i_usrbuf);
         break;
      }
      
   case NCSMIB_OP_RSP_CLI:
   case NCSMIB_OP_RSP_CLI_DONE:
      {
         NCSMIB_CLI_RSP* rsp  = &arg->rsp.info.cli_rsp;
         if(rsp->o_answer != NULL)
           m_MMGR_FREE_BUFR_LIST(rsp->o_answer);
         break;
      }

   case NCSMIB_OP_RSP_GET_INFO:
     break;

      /*****************************************************************************
       The Request Cases
       *****************************************************************************/

   case NCSMIB_OP_REQ_GET  :
   case NCSMIB_OP_REQ_GET_INFO:
   case NCSMIB_OP_REQ_NEXT :
   case NCSMIB_OP_REQ_GETROW  :
   case NCSMIB_OP_REQ_NEXTROW :
      break;

   case NCSMIB_OP_REQ_SET  :
   case NCSMIB_OP_REQ_TEST :
      {
         NCSMIB_SET_REQ* req = &arg->req.info.set_req;

         if (req->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
           {
           if ((req->i_param_val.i_length != 0) && (req->i_param_val.info.i_oct != NULL))
             m_MMGR_FREE_MIB_OCT(req->i_param_val.info.i_oct);
           }
         break;
      }

   case NCSMIB_OP_REQ_SETROW  :
   case NCSMIB_OP_REQ_TESTROW :
   case NCSMIB_OP_REQ_SETALLROWS:
   case NCSMIB_OP_REQ_REMOVEROWS:
      {
         NCSMIB_SETROW_REQ* req = &arg->req.info.setrow_req;
         if (req->i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(req->i_usrbuf);

         break;
      }

   case NCSMIB_OP_REQ_MOVEROW:
     {
        NCSMIB_MOVEROW_REQ* req = &arg->req.info.moverow_req;
        if (req->i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(req->i_usrbuf);

        break;
     }

   case NCSMIB_OP_REQ_CLI:
     {
        NCSMIB_CLI_REQ* req = &arg->req.info.cli_req;
        if (req->i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(req->i_usrbuf);

        break;
     }

   default:
      {
        m_MMGR_FREE_NCSMIB_ARG(arg);
        m_LEAP_DBG_SINK_VOID(0);
        return;
      }
   }
   switch (arg->i_op)
   {
       case NCSMIB_OP_RSP_GET  :
       case NCSMIB_OP_RSP_SET  :
       case NCSMIB_OP_RSP_TEST :
       case NCSMIB_OP_RSP_NEXT :
       case NCSMIB_OP_RSP_GETROW :
       case NCSMIB_OP_RSP_SETROW :
       case NCSMIB_OP_RSP_TESTROW:
       case NCSMIB_OP_RSP_SETALLROWS:
       case NCSMIB_OP_RSP_REMOVEROWS:
       case NCSMIB_OP_RSP_NEXTROW :
       case NCSMIB_OP_RSP_MOVEROW :
       case NCSMIB_OP_RSP_CLI:
       case NCSMIB_OP_RSP_CLI_DONE:
       case NCSMIB_OP_RSP_GET_INFO:
             if(arg->rsp.add_info_len != 0)
               if(arg->rsp.add_info != NULL)
               {
                  m_MMGR_FREE_MIB_OCT(arg->rsp.add_info);
                  arg->rsp.add_info = NULL;
                  arg->rsp.add_info_len = 0;
               }
               break;
        default:
               break;
   }

   m_MMGR_FREE_NCSMIB_ARG(arg);
}



/*****************************************************************************
 *****************************************************************************

  H J M I B _ A R G    E n c o d e / D e c o d e  F u n c t i o n s

 *****************************************************************************
 *****************************************************************************/


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_rsp_encode

  DESCRIPTION:
    Encode in the passed USRBUF the passed in NCSMIB_RSP data.

  RETURNS:
    NCSCC_RC_SUCCESS     - Successful encode
    NCSCC_RC_FAILURE     - something went wrong

*****************************************************************************/
uns32   ncsmib_rsp_encode( NCSMIB_OP     op,
                          NCSMIB_RSP*   rsp, 
                          NCS_UBAID*    uba,
                          uns16 msg_fmt_ver)
{
   uns8* stream = NULL;

   if ((NULL == rsp) || (NULL == uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (m_NCSMIB_ISIT_A_REQ(op))
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream,rsp->i_status);
   ncs_enc_claim_space(uba,sizeof(uns32));


   switch (op)
   {
      case NCSMIB_OP_RSP_GET    :
      case NCSMIB_OP_RSP_SET    :
      case NCSMIB_OP_RSP_TEST   :

         if (ncsmib_param_val_encode(&(rsp->info.get_rsp.i_param_val),uba) != NCSCC_RC_SUCCESS)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

         break;

      case NCSMIB_OP_RSP_NEXT :

         if (ncsmib_param_val_encode(&(rsp->info.next_rsp.i_param_val),uba) != NCSCC_RC_SUCCESS)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

         stream = ncs_enc_reserve_space(uba,sizeof(uns32));
         if (stream == NULL)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         ncs_encode_32bit(&stream,rsp->info.next_rsp.i_next.i_inst_len);
         ncs_enc_claim_space(uba,sizeof(uns32));

         if (ncsmib_inst_encode(rsp->info.next_rsp.i_next.i_inst_ids,rsp->info.next_rsp.i_next.i_inst_len,uba) != NCSCC_RC_SUCCESS)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

         break;

      case NCSMIB_OP_RSP_GETROW :
      case NCSMIB_OP_RSP_SETROW :
      case NCSMIB_OP_RSP_TESTROW:
      case NCSMIB_OP_RSP_REMOVEROWS:
      case NCSMIB_OP_RSP_SETALLROWS:
         {
            NCSMIB_GETROW_RSP* gr_rsp = &rsp->info.getrow_rsp;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(gr_rsp->i_usrbuf);

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(gr_rsp->i_usrbuf));
         }
         break;

      case NCSMIB_OP_RSP_NEXTROW :
         {
            NCSMIB_NEXTROW_RSP* nr_rsp = &rsp->info.nextrow_rsp;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(nr_rsp->i_usrbuf);

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,nr_rsp->i_next.i_inst_len);
            ncs_enc_claim_space(uba,sizeof(uns32));

            if (ncsmib_inst_encode(nr_rsp->i_next.i_inst_ids,nr_rsp->i_next.i_inst_len,uba) != NCSCC_RC_SUCCESS)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(nr_rsp->i_usrbuf));
         }
         break;

      case NCSMIB_OP_RSP_MOVEROW :
         {
            NCSMIB_MOVEROW_RSP* mr_rsp = &rsp->info.moverow_rsp;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(mr_rsp->i_usrbuf);

            stream = ncs_enc_reserve_space(uba,sizeof(MDS_DEST));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            mds_st_encode_mds_dest(&stream,&mr_rsp->i_move_to);
            ncs_enc_claim_space(uba,sizeof(MDS_DEST));

            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(mr_rsp->i_usrbuf));
         }
         break;

      case NCSMIB_OP_RSP_CLI:
      case NCSMIB_OP_RSP_CLI_DONE:
         {
            NCSMIB_CLI_RSP* cli_rsp = &rsp->info.cli_rsp;
            uns32 ubsize = m_MMGR_LINK_DATA_LEN(cli_rsp->o_answer);

            /* encode i_cmnd_id */
            stream = ncs_enc_reserve_space(uba,sizeof(uns16));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_16bit(&stream, cli_rsp->i_cmnd_id);
            ncs_enc_claim_space(uba,sizeof(uns16));
        
            /* encode o_partial */
            stream = ncs_enc_reserve_space(uba,sizeof(uns16));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_16bit(&stream, cli_rsp->o_partial);
            ncs_enc_claim_space(uba,sizeof(uns16));
 
            stream = ncs_enc_reserve_space(uba,sizeof(uns32));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            ncs_encode_32bit(&stream,ubsize);
            ncs_enc_claim_space(uba,sizeof(uns32));

            ncs_enc_append_usrbuf(uba,m_MMGR_DITTO_BUFR(cli_rsp->o_answer));
         }
         break;

      default:
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }
   if(msg_fmt_ver == 2)
   {
       stream = ncs_enc_reserve_space(uba,sizeof(uns16));
       if (stream == NULL)
           return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       ncs_encode_16bit(&stream,rsp->add_info_len);
       ncs_enc_claim_space(uba,sizeof(uns16));

       if(rsp->add_info_len != 0)
       {
           if(ncs_encode_n_octets_in_uba(uba,rsp->add_info,rsp->add_info_len) != NCSCC_RC_SUCCESS)
              return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       }
   }
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_param_val_encode

  DESCRIPTION:
    Encode in the passed USRBUF the passed in NCSMIB_PARAM_VAL data.

  RETURNS:
    NCSCC_RC_SUCCESS     - Successful encode
    NCSCC_RC_FAILURE     - something went wrong

*****************************************************************************/

uns32 ncsmib_param_val_encode( NCSMIB_PARAM_VAL* mpv, NCS_UBAID* uba)
{
   uns8* stream;

   if (uba == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   ncs_encode_32bit(&stream,mpv->i_param_id);
   ncs_enc_claim_space(uba,sizeof(uns32));

   stream = ncs_enc_reserve_space(uba,sizeof(uns8));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   ncs_encode_8bit(&stream,mpv->i_fmat_id);
   ncs_enc_claim_space(uba,sizeof(uns8));

   switch (mpv->i_fmat_id)
   {
   case NCSMIB_FMAT_OCT:
      {
         uns16 i;
         uns16 max;
         uns8* octets;

         stream = ncs_enc_reserve_space(uba,sizeof(uns16));
         if (stream == NULL)
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

         ncs_encode_16bit(&stream,mpv->i_length);
         ncs_enc_claim_space(uba,sizeof(uns16));

         for (i = 0, max = mpv->i_length, octets = (uns8*)mpv->info.i_oct; i < max; i++)
         {
            stream = ncs_enc_reserve_space(uba,sizeof(uns8));
            if (stream == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            ncs_encode_8bit(&stream,*(octets + i));
            ncs_enc_claim_space(uba,sizeof(uns8));
         }

      }
      break;

   case NCSMIB_FMAT_INT:
   case NCSMIB_FMAT_BOOL:
      stream = ncs_enc_reserve_space(uba,sizeof(uns32));
      if (stream == NULL)
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

      ncs_encode_32bit(&stream,mpv->info.i_int);
      ncs_enc_claim_space(uba,sizeof(uns32));

      break;

   case 0:
      /* i_fmat_id is 0 when we are dealing with FAILURE replies...
         where we might have a valid param_id only...*/
      break;

   default:
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_inst_encode

  DESCRIPTION:
    Encode in the passed USRBUF the passed in uns32 string.

  RETURNS:
    NCSCC_RC_SUCCESS     - Successful encode
    NCSCC_RC_FAILURE     - something went wrong

*****************************************************************************/

uns32 ncsmib_inst_encode( const uns32* inst_ids,uns32 inst_len,NCS_UBAID* uba)
{
   uns32 i;
   uns8* stream;

   for (i = 0; i < inst_len ; i++)
   {
      stream = ncs_enc_reserve_space(uba,sizeof(uns32));
      if (stream == NULL)
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

      ncs_encode_32bit(&stream,*(inst_ids + i));
      ncs_enc_claim_space(uba,sizeof(uns32));
   }

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_rsp_decode

  DESCRIPTION:
    Decode the passed USRBUF into a NCSMIB_RSP.

  RETURNS:
    NCSCC_RC_SUCCESS     - Successful decode
    NCSCC_RC_FAILURE     - something went wrong

*****************************************************************************/

uns32 ncsmib_rsp_decode(NCSMIB_OP     op,
                       NCSMIB_RSP*   rsp,
                       NCS_UBAID*    uba,
                       uns16         msg_fmt_ver)
{
   uns8*      stream;
   uns8       space[20];

   if ((NULL == rsp) || (NULL == uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if (m_NCSMIB_ISIT_A_REQ(op))
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   rsp->i_status = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));


   switch (op)
   {
      case NCSMIB_OP_RSP_GET    :
      case NCSMIB_OP_RSP_SET    :
      case NCSMIB_OP_RSP_TEST   :
         {
            NCSMIB_GET_RSP* get_rsp = &rsp->info.get_rsp;
            if (ncsmib_param_val_decode(&(get_rsp->i_param_val),NULL,uba) != NCSCC_RC_SUCCESS)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            break;
         }

      case NCSMIB_OP_RSP_NEXT :
         {
            NCSMIB_NEXT_RSP* nxt_rsp = &rsp->info.next_rsp;
            if (ncsmib_param_val_decode(&(nxt_rsp->i_param_val),NULL,uba) != NCSCC_RC_SUCCESS)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            nxt_rsp->i_next.i_inst_len = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            if (ncsmib_inst_decode((uns32**)&nxt_rsp->i_next.i_inst_ids, nxt_rsp->i_next.i_inst_len,uba) != NCSCC_RC_SUCCESS)
            {
               if (nxt_rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
               {
                 if(nxt_rsp->i_param_val.info.i_oct != NULL)
                   m_MMGR_FREE_MIB_OCT(nxt_rsp->i_param_val.info.i_oct);
               }
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            }

            break;
         }

      case NCSMIB_OP_RSP_GETROW :
      case NCSMIB_OP_RSP_SETROW :
      case NCSMIB_OP_RSP_TESTROW:
      case NCSMIB_OP_RSP_SETALLROWS:
      case NCSMIB_OP_RSP_REMOVEROWS:
         {
            uns32 RspUbsize;  /* amount of data in payload that
                              ** belongs with the RSP */
            uns32 FullUbsize; /* Total data in payload */
            NCSMIB_GETROW_RSP* gr_rsp = &rsp->info.getrow_rsp;

            stream    = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            RspUbsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            gr_rsp->i_usrbuf = m_MMGR_COPY_BUFR(uba->ub);

            /* See if anything was encoded following this RSP.
            ** If so, we need to split the USRBUF.
            */
            FullUbsize = m_MMGR_LINK_DATA_LEN(gr_rsp->i_usrbuf);

            /* Is there ehough in the USRBUF? */
            if (FullUbsize < RspUbsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            if (FullUbsize > RspUbsize)
            {
               m_MMGR_REMOVE_FROM_END(gr_rsp->i_usrbuf, (FullUbsize - RspUbsize));
            }
            ncs_dec_skip_space(uba, RspUbsize);

            break;
         }

      case NCSMIB_OP_RSP_NEXTROW :
         {
            uns32 RspUbsize;  /* amount of data in payload that
                              ** belongs with the RSP */
            uns32 FullUbsize; /* Total data in payload */
            NCSMIB_NEXTROW_RSP* nr_rsp = &rsp->info.nextrow_rsp;

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            nr_rsp->i_next.i_inst_len = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            if (ncsmib_inst_decode((uns32**)&nr_rsp->i_next.i_inst_ids, nr_rsp->i_next.i_inst_len,uba) != NCSCC_RC_SUCCESS)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            stream    = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            RspUbsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            nr_rsp->i_usrbuf = m_MMGR_COPY_BUFR(uba->ub);

            /* See if anything was encoded following this RSP.
            ** If so, we need to split the USRBUF.
            */
            FullUbsize = m_MMGR_LINK_DATA_LEN(nr_rsp->i_usrbuf);

            if (FullUbsize < RspUbsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            if (FullUbsize > RspUbsize)
            {
               m_MMGR_REMOVE_FROM_END(nr_rsp->i_usrbuf, (FullUbsize - RspUbsize));
            }
            ncs_dec_skip_space(uba, RspUbsize);

            break;
         }

      case NCSMIB_OP_RSP_MOVEROW :
         {
            uns32 ubsize;
            uns32 FullUbsize;
            NCSMIB_MOVEROW_RSP* mr_rsp = &rsp->info.moverow_rsp;

            stream = ncs_dec_flatten_space(uba,space,sizeof(MDS_DEST));
            mds_st_decode_mds_dest(&stream, &mr_rsp->i_move_to);
            ncs_dec_skip_space(uba,sizeof(MDS_DEST));

            stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            ubsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            /* KCQ: because the usrbuf is the last thing in the uba,
            we just copy whatever's left...
            */
            mr_rsp->i_usrbuf = m_MMGR_COPY_BUFR(uba->ub);
            FullUbsize = m_MMGR_LINK_DATA_LEN(mr_rsp->i_usrbuf);

            if (FullUbsize < ubsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            if (FullUbsize > ubsize)
            {
               m_MMGR_REMOVE_FROM_END(mr_rsp->i_usrbuf, (FullUbsize - ubsize));
            }
            ncs_dec_skip_space(uba, ubsize);

            break;
         }

      case NCSMIB_OP_RSP_CLI:
      case NCSMIB_OP_RSP_CLI_DONE:
         {
            uns32 RspUbsize;  /* amount of data in payload that
                              ** belongs with the RSP */
            uns32 FullUbsize; /* Total data in payload */
            NCSMIB_CLI_RSP* cli_rsp = &rsp->info.cli_rsp;

            /* Get i_cmnd_id */
            stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
            cli_rsp->i_cmnd_id = ncs_decode_16bit(&stream);
            ncs_dec_skip_space(uba, sizeof(uns16));

            /* Get o_partial */
            stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
            cli_rsp->o_partial = ncs_decode_16bit(&stream);
            ncs_dec_skip_space(uba, sizeof(uns16));

            stream    = ncs_dec_flatten_space(uba,space,sizeof(uns32));
            RspUbsize = ncs_decode_32bit(&stream);
            ncs_dec_skip_space(uba,sizeof(uns32));

            cli_rsp->o_answer = m_MMGR_COPY_BUFR(uba->ub);

            /* See if anything was encoded following this RSP.
            ** If so, we need to split the USRBUF.
            */
            FullUbsize = m_MMGR_LINK_DATA_LEN(cli_rsp->o_answer);

            /* Is there ehough in the USRBUF? */
            if (FullUbsize < RspUbsize)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

            if (FullUbsize > RspUbsize)
            {
               m_MMGR_REMOVE_FROM_END(cli_rsp->o_answer, (FullUbsize - RspUbsize));
            }
            ncs_dec_skip_space(uba, RspUbsize);

            break;
         }
         

      default:
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   };
   if(msg_fmt_ver == 2)
   {
       stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
       if( stream == NULL )
            return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
       rsp->add_info_len = ncs_decode_16bit(&stream);
       ncs_dec_skip_space(uba,sizeof(uns16));

       if(rsp->add_info_len != 0)
       {
         if(rsp->add_info == NULL)
         {
            if((rsp->add_info = m_MMGR_ALLOC_MIB_OCT(rsp->add_info_len)) == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            m_NCS_MEMSET(rsp->add_info, '\0', rsp->add_info_len);
            if(ncs_decode_n_octets_from_uba(uba, rsp->add_info, rsp->add_info_len) != NCSCC_RC_SUCCESS)
            {
                m_MMGR_FREE_MIB_OCT(rsp->add_info); 
                rsp->add_info = NULL;
                rsp->add_info_len = 0;
                return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
            }
         }  
       }
   }
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_param_val_decode

  DESCRIPTION:
    Decode the passed USRBUF into a NCSMIB_PARAM_VAL.

  ARGUMENTS:
      mpv - Param space to put the collected parts of a parameter
      ma  - IFF
              (ma != NULL) Alloc OCTET-STRING answer in passed stack space
                           a la ncsmem_aid_alloc().
              (ma == NULL), get memory from HEAP via OS Services mem macro

            The issue here is, where does the invoker want the memory to
            come from? Strategy is driven by the duration of answer or goals
            of invoker.
      uba - the encoded NCSMIB_ARG is being decoded from this USRBUF 
             abstraction.

  RETURNS:
    NCSCC_RC_SUCCESS - The decode happened, and data is available
    NCSCC_RC_FAILURE - Usrbuf is entirely consumed, OR Something went wrong.

*****************************************************************************/
#define NCSMIB_BIG_OCT 200

uns32 ncsmib_param_val_decode( NCSMIB_PARAM_VAL* mpv,
                              NCSMEM_AID*       ma,
                              NCS_UBAID*        uba)
{
   uns8  space[NCSMIB_BIG_OCT];
   uns8* stream;

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32) + sizeof(uns8));
   mpv->i_param_id = ncs_decode_32bit(&stream);
   mpv->i_fmat_id  = ncs_decode_8bit(&stream);
   mpv->info.i_int = 0; /* maps to null terminated string for i_oct as well */
   ncs_dec_skip_space(uba,sizeof(uns32) + sizeof(uns8));

   switch (mpv->i_fmat_id)
   {
   case NCSMIB_FMAT_OCT:
      {
         uns8*     octets;

         stream        = ncs_dec_flatten_space(uba,space,sizeof(uns16));
         mpv->i_length = ncs_decode_16bit(&stream);
         ncs_dec_skip_space(uba,sizeof(uns16));

         if (mpv->i_length == 0)
           return NCSCC_RC_SUCCESS; /* Apparently it is an empty OCT value */

         if (ma == NULL) /* no 'managed' stack space came down in NCSMEM_AID */
         {
            /* the user wants us to allocate memory from the HEAP */
            if ((octets = m_MMGR_ALLOC_MIB_OCT(mpv->i_length)) == NULL)
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         else
         {
            /* the user passed us memory... is it enough? SMM changed here */
            if ((octets = ncsmem_aid_alloc(ma, mpv->i_length)) == NULL) 
               return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
         }
         mpv->info.i_oct = octets;
         /* Flatten it. If stream == octets, flatten used octets to fillin answer */
         stream = ncs_dec_flatten_space(uba, octets, mpv->i_length);
         if (stream != octets)            /* Data may be in right place already ! */
            memcpy(octets, stream, mpv->i_length);
         ncs_dec_skip_space(uba, mpv->i_length);

      }
      break;

   case NCSMIB_FMAT_INT:
   case NCSMIB_FMAT_BOOL:
      stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
      mpv->info.i_int = ncs_decode_32bit(&stream);
      ncs_dec_skip_space(uba,sizeof(uns32));
      break;

   case 0:
      /* i_fmat_id is 0 when we are dealing with FAILURE replies...
         where we might have a valid param_id only...*/
      break;

   default:
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsmib_inst_decode

  DESCRIPTION:
    Decode the passed USRBUF into a uns32 string.

  RETURNS:
    NCSCC_RC_SUCCESS - The decode did not go so well
    NCSCC_RC_FAILURE - Something went wrong.

*****************************************************************************/

uns32 ncsmib_inst_decode( uns32**          inst_ids,
                         uns32            inst_len,
                         NCS_UBAID*        uba)
{
   uns32 i;
   uns8  space[20];
   uns8* stream;

   if (inst_ids == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   if ((*inst_ids == NULL) && (inst_len != 0))
   {
      /* the user wants us allocate memory from the HEAP for this instance id value */
      *inst_ids = m_MMGR_ALLOC_MIB_INST_IDS(inst_len);

      if (*inst_ids == NULL)
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   }

   for (i = 0; i < inst_len; i++)
   {
      stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
      if (stream == NULL)
         return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

      *(*inst_ids + i) = ncs_decode_32bit(&stream);
      ncs_dec_skip_space(uba,sizeof(uns32));
   }

   return NCSCC_RC_SUCCESS;
}


/******************************************************************************
  H J P A R M _ A I D  Encode functions
******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_enc_init

  DESCRIPTION:
      Puts NCSPARM_AID in start state; set ptrs ttl_len/ttl_cnt

  RETURNS:
     NONE

*****************************************************************************/

void ncsparm_enc_init(NCSPARM_AID* pa)
{
   uns8*     stream;
   NCS_UBAID* uba = NULL;
   uns16     marker = PARM_ENC_SEQ_MARKER;

   if (pa == NULL)
   {
      m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return;
   }

   uba = &pa->uba;

   if (NCSCC_RC_SUCCESS != ncs_enc_init_space(uba))
   {
      m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return;
   }

   pa->cnt = 0;
   pa->len = 0;

   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
   {
      m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return;
   }
   ncs_encode_16bit(&stream,marker);
   ncs_enc_claim_space(uba,sizeof(uns16));

   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
   {
      m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return;
   }

   pa->p_cnt = (uns16*)stream;

   ncs_encode_16bit(&stream,pa->cnt);
   ncs_enc_claim_space(uba,sizeof(uns16));

   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
   {
      m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
      return;
   }

   pa->p_len = (uns16*)stream;

   ncs_encode_16bit(&stream,pa->len);
   ncs_enc_claim_space(uba,sizeof(uns16));

}

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_enc_int

  DESCRIPTION:
      Encodes a param_val of type integer.

  RETURNS:
     NCSCC_RC_SUCCESS 
     NCSCC_RC_FAILURE

*****************************************************************************/

uns32 ncsparm_enc_int(NCSPARM_AID* pa,NCSMIB_PARAM_ID id,uns32 val)
{
   NCSMIB_PARAM_VAL param;
   uns16           ub_len_before = 0;

   if (pa == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   m_NCS_MEMSET(&param,0,sizeof(param));
   param.i_param_id = id;
   param.i_fmat_id = NCSMIB_FMAT_INT;
   param.info.i_int = val;

   ub_len_before = (uns16)pa->uba.ttl;

   if (NCSCC_RC_SUCCESS != ncsmib_param_val_encode(&param,&pa->uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   pa->cnt++;
   pa->len = (uns16)(pa->len + (pa->uba.ttl - ub_len_before));

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_enc_oct

  DESCRIPTION:
      Encodes a param_val of type octetstring.

  RETURNS:
     NCSCC_RC_SUCCESS 
     NCSCC_RC_FAILURE

*****************************************************************************/

uns32 ncsparm_enc_oct(NCSPARM_AID* pa,NCSMIB_PARAM_ID id,uns16 len,uns8* octs)
{
   NCSMIB_PARAM_VAL param;
   uns16           ub_len_before = 0;

   if ((pa == NULL) || (octs == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   m_NCS_MEMSET(&param,0,sizeof(param));
   param.i_param_id = id;
   param.i_fmat_id  = NCSMIB_FMAT_OCT;
   param.i_length   = len;
   param.info.i_oct = octs;

   ub_len_before = (uns16)pa->uba.ttl;

   if (NCSCC_RC_SUCCESS != ncsmib_param_val_encode(&param,&pa->uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   pa->cnt++;
   pa->len = (uns16)(pa->len + (pa->uba.ttl - ub_len_before));

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_enc_param

  DESCRIPTION:
      Encodes a properly populated param_val structure.

  RETURNS:
     NCSCC_RC_SUCCESS 
     NCSCC_RC_FAILURE

*****************************************************************************/

uns32 ncsparm_enc_param(NCSPARM_AID* pa, NCSMIB_PARAM_VAL* val)
{
   uns16 ub_len_before = 0;

   if ((pa == NULL) || (val == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   ub_len_before = (uns16)pa->uba.ttl;

   if (NCSCC_RC_SUCCESS != ncsmib_param_val_encode(val,&pa->uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   pa->cnt++;
   pa->len = (uns16)(pa->len + (pa->uba.ttl - ub_len_before));

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_enc_done

  DESCRIPTION:
      Fills in ttl_len and ttl_cnt and return encoded USRBUF.

  RETURNS:
     NULL
     ...

*****************************************************************************/

USRBUF* ncsparm_enc_done(NCSPARM_AID* pa)
{
   if (pa == NULL)
   {
      m_LEAP_DBG_SINK((long)NULL);
      return (USRBUF*)NULL;
   }
   if ((pa->p_cnt == NULL) || (pa->p_len == NULL))
   {
      m_LEAP_DBG_SINK((long)NULL);
      return (USRBUF*)NULL;
   }

   ncs_encode_16bit((uns8**)&pa->p_cnt,pa->cnt);
   ncs_encode_16bit((uns8**)&pa->p_len,pa->len);

   return pa->uba.start;
}

/******************************************************************************
  H J P A R M _ A I D  Decode functions
******************************************************************************/

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_dec_init

  DESCRIPTION:
      Puts NCSPARM_AID in start state; returns #of params encoded.

  RETURNS:
     NCSCC_RC_SUCCESS 
     NCSCC_RC_FAILURE

*****************************************************************************/

uns32 ncsparm_dec_init(NCSPARM_AID* pa,USRBUF* ub)
{
   uns16     marker;
   uns8      space[10];
   uns8*     stream = NULL;
   NCS_UBAID* uba    = NULL;

   if ((pa == NULL) || (ub == NULL))
      return m_LEAP_DBG_SINK(0);

   uba = &pa->uba;

   ncs_dec_init_space(uba,ub);

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);

   marker = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));

   if (marker != PARM_ENC_SEQ_MARKER)
      return m_LEAP_DBG_SINK(0);

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);

   pa->cnt = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));

   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);

   pa->len = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));

   if (pa->len != (pa->uba.max - pa->uba.ttl))
      return m_LEAP_DBG_SINK(0);

   return pa->cnt;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsparm_dec_parm

  DESCRIPTION:
      Populates the passed NCSMIB_PARM_VAL with decoded stuff.

  ARGUMENTS:
      pa  - The PARAM USRBUF manager that keeps track of where we are in the 
            decode process.
      val - The place to put the answer of the decoded param.
      ma  - IFF
              (ma != NULL) Alloc OCTET-STRING answer in passed stack space
                           a la ncsmem_aid_alloc().
              (ma == NULL), get memory from HEAP via OS Services mem macro

            The issue here is, where does the invoker want the memory to
            come from? Strategy is driven by the duration of answer or goals
            of invoker. 

  RETURNS:
     NCSCC_RC_SUCCESS 
     NCSCC_RC_FAILURE

*****************************************************************************/

uns32 ncsparm_dec_parm(NCSPARM_AID* pa,NCSMIB_PARAM_VAL* val,NCSMEM_AID* ma)
{
   uns32 ret_code;
   uns16 ub_len_before = 0;

   if ((pa == NULL) || (val == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   ub_len_before = (uns16)pa->uba.ttl;

   ret_code = ncsmib_param_val_decode(val,ma,&pa->uba);

   if (ret_code != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   pa->cnt--;
   /* for decoding, ttl contains the count for how much data has been decoded so far */
   pa->len = (uns16)(pa->len - (pa->uba.ttl - ub_len_before));

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:   ncsparm_dec_done

  DESCRIPTION:
      Confirms all has been consumed and len & cnt checks align.

  RETURNS:
     NCSCC_RC_SUCCESS 
     NCSCC_RC_FAILURE

*****************************************************************************/

uns32 ncsparm_dec_done(NCSPARM_AID* pa)
{
   if ((pa->cnt == 0) && (pa->len == 0))
      return NCSCC_RC_SUCCESS;

   return NCSCC_RC_FAILURE;
}

/******************************************************************************
H J R O W _ A I D  Encode functions
******************************************************************************/

uns32 ncssetallrows_enc_init(NCSROW_AID* ra)
{
   uns8*     stream;
   NCS_UBAID* uba = NULL;
   uns16     marker = ROW_ENC_SEQ_MARKER;
   
   if (ra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   uba = &ra->uba;
   
   if (NCSCC_RC_SUCCESS != ncs_enc_init_space(uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ra->cnt = 0;
   ra->len = 0;
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_16bit(&stream,marker);
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ra->p_cnt = (uns16*)stream;
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ra->p_len = (uns16*)stream;
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_enc_init(NCSROW_AID* ra)
{
   uns8*     stream;
   NCS_UBAID* uba = NULL;
   NCSPARM_AID * pa;
   uns16     marker = ROW_ENC_SEQ_MARKER;
   uns16     ub_len_before = 0;
   
   if (ra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   uba = &ra->uba;
   pa  = &ra->parm;
   
   pa->cnt = 0;
   pa->len = 0;
   
   ub_len_before = (uns16)uba->ttl;
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_16bit(&stream,marker);
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->p_cnt = (uns16*)stream;
   
   ncs_encode_16bit(&stream,pa->cnt);
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->p_len = (uns16*)stream;
   
   ncs_encode_16bit(&stream,pa->len);
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   ra->len = (uns16)(ra->len + uba->ttl - ub_len_before);
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_enc_inst_ids(NCSROW_AID* ra, NCSMIB_IDX* idx)
{
   NCS_UBAID *   uba;
   NCSPARM_AID * pa;
   uns16        ub_len_before = 0;
   uns8 *       stream;
   
   if ((ra == NULL) || (idx == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   uba = &ra->uba;
   pa  = &ra->parm;
   
   ub_len_before = (uns16)uba->ttl;
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream, idx->i_inst_len);
   ncs_enc_claim_space(uba,sizeof(uns32));
   
   if (ncsmib_inst_encode(idx->i_inst_ids, idx->i_inst_len, uba) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->len = (uns16)(pa->len + uba->ttl - ub_len_before);
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_enc_int(NCSROW_AID* ra, NCSMIB_PARAM_ID id, uns32 val)
{
   NCSMIB_PARAM_VAL param;
   uns16           ub_len_before = 0;
   NCSPARM_AID *    pa;
   
   if (ra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa = &ra->parm;
   
   m_NCS_MEMSET(&param,0,sizeof(param));
   param.i_param_id = id;
   param.i_fmat_id = NCSMIB_FMAT_INT;
   param.info.i_int = val;
   
   ub_len_before = (uns16)ra->uba.ttl;
   
   if (NCSCC_RC_SUCCESS != ncsmib_param_val_encode(&param,&ra->uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->cnt++;
   pa->len = (uns16)(pa->len + (ra->uba.ttl - ub_len_before));
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_enc_oct(NCSROW_AID* ra, NCSMIB_PARAM_ID id, uns16 len, uns8* octs)
{
   NCSMIB_PARAM_VAL param;
   uns16           ub_len_before = 0;
   NCSPARM_AID *    pa;
   
   if ((ra == NULL) || (octs == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa = &ra->parm;
   
   m_NCS_MEMSET(&param,0,sizeof(param));
   param.i_param_id = id;
   param.i_fmat_id  = NCSMIB_FMAT_OCT;
   param.i_length   = len;
   param.info.i_oct = octs;
   
   ub_len_before = (uns16)ra->uba.ttl;
   
   if (NCSCC_RC_SUCCESS != ncsmib_param_val_encode(&param,&ra->uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->cnt++;
   pa->len = (uns16)(pa->len + ra->uba.ttl - ub_len_before);
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_enc_param(NCSROW_AID* ra, NCSMIB_PARAM_VAL* val)
{
   uns16 ub_len_before = 0;
   NCSPARM_AID * pa;
   
   if ((ra == NULL) || (val == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa = & ra->parm;
   ub_len_before = (uns16)ra->uba.ttl;
   
   if (NCSCC_RC_SUCCESS != ncsmib_param_val_encode(val,&ra->uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->cnt++;
   pa->len = (uns16)(pa->len + (ra->uba.ttl - ub_len_before));
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_enc_done(NCSROW_AID* ra)
{
   NCSPARM_AID * pa;
   
   if (ra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa = &ra->parm;
   if ((pa->p_cnt == NULL) || (pa->p_len == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ncs_encode_16bit((uns8**)&pa->p_cnt,pa->cnt);
   ncs_encode_16bit((uns8**)&pa->p_len,pa->len);
   
   ra->cnt++;
   ra->len = (uns16)(ra->len + pa->len);
   
   return NCSCC_RC_SUCCESS;
}


USRBUF* ncssetallrows_enc_done(NCSROW_AID* ra)
{
   if (ra == NULL)
   {
      m_LEAP_DBG_SINK((long)NULL);
      return (USRBUF*)NULL;
   }
   
   if ((ra->p_cnt == NULL) || (ra->p_len == NULL))
   {
      m_LEAP_DBG_SINK((long)NULL);
      return (USRBUF*)NULL;
   }
   
   ncs_encode_16bit((uns8**)&ra->p_cnt,ra->cnt);
   ncs_encode_16bit((uns8**)&ra->p_len,ra->len);
   
   return ra->uba.start;
}


/******************************************************************************
H J R O W _ A I D  Decode functions
******************************************************************************/

uns32 ncssetallrows_dec_init(NCSROW_AID* ra, USRBUF* ub)
{
   uns16     marker;
   uns8      space[10];
   uns8*     stream = NULL;
   NCS_UBAID* uba    = NULL;
   
   if ((ra == NULL) || (ub == NULL))
      return m_LEAP_DBG_SINK(0);
   
   uba = &ra->uba;
   
   ncs_dec_init_space(uba,ub);
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   marker = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   if (marker != ROW_ENC_SEQ_MARKER)
      return m_LEAP_DBG_SINK(0);
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   ra->cnt = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   ra->len = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   if (ra->len != (ra->uba.max - ra->uba.ttl))
      return m_LEAP_DBG_SINK(0);
   
   return ra->cnt;
}


uns32 ncsrow_dec_init(NCSROW_AID* ra)
{
   uns16     marker;
   uns8      space[10];
   uns8*     stream = NULL;
   NCS_UBAID* uba    = NULL;
   NCSPARM_AID* pa;
   uns16     ub_len_before = 0;
   
   if (ra == NULL)
      return m_LEAP_DBG_SINK(0);
   
   uba = &ra->uba;
   pa  = &ra->parm;
   ub_len_before = (uns16)uba->ttl;
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   marker = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   if (marker != ROW_ENC_SEQ_MARKER)
      return m_LEAP_DBG_SINK(0);
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   pa->cnt = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   pa->len = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   ra->len = (uns16)(ra->len - (uba->ttl - ub_len_before));
   return pa->cnt;
}


uns32 ncsrow_dec_inst_ids (NCSROW_AID* ra, NCSMIB_IDX* idx, NCSMEM_AID* ma)
{
   uns16 ub_len_before = 0;
   NCSPARM_AID * pa;
   NCS_UBAID   * uba;
   uns8 *       stream;
   uns8         space[10];
   
   if ((ra == NULL) || (idx == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ub_len_before = (uns16)ra->uba.ttl;
   pa = &ra->parm;
   uba = &ra->uba;
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   idx->i_inst_len = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));
   
   if (ncsmib_inst_decode((uns32**)&idx->i_inst_ids,idx->i_inst_len,uba) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   /* for decoding, ttl contains the count for how much data has been decoded so far */
   pa->len = (uns16)(pa->len - (ra->uba.ttl - ub_len_before));
   ra->len = (uns16)(ra->len - (ra->uba.ttl - ub_len_before));
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_dec_param(NCSROW_AID* ra, NCSMIB_PARAM_VAL* val, NCSMEM_AID* ma)
{
   uns32 ret_code;
   uns16 ub_len_before = 0;
   NCSPARM_AID * pa;
   
   if ((ra == NULL) || (val == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ub_len_before = (uns16)ra->uba.ttl;
   pa = &ra->parm;
   
   ret_code = ncsmib_param_val_decode(val,ma,&ra->uba);
   
   if (ret_code != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa->cnt--;
   /* for decoding, ttl contains the count for how much data has been decoded so far */
   pa->len = (uns16)(pa->len - (ra->uba.ttl - ub_len_before));
   ra->len = (uns16)(ra->len - (ra->uba.ttl - ub_len_before));
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsrow_dec_done(NCSROW_AID* ra)
{
   NCSPARM_AID * pa;
   
   if(ra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   pa = &ra->parm;
   ra->cnt--;
   
   if ((pa->cnt == 0) && (pa->len == 0))
      return NCSCC_RC_SUCCESS;
   
   return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
}


uns32 ncssetallrows_dec_done(NCSROW_AID* ra)
{
   if(ra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   if((ra->cnt == 0) && (ra->len == 0))
      return NCSCC_RC_SUCCESS;
   
   return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
}


/******************************************************************************
H J R E M R O W _ A I D  Encode functions
******************************************************************************/

uns32 ncsremrow_enc_init(NCSREMROW_AID* rra)
{
   uns8*     stream;
   NCS_UBAID* uba = NULL;
   uns16     marker = REMROW_ENC_SEQ_MARKER;
   
   if (rra == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   uba = &rra->uba;
   
   if (NCSCC_RC_SUCCESS != ncs_enc_init_space(uba))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   rra->cnt = 0;
   rra->len = 0;
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_16bit(&stream,marker);
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   rra->p_cnt = (uns16*)stream;
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   rra->p_len = (uns16*)stream;
   ncs_enc_claim_space(uba,sizeof(uns16));
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsremrow_enc_inst_ids(NCSREMROW_AID* rra, NCSMIB_IDX* idx)
{
   NCS_UBAID *   uba;
   uns16        ub_len_before = 0;
   uns8 *       stream;
   
   if ((rra == NULL) || (idx == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   
   uba = &rra->uba;
   
   ub_len_before = (uns16)uba->ttl;
   
   stream = ncs_enc_reserve_space(uba,sizeof(uns32));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   ncs_encode_32bit(&stream, idx->i_inst_len);
   ncs_enc_claim_space(uba,sizeof(uns32));
   
   if (ncsmib_inst_encode(idx->i_inst_ids, idx->i_inst_len, uba) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   rra->len = (uns16)(rra->len + uba->ttl - ub_len_before);
   rra->cnt++;
   
   return NCSCC_RC_SUCCESS;
}


USRBUF* ncsremrow_enc_done(NCSREMROW_AID* rra)
{
   if (rra == NULL)
   {
      m_LEAP_DBG_SINK((long)NULL);
      return (USRBUF*)NULL;
   }
   
   if ((rra->p_cnt == NULL) || (rra->p_len == NULL))
   {
      m_LEAP_DBG_SINK((long)NULL);
      return (USRBUF*)NULL;
   }
   
   ncs_encode_16bit((uns8**)&rra->p_cnt, rra->cnt);
   ncs_encode_16bit((uns8**)&rra->p_len, rra->len);
   
   return rra->uba.start;
}

/******************************************************************************
H J R E M R O W _ A I D  Decode functions
******************************************************************************/

uns32 ncsremrow_dec_init(NCSREMROW_AID* rra, USRBUF* ub)
{
   uns16     marker;
   uns8      space[10];
   uns8*     stream = NULL;
   NCS_UBAID* uba    = NULL;
   
   if ((rra == NULL) || (ub == NULL))
      return m_LEAP_DBG_SINK(0);
   
   uba = &rra->uba;
   
   ncs_dec_init_space(uba,ub);
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   marker = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   if (marker != REMROW_ENC_SEQ_MARKER)
      return m_LEAP_DBG_SINK(0);
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   rra->cnt = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns16));
   if (stream == NULL)
      return m_LEAP_DBG_SINK(0);
   
   rra->len = ncs_decode_16bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns16));
   
   if (rra->len != (rra->uba.max - rra->uba.ttl))
      return m_LEAP_DBG_SINK(0);
   
   return rra->cnt;
}


uns32 ncsremrow_dec_inst_ids(NCSREMROW_AID* rra, NCSMIB_IDX* idx, NCSMEM_AID* ma)
{
   uns16        ub_len_before = 0;
   NCS_UBAID   * uba;
   uns8 *       stream;
   uns8         space[10];
   
   if ((rra == NULL) || (idx == NULL))
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   ub_len_before = (uns16)rra->uba.ttl;
   uba = &rra->uba;
   
   stream = ncs_dec_flatten_space(uba,space,sizeof(uns32));
   idx->i_inst_len = ncs_decode_32bit(&stream);
   ncs_dec_skip_space(uba,sizeof(uns32));
   
   if (ncsmib_inst_decode((uns32**)&idx->i_inst_ids,idx->i_inst_len,uba) != NCSCC_RC_SUCCESS)
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
   
   /* for decoding, ttl contains the count for how much data has been decoded so far */
   rra->len = (uns16)(rra->len - (rra->uba.ttl - ub_len_before));
   rra->cnt--;
   
   return NCSCC_RC_SUCCESS;
}


uns32 ncsremrow_dec_done(NCSREMROW_AID* rra)
{
   if ((rra->cnt == 0) && (rra->len == 0))
      return NCSCC_RC_SUCCESS;
   
   return NCSCC_RC_FAILURE;
}



/**********************************************************************************/
/****************************************************************************
 *
 * Function Name: ncsmib_arg_free_resources
 *
 * Purpose:       Frees HEAP allocated NCSMIB_ARG and it's HEAP allocated parts
 *
 ****************************************************************************/

uns32 ncsmib_arg_free_resources(NCSMIB_ARG* arg, NCS_BOOL is_req)
{
   uns32 op;

   if (arg == NULL)
      return NCSCC_RC_SUCCESS;

   op = arg->i_op;

   if (arg->i_idx.i_inst_ids != NULL)
      m_MMGR_FREE_MIB_INST_IDS(arg->i_idx.i_inst_ids);

   /* KCQ:
   we can not always trust the value in arg->i_op,
   because could be mangled by the subsystem that an OAC called,
   where the subsystem modifies the request into a reply.
   */
   switch (is_req)
   {
   case TRUE:
      if (m_NCSMIB_ISIT_A_RSP(arg->i_op))
      {
         if (arg->i_op == NCSMIB_OP_RSP_CLI_DONE)
            op = NCSMIB_OP_REQ_CLI;
         else
            op = m_NCSMIB_RSP_TO_REQ(op);
      }
      break;
   case FALSE:
      if (m_NCSMIB_ISIT_A_REQ(arg->i_op))
         op = m_NCSMIB_REQ_TO_RSP(op);
      break;
   }

   switch (op)
   {
   /*****************************************************************************
    The Response Cases
    *****************************************************************************/
   case NCSMIB_OP_RSP_GET    :
   case NCSMIB_OP_RSP_SET    :
   case NCSMIB_OP_RSP_TEST   :
      {
         NCSMIB_GET_RSP* rsp = &arg->rsp.info.get_rsp;

         if ((rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT) &&
             (rsp->i_param_val.info.i_oct != NULL))
         {
            m_MMGR_FREE_MIB_OCT(rsp->i_param_val.info.i_oct);
         }
         break;
      }

   case NCSMIB_OP_RSP_NEXT :
      {
         NCSMIB_NEXT_RSP* rsp = &arg->rsp.info.next_rsp;

         if ((rsp->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT) &&
             (rsp->i_param_val.info.i_oct != NULL))
         {
            m_MMGR_FREE_MIB_OCT(rsp->i_param_val.info.i_oct);
         }

         if ((rsp->i_next.i_inst_len > 0) && (rsp->i_next.i_inst_ids != NULL))
         {
            m_MMGR_FREE_MIB_INST_IDS(rsp->i_next.i_inst_ids);
         }
         break;
      }

   case NCSMIB_OP_RSP_GETROW :
   case NCSMIB_OP_RSP_TESTROW:
   case NCSMIB_OP_RSP_SETROW :
   case NCSMIB_OP_RSP_MOVEROW:
   case NCSMIB_OP_RSP_GET_INFO:
   case NCSMIB_OP_RSP_SETALLROWS:
   case NCSMIB_OP_RSP_REMOVEROWS:
   case NCSMIB_OP_RSP_CLI:
   case NCSMIB_OP_RSP_CLI_DONE:
      {
         break;
      }

   case NCSMIB_OP_RSP_NEXTROW :
      {
         NCSMIB_NEXTROW_RSP* rsp = &arg->rsp.info.nextrow_rsp;
         if ((rsp->i_next.i_inst_len > 0) && (rsp->i_next.i_inst_ids != NULL))
         {
            m_MMGR_FREE_MIB_INST_IDS(rsp->i_next.i_inst_ids);
         }
         break;
      }

      /*****************************************************************************
       The Request Cases
       *****************************************************************************/

   case NCSMIB_OP_REQ_GET     :
   case NCSMIB_OP_REQ_NEXT    :
   case NCSMIB_OP_REQ_GETROW  :
   case NCSMIB_OP_REQ_NEXTROW :

      break;

   case NCSMIB_OP_REQ_SET  :
   case NCSMIB_OP_REQ_TEST :
      {
         NCSMIB_SET_REQ* req = &arg->req.info.set_req;

         if ((req->i_param_val.i_fmat_id == NCSMIB_FMAT_OCT) &&
             (req->i_param_val.info.i_oct != NULL))
         {
            m_MMGR_FREE_MIB_OCT(req->i_param_val.info.i_oct);
         }

         break;
      }

   case NCSMIB_OP_REQ_SETROW :
   case NCSMIB_OP_REQ_TESTROW:
   case NCSMIB_OP_REQ_MOVEROW:
   case NCSMIB_OP_REQ_GET_INFO:
   case NCSMIB_OP_REQ_SETALLROWS:
   case NCSMIB_OP_REQ_REMOVEROWS:
      {
         break;
      }

   case NCSMIB_OP_REQ_CLI:
      {
         NCSMIB_CLI_REQ* req = &arg->req.info.cli_req;
         if(req->i_usrbuf != NULL)
            m_MMGR_FREE_BUFR_LIST(req->i_usrbuf);
         req->i_usrbuf = NULL;

         break;
      }


   default:
      return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

   }
   switch (op)
   {
       case NCSMIB_OP_RSP_GET    :
       case NCSMIB_OP_RSP_SET    :
       case NCSMIB_OP_RSP_TEST   :
       case NCSMIB_OP_RSP_NEXT :
       case NCSMIB_OP_RSP_GETROW :
       case NCSMIB_OP_RSP_TESTROW:
       case NCSMIB_OP_RSP_SETROW :
       case NCSMIB_OP_RSP_MOVEROW:
       case NCSMIB_OP_RSP_GET_INFO:
       case NCSMIB_OP_RSP_SETALLROWS:
       case NCSMIB_OP_RSP_REMOVEROWS:
       case NCSMIB_OP_RSP_CLI:
       case NCSMIB_OP_RSP_CLI_DONE:
       case NCSMIB_OP_RSP_NEXTROW :
             if(arg->rsp.add_info_len != 0)
             {
                 if(arg->rsp.add_info != NULL)
                 {
                     m_MMGR_FREE_MIB_OCT(arg->rsp.add_info);
                     arg->rsp.add_info = NULL;
                     arg->rsp.add_info_len = 0;
                 }
             } 
             break;
       default:
             break;
   }

   m_MMGR_FREE_NCSMIB_ARG(arg);

   return NCSCC_RC_SUCCESS;
}

uns32
ncsmib_oid_extract(uns16 i_oid_length_in_ints, uns8 *i_oct, uns32 *o_obj_id) 
{
    uns8    *temp; 
    uns32   *p_int; 
    uns32   i; 
    
    if ((i_oct == NULL) || (o_obj_id == NULL))
        return NCSCC_RC_INVALID_INPUT; 
    
    /* decode the buffer */
    temp = i_oct; 
    p_int = o_obj_id;

    /* put in the host order */ 
    for (i = 0; i<i_oid_length_in_ints; i++)
        p_int[i] = ncs_decode_32bit(&temp);
    
    return NCSCC_RC_SUCCESS;

}
    

uns32
ncsmib_oid_put(uns16    i_oid_len_in_ints,    
               uns32    *i_oid, 
               uns8     *io_oid_buff, 
               NCSMIB_PARAM_VAL *io_param_val)
{
    uns32   i; 
    uns8    *temp_oid = io_oid_buff;

    if (i_oid == NULL)
        return NCSCC_RC_INVALID_INPUT; 

    if (io_oid_buff == NULL)
        return NCSCC_RC_INVALID_INPUT; 
        
    if (io_param_val == NULL)
        return NCSCC_RC_INVALID_INPUT; 

    /* encode the data in network order */ 
    for (i=0; i<i_oid_len_in_ints; i++)
        ncs_encode_32bit(&temp_oid, i_oid[i]);

    io_param_val->i_fmat_id = NCSMIB_FMAT_OCT; 
    io_param_val->i_length = i_oid_len_in_ints*4; 
    io_param_val->info.i_oct = io_oid_buff; 
    
    return NCSCC_RC_SUCCESS;
}

/******************************************************************************
 *  Name:          ncs_trap_eda_trap_varbinds_free
 *
 *  Description:   To free the trap varbinds recieved from the event 
 *                  buffer. 
 *                      - Free the memory allocated by the decode routines
 *                      - Handle care with memory for 
 *                        the ASN_OBJECT_ID type
 *                          - Mahesh TBD, MAB Enhancement 
 * 
 *  Arguments:     NCS_TRAP_VARBIND* -List of varbinds to be freed 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   - failure
 *  NOTE: 
 *****************************************************************************/
uns32
ncs_trap_eda_trap_varbinds_free(NCS_TRAP_VARBIND *i_trap_varbinds)
{
    NCS_TRAP_VARBIND     *prev, *current; 

    /* validate the inputs */
    if (i_trap_varbinds == NULL)
    {
        /* log the error, but nothing to free */
        return NCSCC_RC_SUCCESS; 
    }

    current = i_trap_varbinds; 

    while (current)
    {
        prev = current; 
       
        /* free the Octets */
        switch(prev->i_param_val.i_fmat_id)
        {
            case NCSMIB_FMAT_INT:
            /* Nothing to do */
            break; 
            case NCSMIB_FMAT_OCT:
            if (prev->i_param_val.info.i_oct != NULL)
            {
                m_NCS_MEM_FREE(prev->i_param_val.info.i_oct,
                               NCS_MEM_REGION_PERSISTENT,
                               NCS_SERVICE_ID_COMMON,2);
            }
            break; 
            default:
            /* log the error */
            break; 
        }

        if (prev->i_idx.i_inst_ids != NULL)
        {
            /* free the index */
            m_NCS_MEM_FREE(prev->i_idx.i_inst_ids,
                           NCS_MEM_REGION_PERSISTENT,
                           NCS_SERVICE_ID_COMMON,4);
        }

        /* go to the next node */
        current = current->next_trap_varbind; 
        
        /* free the varbind now */
        m_MMGR_NCS_TRAP_VARBIND_FREE(prev);
    } /* while */
    return NCSCC_RC_SUCCESS; 
}



/*****************************************************************************
 *****************************************************************************

  H J M I B _ A R G    P r e t t y   P r i n t 

  The functions within this #if (ENABLE_LEAP_DBG == 1) all contribute to 
  the pretty print of an NCSMIB_ARG.

 *****************************************************************************
 *****************************************************************************/

#if (ENABLE_LEAP_DBG == 1)

/*****************************************************************************

  PROCEDURE NAME:   mibpp_opstr

  DESCRIPTION:
      Get a string that maps to the MIB operation value.

  RETURNS:
     nothing

*****************************************************************************/

static char* mibpp_opstr(uns32 op)
{
   switch (op)
   {
   case NCSMIB_OP_RSP_GET     : return "GET Resp";      break;
   case NCSMIB_OP_RSP_SET     : return "SET Resp";      break;
   case NCSMIB_OP_RSP_TEST    : return "TEST Resp";     break;
   case NCSMIB_OP_RSP_NEXT    : return "GET NEXT Resp"; break;
   case NCSMIB_OP_RSP_GETROW  : return "GET ROW Resp";  break;
   case NCSMIB_OP_RSP_NEXTROW : return "NEXT ROW Resp"; break;
   case NCSMIB_OP_RSP_SETROW  : return "SET ROW Resp" ; break;
   case NCSMIB_OP_RSP_TESTROW : return "TEST ROW Resp"; break;
   case NCSMIB_OP_RSP_MOVEROW : return "MOVE ROW Resp"; break;
   case NCSMIB_OP_RSP_GET_INFO: return "GETINFO Resp" ; break;
   case NCSMIB_OP_RSP_REMOVEROWS: return "REMOVE ROWS Resp"; break;
   case NCSMIB_OP_RSP_SETALLROWS: return "SETALL ROWS Resp"; break;
   case NCSMIB_OP_RSP_CLI     : return "CLI Resp";      break;
   case NCSMIB_OP_RSP_CLI_DONE: return "CLI Resp, for wild-card req";      break;

   case NCSMIB_OP_REQ_GET     : return "GET Req";       break;
   case NCSMIB_OP_REQ_NEXT    : return "GET NEXT Req";  break;
   case NCSMIB_OP_REQ_SET     : return "SET Req";       break;
   case NCSMIB_OP_REQ_TEST    : return "TEST Req";      break;
   case NCSMIB_OP_REQ_GETROW  : return "GET ROW Req";   break;
   case NCSMIB_OP_REQ_NEXTROW : return "NEXT ROW Req";  break;
   case NCSMIB_OP_REQ_SETROW  : return "SET ROW Req" ;  break;
   case NCSMIB_OP_REQ_TESTROW : return "TEST ROW Req";  break;
   case NCSMIB_OP_REQ_MOVEROW : return "MOVE ROW Req";  break;
   case NCSMIB_OP_REQ_GET_INFO: return "GETINFO Req" ;  break;
   case NCSMIB_OP_REQ_REMOVEROWS: return "REMOVE ROWS Req"; break;
   case NCSMIB_OP_REQ_SETALLROWS: return "SETALL ROWS Req"; break;
   case NCSMIB_OP_REQ_CLI     : return "CLI Req";      break;
   }
   m_LEAP_DBG_SINK_VOID(0);  
   return "UNKNOWN OP";   /* effectively, the default */
}

/*****************************************************************************

  PROCEDURE NAME:   mibpp_inst

  DESCRIPTION:
      print out the sequence of uns32 sequence values

  RETURNS:
     nothing

*****************************************************************************/

static void mibpp_inst(const uns32* inst, uns32 len)
{
   uns32 i;

   m_NCS_CONS_PRINTF("Index vals as int: ");
   if (inst == NULL)
      m_NCS_CONS_PRINTF("NULL\n");
   else
   {
      for (i = 0; i < len; i++)
         m_NCS_CONS_PRINTF(", %d",inst[i]);
      m_NCS_CONS_PRINTF(" \n");
   }

   m_NCS_CONS_PRINTF("Index vals as ascii: ");
   if (inst == NULL)
      m_NCS_CONS_PRINTF("NULL\n");
   else
   {
      for (i = 0; i < len; i++)
         m_NCS_CONS_PRINTF(", %c",inst[i]);
      m_NCS_CONS_PRINTF(" \n");
   }
}


/*****************************************************************************

  PROCEDURE NAME:   mibpp_param_val

  DESCRIPTION:
      print out the content of the passed Param Val

  RETURNS:
     nothing

*****************************************************************************/

static const char* const pp_fmat_str[4] = { NULL, "Integer", "OctetStr", "Bool"};


static void mibpp_param_val(NCSMIB_PARAM_VAL* val)
{
   uns32 i;

   m_NCS_CONS_PRINTF("Param Val: ");
   m_NCS_CONS_PRINTF("Id %d, %s, Val ",val->i_param_id, pp_fmat_str[val->i_fmat_id]);
   if (val->i_fmat_id == NCSMIB_FMAT_INT || val->i_fmat_id == NCSMIB_FMAT_BOOL)
      m_NCS_CONS_PRINTF("%d\n",val->info.i_int );

   else if (val->i_fmat_id == NCSMIB_FMAT_OCT)
   {
      for (i = 0; i < val->i_length; i++)
         m_NCS_CONS_PRINTF(", 0x%x ",val->info.i_oct[i]);
      m_NCS_CONS_PRINTF(" \n");
   }
}

/*****************************************************************************

  PROCEDURE NAME:   mibpp_status

  DESCRIPTION:
     Pretty Print the Status value. These are the only legal values allowed 
     for NCSMIB_ARG.

  RETURNS:
     nothing

*****************************************************************************/

char*  mibpp_status(uns32 status)
{
   char* ptr = "Unknown MIB Error!!";
   switch (status)
   {
   case NCSCC_RC_SUCCESS:          ptr = "NCSCC_RC_SUCCESS";          break;
   case NCSCC_RC_NO_SUCH_TBL:      ptr = "NCSCC_RC_NO_SUCH_TBL";      break;     
   case NCSCC_RC_NO_OBJECT:        ptr = "NCSCC_RC_NO_OBJECT";        break;        
   case NCSCC_RC_NO_INSTANCE:      ptr = "NCSCC_RC_NO_INSTANCE";      break;      
   case NCSCC_RC_INV_VAL:          ptr = "NCSCC_RC_INV_VAL";          break;          
   case NCSCC_RC_INV_SPECIFIC_VAL: ptr = "NCSCC_RC_INV_SPECIFIC_VAL"; break; 
   case NCSCC_RC_REQ_TIMOUT:       ptr = "NCSCC_RC_REQ_TIMOUT";       break;       
   case NCSCC_RC_FAILURE:          ptr = "NCSCC_RC_FAILURE";          break;
   case NCSCC_RC_NO_ACCESS:        ptr = "NCSCC_RC_NO_ACCESS";        break;
   case NCSCC_RC_NO_CREATION:      ptr = "NCSCC_RC_NO_CREATION";      break;
   }
   return ptr;
}


/*****************************************************************************

  PROCEDURE NAME:   mibpp_row

  DESCRIPTION:
     Pretty Print encoded Param Vals.

  RETURNS:
     nothing

*****************************************************************************/

static void mibpp_row(USRBUF* inub)
{
  NCSPARM_AID      pa;
  NCSMIB_PARAM_VAL pv;
  USRBUF*         ub = NULL;

  m_NCS_CONS_PRINTF("ROW STARTS\n");

  if(inub == NULL)
    return;

  ub = m_MMGR_DITTO_BUFR(inub);

  if(ub == NULL)
    return;

  ncsparm_dec_init(&pa,ub);

  while((pa.cnt > 0) && (pa.len > 0))
    {
    ncsparm_dec_parm(&pa,&pv,NULL);
    
    mibpp_param_val(&pv);
    
    if((pv.i_fmat_id == NCSMIB_FMAT_OCT) && (pv.info.i_oct != NULL))
      m_MMGR_FREE_MIB_OCT(pv.info.i_oct);
    
    }

  if(ncsparm_dec_done(&pa) != NCSCC_RC_SUCCESS)
    {
    m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);
    return;
    }

  m_NCS_CONS_PRINTF("ROW ENDS\n");

  return;
}


/*****************************************************************************

  PROCEDURE NAME:   ncsmib_pp

  DESCRIPTION:
     Pretty Print NCSMIB_ARG structure when LEAP DEBUG flag is on.

  RETURNS:
     nothing

*****************************************************************************/

void ncsmib_pp( NCSMIB_ARG* arg)
{
   uns32 i;
   m_NCS_CONS_PRINTF("=================================\n");
   m_NCS_CONS_PRINTF("MIB_ARG  PP @ address %lx\n", (unsigned long)arg);
   m_NCS_CONS_PRINTF("=================================\n");
   m_NCS_CONS_PRINTF("Op: %s, Tbl Id: %d, Exch Id: %x\n", mibpp_opstr(arg->i_op),
                    arg->i_tbl_id, 
                    arg->i_xch_id);
   m_NCS_CONS_PRINTF("User Key: %lu\n", arg->i_usr_key);
   m_NCS_CONS_PRINTF("MIB Key: %lu\n", arg->i_mib_key);
   m_NCS_CONS_PRINTF("Source: %d\n", arg->i_policy);

   if (m_NCSMIB_ISIT_A_RSP(arg->i_op))
   {
      if (arg->rsp.i_status != 0)
          m_NCS_CONS_PRINTF("Status : %s\n", mibpp_status(arg->rsp.i_status));
      if (arg->rsp.i_status == NCSCC_RC_SUCCESS)
      {
         switch (arg->i_op)
         {
         /*****************************************************************************
         The Response Cases
         *****************************************************************************/
         case NCSMIB_OP_RSP_GET  :
         case NCSMIB_OP_RSP_SET  :
         case NCSMIB_OP_RSP_TEST :
            {
               NCSMIB_GET_RSP* rsp = &arg->rsp.info.get_rsp;
               mibpp_inst(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
               mibpp_param_val(&rsp->i_param_val);
               break;
            }

         case NCSMIB_OP_RSP_NEXT :
            {
               NCSMIB_NEXT_RSP* rsp  = &arg->rsp.info.next_rsp;
               mibpp_inst(rsp->i_next.i_inst_ids,rsp->i_next.i_inst_len);
               mibpp_param_val(&rsp->i_param_val);
               break;
            }

         case NCSMIB_OP_RSP_GETROW :
            {
               NCSMIB_GETROW_RSP* rsp = &arg->rsp.info.getrow_rsp;
               mibpp_inst(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
               mibpp_row(rsp->i_usrbuf);
               break;
            }

         case NCSMIB_OP_RSP_NEXTROW :
            {
               NCSMIB_NEXTROW_RSP* rsp  = &arg->rsp.info.nextrow_rsp;
               mibpp_inst(rsp->i_next.i_inst_ids,rsp->i_next.i_inst_len);
               mibpp_row(rsp->i_usrbuf);
               break;
            }

         case NCSMIB_OP_RSP_MOVEROW:
            {
               NCSMIB_MOVEROW_RSP* rsp = &arg->rsp.info.moverow_rsp;
               /* m_NCS_CONS_PRINTF("move_to vcard:%u\n",rsp->i_move_to.info.v1.vcard); */
               mibpp_row(rsp->i_usrbuf);
               break;
            }

         case NCSMIB_OP_RSP_SETROW:
         case NCSMIB_OP_RSP_SETALLROWS:
         case NCSMIB_OP_RSP_TESTROW:
         case NCSMIB_OP_RSP_REMOVEROWS:
            mibpp_inst(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);               
            break;
         
         case NCSMIB_OP_RSP_CLI:
         case NCSMIB_OP_RSP_CLI_DONE:
            {
               NCSMIB_CLI_RSP* rsp = &arg->rsp.info.cli_rsp;
               mibpp_inst(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
               if (rsp->o_answer != NULL)
                  mibpp_row(rsp->o_answer);
               break;
            }
         default:
            {
               m_NCS_CONS_PRINTF("!!! Invalid Operation Type !!!\n");
               m_LEAP_DBG_SINK_VOID(0);
               return;
            }
         }
      }
      else 
      {
          arg->i_op = m_NCSMIB_RSP_TO_REQ(arg->i_op); /* Converting response to corresponding reques op */
          ncsmib_req_pp(arg); 
          arg->i_op = m_NCSMIB_REQ_TO_RSP(arg->i_op); /* Converting request op to corresponding response op */
      }
      if(arg->rsp.add_info_len > 0){
          if(arg->rsp.add_info != NULL){
              for(i=0; i<arg->rsp.add_info_len; i++)
                 m_NCS_CONS_PRINTF("%c", arg->rsp.add_info[i]);
              m_NCS_CONS_PRINTF("\n");
          }  
      }  
   }
   else
   {
       /* code existing in this else part is moved to function ncsmib_req_pp while fixing the bug 60198 */
       ncsmib_req_pp(arg);
   }
}

void       ncsmib_req_pp(NCSMIB_ARG* arg)
{
      mibpp_inst(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len);
      switch (arg->i_op)
      {
      /*****************************************************************************
      The Request Cases
      *****************************************************************************/

      case NCSMIB_OP_REQ_GET  :
      case NCSMIB_OP_REQ_NEXT :
         {
            NCSMIB_GET_REQ* req = &arg->req.info.get_req;
            m_NCS_CONS_PRINTF("Param ID %d\n",req->i_param_id);
            break;
         }

      case NCSMIB_OP_REQ_SET  :
      case NCSMIB_OP_REQ_TEST :
         {
            NCSMIB_SET_REQ* req = &arg->req.info.set_req;
            mibpp_param_val(&req->i_param_val);
            break;
         }

      case NCSMIB_OP_REQ_GETROW  :
      case NCSMIB_OP_REQ_NEXTROW :
         {
            NCSMIB_GETROW_REQ* req = &arg->req.info.getrow_req;
            USE(req);
            /* nothing to do here... */ 
            break;
         }

      case NCSMIB_OP_REQ_SETROW  :
      case NCSMIB_OP_REQ_TESTROW :
         {
            NCSMIB_SETROW_REQ* req = &arg->req.info.setrow_req;
            mibpp_row(req->i_usrbuf);
            break;
         }

      case NCSMIB_OP_REQ_MOVEROW :
        {
            NCSMIB_MOVEROW_REQ* req = &arg->req.info.moverow_req;
            if (m_NCS_NODE_ID_FROM_MDS_DEST(req->i_move_to) == 0)
            {
                m_NCS_CONS_PRINTF("move_to VDEST:%lld\n",req->i_move_to);
            }
            else
            {
                m_NCS_CONS_PRINTF("move_to ADEST:%lld\n",req->i_move_to);
            }
            mibpp_row(req->i_usrbuf);
            break;
        }

      case NCSMIB_OP_REQ_CLI  :
         {
            NCSMIB_CLI_REQ* req = &arg->req.info.cli_req;
            if (req->i_usrbuf != NULL)
               mibpp_row(req->i_usrbuf);
            break;
         }

      case NCSMIB_OP_REQ_REMOVEROWS:
      case NCSMIB_OP_REQ_SETALLROWS:
          break;

      default:
         {
            m_NCS_CONS_PRINTF("!!! Invalid Operation Type !!!\n");
            m_LEAP_DBG_SINK_VOID(0);
            return;
         }
      }
}
#else

/*****************************************************************************

  LEAP DEBUG is not enabled; stub out the Pretty Print call

*****************************************************************************/

void ncsmib_pp( NCSMIB_ARG* arg)
{
   return;
}


#endif


