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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.1/base/products/mab/oac/oac_pvt.c 39    9/11/01 4:30p Questk $


..............................................................................

  DESCRIPTION:

    This has the private functions of the Object Access Client (OAC), 
    a subcomponent of the MIB Access Broker (MAB) subystem.This file 
    contains these groups of private functions

  - The master OAC dispatch loop functions
  - The OAC Table Record Manipulation Routines
  - The OAC Filter Manipulation routines  
  - OAC message handling routines 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"

#if (NCS_MAB == 1)

/*****************************************************************************

  The master OAC dispatch loop functions

  OVERVIEW:   The OAC instance or instances has one or more dedicated 
              threads. Each sits in an inifinite loop governed by these
              functions.

  PROCEDURE NAME: oac_do_evts & oac_do_evt

  DESCRIPTION:       

     oac_do_evts      Infinite loop services the passed SYSF_MBX
     oac_do_evt       Master Dispatch function and services off OAC work queue

*****************************************************************************/

/*****************************************************************************
   oac_do_evts
*****************************************************************************/

void
oac_do_evts( SYSF_MBX*  mbx)
{
  MAB_MSG   *msg;
  
  while ((msg = m_OAC_RCV_MSG(mbx, msg)) != NULL)
  {
    m_MAB_DBG_TRACE("\noac_do_evt():entered.");
    if(oac_do_evt(msg) == NCSCC_RC_DISABLED)
    {
      m_MAB_DBG_TRACE("\noac_do_evt():left.");
      return;
    }
    m_MAB_DBG_TRACE("\noac_do_evt():left.");
  }
}

/*****************************************************************************
   oac_do_evt
*****************************************************************************/

uns32
oac_do_evt( MAB_MSG* msg)
{  
  switch (msg->op)
  {
  case MAB_OAC_REQ_HDLR:
    return oac_mib_request(msg);
    
  case MAB_OAC_MAS_REFRESH:
    return oac_mas_fltrs_refresh(msg);

  case MAB_OAC_RSP_HDLR:
    return oac_mab_response(msg);
    
  case MAB_OAC_FIR_HDLR:
    return oac_free_idx_request(msg);

  case MAB_OAC_DEF_REQ_HDLR:
    return oac_sa_mib_request(msg);

  case MAB_OAC_PLAYBACK_START:
    return oac_playback_start(msg);

  case MAB_OAC_PLAYBACK_END:
      return oac_playback_end(msg);

  case MAB_OAC_PSS_ACK:
      return oac_handle_pss_ack(msg);

  case MAB_OAC_PSS_EOP_EVT:
      return oac_handle_pss_eop_ind(msg);

  case MAB_OAC_DESTROY:
      m_MMGR_FREE_MAB_MSG(msg);
      return oaclib_oac_destroy(NULL);

  case MAB_OAC_SVC_MDS_EVT:
      return oac_handle_svc_mds_evt(msg);

  default:
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_RCVD_INVALID_OP);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    
  }
  return NCSCC_RC_SUCCESS; /* SMM revisit this return.. */
}


/*****************************************************************************

  The OAC Table Record Manipulation Routines: 

  OVERVIEW:       The OAC Tabke Rocord Manipulation Routines oversee a hash table
                  of registered mib tables.

  PROCEDURE NAME: oac_tbl_rec_init & oac_tbl_rec_clr & 
                  oac_tbl_rec_put  & oac_tbl_rec_rmv & oac_tbl_rec_find

  DESCRIPTION:       

    oac_tbl_rec_init  - Put a OAC_TBL hash table in start state.
    oac_tbl_rec_clr   - remove all table records from hash table. Free all 
                        resources.

    oac_tbl_rec_put   - put a table record in the table record hash table.
    oac_tbl_rec_rmv   - remove a table record from the table record hash table.
    oac_tbl_rec_find  - find (but don't remove) a table record


*****************************************************************************/

/*****************************************************************************
  oac_tbl_rec_init
*****************************************************************************/

void oac_tbl_rec_init(OAC_TBL* inst)
{
  uns32 i;
  
  for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
    inst->hash[i] = NULL;
  
}

/*****************************************************************************
  oac_tbl_rec_clr
*****************************************************************************/

void oac_tbl_rec_clr(OAC_TBL* inst)
{
  uns32        i;
  OAC_TBL_REC* tbl_rec;
  
  for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
  {
    while((tbl_rec = inst->hash[i]) != NULL)
    {
      inst->hash[i] = tbl_rec->next;
      /* need to free the heap tbl rec fields ...  */
      m_MMGR_FREE_OAC_TBL_REC(tbl_rec);
    }
  }
}


/*****************************************************************************
  oac_tbl_rec_put
*****************************************************************************/

void oac_tbl_rec_put(OAC_TBL* inst, OAC_TBL_REC* tbl_rec)
{
  uns32 bkt = tbl_rec->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
  
  tbl_rec->next = inst->hash[bkt];
  inst->hash[bkt] = tbl_rec;
}

/*****************************************************************************
  oac_tbl_rec_rmv
*****************************************************************************/

OAC_TBL_REC* oac_tbl_rec_rmv(OAC_TBL* inst, uns32 tbl_id)
{
  OAC_TBL_REC*   tbl_rec;
  OAC_TBL_REC*   ret = NULL;
  uns32          bkt = tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;

  tbl_rec = inst->hash[bkt]; 
  
  if(tbl_rec != NULL)
  {
    if(tbl_rec->tbl_id == tbl_id)
    {
      inst->hash[bkt] = tbl_rec->next;  
      ret = tbl_rec;
    }
    else
    {
      while(tbl_rec->next != NULL)
      {
        if (tbl_rec->next->tbl_id == tbl_id)
        {
          ret = tbl_rec->next;
          tbl_rec->next = tbl_rec->next->next;
          return ret;
        }
        tbl_rec = tbl_rec->next;
      }
    }
  }
  
  return ret;
}

/*****************************************************************************
  oac_tbl_rec_find
*****************************************************************************/

OAC_TBL_REC* oac_tbl_rec_find(OAC_TBL* inst, uns32 tbl_id)
{
  OAC_TBL_REC*   tbl_rec;
  OAC_TBL_REC*   ret = NULL;
  uns32          bkt = tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
  
  tbl_rec = inst->hash[bkt]; 
  
  if(tbl_rec != NULL)
  {
    if(tbl_rec->tbl_id == tbl_id)
    {
      ret = tbl_rec;
    }
    else
    {
      while(tbl_rec->next != NULL)
      {
        if (tbl_rec->next->tbl_id == tbl_id)
        {
          ret = tbl_rec->next;
          return ret;
        }
        tbl_rec = tbl_rec->next;
      }
    }
  }
  
  return ret;
}


/********************************************************************************

  OVERVIEW:       The OAC Filter Manipulation routines

  PROCEDURE NAME: oac_fltr_put & oac_fltr_rmv & oac_fltr_find & oac_fltr_create
                  oac_fltr_reg_xmit & oac_fltr_unreg_xmit

  DESCRIPTION:       

    oac_fltr_put       - splice filter on to filter list
    oac_fltr_rmv       - find a filter based on ID and splice out of filter list
    oac_fltr_find      - find a filter based on ID. Return record.
    oac_fltr_create    - create and properly populate an OAC_FLTR object
    oac_fltr_reg_xmit  - Build a MAB_MSG to register MIB info w/MAS; MDS Send
    oac_fltr_unreg_xmit- Build a MAB_MSG to unregister MIB info w/MAS; MDS Send

    oac_sync_fltrs_with_mas - Sync back up with MAS with local filter ownership.
  
 ********************************************************************************/

/*****************************************************************************
  oac_fltr_put
*****************************************************************************/

void oac_fltr_put(OAC_FLTR** head, OAC_FLTR* new_fltr)
{
  if(head == NULL)
    {
    m_MAB_DBG_VOID;
    return;
    }
  
  new_fltr->next = *head;
  *head          = new_fltr;
}

/*****************************************************************************
  oac_fltr_rmv
*****************************************************************************/

OAC_FLTR* oac_fltr_rmv(OAC_FLTR** head, uns32 fltr_id)
{
  OAC_FLTR* fltr;
  OAC_FLTR* ret = NULL;
  
  if(head == NULL)
  {
    return (OAC_FLTR*) m_MAB_DBG_SINK((uns32)NULL);
  }
  
  fltr = *head;
  
  
  if(fltr != NULL)
  {
    if(fltr->fltr_id == fltr_id)
    {
      *head = fltr->next;
      ret = fltr;
    }
    else
    {
      while(fltr->next != NULL)
      {
        if (fltr->next->fltr_id == fltr_id)
        {
          ret = fltr->next;
          fltr->next = fltr->next->next;
          return ret;
        }
        fltr = fltr->next;
      }
    }
  }
  
  return ret;
}

/*****************************************************************************
  oac_fltr_find
*****************************************************************************/

OAC_FLTR* oac_fltr_find(OAC_FLTR** head, uns32 fltr_id)
{
  OAC_FLTR* fltr;
  OAC_FLTR* ret = NULL;
  
  if(head == NULL)
  {
    return (OAC_FLTR*) m_MAB_DBG_SINK((uns32)NULL);
  }
  
  fltr = *head;
  
  if(fltr != NULL)
  {
    if(fltr->fltr_id == fltr_id)
    {
      ret = fltr;
    }
    else
    {
      while(fltr->next != NULL)
      {
        if (fltr->next->fltr_id == fltr_id)
        {
          ret = fltr->next;
          return ret;
        }
        fltr = fltr->next;
      }
    }
  }
  
  return ret;
}

/*****************************************************************************
  oac_fltr_create
*****************************************************************************/
OAC_FLTR* oac_fltr_create(OAC_TBL* inst,NCSMAB_FLTR* mab_fltr)
  {  
  uns32        status = NCSCC_RC_FAILURE; 
  OAC_FLTR*    ret = NULL;
  
  ret = m_MMGR_ALLOC_OAC_FLTR;
  if(ret == NULL)
  {
    m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAC_FLTR_CREATE_FAILED, 
                      "oac_fltr_create()"); 
    return (OAC_FLTR*) m_MAB_DBG_SINK((uns32)NULL);
  }
  
  m_NCS_OS_MEMSET(ret,0,sizeof(OAC_FLTR));
  
  /* nxt_fltr_id needs to be protected by a lock */
  inst->nxt_fltr_id++;
  ret->fltr_id = inst->nxt_fltr_id;
 
  m_NCS_OS_MEMCPY(&(ret->fltr),mab_fltr,sizeof(NCSMAB_FLTR));
 
  switch (ret->fltr.type)
   {
       case NCSMAB_FLTR_RANGE:
       status = mas_mab_range_fltr_clone(&mab_fltr->fltr.range, &ret->fltr.fltr.range);
       break; 

       case NCSMAB_FLTR_EXACT:
       status = mas_mab_exact_fltr_clone(&mab_fltr->fltr.exact, &ret->fltr.fltr.exact);
       break; 

       case NCSMAB_FLTR_ANY:
       case NCSMAB_FLTR_SAME_AS:
       case NCSMAB_FLTR_DEFAULT:
       status = NCSCC_RC_SUCCESS;
       break; 
               
       case NCSMAB_FLTR_SCALAR:
       default:
       status = NCSCC_RC_FAILURE;
       break; 
    }
   if (status != NCSCC_RC_SUCCESS)
   {
       m_MMGR_FREE_OAC_FLTR(ret);
       return (OAC_FLTR*) m_MAB_DBG_SINK((uns32)NULL);
   }
  
  return ret;
  }

OAC_FLTR*
oac_fltrs_exact_fltr_locate(OAC_FLTR *fltr_list, NCSMAB_EXACT *exact)
{
    OAC_FLTR    *fltr; 
    if (fltr_list == NULL)
        return NULL; 
    if (exact == NULL)
        return NULL; 

    for(fltr=fltr_list; fltr!=NULL; fltr=fltr->next)
    {
        if ((exact->i_bgn_idx == fltr->fltr.fltr.exact.i_bgn_idx) &&
            (exact->i_idx_len == fltr->fltr.fltr.exact.i_idx_len) &&
            (m_NCS_OS_MEMCMP(exact->i_exact_idx,
                    fltr->fltr.fltr.exact.i_exact_idx,
                    (fltr->fltr.fltr.exact.i_idx_len) * sizeof(uns32)) == 0))
        {
            return fltr; 
        }
    }   
    return NULL; 
}
/*****************************************************************************
  oac_fltr_reg_xmit
*****************************************************************************/

uns32 oac_fltr_reg_xmit(OAC_TBL* inst,OAC_FLTR* fltr,uns32 tbl_id)
  {
  MAB_MSG  msg;
  uns32    code;

  m_MAB_DBG_TRACE("\noac_fltr_reg_xmit():entered.");

  if(inst->mas_here == TRUE)
    { 
    msg.vrid                  = inst->vrid;

    /* by setting fr_card & fr_svc = 0, MDS enc/dec/cpy funcs fill in fields */
    m_NCS_MEMSET(&msg.fr_card, 0, sizeof(msg.fr_card));
    msg.fr_svc                = 0;
    msg.fr_anc                = inst->my_anc;
    msg.op                    = MAB_MAS_REG_HDLR;
    msg.data.data.reg.fltr_id = fltr->fltr_id;
    m_NCS_OS_MEMCPY(&(msg.data.data.reg.fltr),&(fltr->fltr),sizeof(NCSMAB_FLTR));
    msg.data.data.reg.tbl_id  = tbl_id;

    /* Send the message */
    code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_MAS,
                       inst->mas_vcard);

    if (code == NCSCC_RC_SUCCESS)
      {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,
                         MAB_HDLN_OAC_SNT_FLTR_TO_MAS);
      }
    else
      {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_SCHD_FLTR_RESEND);
      m_MAB_DBG_TRACE("\noac_fltr_reg_xmit():left.");
      return NCSCC_RC_SUCCESS;
      }

    }
  else
    {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_SCHD_FLTR_RESEND);
    }
  
  m_MAB_DBG_TRACE("\noac_fltr_reg_xmit():left.");
  return NCSCC_RC_SUCCESS;
  }

/*****************************************************************************
  oac_fltr_unreg_xmit
*****************************************************************************/

uns32 oac_fltr_unreg_xmit(OAC_TBL* inst,uns32 fltr_id,uns32 tbl_id)
{
  MAB_MSG      msg;
  OAC_TBL_REC* tbl;
  OAC_FLTR*    fltr;
  uns32        code;

  m_MAB_DBG_TRACE("\noac_fltr_unreg_xmit():entered.");
  
  /* find the OAC filter */
  if((tbl = oac_tbl_rec_find(inst,tbl_id)) == NULL)
  {
    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          MAB_OAC_FLTR_REG_NO_TBL,
                          inst->vrid, 
                          tbl_id);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  if(inst->mas_here == TRUE)
  { 
    msg.vrid = inst->vrid;
    /* by setting fr_card and fr_svc to 0, 
    we allow MDS enc/dec/cpy functions fill in these fields 
    */
    m_NCS_MEMSET(&msg.fr_card, 0, sizeof(msg.fr_card));
    msg.fr_svc  = 0;
    msg.fr_anc  = inst->my_anc;
    msg.op = MAB_MAS_UNREG_HDLR;
    msg.data.data.unreg.fltr_id = fltr_id;
    msg.data.data.unreg.tbl_id = tbl_id;

    /* Send the message */
#if 0
    code = m_NCS_MDS_SEND(inst->mds_hdl,
      NCSMDS_SVC_ID_OAC,
      &msg,
      inst->mas_vcard,
      NCSMDS_SVC_ID_MAS, NULL);
#endif
    code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_MAS,
                       inst->mas_vcard);
  } 

  if((fltr_id != 0) && (fltr_id != 1))
    {
    if((fltr = oac_fltr_rmv(&(tbl->fltr_list),fltr_id)) == NULL)
      {
      m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, 
                             MAB_OAC_FLTR_RMV_FAILED, tbl->tbl_id,
                             fltr_id, -1);
      
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }
    
    mas_mab_fltr_indices_cleanup(&fltr->fltr); 
    m_MMGR_FREE_OAC_FLTR(fltr);
    }

  
  m_MAB_DBG_TRACE("\noac_fltr_unreg_xmit():left.");
  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  oac_sync_fltrs_with_mas
*****************************************************************************/

void oac_sync_fltrs_with_mas(OAC_TBL* inst )
{
    MAB_MSG      msg;
    OAC_TBL_REC* tbl_rec;
    OAC_FLTR*    fltr;
    uns32        code      = NCSCC_RC_FAILURE;
    uns32        index;

    m_MAB_DBG_TRACE("\noac_sync_fltrs_with_mas():entered.");

    for(index = 0;index < MAB_MIB_ID_HASH_TBL_SIZE; index++)
    {
        for(tbl_rec = inst->hash[index]; tbl_rec != NULL; tbl_rec = tbl_rec->next)
        {
            /* sync the default filter also, if it is registered */
            if (tbl_rec->dfltr_regd == TRUE)
            {
                OAC_FLTR    def_fltr;
                m_NCS_OS_MEMSET(&def_fltr,0,sizeof(fltr));
                def_fltr.fltr.type = NCSMAB_FLTR_DEFAULT;
                code = oac_fltr_reg_xmit(inst,&def_fltr,tbl_rec->tbl_id);
            }
            for(fltr = tbl_rec->fltr_list; fltr != NULL; fltr = fltr->next)
            {
                /* mas_here needs to be protected by a lock */
                if(inst->mas_here == FALSE)
                {
                    m_MAB_DBG_TRACE("\noac_sync_fltrs_with_mas():left.");
                    return;
                }

                msg.vrid = inst->vrid;
                /* by setting fr_card and fr_svc to 0, 
                we allow MDS enc/dec/cpy functions fill in these fields 
                */
                m_NCS_MEMSET(&msg.fr_card, 0, sizeof(msg.fr_card));
                msg.fr_svc                = 0;
#if 0
                msg.fr_anc                = inst->my_anc;
#endif
                msg.op                    = MAB_MAS_REG_HDLR;
                msg.data.data.reg.fltr_id = fltr->fltr_id;
                msg.data.data.reg.tbl_id  = tbl_rec->tbl_id;
                m_NCS_OS_MEMCPY(&(msg.data.data.reg.fltr),&(fltr->fltr),sizeof(NCSMAB_FLTR));

                /* Send the message */
                code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_MAS,
                                 inst->mas_vcard);
            } /* end of for(fltr = tbl_rec->fltr_list; fltr != NULL; fltr = fltr->next) */
         } /* end of for(tbl_rec = inst->hash[i];tbl_rec != NULL; tbl_rec = tbl_rec->next) */
    } /* end of for(index = 0;index < MAB_MIB_ID_HASH_TBL_SIZE; index++)  */
    m_MAB_DBG_TRACE("\noac_sync_fltrs_with_mas():left.");
}

/****************************************************************************

  OVERVIEW:       OAC message handling  routines

  PROCEDURE NAME: oac_mib_response & oac_mib_request & oac_mas_flts_refresh
                  oac_mab_response & oac_free_idx_request & oac_sa_mib_request 
                  

  DESCRIPTION:       

    oac_mib_response        - Pop NCSMIB_ARG stack and direct back to initiator
    oac_mib_request         - MDS Rcvd & forwarded request to OAC
    oac_mas_fltrs_refresh   - Stub code for now        SMM ISSUE 
    oac_mab_response        - MDS Rcvd & forwarded response to OAC

*****************************************************************************/

/*****************************************************************************
   oac_mib_response
*****************************************************************************/

static uns32 oac_mib_response(NCSMIB_ARG* rsp)
  {
  MAB_MSG     msg;
  OAC_TBL*    inst;
  NCS_SE*      se;
  uns8*       stream;
  uns32       code;
  MDS_DEST    dst_vcard;
  uns16       dst_svc_id = 0;
  uns16       vrid       = 0;
  uns32       hm_hdl = (uns32)rsp->i_usr_key;
  uns8        forward = FALSE;
  MAB_MSG     msg2;

  m_OAC_LK_INIT;

  m_MAB_DBG_TRACE("\noac_mib_response():entered.");

  if((inst = (OAC_TBL*)m_OAC_VALIDATE_HDL(hm_hdl)) == NULL) 
    {
    m_LOG_MAB_NO_CB("oac_mib_response()"); 
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
    }
  
  m_OAC_LK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);

  /* Pop the stack and check if this response needs to be forwarded to the PSR */
  se = ncsstack_pop(&rsp->stack);
  if (se == NULL)
  {
     m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_FLAG_FAILED);
     m_OAC_UNLK(&inst->lock);
     m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
     ncshm_give_hdl(hm_hdl);
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);              
  }
  
  stream = m_NCSSTACK_SPACE(se);
  if(se->type != NCS_SE_TYPE_FORWARD_TO_PSR)
  {
     m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_XSE);
     m_OAC_UNLK(&inst->lock);
     m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
     ncshm_give_hdl(hm_hdl);
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
  }
  
  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_POP_FLAG_SUCCESS);
  forward = ncs_decode_8bit(&stream);
  
  if((se = ncsstack_pop(&rsp->stack)) == NULL)
    {
    ncshm_give_hdl(hm_hdl);
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_SE_FAILED);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);              
    }
  
  stream = m_NCSSTACK_SPACE(se);
  
  if(se->type != NCS_SE_TYPE_BACKTO)
    {
    ncshm_give_hdl(hm_hdl);
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_XSE);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_POP_BTSE_SUCCESS);

  mds_st_decode_mds_dest(&stream, &dst_vcard);
  dst_svc_id = ncs_decode_16bit(&stream);
  vrid       = ncs_decode_16bit(&stream);

  /* KCQ: I'm paranoid ;-]  */
  if(inst->vrid != vrid)
    {
    m_LOG_MAB_ERROR_II(NCSFL_LC_DATA, NCSFL_SEV_ERROR, MAB_OAC_ERR_ENV_ID_MM,
                    inst->vrid, vrid);    
    ncshm_give_hdl(hm_hdl);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
    }

  msg.vrid    = vrid;

  m_NCS_MEMSET(&msg.fr_card, 0, sizeof(msg.fr_card));
  msg.fr_svc  = 0;
  msg.fr_anc  = inst->my_anc;

  switch(dst_svc_id)
    {
    case NCSMDS_SVC_ID_MAC:
      msg.op = MAB_MAC_RSP_HDLR;
      break;
    case NCSMDS_SVC_ID_MAS:
      msg.op = MAB_MAS_RSP_HDLR;
      break;
    default:
      m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_OAC_ERR_RSP_UNKNWN_SVC_ID, dst_svc_id);    
      ncshm_give_hdl(hm_hdl);
      m_OAC_UNLK(&inst->lock);
      m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);    
    }

  msg.data.data.snmp = rsp;

  /* Send the message */
    {    
    ncshm_give_hdl(hm_hdl);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
#if 0
    code = m_NCS_MDS_SEND(inst->mds_hdl,
                         NCSMDS_SVC_ID_OAC,
                         &msg,
                         dst_vcard,
                        (SS_SVC_ID)dst_svc_id, NULL);
#endif
    code = mab_mds_snd(inst->mds_hdl, &msg, NCSMDS_SVC_ID_OAC, dst_svc_id, dst_vcard);
    if (code != NCSCC_RC_SUCCESS)
      {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_FRWD_PSR_FAILED);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
      }

    if ((rsp->i_tbl_id >= NCSMIB_TBL_PSR_START) &&
       (rsp->i_tbl_id <= NCSMIB_TBL_PSR_END))
    {
       /* Is this check required? TBD */
       goto cleanup;
    }

    if ((forward == TRUE) &&
       (rsp->rsp.i_status == NCSCC_RC_SUCCESS) &&
       /* Ensure that playback session requests are not re-forwarded to PSSv */
       (inst->playback_session == FALSE) &&
       ((rsp->i_op == NCSMIB_OP_RSP_SET) ||
       (rsp->i_op == NCSMIB_OP_RSP_SETROW) ||
       (rsp->i_op == NCSMIB_OP_RSP_MOVEROW) ||
       (rsp->i_op == NCSMIB_OP_RSP_SETALLROWS) ||
       (rsp->i_op == NCSMIB_OP_RSP_REMOVEROWS)))
    {
       /* Forward this message to the PSR */
       m_NCS_MEMSET(&msg2, 0, sizeof(msg2));

       msg2.data.data.snmp = rsp;
       msg2.vrid    = vrid;
       msg2.fr_anc  = inst->my_anc;
       msg2.op      = MAB_PSS_SET_REQUEST;

       msg2.data.seq_num = inst->seq_num;
       ++ inst->seq_num;
       /* Add this message to the buffer zone. */
       /* Alloc msg_ptr, and duplicate arg->info.push_mibarg_data.arg here. */
       oac_psr_add_to_buffer_zone(inst, &msg2);

       if((inst->psr_here == TRUE) && (inst->is_active == TRUE))
       {
          code = mab_mds_snd(inst->mds_hdl, &msg2, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS,
                          inst->psr_vcard);
          if (code != NCSCC_RC_SUCCESS)
          {
             m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR,MAB_HDLN_OAC_SENDTO_PSR_FAILED, msg2.data.seq_num);
             return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
          }
          m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG,MAB_HDLN_OAC_SENDTO_PSR_SUCCESS, msg2.data.seq_num);
       }
    }

cleanup:

    switch(rsp->i_op)
      {
      case NCSMIB_OP_RSP_GETROW :
      case NCSMIB_OP_RSP_SETROW :
      case NCSMIB_OP_RSP_TESTROW:
      case NCSMIB_OP_RSP_SETALLROWS:
      case NCSMIB_OP_RSP_REMOVEROWS:
        m_MMGR_FREE_BUFR_LIST(rsp->rsp.info.getrow_rsp.i_usrbuf);
        rsp->rsp.info.getrow_rsp.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_NEXTROW :
        m_MMGR_FREE_BUFR_LIST(rsp->rsp.info.nextrow_rsp.i_usrbuf);
        rsp->rsp.info.nextrow_rsp.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_MOVEROW:
        m_MMGR_FREE_BUFR_LIST(rsp->rsp.info.moverow_rsp.i_usrbuf);
        rsp->rsp.info.moverow_rsp.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_CLI:
      case NCSMIB_OP_RSP_CLI_DONE:
        m_MMGR_FREE_BUFR_LIST(rsp->rsp.info.cli_rsp.o_answer);
        rsp->rsp.info.cli_rsp.o_answer = NULL;
        break;

      default:
        break;
      }
    }
    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_FRWD_RSP_SUCCESS);
    m_MAB_DBG_TRACE("\noac_mib_response():left.");

    return NCSCC_RC_SUCCESS;
 }


/*****************************************************************************
   oac_mib_request
*****************************************************************************/

uns32 oac_mib_request(MAB_MSG* msg)
{
  OAC_TBL*     inst;
  OAC_TBL_REC* tbl_rec;
  NCSMIB_ARG*  mib_req;
  uns32        status = NCSCC_RC_FAILURE; 

  m_OAC_LK_INIT;

  m_MAB_DBG_TRACE("\noac_mib_request():entered.\n");
  
  if((inst = (OAC_TBL*)msg->yr_hdl) == NULL)
  {
    m_LOG_MAB_NO_CB("oac_mib_request()"); 
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
  }

  m_OAC_LK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);

  mib_req = msg->data.data.snmp;

  if((tbl_rec = oac_tbl_rec_find(inst, mib_req->i_tbl_id)) == NULL)
  {
    NCS_SE*      se;
    uns8*       stream;
    MDS_DEST    dst_vcard;
    uns16       dst_svc_id = 0;
    uns16       vrid       = 0;
    uns32       code;

    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          MAB_OAC_FLTR_REG_NO_TBL,
                          inst->vrid, 
                          mib_req->i_tbl_id);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    /* send "no such table" error message back to the msg sender */

    if((se = ncsstack_pop(&mib_req->stack)) == NULL)
    {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_SE_FAILED);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);              
    }

    stream = m_NCSSTACK_SPACE(se);

    if(se->type != NCS_SE_TYPE_BACKTO)
    {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_XSE);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_POP_BTSE_SUCCESS);

    mds_st_decode_mds_dest(&stream, &dst_vcard);
    dst_svc_id = ncs_decode_16bit(&stream);
    vrid       = ncs_decode_16bit(&stream);

    msg->vrid    = vrid;

    m_NCS_MEMSET(&msg->fr_card, 0, sizeof(msg->fr_card));
    msg->fr_svc  = 0;
    msg->fr_anc  = inst->my_anc;

    switch(dst_svc_id)
    {
    case NCSMDS_SVC_ID_MAC:
      msg->op = MAB_MAC_RSP_HDLR;
      break;
    case NCSMDS_SVC_ID_MAS:
      msg->op = MAB_MAS_RSP_HDLR;
      break;
    default:
      m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, 
                        MAB_OAC_ERR_REQ_UNKNWN_SVC_ID, dst_svc_id);    
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);      
    }

    switch(mib_req->i_op)
    {
    case NCSMIB_OP_REQ_SETROW  :
    case NCSMIB_OP_REQ_TESTROW :
    case NCSMIB_OP_REQ_SETALLROWS:
    case NCSMIB_OP_REQ_REMOVEROWS:
      m_MMGR_FREE_BUFR_LIST(mib_req->req.info.setrow_req.i_usrbuf);
      mib_req->req.info.setrow_req.i_usrbuf = NULL;
      break;

    case NCSMIB_OP_REQ_MOVEROW:
      m_MMGR_FREE_BUFR_LIST(mib_req->req.info.moverow_req.i_usrbuf);
      mib_req->req.info.moverow_req.i_usrbuf = NULL;
      break;

    case NCSMIB_OP_REQ_CLI:
      m_MMGR_FREE_BUFR_LIST(mib_req->req.info.cli_req.i_usrbuf);
      mib_req->req.info.cli_req.i_usrbuf = NULL;
      break;

    default:
      break;
    }

    mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);
    mib_req->rsp.i_status = NCSCC_RC_NO_SUCH_TBL;

    /* Send the message */
#if 0
      code = m_NCS_MDS_SEND(inst->mds_hdl,
        NCSMDS_SVC_ID_OAC,
        msg,
        dst_vcard,
        (SS_SVC_ID)dst_svc_id, NULL);
#endif
      code = mab_mds_snd(inst->mds_hdl, msg, NCSMDS_SVC_ID_OAC, dst_svc_id, dst_vcard);
      if (code != NCSCC_RC_SUCCESS)
      {
        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_FRWD_PSR_FAILED);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
      }
      
      m_MMGR_FREE_MAB_MSG(msg);
      
      if(ncsmib_arg_free_resources(mib_req,TRUE) != NCSCC_RC_SUCCESS)
      {
        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, 
                       MAB_HDLN_OAC_MIBARG_FREE_FAILED); 
      }
      
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
  
  m_MMGR_FREE_MAB_MSG(msg);

  mib_req->i_mib_key = tbl_rec->handle;
  mib_req->i_usr_key = inst->hm_hdl;
  mib_req->i_rsp_fnc = oac_mib_response;

  m_OAC_UNLK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);

  /* Push a flag onto the stack that determines whether oac_mib_response
   * should forward the response to the PSR
   */
  {
      NCS_SE*      se;
      uns8*       stream;
      NCS_BOOL     flag = FALSE;

      se = ncsstack_push(&mib_req->stack,
                        NCS_SE_TYPE_FORWARD_TO_PSR,
                        sizeof(NCS_SE_FORWARD_TO_PSR));
      if (se == NULL)
      {
          m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_PUSH_FLAG_FAILED);
          return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }

      stream = m_NCSSTACK_SPACE (se);

      /* Lookup for OAC_TBL_REC and see if this table is persistent or not. */
      if(tbl_rec->is_persistent && 
         ((mib_req->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME))
         flag = TRUE;

      ncs_encode_8bit(&stream, (uns32) flag);
  }

  status = (tbl_rec->req_fnc)(mib_req); 
  if(status != NCSCC_RC_SUCCESS)
  {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_MIB_REQ_FAILED);
    mib_req->i_mib_key = 0;
    mib_req->i_usr_key = 0;

    if(ncsmib_arg_free_resources(mib_req,TRUE) != NCSCC_RC_SUCCESS) /* Fix for the bug IR00082719 */
    {
       m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, 
                         MAB_HDLN_OAC_MIBARG_FREE_FAILED); 
       m_MMGR_FREE_NCSMIB_ARG(mib_req); /* Fix for the bug IR00082719 */
       m_MAB_DBG_TRACE("\noac_mib_request():left.");
       return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    m_MAB_DBG_TRACE("\noac_mib_request():left.");
    return m_MAB_DBG_SINK(status);
  }

  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_OAC_MIB_REQ_SUCCESS);

  mib_req->i_mib_key = 0;
  mib_req->i_usr_key = 0;

  if(ncsmib_arg_free_resources(mib_req,TRUE) != NCSCC_RC_SUCCESS)
  {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, 
                       MAB_HDLN_OAC_MIBARG_FREE_FAILED); 
    m_MAB_DBG_TRACE("\noac_mib_request():left.");
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }
  
  m_MAB_DBG_TRACE("\noac_mib_request():left.");
  return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
   oac_move_row_fltr_delete
*****************************************************************************/

uns32 oac_move_row_fltr_delete(OAC_TBL* inst, NCSMIB_ARG* arg)
{
  OAC_TBL_REC* tbl_rec;
  OAC_FLTR*    fltr;
  OAC_FLTR*    del_fltr;

  m_OAC_LK_INIT;

  m_MAB_DBG_TRACE("\noac_move_row_fltr_delete():entered.");

  if((tbl_rec = oac_tbl_rec_find(inst, arg->i_tbl_id)) == NULL)
    {
    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          MAB_OAC_FLTR_REG_NO_TBL,
                          inst->vrid, 
                          arg->i_tbl_id);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

  if((fltr = tbl_rec->fltr_list) == NULL)
    {
    m_MAB_DBG_TRACE("\noac_move_row_fltr_delete():left.");
    return NCSCC_RC_SUCCESS;
    }
  if(fltr->fltr.is_move_row_fltr == TRUE)
    {
    if(memcmp((arg->i_idx.i_inst_ids + fltr->fltr.fltr.range.i_bgn_idx),
      (fltr->fltr.fltr.range.i_min_idx_fltr),
      (fltr->fltr.fltr.range.i_idx_len) * sizeof(uns32)) == 0)
      {
      tbl_rec->fltr_list = fltr->next;

      if(fltr->fltr.type == NCSMAB_FLTR_RANGE)
        {
        if(fltr->fltr.fltr.range.i_min_idx_fltr != NULL)
          m_MMGR_FREE_MIB_INST_IDS(fltr->fltr.fltr.range.i_min_idx_fltr);
        if(fltr->fltr.fltr.range.i_max_idx_fltr != NULL)
          m_MMGR_FREE_MIB_INST_IDS(fltr->fltr.fltr.range.i_max_idx_fltr);
        }

      m_MMGR_FREE_OAC_FLTR(fltr);

      m_MAB_DBG_TRACE("\noac_move_row_fltr_delete():left.");
      return NCSCC_RC_SUCCESS;
      }
    }


  for(; fltr->next != NULL; fltr = fltr->next)
    {
    if(fltr->next->fltr.is_move_row_fltr == TRUE)
      {
      if(memcmp((arg->i_idx.i_inst_ids + fltr->next->fltr.fltr.range.i_bgn_idx),
        (fltr->next->fltr.fltr.range.i_min_idx_fltr),
        (fltr->next->fltr.fltr.range.i_idx_len) * sizeof(uns32)) == 0)
        {
        del_fltr = fltr->next;
        fltr->next = del_fltr->next;

        if(del_fltr->fltr.type == NCSMAB_FLTR_RANGE)
          {
          if(del_fltr->fltr.fltr.range.i_min_idx_fltr != NULL)
            m_MMGR_FREE_MIB_INST_IDS(del_fltr->fltr.fltr.range.i_min_idx_fltr);
          if(del_fltr->fltr.fltr.range.i_max_idx_fltr != NULL)
            m_MMGR_FREE_MIB_INST_IDS(del_fltr->fltr.fltr.range.i_max_idx_fltr);
          }

        m_MMGR_FREE_OAC_FLTR(del_fltr);
        break;
        }
      }
    }


  m_MAB_DBG_TRACE("\noac_move_row_fltr_delete():left.");
  return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
   oac_mab_response
*****************************************************************************/

uns32 oac_mab_response(MAB_MSG* msg)
{
  OAC_TBL*     inst;
  NCSMIB_ARG*   rsp;

  m_OAC_LK_INIT;
  
  m_MAB_DBG_TRACE("\noac_mab_response():entered.");
  
  inst = (OAC_TBL*)msg->yr_hdl;
  
  m_OAC_LK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);

  rsp = msg->data.data.snmp;

  if (rsp == NULL)
    {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_NULL_MIB_RSP_RCVD); 
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

  if(! m_NCSMIB_ISIT_A_RSP(rsp->i_op))
    {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_INVALID_MIB_RSP_RCVD);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

  /* recover the information about the original MIB request sender */
    {
    NCS_SE*        se;
    uns8*         stream;
    
    if((se = ncsstack_pop(&rsp->stack)) == NULL)
      {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_FLAG_FAILED);
      m_OAC_UNLK(&inst->lock);
      m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }   
    
    stream = m_NCSSTACK_SPACE(se);
    
    if(se->type != NCS_SE_TYPE_MIB_ORIG)
      {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_POP_XSE);
      m_OAC_UNLK(&inst->lock);
      m_LOG_MAB_LOCK(MAB_LK_MAC_UNLOCKED,&inst->lock);
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
      }

  #if 0
    rsp->i_usr_key = ncs_decode_32bit(&stream);
    rsp->i_rsp_fnc = (NCSMIB_FNC)ncs_decode_32bit(&stream);
  #endif

    rsp->i_usr_key = ncs_decode_64bit(&stream);
    if(4 == sizeof(void *))
       rsp->i_rsp_fnc = (NCSMIB_FNC)(long)ncs_decode_32bit(&stream);
    else if(8 == sizeof(void *))
       rsp->i_rsp_fnc = (NCSMIB_FNC)(long)ncs_decode_64bit(&stream);

    }

   if(rsp->i_op == NCSMIB_OP_RSP_MOVEROW)
    {
    oac_move_row_fltr_delete(inst,rsp);
    } 

  m_OAC_UNLK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
  
  m_MMGR_FREE_MAB_MSG(msg);
  
  rsp->i_rsp_fnc(rsp);
  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_OAC_USR_RSP_FNC_RET); 

  rsp->i_usr_key = 0;

  if (rsp->i_op == NCSMIB_OP_RSP_MOVEROW)
  {
     if (rsp->rsp.info.moverow_rsp.i_usrbuf != NULL)
     {
        m_MMGR_FREE_BUFR_LIST(rsp->rsp.info.moverow_rsp.i_usrbuf);
        rsp->rsp.info.moverow_rsp.i_usrbuf = NULL;
     }
  }
  if(ncsmib_arg_free_resources(rsp,FALSE) != NCSCC_RC_SUCCESS)
  {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, 
                       MAB_HDLN_OAC_MIBARG_FREE_FAILED); 
    m_MAB_DBG_TRACE("\noac_mab_response():left.");
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }
  
  m_MAB_DBG_TRACE("\noac_mab_response():left.");

  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
   oac_free_idx_request
*****************************************************************************/

uns32 oac_free_idx_request(MAB_MSG* msg)
  {
  OAC_TBL*        inst;
  OAC_TBL_REC*    tbl_rec;
  NCSMAB_IDX_FREE* idx_free;
  NCSOAC_SS_CB_ARG cbarg;

  m_OAC_LK_INIT;
  
  m_MAB_DBG_TRACE("\noac_free_idx_request():entered.");
  
  inst = (OAC_TBL*)msg->yr_hdl;
  idx_free = &msg->data.data.idx_free;

  m_OAC_LK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);

  if((tbl_rec = oac_tbl_rec_find(inst, idx_free->idx_tbl_id)) == NULL)
  {
    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          MAB_OAC_FLTR_REG_NO_TBL,
                          inst->vrid, 
                          idx_free->idx_tbl_id);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  cbarg.i_op = NCSOAC_SS_CB_OP_IDX_FREE;
  cbarg.i_ss_hdl = tbl_rec->ss_cb_hdl;
  cbarg.info.idx_free = idx_free;

  if(tbl_rec->ss_cb_fnc(&cbarg) != NCSCC_RC_SUCCESS)
    {
    uns32 dummy; /* SMM this keeps VxWorks/LINUX compilers from complaining */
    dummy = m_MAB_DBG_SINK(NCSCC_RC_FAILURE); /* IF here, OAC client failed, not OAC!! */
    }

  m_OAC_UNLK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
  
  if(idx_free->idx_free_data.range_idx_free.i_min_idx_fltr != NULL)
      m_MMGR_FREE_MIB_INST_IDS(idx_free->idx_free_data.range_idx_free.i_min_idx_fltr);
  if(idx_free->idx_free_data.range_idx_free.i_max_idx_fltr != NULL)
      m_MMGR_FREE_MIB_INST_IDS(idx_free->idx_free_data.range_idx_free.i_max_idx_fltr);

  m_MMGR_FREE_MAB_MSG(msg);

  m_MAB_DBG_TRACE("\noac_free_idx_request():left.");

  return NCSCC_RC_SUCCESS;
  }

/*****************************************************************************
   oac_sa_mib_request
*****************************************************************************/

uns32 oac_sa_mib_request(MAB_MSG* msg)
  {
  OAC_TBL*        inst;
  OAC_TBL_REC*    tbl_rec;
  NCSMIB_ARG*      mib_arg;
  NCSOAC_SS_CB_ARG cbarg;

  m_OAC_LK_INIT;
  
  m_MAB_DBG_TRACE("\noac_sa_mib_request():entered.");
  
  inst = (OAC_TBL*)msg->yr_hdl;
  mib_arg = msg->data.data.snmp;

  m_OAC_LK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);

  if((tbl_rec = oac_tbl_rec_find(inst,mib_arg->i_tbl_id)) == NULL)
  {
    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          MAB_OAC_FLTR_REG_NO_TBL,
                          inst->vrid, 
                          mib_arg->i_tbl_id);
    m_OAC_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  cbarg.i_op = NCSOAC_SS_CB_OP_SA_DATA;
  cbarg.i_ss_hdl = tbl_rec->ss_cb_hdl;
  cbarg.info.mib_req = mib_arg;

  m_MMGR_FREE_MAB_MSG(msg);

  mib_arg->i_mib_key = tbl_rec->handle;
  mib_arg->i_usr_key = inst->hm_hdl;
  mib_arg->i_rsp_fnc = oac_mib_response;

  /* Push a flag onto the stack that determines whether oac_mib_response
   * should forward the response to the PSR
   */
  {
      NCS_SE*      se;
      uns8*       stream;
      NCS_BOOL     flag = FALSE;

      se = ncsstack_push(&mib_arg->stack,
                        NCS_SE_TYPE_FORWARD_TO_PSR,
                        sizeof(NCS_SE_FORWARD_TO_PSR));
      if (se == NULL)
      {
          m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_PUSH_FLAG_FAILED);
          m_OAC_UNLK(&inst->lock);
          m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
          return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
      }

      stream = m_NCSSTACK_SPACE (se);

      /* Lookup for OAC_TBL_REC and see if this table is persistent or not. */
      if(tbl_rec->is_persistent && 
         ((mib_arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME))
         flag = TRUE;

      ncs_encode_8bit(&stream, (uns32) flag);
  }

  if(tbl_rec->ss_cb_fnc(&cbarg) != NCSCC_RC_SUCCESS)
    {
    uns32 dummy; /* SMM this keeps VxWorks/LINUX compilers from complaining */
    m_LOG_MAB_HEADLINE(NCSFL_SEV_WARNING,
                       MAB_HDLN_OAC_SU_CBK_FAILED);
    dummy = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);/* IF here, OAC client failed, not OAC!! */
    }

  m_OAC_UNLK(&inst->lock);
  m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);

  mib_arg->i_mib_key = 0;
  mib_arg->i_usr_key = 0;

  if(ncsmib_arg_free_resources(mib_arg,TRUE) != NCSCC_RC_SUCCESS)
  {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, 
                       MAB_HDLN_OAC_MIBARG_FREE_FAILED); 
    m_MAB_DBG_TRACE("\noac_sa_mib_request():left.");
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  m_MAB_DBG_TRACE("\noac_sa_mib_request():left.");

  return NCSCC_RC_SUCCESS;
  }

  /*****************************************************************************
  oac_playback_start
*****************************************************************************/
uns32 oac_playback_start(MAB_MSG * msg)
{
   OAC_TBL * inst;
   
   m_OAC_LK_INIT;
   
   m_MAB_DBG_TRACE("\noac_playback_start():entered.");
   
   inst = (OAC_TBL*)msg->yr_hdl;
   if (inst == NULL)
   {
      m_LOG_MAB_NO_CB("oac_playback_start()"); 
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   m_OAC_LK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);
   
   inst->playback_session = TRUE;
   
   m_OAC_UNLK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
   m_MAB_DBG_TRACE("\noac_playback_start():left.");
   
   m_MMGR_FREE_MAB_MSG(msg);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
oac_handle_svc_mds_evt
*****************************************************************************/
uns32 oac_handle_svc_mds_evt(MAB_MSG * msg)
{
   OAC_TBL*    inst;
   
   m_OAC_LK_INIT;
   
   m_MAB_DBG_TRACE("\noac_handle_svc_mds_evt():entered.");
   
   /* Fix for the bug IR00061338 */
   /*inst = (OAC_TBL*)msg->yr_hdl;*/
   inst = (OAC_TBL*)m_OAC_VALIDATE_HDL((uns32)(long)msg->yr_hdl); /* IR00061338 */
   if (inst == NULL)
   {
      m_LOG_MAB_NO_CB("oac_handle_svc_mds_evt()"); 
      m_MMGR_FREE_MAB_MSG(msg); /* IR00061338 & IR00061413 */
      return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   m_OAC_LK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);

   m_LOG_MAB_SVC_PRVDR_EVT(NCSFL_SEV_DEBUG, MAB_SP_OAC_MDS_RCV_EVT,
                           msg->fr_svc,
                           msg->fr_card,
                           msg->data.data.oac_mds_svc_evt.change,
                           msg->data.data.oac_mds_svc_evt.anc);

   if((msg->data.data.oac_mds_svc_evt.role != V_DEST_RL_ACTIVE) &&
      (msg->fr_svc != NCSMDS_SVC_ID_OAC))
   {
      m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_IGNORING_NON_PRIMARY_MDS_SVC_EVT);
      m_MMGR_FREE_MAB_MSG(msg); /* IR00061338 */
      ncshm_give_hdl(inst->hm_hdl); /* IR00061338 */
      return NCSCC_RC_SUCCESS;
   }

   switch(msg->fr_svc)
   {
   case NCSMDS_SVC_ID_MAS:
      switch(msg->data.data.oac_mds_svc_evt.change)
      {
      case NCSMDS_DOWN:
         inst->mas_here = FALSE;
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_MAS_MDS_DOWN);
         break;

      case NCSMDS_NO_ACTIVE:
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_MAS_MDS_NO_ACTIVE);
         break;

      case NCSMDS_NEW_ACTIVE:
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_MAS_MDS_NEW_ACTIVE);
         if((inst->psr_here) && (inst->is_active == TRUE))
         {
            oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */

            /* Send buffer-zone events to PSS */
            (void)oac_send_buffer_zone_evts(inst);

            oac_send_pending_warmboot_reqs_to_pssv(inst);
         }
         break;

      case NCSMDS_UP:
         inst->mas_here = TRUE;
         inst->mas_vcard = msg->fr_card;
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_MAS_MDS_UP);

         oac_sync_fltrs_with_mas(inst);
         if((inst->psr_here) && (inst->is_active == TRUE))
         {
            oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */
            /* Send buffer-zone events to PSS */
            (void)oac_send_buffer_zone_evts(inst);
            oac_send_pending_warmboot_reqs_to_pssv(inst);
         }
         break;

      default:
         break;
      }
      break;
      
   case NCSMDS_SVC_ID_PSS:
      switch(msg->data.data.oac_mds_svc_evt.change)
      {
      case NCSMDS_DOWN:
         inst->psr_here = FALSE;
         inst->psr_vcard = 0;
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_PSS_MDS_DOWN);
         break;

      case NCSMDS_NO_ACTIVE:
         /* Treating this as DOWN event */
         inst->psr_here = FALSE;
         inst->psr_vcard = 0;
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_PSS_MDS_NO_ACTIVE);
         break;

      case NCSMDS_NEW_ACTIVE:
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_PSS_MDS_NEW_ACTIVE);
         /* Treating this as UP event */
         inst->psr_here = TRUE;
         inst->psr_vcard = msg->fr_card;
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_PSS_MDS_NEW_ACTIVE_AS_UP_EVT);

         if((inst->mas_here) && (inst->is_active == TRUE))
         {
            oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */
            /* Send buffer-zone events to PSS */
            (void)oac_send_buffer_zone_evts(inst);
            oac_send_pending_warmboot_reqs_to_pssv(inst);
         }
         break;

      case NCSMDS_UP:
         inst->psr_here = TRUE;
         inst->psr_vcard = msg->fr_card;
         m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE, MAB_HDLN_OAC_PSS_MDS_UP);

         if((inst->mas_here) && (inst->is_active == TRUE))
         {
            oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */
            /* Send buffer-zone events to PSS */
            (void)oac_send_buffer_zone_evts(inst);
            oac_send_pending_warmboot_reqs_to_pssv(inst);
         }
         break;

      default: 
         break;
      }
      break;

   case NCSMDS_SVC_ID_OAC:
      if((inst->my_vcard == msg->fr_card) &&
         ((msg->fr_anc == 0) || (msg->fr_anc == inst->my_anc)))
      {
          /* This is local SVC event from MDS */
          switch(msg->data.data.oac_mds_svc_evt.change)
          {
          case NCSMDS_NO_ACTIVE: /* Quiesced role */
          case NCSMDS_RED_DOWN:
             if(inst->is_active == TRUE)
             {
                inst->is_active = FALSE;
             }
             break;

          case NCSMDS_CHG_ROLE:
             /* This comes only for VDEST */
             if(inst->is_active == FALSE)
             {
                if(msg->data.data.oac_mds_svc_evt.role == V_DEST_RL_ACTIVE)
                {
                   inst->is_active = TRUE; 
                   if((inst->mas_here) && (inst->psr_here))
                   {
                      oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */
                      /* Send buffer-zone events to PSS */
                      (void)oac_send_buffer_zone_evts(inst);
                      oac_send_pending_warmboot_reqs_to_pssv(inst);
                   }
                }
             }
             else
             {
                if(msg->data.data.oac_mds_svc_evt.role != V_DEST_RL_ACTIVE)
                {
                   inst->is_active = FALSE;
                }
             }
             break;

          case NCSMDS_RED_UP:
          case NCSMDS_NEW_ACTIVE: /* Fall-through allowed here */
             if(inst->is_active == FALSE)
             {
                if(msg->data.data.oac_mds_svc_evt.role == V_DEST_RL_ACTIVE)
                {
                   inst->is_active = TRUE;
                   if((inst->mas_here) && (inst->psr_here))
                   {
                      oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */
                      /* Send buffer-zone events to PSS */
                      (void)oac_send_buffer_zone_evts(inst);
                      oac_send_pending_warmboot_reqs_to_pssv(inst);
                   }
                }
             }
             else
             {
                if(msg->data.data.oac_mds_svc_evt.role != V_DEST_RL_ACTIVE)
                {
                   inst->is_active = FALSE;
                }
             }
             break;

          case NCSMDS_UP:
             if(inst->is_active == FALSE)
             {
                inst->is_active = TRUE;
                if((inst->mas_here) && (inst->psr_here))
                {
                   oac_refresh_table_bind_to_pssv(inst); /* No ACK for this */
                   /* Send buffer-zone events to PSS */
                   (void)oac_send_buffer_zone_evts(inst);
                   oac_send_pending_warmboot_reqs_to_pssv(inst);
                }
             }
             break;

          default:
             break;
          }
      }
      break;

   default:
       m_LOG_MAB_SVC_PRVDR_EVT(NCSFL_SEV_NOTICE, MAB_SP_OAC_MDS_RCV_EVT,
                               msg->fr_svc,
                               msg->fr_card,
                               msg->data.data.oac_mds_svc_evt.change,
                               msg->data.data.oac_mds_svc_evt.anc);
      break;
   }
   m_MMGR_FREE_MAB_MSG(msg);
   
   m_OAC_UNLK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);

   m_MAB_DBG_TRACE("\noac_sa_mib_request():left.");

   ncshm_give_hdl(inst->hm_hdl); /* IR00061338 */

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
oac_playback_end
*****************************************************************************/
uns32 oac_playback_end(MAB_MSG * msg)
{
   OAC_TBL * inst;
   
   m_OAC_LK_INIT;
   
   m_MAB_DBG_TRACE("\noac_playback_end():entered.");
   
   inst = (OAC_TBL*)msg->yr_hdl;
   if (inst == NULL)
   {
        m_LOG_MAB_NO_CB("oac_playback_end()"); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   m_OAC_LK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);
   
   inst->playback_session = FALSE;
   
   m_OAC_UNLK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
   m_MAB_DBG_TRACE("\noac_playback_end():left.");
   
   m_MMGR_FREE_MAB_MSG(msg);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
oac_handle_pss_ack
*****************************************************************************/
uns32 oac_handle_pss_ack(MAB_MSG * msg)
{
   OAC_TBL * inst;
   OAA_BUFR_HASH_LIST *bptr = NULL, *prv_bptr = NULL;
   
   m_OAC_LK_INIT;
   
   m_MAB_DBG_TRACE("\noac_handle_pss_ack():entered.");
   
   inst = (OAC_TBL*)msg->yr_hdl;
   if (inst == NULL)
   {
        m_LOG_MAB_NO_CB("oac_handle_pss_ack()"); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_ACK_FRM_PSS_RCVD, msg->data.seq_num);

   m_OAC_LK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);
   
   for(bptr = inst->hash_bufr[msg->data.seq_num % MAB_MIB_ID_HASH_TBL_SIZE]; bptr != NULL; 
       prv_bptr = bptr, bptr = bptr->next)
   {
      if(bptr->msg->data.seq_num == msg->data.seq_num)
      {
         /* Free this message */
         if(prv_bptr != NULL)
            prv_bptr->next = bptr->next;
         else
            inst->hash_bufr[msg->data.seq_num % MAB_MIB_ID_HASH_TBL_SIZE] = bptr->next;

         oac_free_buffer_zone_entry(bptr);
         break;        
      }
   }
   
   m_OAC_UNLK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
   m_MAB_DBG_TRACE("\noac_handle_pss_ack():left.");
   
   m_MMGR_FREE_MAB_MSG(msg);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
oac_handle_pss_eop_ind
*****************************************************************************/
uns32 oac_handle_pss_eop_ind(MAB_MSG * msg)
{
   OAC_TBL * inst;
   OAA_WBREQ_HDL_LIST *list = NULL, *prv_list = NULL;
   NCSOAC_EOP_USR_IND_CB argcb;
   
   m_OAC_LK_INIT;
   
   m_MAB_DBG_TRACE("\noac_handle_pss_eop_ind():entered.");
   
   inst = (OAC_TBL*)msg->yr_hdl;
   if (inst == NULL)
   {
        m_LOG_MAB_NO_CB("oac_handle_pss_eop_ind()"); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
   }
   
   m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_EOP_STATUS_FOR_HDL_RCVD, msg->data.data.oac_pss_eop_ind.wbreq_hdl);

   m_OAC_LK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_LOCKED,&inst->lock);
   
   for(list = inst->wbreq_hdl_list; list != NULL; prv_list = list, list = list->next)
   {
      if(list->wbreq_hdl == msg->data.data.oac_pss_eop_ind.wbreq_hdl)
      {
         if(list->eop_usr_ind_fnc == NULL) 
         {
            break;
         }
         m_NCS_MEMSET(&argcb, '\0', sizeof(argcb));
         argcb.i_mib_key = msg->data.data.oac_pss_eop_ind.mib_key;
         argcb.i_wbreq_hdl = msg->data.data.oac_pss_eop_ind.wbreq_hdl;
         argcb.i_rsp_status = msg->data.data.oac_pss_eop_ind.status;

         /* Invoke the user-provided indication handler */
         m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_INVKNG_EOP_STATUS_CB_FOR_USR, msg->data.data.oac_pss_eop_ind.wbreq_hdl);
         (*list->eop_usr_ind_fnc)(&argcb);
         m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_INVKD_EOP_STATUS_CB_FOR_USR, msg->data.data.oac_pss_eop_ind.wbreq_hdl);

         if(prv_list != NULL)
            prv_list->next = list->next;
         else
            inst->wbreq_hdl_list = list->next;
         m_MMGR_FREE_OAA_WBREQ_HDL_LIST(list);
         list = NULL;

         break;
      }
   }
   
   m_OAC_UNLK(&inst->lock);
   m_LOG_MAB_LOCK(MAB_LK_OAC_UNLOCKED,&inst->lock);
   m_MAB_DBG_TRACE("\noac_handle_pss_eop_ind():left.");
   
   m_MMGR_FREE_MAB_MSG(msg);
   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_send_buffer_zone_evts

  DESCRIPTION:       Send the events queued in the buffer-zone to PSS.

  RETURNS:           void

*****************************************************************************/
uns32 oac_send_buffer_zone_evts(OAC_TBL *inst)
{
   OAA_BUFR_HASH_LIST *bptr = NULL, *prv_bptr = NULL;
   uns32 code = NCSCC_RC_SUCCESS, i = 0;
   
   m_MAB_DBG_TRACE("\noac_send_buffer_zone_evts():entered.");
   
   for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
   {
      for(bptr = inst->hash_bufr[i]; bptr != NULL; 
          prv_bptr = bptr, bptr = bptr->next)
      {
         code = mab_mds_snd(inst->mds_hdl, bptr->msg, NCSMDS_SVC_ID_OAC, NCSMDS_SVC_ID_PSS,
                       inst->psr_vcard);
         if (code == NCSCC_RC_SUCCESS)
         {
            m_LOG_MAB_HEADLINE(NCSFL_SEV_NOTICE,
                         MAB_HDLN_OAC_BUFR_ZONE_EVT_SEND_SUCCESS);
         }
         else
         {
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_OAC_BUFR_ZONE_EVT_SEND_FAIL);
            return NCSCC_RC_SUCCESS;
         }
      }
   }
   
   m_MAB_DBG_TRACE("\noac_send_buffer_zone_evts():left.");
   
   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  oac_mas_fltrs_refresh
*****************************************************************************/

uns32 oac_mas_fltrs_refresh(MAB_MSG* msg)
{
  return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_psr_add_to_buffer_zone

  DESCRIPTION:       Add event to the buffer-zone.

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 oac_psr_add_to_buffer_zone(OAC_TBL *inst, MAB_MSG* msg)
{
   MAB_MSG *lcl_msg = NULL;
   OAA_BUFR_HASH_LIST *bptr = NULL, *prv_bptr = NULL;

   switch(msg->op)
   {
   case MAB_PSS_SET_REQUEST:
   case MAB_PSS_WARM_BOOT:
   case MAB_PSS_TBL_BIND:
   case MAB_PSS_TBL_UNBIND:
      break;
   default:
      m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_INV_BUFZONE_EVT_TYPE, msg->op);
      return NCSCC_RC_FAILURE;
   }

   /* Add this message to the buffer zone. */
   /* Alloc msg_ptr, and duplicate arg->info.push_mibarg_data.arg here. */
   lcl_msg = m_MMGR_ALLOC_MAB_MSG;
   m_NCS_OS_MEMSET(lcl_msg, '\0', sizeof(MAB_MSG));

   m_NCS_OS_MEMCPY(lcl_msg, msg, sizeof(MAB_MSG));
   switch(msg->op)
   {
   case MAB_PSS_SET_REQUEST:
      if(((MAB_MSG*)msg)->data.data.snmp != NULL)
      {
         lcl_msg->data.data.snmp = ncsmib_memcopy(((MAB_MSG*)msg)->data.data.snmp);
         if(lcl_msg->data.data.snmp == NULL)
         {
            m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_BUFZONE_SNMP_DUP_FAILED, msg->data.seq_num);
            return NCSCC_RC_FAILURE;
         }
      }
      break;

   case MAB_PSS_WARM_BOOT:
      /* The memory ownership is also transferred here. */
      lcl_msg->data.data.oac_pss_warmboot_req = ((MAB_MSG*)msg)->data.data.oac_pss_warmboot_req;
      break;

   case MAB_PSS_TBL_BIND:
      /* The memory ownership is also transferred here. */
      lcl_msg->data.data.oac_pss_tbl_bind = ((MAB_MSG*)msg)->data.data.oac_pss_tbl_bind;
      break;

   case MAB_PSS_TBL_UNBIND:
      /* No pointers used here for the present. */
      break;

   default:
      return NCSCC_RC_FAILURE;
   }

   for(bptr = inst->hash_bufr[lcl_msg->data.seq_num % MAB_MIB_ID_HASH_TBL_SIZE]; bptr != NULL; 
       prv_bptr = bptr, bptr = bptr->next)
      ;
   if((bptr = m_MMGR_ALLOC_OAA_BUFR_HASH_LIST) == NULL)
   {
      m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_BUFZONE_NODE_ALLOC_FAIL, msg->data.seq_num);
      oac_free_buffer_zone_entry_msg(lcl_msg);
      return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(bptr, '\0', sizeof(OAA_BUFR_HASH_LIST));
   bptr->msg = lcl_msg;

   if(prv_bptr == NULL)
      inst->hash_bufr[lcl_msg->data.seq_num % MAB_MIB_ID_HASH_TBL_SIZE] = bptr;
   else
      prv_bptr->next = bptr;

   m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_BUFZONE_NODE_ADDED, lcl_msg->data.seq_num);

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_free_buffer_zone_entry

  DESCRIPTION:       Free the buffer-zone entry.

  RETURNS:           void

*****************************************************************************/
void oac_free_buffer_zone_entry(OAA_BUFR_HASH_LIST *ptr)
{
   uns32 seq_num = ptr->msg->data.seq_num;

   oac_free_buffer_zone_entry_msg(ptr->msg);
   m_MMGR_FREE_OAA_BUFR_HASH_LIST(ptr);

   m_LOG_MAB_HDLN_I(NCSFL_SEV_DEBUG, MAB_HDLN_BUFZONE_NODE_REMOVED, seq_num);

   return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_free_buffer_zone_entry_msg

  DESCRIPTION:       Free the message pointer of the buffer-zone entry.

  RETURNS:           void

*****************************************************************************/
void oac_free_buffer_zone_entry_msg(MAB_MSG *msg)
{
   if(!msg)
      return;

   switch(msg->op)
   {
   case MAB_PSS_SET_REQUEST:
      if(msg->data.data.snmp != NULL)
         ncsmib_memfree(msg->data.data.snmp);
      break;

   case MAB_PSS_WARM_BOOT:
      oac_free_wbreq(msg->data.data.oac_pss_warmboot_req.next);
      /* Since First element is not an allocated-pointer */

      m_MMGR_FREE_MAB_PCN_STRING(msg->data.data.oac_pss_warmboot_req.pcn_list.pcn);
      oac_free_pss_tbl_list(msg->data.data.oac_pss_warmboot_req.pcn_list.tbl_list);
      break;

   case MAB_PSS_TBL_BIND:
      oac_free_bind_req_list(msg->data.data.oac_pss_tbl_bind.next);
      m_MMGR_FREE_MAB_PCN_STRING(msg->data.data.oac_pss_tbl_bind.pcn_list.pcn);
      oac_free_pss_tbl_list(msg->data.data.oac_pss_tbl_bind.pcn_list.tbl_list);
      break;

   case MAB_PSS_TBL_UNBIND:
      /* No pointers used here for the present. */
      break;

   default:
      return;
   }

   m_MMGR_FREE_MAB_MSG(msg);
   return;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_dup_warmboot_req

  DESCRIPTION:       Duplicate the warmboot-request info

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 oac_dup_warmboot_req(MAB_PSS_WARMBOOT_REQ *in, MAB_PSS_WARMBOOT_REQ *out)
{
   MAB_PSS_WARMBOOT_REQ *in_req = in, *req = out, *req_head = NULL, *p_req = NULL;
   MAB_PSS_TBL_LIST *tlist = NULL, *olist = NULL, *p_tlist = NULL;

   while(in_req != NULL)
   {
      if((req = m_MMGR_ALLOC_MAB_PSS_WARMBOOT_REQ) == NULL)
      {
         m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PSS_WARMBOOT_REQ,
            "oac_dup_warmboot_req()");
         oac_free_wbreq(req_head);
         return NCSCC_RC_FAILURE;
      }
      m_NCS_MEMSET(req, '\0', sizeof(MAB_PSS_WARMBOOT_REQ));
      req->is_system_client = in_req->is_system_client;
      if((req->pcn_list.pcn = 
         m_MMGR_ALLOC_MAB_PCN_STRING(m_NCS_STRLEN(in_req->pcn_list.pcn)+1)) == NULL)
      {
         m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL,
            "oac_dup_warmboot_req()");
         m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req);
         oac_free_wbreq(req_head);
         return NCSCC_RC_FAILURE;
      }
      m_NCS_STRCPY(req->pcn_list.pcn, in_req->pcn_list.pcn);

      /* Duplicate the table-list */
      tlist = in_req->pcn_list.tbl_list;
      p_tlist = NULL;
      while(tlist != NULL)
      {
         if((olist = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL)
         {
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
               "oac_dup_warmboot_req()");
            oac_free_pss_tbl_list(req->pcn_list.tbl_list);
            m_MMGR_FREE_MAB_PCN_STRING(req->pcn_list.pcn);
            m_MMGR_FREE_MAB_PSS_WARMBOOT_REQ(req);
            oac_free_wbreq(req_head);
            return NCSCC_RC_FAILURE;
         }
         m_NCS_MEMSET(olist, '\0', sizeof(MAB_PSS_TBL_LIST));
         olist->tbl_id = tlist->tbl_id;

         /* Add the element to the list */
         if(p_tlist == NULL)
         {
            p_tlist = req->pcn_list.tbl_list = olist;
         }
         else
         {
            p_tlist->next = olist;
            p_tlist = olist;
         }
         tlist = tlist->next;
      }

      if(req_head == NULL)
         p_req = req_head = req;
      else
      { 
         p_req->next = req;
         p_req = req;
      }

      in_req = in_req->next;
   }

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_gen_tbl_bind

  DESCRIPTION:       Generate the Table-BIND event of type MAB_PSS_TBL_BIND_EVT

  RETURNS:           NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE

*****************************************************************************/
uns32 oac_gen_tbl_bind(OAA_PCN_LIST *pcn_list, MAB_PSS_TBL_BIND_EVT **o_bind)
{
   OAA_PCN_LIST *in_req = NULL;
   MAB_PSS_TBL_BIND_EVT *list = NULL, *list_head = NULL, *prv_list = NULL;

   list_head = *o_bind;
   for(in_req = pcn_list; in_req != NULL; in_req = in_req->next)
   {
      MAB_PSS_TBL_LIST *olist = NULL, *prv_olist = NULL;

      for(list = list_head; list != NULL; prv_list = list, list = list->next)
      {
         if(m_NCS_STRCMP(list->pcn_list.pcn, in_req->pcn) == 0)
         {
            break;
         }
      }
      if(list == NULL)
      {
         /* Add new PCN entry */
         if((list = m_MMGR_ALLOC_MAB_PSS_TBL_BIND_EVT) == NULL)
         {
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PSS_TBL_BIND_EVT,
               "oac_gen_tbl_bind()");
            oac_free_bind_req_list(list_head);
            return NCSCC_RC_FAILURE;
         }
         m_NCS_MEMSET(list, '\0', sizeof(MAB_PSS_TBL_BIND_EVT));
         
         if((list->pcn_list.pcn = 
            m_MMGR_ALLOC_MAB_PCN_STRING(m_NCS_STRLEN(in_req->pcn)+1)) == NULL)
         {
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_OAA_PCN_STRING_ALLOC_FAIL,
               "oac_gen_tbl_bind()");
            m_MMGR_FREE_MAB_PSS_TBL_BIND_EVT(list);
            oac_free_bind_req_list(list_head);
            return NCSCC_RC_FAILURE;
         }
         m_NCS_STRCPY(list->pcn_list.pcn, in_req->pcn);

         if(list_head == NULL)
            prv_list = list_head = list;
         else
         { 
            prv_list->next = list;
            prv_list = list;
         }
      }
      /* Look for table entry */
      for(olist = list->pcn_list.tbl_list; olist != NULL; olist = olist->next)
      {
         if(olist->tbl_id == in_req->tbl_id)
            break;
         prv_olist = olist;
      }
      if(olist == NULL)
      {
         /* Add new table entry */
         if((olist = m_MMGR_ALLOC_MAB_PSS_TBL_LIST) == NULL)
         {
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAB_PSS_TBL_LIST_ALLOC_FAIL,
               "oac_gen_tbl_bind()");
            oac_free_bind_req_list(list_head);
            return NCSCC_RC_FAILURE;
         }
         m_NCS_MEMSET(olist, '\0', sizeof(MAB_PSS_TBL_LIST));
         olist->tbl_id = in_req->tbl_id;
         if(prv_olist == NULL)
            list->pcn_list.tbl_list = olist;
         else
            prv_olist->next = olist;
      }
   }
   *o_bind = list_head;

   return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  PROCEDURE NAME:    oac_free_bind_req_list

  DESCRIPTION:       Free the list of type MAB_PSS_TBL_BIND_EVT

  RETURNS:           void

*****************************************************************************/
void oac_free_bind_req_list(MAB_PSS_TBL_BIND_EVT *bind_list)
{
   MAB_PSS_TBL_BIND_EVT *bptr;

   while(bind_list != NULL)
   {
      bptr = bind_list->next;

      m_MMGR_FREE_MAB_PCN_STRING(bind_list->pcn_list.pcn);
      oac_free_pss_tbl_list(bind_list->pcn_list.tbl_list);
      m_MMGR_FREE_MAB_PSS_TBL_BIND_EVT(bind_list);

      bind_list = bptr;
   }
   return;
}

#endif /* (NCS_MAB == 1) */




