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

  DESCRIPTION:

    This has the private functions of the Object Access Server (MAS), 
    a subcomponent of the MIB Access Broker (MAB) subystem.This file 
    contains these groups of private functions

  - The master MAS dispatch loop functions
  - The MAS Table Record Manipulation Routines
  - The MAS Filter Manipulation routines  
  - MAB message handling routines 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"

#if (NCS_MAB == 1)

static uns32 dummy;
static uns32 mas_get_crd_role(MDS_HDL  mds_hdl, MDS_DEST dest,
                              MAB_ANCHOR anc, V_DEST_RL *role);
static MAS_FLTR* mas_get_wild_card_fltr(MAS_FLTR **head);
static void mas_reset_wild_card_flg(MAS_FLTR **head);

/* process the exact filter */
static uns32
mas_exact_fltr_process(MAB_MSG      *msg,
                       MAS_TBL      *inst, 
                       MAS_ROW_REC  *tbl_rec, 
                       MAS_FLTR     *new_fltr,
                       uns32        i_fltr_id,
                       V_DEST_RL    oaa_role);

static uns32
mas_exact_fltr_almighty_inform(MAS_TBL *inst, MAS_ROW_REC *tbl_rec, 
                              MAS_FLTR *new_fltr, uns32 event_type); 

/* to get the filter-id for a given anchor value */ 
static uns32 
mab_fltrid_list_get(MAS_FLTR_IDS    *fltr_list,
                    MAB_ANCHOR      i_anchor, 
                    uns32           *o_fltr_id); 

/* to add this filtr_id and anchor value pair */  
static uns32   
mab_fltrid_list_add(MAS_FLTR_IDS    *i_list,
                    MAB_ANCHOR      i_anchor, 
                    V_DEST_RL       oaa_role,
                    uns32           i_fltr_id);

/* to add this filtr_id and anchor value pair */  
static uns32   
mab_fltrid_list_del(MAS_FLTR_IDS    *fltr_list,
                    MAB_ANCHOR      i_anchor);

/* inform the MAA with the given MIB_ARG and error_code */
static uns32
mas_inform_maa(MAB_MSG *msg, MAS_TBL *inst, 
               NCSMIB_ARG *mib_req, uns32 error_code);

/* to remove the filters of a particular OAA */ 
static uns32
mas_flush_fltrs(MAS_TBL*    inst, MDS_DEST    vcard,
                MAB_ANCHOR  anc, uns32       ss_id);

/* to process the OAA down event */ 
static uns32
mas_oaa_down_process(MAB_MSG *msg); 

/* removes a default fitler from the list od tables */ 
static NCS_BOOL 
mas_def_flter_del(MAS_TBL *inst, MAS_ROW_REC *tbl_rec, MDS_DEST *vcard,
                  MAB_ANCHOR anc, MAS_ROW_REC** next_tbl_rec); 

/* to process the OAA state change */ 
static uns32
mas_oaa_role_chg_process(MAB_MSG *msg); 

/* adjust the filter-id/anchor value mapping list */ 
static uns32
mas_fltr_ids_adjust(MAS_FLTR_IDS    *fltr_list,
                    MAB_ANCHOR      i_anchor, 
                    V_DEST_RL       i_role); 

/*****************************************************************************

  The master MAS dispatch loop functions

  OVERVIEW:   The MAS instance or instances has one or more dedicated 
              threads. Each sits in an inifinite loop governed by these
              functions.

  PROCEDURE NAME: mas_do_evts & mas_do_evt

  DESCRIPTION:       

     mas_do_evts      Infinite loop services the passed SYSF_MBX
     mas_do_evt       Master Dispatch function and services off MAS work queue

*****************************************************************************/
/*****************************************************************************
   mas_do_evts
*****************************************************************************/

uns32
mas_do_evts( SYSF_MBX*  mbx)
{
    MAB_MSG   *msg;

    if ((msg = m_MAS_RCV_MSG(mbx, msg)) != NULL)
    {
        m_MAB_DBG_TRACE("\nmas_do_evt():entered.");
        if(mas_do_evt(msg) == NCSCC_RC_DISABLED)
        {
            m_MAB_DBG_TRACE("\nmas_do_evt():left.");
            return NCSCC_RC_DISABLED;
        }
        m_MAB_DBG_TRACE("\nmas_do_evt():left.");
    }
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************
  mas_do_evt
*****************************************************************************/
uns32
mas_do_evt( MAB_MSG* msg)
{  
    uns32 status = NCSCC_RC_FAILURE; 
    switch (msg->op)
    {
        /* no support for this kind of request */ 
        case  MAB_MAS_TRAP_FWDR:
        break;

        /* Handle a NCSMIB_ARG request from MAA */ 
        case  MAB_MAS_REQ_HDLR:
            return mas_info_request(msg);

        /* handle the GETNEXT response/Moverow response */ 
        case  MAB_MAS_RSP_HDLR:
            return mas_info_response(msg);

        /* handle new fitler registration request */ 
        case  MAB_MAS_REG_HDLR:
            return mas_info_register(msg);

        /* handle filter unregister request */ 
        case  MAB_MAS_UNREG_HDLR:
            return mas_info_unregister(msg);

        /* process the OAA down event */
        case MAB_OAA_DOWN_EVT:
            status = mas_oaa_down_process(msg);
            m_MMGR_FREE_MAB_MSG(msg); 
            return status;
        break; 

        /* kill yourself */ 
        case  MAB_MAS_DESTROY:
            m_MMGR_FREE_MAB_MSG(msg); 
            status = maslib_mas_destroy();
        return status;

        /* Keep rtetrying AMF Interface */ 
        case  MAB_MAS_AMF_INIT_RETRY:
            m_MMGR_FREE_MAB_MSG(msg); 
            return mas_amf_initialize(&gl_mas_amf_attribs.amf_attribs);

        /* QUISCED_ACK received from MDS */ 
        case MAB_SVC_VDEST_ROLE_QUIESCED: 
            status = mas_amf_mds_quiesced_process(msg->yr_hdl); 
            m_MMGR_FREE_MAB_MSG(msg); 
            return status;

        /* process the role change */ 
        case MAB_OAA_CHG_ROLE: 
            status = mas_oaa_role_chg_process(msg);
            m_MMGR_FREE_MAB_MSG(msg); 
            return status;

        default:
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_RCVD_INVALID_OP);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    return NCSCC_RC_SUCCESS; 
}


/*****************************************************************************

  The MAS Table Record Manipulation Routines: 

  OVERVIEW:       The MAS Table Rocord Manipulation Routines oversee a hash table
                  of registered mib tables.

  PROCEDURE NAME: mas_row_rec_init & mas_row_rec_clr & 
                  mas_row_rec_put  & mas_row_rec_rmv & mas_row_rec_find

  DESCRIPTION:       

    mas_row_rec_init  - Put a MAS_TBL hash table in start state.
    mas_row_rec_clr   - remove all table records from hash table. Free all 
                        resources.

    mas_row_rec_put   - put a table record in the table record hash table.
    mas_row_rec_rmv   - remove a table record from the table record hash table.
    mas_row_rec_find  - find (but don't remove) a table record


*****************************************************************************/

uns32 mas_get_crd_role(MDS_HDL  mds_hdl, MDS_DEST dest, MAB_ANCHOR anc, V_DEST_RL *role)
{
    uns32       status; 
    NCSMDS_INFO mds_info;


    if (role == NULL)
    {
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    if(m_NCS_NODE_ID_FROM_MDS_DEST(dest)!=0)
    {/* Destination is ADEST */
        *role = V_DEST_RL_ACTIVE;
    }
    else
    {/* Destination is VDEST */
      
        memset( &mds_info, 0, sizeof(mds_info));

        mds_info.i_mds_hdl = mds_hdl;
        mds_info.i_svc_id = NCSMDS_SVC_ID_MAS;
        mds_info.i_op = MDS_QUERY_DEST; 
        mds_info.info.query_dest.i_dest = dest;
        mds_info.info.query_dest.i_svc_id = NCSMDS_SVC_ID_OAC;
        mds_info.info.query_dest.i_query_for_role = TRUE;
        mds_info.info.query_dest.info.query_for_role.i_anc = anc;
        
        status = ncsmds_api(&mds_info); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the dest */
            char addr_str[255] = {0};
            
            m_LOG_MAB_ERROR_I(NCSFL_SEV_NOTICE, MAB_MAS_ERR_GET_ROLE_FAILED, status);  
            
            /* convert the MDS_DEST into a string */
            if (m_NCS_NODE_ID_FROM_MDS_DEST(dest) == 0)
               sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(dest));
            else
               sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%lu",
                        m_NCS_NODE_ID_FROM_MDS_DEST(dest), 0, (long)(dest));

            /* now log the message */
            ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_NO_CB, MAB_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TIC, MAB_HDLN_MAS_MDS_DEST, addr_str);
            /* log the error code */ 
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
        *role = mds_info.info.query_dest.info.query_for_role.o_vdest_rl;
    }

    return NCSCC_RC_SUCCESS;
}
/*****************************************************************************
  mas_row_rec_init
*****************************************************************************/

void mas_row_rec_init(MAS_TBL* inst)
{
  uns32 i;
  
  for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
    inst->hash[i] = NULL;
  
}

/*****************************************************************************
  mas_row_rec_clr
*****************************************************************************/

void mas_row_rec_clr(MAS_TBL* inst)
{
  uns32        i;
  MAS_ROW_REC* tbl_rec;
  
  for(i = 0; i < MAB_MIB_ID_HASH_TBL_SIZE; i++)
  {
    while((tbl_rec = inst->hash[i]) != NULL)
    {
      inst->hash[i] = tbl_rec->next;
      /* need to free the heap tbl rec fields ...  */
      m_MMGR_FREE_MAS_ROW_REC(tbl_rec);
    }
  }
}


/*****************************************************************************
  mas_row_rec_put
*****************************************************************************/

void mas_row_rec_put(MAS_TBL* inst, MAS_ROW_REC* tbl_rec)
{
  uns32 bkt = tbl_rec->tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
  
  tbl_rec->next = inst->hash[bkt];
  inst->hash[bkt] = tbl_rec;
}

/*****************************************************************************
  mas_row_rec_rmv
*****************************************************************************/

MAS_ROW_REC* mas_row_rec_rmv(MAS_TBL* inst, uns32 tbl_id)
{
  MAS_ROW_REC* tbl_rec;
  MAS_ROW_REC* ret = NULL;
  uns32        bkt = tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;

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
  mas_row_rec_find
*****************************************************************************/

MAS_ROW_REC* mas_row_rec_find(MAS_TBL* inst, uns32 tbl_id)
{
  MAS_ROW_REC* tbl_rec;
  MAS_ROW_REC* ret = NULL;
  uns32        bkt = tbl_id % MAB_MIB_ID_HASH_TBL_SIZE;
  

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

  OVERVIEW:       The MAS Filter Manipulation routines

  PROCEDURE NAME: mas_fltr_put & mas_fltr_rmv & mas_fltr_find & mas_fltr_create
                  mas_classify_range & mas_classify_any & mas_classify_same_as
                  mas_locate_dst_oac & mas_flush_fltrs 

  DESCRIPTION:       

    mas_fltr_put        - splice filter on to filter list
    mas_fltr_rmv        - find a filter based on ID and splice out of filter list
    mas_fltr_find       - find a filter based on ID. Return record.
    mas_fltr_create     - create and properly populate an MAS_FLTR object

    mas_classify_range  - determine if index info is satisfied by this range filter
    mas_classify_any    - Filter type ANY satisfies all compares
    mas_classify_same_as- Filter compare is de-referenced to another MIB Table ID
    mas_classify_exact  - determine if index info is satisfied by this exact filter
    mas_locate_dst_oac  - Find a filter match, which will yeild OAC to talk to
    mas_flush_fltrs     - Flush all filters that belong to the given OAC
    mas_reset_wild_card_flg - Resets the wild card flags of all the instances(filters) 
    mas_get_wild_card_fltr  - Get the filter/instance to which wild_card CLI-REQ need
                              to send.
 ********************************************************************************/

/*****************************************************************************
  mas_fltr_put
*****************************************************************************/

void mas_fltr_put(MAS_FLTR** insert_after, MAS_FLTR* new_fltr)
{
  if(insert_after == NULL)
    {
    m_MAB_DBG_VOID;
    return;
    }

  if(*insert_after == NULL)
  {
    /* we are dealing with an empty bucket here */
    new_fltr->next = NULL;
    *insert_after = new_fltr;
  }
  else
  {
    new_fltr->next = (*insert_after)->next;
    (*insert_after)->next = new_fltr;
  }
  
}

/*****************************************************************************
  mas_fltr_rmv
*****************************************************************************/

MAS_FLTR* mas_fltr_rmv(MAS_FLTR** head, MAS_FLTR *del_me)
{
    MAS_FLTR* fltr;
    MAS_FLTR* prev_fltr = NULL;

    if(head == NULL)
        return (MAS_FLTR*) m_MAB_DBG_SINK((long)NULL);

    if (del_me == NULL)
        return NULL; 

    fltr = *head;

    if(fltr == NULL)
        return NULL;

    prev_fltr = NULL; 
    while (fltr)
    {
        if (fltr == del_me)
            break; 

        /* go to the next filter */
        prev_fltr = fltr; 
        fltr = fltr->next;
    }

    /* before unlink this fltr information, make sure that there are 
    * no other anchot<=>fltr-id associations */
    if ((del_me->fltr_ids.fltr_id_list == NULL)&&(del_me->fltr_ids.active_fltr == NULL))
    {
        if (prev_fltr == NULL)
            *head = (*head)->next; 
        else
            prev_fltr->next = del_me->next; 
    }
    else 
        return NULL; 

    return del_me; 
}

/*****************************************************************************
                 mas_reset_wild_card_flg
*****************************************************************************/
void mas_reset_wild_card_flg(MAS_FLTR **head)
{
  MAS_FLTR *fltr;

  /* Filter list exists?? */
  if(head == NULL)  return;

  fltr = *head;

  /* Traverse through all the filters (represents for appl. instances) and 
     reset the wild_card flag */
  while (fltr)
  {
     fltr->wild_card = FALSE;
     fltr = fltr->next;
  }

  return;
}

/*****************************************************************************
                          mas_get_wild_card_fltr 
*****************************************************************************/
MAS_FLTR* mas_get_wild_card_fltr(MAS_FLTR **head)
{
  MAS_FLTR *fltr;
  MAS_FLTR *ret_fltr = NULL;

  /* No filters?? */
  if (head == NULL)
    return (MAS_FLTR*) m_MAB_DBG_SINK((long)NULL);

  fltr = *head;

  /* Traverse through the filter list and get the filter node to which 
   * wild-card request is still required to send to. Once the wild_card
   * request is processed the corresponding wild_card flag of filter is set
   * to TRUE 
   */
  while (fltr)
  {
     if (fltr->wild_card == FALSE)
     {
        ret_fltr = fltr;
        break;
     }

     fltr = fltr->next;
  }

  return ret_fltr;
}


/*****************************************************************************
  mas_fltr_find
*****************************************************************************/

MAS_FLTR* mas_fltr_find(MAS_TBL* inst,MAS_FLTR** head, uns32 fltr_id,MAB_ANCHOR anc, MDS_DEST vcard)
{
  MAS_FLTR* fltr;
  MAS_FLTR* ret = NULL;
  uns32     status;
  uns32     exist_fltr_id = 0; 
  
  if(head == NULL)
  {
    return (MAS_FLTR*) m_MAB_DBG_SINK((long)NULL);
  }
  
  fltr = *head;
  
  /* there are some filters to search */ 
  if(fltr != NULL)
  {
    /* if the filter type is SAME_AS, get the filter details of
     * that table
     */
    if (fltr->fltr.type == NCSMAB_FLTR_SAME_AS)
    {
        MAS_ROW_REC* tbl_rec;

        /* get the table details */ 
      if((tbl_rec = mas_row_rec_find(inst, fltr->fltr.fltr.same_as.i_table_id)) == NULL)
      {
            /* log the failure */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_FLTR_SA_INVALID_TBL, 
                          fltr->fltr.fltr.same_as.i_table_id);  
        return (void*)m_MAB_DBG_SINK((long)NULL);
      }

        /* get the filter details */
      fltr = tbl_rec->fltr_list;
    }

    /* searh for the fltr-id for this anchor value */
    while (fltr)
    {
        status = mab_fltrid_list_get(&fltr->fltr_ids, anc, &exist_fltr_id); 
        /* make sure that this is the filter that the caller is looking for */ 
        if ((status == NCSCC_RC_SUCCESS) &&
            (exist_fltr_id == fltr_id)   && /* compare the filter id */ 
            (memcmp(&fltr->vcard, &vcard, sizeof(vcard)) == 0)) /* comapre OAA address */ 
        {
            /* found the required filter */ 
            ret = fltr;
            break; 
        }

        /* go to the next filter */
        fltr = fltr->next;
    } /* while (fltr) */ 
  } /* if (fltr != NULL) */
  
  return ret;
}

/*****************************************************************************
  mas_fltr_create
*****************************************************************************/
MAS_FLTR* mas_fltr_create(MAS_TBL*     inst,
                          NCSMAB_FLTR* mab_fltr,
                          MDS_DEST     vcard,
                          V_DEST_RL    fr_oaa_role,
                          MAB_ANCHOR   anc,
                          uns32        fltr_id,
                          MAS_FLTR_FNC fltr_fnc )
{
    MAS_FLTR    *ret = NULL;
    uns32       status = NCSCC_RC_FAILURE; 

    /* allocate the memory for the new filter */ 
    ret = m_MMGR_ALLOC_MAS_FLTR;
    if(ret == NULL)
    {
        m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, 
                          MAB_MF_MAS_FLTR_CREATE_FAILED,  
                          "mas_fltr_create()"); 
        return (MAS_FLTR*) m_MAB_DBG_SINK((long)NULL);
    }
    memset(ret,0,sizeof(MAS_FLTR));

    /* copy the required information into the fitler from the message */ 
    ret->vcard = vcard;
    
    /* add the anchor value and fltr-is association */
    status = mab_fltrid_list_add(&ret->fltr_ids, anc, fr_oaa_role, fltr_id); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_MAS_FLTR_FREE(ret); 
        return (MAS_FLTR*) m_MAB_DBG_SINK((long)NULL);
    }

    /* copy the test function */
    ret->test = fltr_fnc;

    /* copy the filter information */ 
    memcpy(&(ret->fltr), mab_fltr, sizeof(NCSMAB_FLTR));

    /* clone the filter specific information */ 
    switch (mab_fltr->type)
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
        status = NCSCC_RC_SUCCESS; 
        break; 
    }
   
    /* in case of failure, cleanup */  
    if (status != NCSCC_RC_SUCCESS)
    {
        m_MAS_FLTR_FREE(ret);
        return (MAS_FLTR*) m_MAB_DBG_SINK((long)NULL);
    }

    /* return the new filter */ 
    return ret;
}

void mas_fltr_free(MAS_FLTR   *fltr)
{
    MAB_FLTR_ANCHOR_NODE    *nxt_fltr_id, *current;

    if (fltr == NULL)
        return;

    /* free the indices, based on the type of the filter */ 
    mas_mab_fltr_indices_cleanup(&fltr->fltr); 

    if (fltr->fltr_ids.active_fltr != NULL)
    {
        m_MMGR_FREE_FLTR_ANCHOR_NODE(fltr->fltr_ids.active_fltr); 
        fltr->fltr_ids.active_fltr = NULL; 
    }

    /* free the fltr-id list */ 
    current = fltr->fltr_ids.fltr_id_list;
    while (current)
    {
        nxt_fltr_id = current->next; 
        m_MMGR_FREE_FLTR_ANCHOR_NODE(current); 
        current = nxt_fltr_id; 
    }
    fltr->fltr_ids.fltr_id_list = NULL; 

    /* free the MAS_FLTR*/ 
    m_MMGR_FREE_MAS_FLTR(fltr);

    return; 
}


/*****************************************************************************
  mas_classify_range
*****************************************************************************/
void* mas_classify_range(MAS_TBL* inst, NCSMAB_FLTR* fltr, uns32* inst_ids, uns32 inst_len,NCS_BOOL is_next_req)
{
  int min_res;
  int max_res;
  uns16 to_be_used_inst_len = fltr->fltr.range.i_idx_len;

  if(inst_len == 0)
    return fltr;

  /* it may be a good idea to check the filter does not go outside the inst_ids */
  if (inst_len <= to_be_used_inst_len)
      to_be_used_inst_len = inst_len; 

  m_MAB_FLTR_CMP(min_res,
                 inst_ids + fltr->fltr.range.i_bgn_idx,
                 fltr->fltr.range.i_min_idx_fltr,
                 to_be_used_inst_len);
  if(min_res == 0)
  {
    return fltr;
  }
  else if(min_res > 0)
  {
    m_MAB_FLTR_CMP(max_res,
                   inst_ids + fltr->fltr.range.i_bgn_idx,
                   fltr->fltr.range.i_max_idx_fltr,
                   to_be_used_inst_len);
    
    if(max_res == 0)
    {
      return fltr;
    }
    else if(max_res > 0)
    {
      return NULL;
    }
    else if(max_res < 0)
    {
      return fltr;
    }
  }
  else if(min_res < 0)
  {
    if(is_next_req == TRUE)
      return fltr;
    else
      return NULL;
  }
  m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_FLTR_INVALID_RANGES);  
  return (void*) m_MAB_DBG_SINK((long)NULL);
}


/*****************************************************************************
  mas_classify_any
*****************************************************************************/

void* mas_classify_any(MAS_TBL* inst, NCSMAB_FLTR* fltr, uns32* inst_ids, uns32 inst_len,NCS_BOOL is_next_req)
{
  return fltr;
}


/*****************************************************************************
  mas_classify_same_as
*****************************************************************************/

void* mas_classify_same_as(MAS_TBL* inst, NCSMAB_FLTR* fltr, uns32* inst_ids, uns32 inst_len,NCS_BOOL is_next_req)
{
  MAS_FLTR*    match_fltr;
  MAS_ROW_REC* tbl_rec;
  
  if((tbl_rec = mas_row_rec_find(inst,fltr->fltr.same_as.i_table_id)) == NULL)
  {
    m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_FLTR_SA_INVALID_TBL, 
                      fltr->fltr.same_as.i_table_id);  
    return NULL;
  }
  
  if((match_fltr = mas_locate_dst_oac(inst,
                                      tbl_rec->fltr_list,
                                      inst_ids,
                                      inst_len,
                                      is_next_req
                                      )) == NULL)
  {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_FLTR_SA_NO_OAA); 
    return (void*)m_MAB_DBG_SINK((long)NULL);
  }
  
  return match_fltr;
}

/*****************************************************************************
  mas_classify_exact
*****************************************************************************/
void* mas_classify_exact(MAS_TBL* inst, NCSMAB_FLTR* fltr, uns32* inst_ids, uns32 inst_len,NCS_BOOL is_next_req)
{
    int32 res = 0;
    uns32 size; 

    if(inst_len == 0)
        return fltr;

    if (is_next_req == FALSE)
    {
        if (inst_len == fltr->fltr.exact.i_idx_len)
        {
            m_MAB_FLTR_CMP(res,
                         inst_ids + fltr->fltr.exact.i_bgn_idx,
                         fltr->fltr.exact.i_exact_idx,
                         fltr->fltr.exact.i_idx_len);  
            if(res == 0)
            {
                return fltr;
            }
        }
    }
    else
    {
        /* compare the idexes in host order */
        if (inst_len<=fltr->fltr.exact.i_idx_len)
            size = inst_len; 
        else 
            size = fltr->fltr.exact.i_idx_len; 
        m_MAB_FLTR_CMP(res,
                     fltr->fltr.exact.i_exact_idx,
                     inst_ids + fltr->fltr.exact.i_bgn_idx,size); 
        if (res == 0)
        {
            /* index length in the request is smaller than the index in the filter */
            if (inst_len < fltr->fltr.exact.i_idx_len)
                return fltr; 
        }
        /* this filter is greater than the requested index */
        if (res > 0)
            return fltr; 
    }
    return (void*) m_MAB_DBG_SINK((long)NULL);
}

/*****************************************************************************
  mas_locate_dst_oac
*****************************************************************************/

MAS_FLTR* mas_locate_dst_oac(MAS_TBL* inst, MAS_FLTR* head, uns32* inst_ids, uns32 inst_len,NCS_BOOL is_next_req)
{
  MAS_FLTR* fltr;
  MAS_FLTR* ret = NULL;

  NCSFL_MEM idx; 

  memset(&idx, 0, sizeof(NCSFL_MEM)); 
  idx.len = inst_len*sizeof(inst_len); 
  idx.addr = idx.dump = (char*)inst_ids; 
  m_LOG_MAB_MEM(NCSFL_SEV_INFO, MAB_HDLN_MAS_IDX_RECEVD, inst_len, idx); 
  
  for(fltr = head;fltr != NULL;fltr = fltr->next)
  {
    if(fltr->fltr.type == NCSMAB_FLTR_SAME_AS)
    {
      if((ret = (MAS_FLTR*) fltr->test(inst,&(fltr->fltr),inst_ids,inst_len,is_next_req)) != NULL)
      {
        break;
      }
    }
    else
    {
      if(fltr->test(inst,&(fltr->fltr),inst_ids,inst_len,is_next_req) != NULL)
      {
          if (fltr->fltr_ids.active_fltr != NULL)
                ret = fltr;
        break; 
      }
    }
  }
  
  return ret;
}



/* to delete a table with default filter information from the list of
 * tables in a MAS instance 
 */ 
static NCS_BOOL 
mas_def_flter_del(MAS_TBL *inst, MAS_ROW_REC *tbl_rec, MDS_DEST *vcard, 
                  MAB_ANCHOR anc, MAS_ROW_REC** next_tbl_rec) 
{
    uns32   status;
    MAS_ROW_REC* del_tbl_rec;
    
    /* compare the VDEST */ 
    if(memcmp(&tbl_rec->dfltr.vcard, vcard, sizeof(MDS_DEST)) == 0)
    {
        status = mab_fltrid_list_del(&tbl_rec->dfltr.fltr_ids, anc); 
        if ((tbl_rec->dfltr.fltr_ids.fltr_id_list == NULL)&&
            (tbl_rec->dfltr.fltr_ids.active_fltr == NULL))
        {
           tbl_rec->dfltr_regd = FALSE; 
           memset(&tbl_rec->dfltr.vcard, 0, sizeof(MDS_DEST));
        }
        else
        {
           /* either Active OAA, or STANDBY OAA is still owning this filter */ 
           return FALSE;
        }

        /* if there are no more filters associated with this table */ 
        if (tbl_rec->fltr_list == NULL)
        {
            /* remove the table record and go to the next table */ 
            if((del_tbl_rec = mas_row_rec_rmv(inst,tbl_rec->tbl_id)) == NULL)
            {
                m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                          MAB_MAS_RMV_ROW_REC_FAILED, inst->vrid, tbl_rec->tbl_id);
                m_MAB_DBG_TRACE("\nmas_def_flter_del():left.");
                return FALSE; 
            }

            /* store the next-table's address to rerun to the caller of this
             * function 
             */
            *next_tbl_rec = tbl_rec->next;

            /* lgo the deleted table's table-id */ 
            m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
                      MAB_MAS_RMV_ROW_REC_SUCCESS, inst->vrid, del_tbl_rec->tbl_id);
            m_MAB_DBG_TRACE2_MAS_TR_DALLOC(del_tbl_rec);

            /* free the table record */
            m_MMGR_FREE_MAS_ROW_REC(del_tbl_rec);
            return TRUE;
        }
        else 
            *next_tbl_rec = NULL; 
    }

    return FALSE;
}

/*****************************************************************************
  mas_flush_fltrs
*****************************************************************************/
uns32 mas_flush_fltrs(MAS_TBL* inst,MDS_DEST vcard,MAB_ANCHOR anc,uns32 ss_id)
{
    uns32        i;
    MAS_ROW_REC* tbl_rec;
    MAS_ROW_REC* next_tbl_rec = NULL;
    MAS_ROW_REC* del_tbl_rec;
    NCS_BOOL      is_next_tbl_set = FALSE;
    MAS_FLTR*    fltr;
    MAS_FLTR*    del_fltr;
    MAS_FLTR*    prev_fltr = NULL;
    uns32        status; 
    NCS_BOOL     tbl_deleted;
    int8         addr_str[128]={0};

    m_MAB_DBG_TRACE("\nmas_flush_fltrs():entered.");

    /* convert the MDS_DEST into a string */
    if (m_NCS_NODE_ID_FROM_MDS_DEST(vcard) == 0)
       sprintf(addr_str, "VDEST:%d", 
               m_MDS_GET_VDEST_ID_FROM_MDS_DEST(vcard));
    else
       sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%lu",
                m_NCS_NODE_ID_FROM_MDS_DEST(vcard), 0, 
                (long)(vcard));

    /* now log the OAA address whose filters are getting wiped off */
    ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_NO_CB, MAB_FC_HDLN,
               NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
               NCSFL_TYPE_TIC, MAB_HDLN_MAS_FLTRS_FLUSH, addr_str);
    
    /* for all the buckets in MAS */ 
    for(i = 0;i < MAB_MIB_ID_HASH_TBL_SIZE;i++)
    {
        /* search for all the tables in a particular bucket */
        for(tbl_rec = inst->hash[i];tbl_rec != NULL;is_next_tbl_set = FALSE)
        {
            /* search by SS-ID, is this the subsystem went down?  */
            if(tbl_rec->ss_id == ss_id)
            {
                /* if there is a deafault filter for this table, see is it from the same OAA */ 
                if (tbl_rec->dfltr_regd == TRUE)
                {
                    /* delete the default fitler traces of this table*/
                    tbl_deleted = mas_def_flter_del(inst, tbl_rec, &vcard, anc, &next_tbl_rec); 
                    if (tbl_deleted == TRUE)
                    {
                        /* table is deleted and we should start from the loop for the next table*/ 
                        if (next_tbl_rec != NULL)
                        {
                            tbl_rec = next_tbl_rec;
                            is_next_tbl_set = TRUE;
                            continue; 
                        }
                        else
                        {
                            /* table is deleted, but there are no more tables in this bucket */ 
                            /* time to jump to the next bucket */
                            break; 
                        }
                    } /* Table deleted or not? */
                }/* table is having a default filter */ 

                prev_fltr = NULL;
                for(fltr = tbl_rec->fltr_list;fltr != NULL;)
                {
                    if(memcmp(&fltr->vcard, &vcard, sizeof(vcard)) == 0)
                    {
                        /* delete the anchor, fltr-id mapping from this fltr */
                        status = mab_fltrid_list_del(&fltr->fltr_ids, anc); 
                        if ((fltr->fltr_ids.fltr_id_list == NULL)&&(fltr->fltr_ids.active_fltr == NULL))
                        {
                            del_fltr = fltr; 

                            /* delink the fltr from the list */ 
                            if (prev_fltr == NULL)
                                tbl_rec->fltr_list = del_fltr->next; 
                            else
                                prev_fltr->next = del_fltr->next; 

                            fltr = fltr->next;

                        }
                        else
                        {
                            prev_fltr = fltr; 
                            fltr = fltr->next;
                            continue;
                        }
                    } 
                    else
                    {
                        prev_fltr = fltr; 
                        fltr = fltr->next;
                        continue;
                    }

                    /* log the filter details of the de-linked filter */
                    m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
                                           MAB_MAS_FLTR_RMV_SUCCESS, tbl_rec->tbl_id, 
                                           -1, del_fltr->fltr.type); 

                    m_MAS_FLTR_FREE(del_fltr);

                    if(tbl_rec->fltr_list == NULL)
                    {
                        /* Check is this a table with default fitler */
                        if (tbl_rec->dfltr_regd == TRUE)
                        {
                            /* this means that, default filter is registered by some other OAA */
                            tbl_rec = tbl_rec->next;
                            is_next_tbl_set = TRUE;
                            break;
                        }

                        /* de-link this table from the list of tables in this bucket */
                        if((del_tbl_rec = mas_row_rec_rmv(inst,tbl_rec->tbl_id)) == NULL)
                        {
                            m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                            MAB_MAS_RMV_ROW_REC_FAILED, inst->vrid, tbl_rec->tbl_id);
                            m_MAB_DBG_TRACE("\nmas_flush_fltrs():left.");
                            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                        }

                        /* go to the next table in this bucket */
                        tbl_rec = tbl_rec->next;
                        is_next_tbl_set = TRUE;

                        /* log the table-id details of the de-linked table record */
                        m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_RMV_ROW_REC_SUCCESS, 
                        inst->vrid, del_tbl_rec->tbl_id);
                        m_MAB_DBG_TRACE2_MAS_TR_DALLOC(del_tbl_rec);

                        /* free the de-linked table-record */
                        m_MMGR_FREE_MAS_ROW_REC(del_tbl_rec);
                    }
                } /* for(fltr = tbl_rec->fltr_list;fltr != NULL;) */
            }
            if(is_next_tbl_set == FALSE)
            {
                /* set the stage for next iteration, for the next table */ 
                tbl_rec = tbl_rec->next;
            }
        } /* for(tbl_rec = inst->hash[i];tbl_rec != NULL;is_next_tbl_set = FALSE) */
    } /* for all the buckets in the hash table */

    m_MAB_DBG_TRACE("\nmas_flush_fltrs():left.");
    return NCSCC_RC_SUCCESS;
}


/****************************************************************************

  OVERVIEW:       MAS message handling routines

  PROCEDURE NAME: 

  DESCRIPTION:       

    mas_info_request      - Service MIB Access Request; forward to OAC
    mas_info_response     - MIB Access Response; check for futher processing
    mas_info_register     - Filter registration arrived; check & install
    mas_info_unregister   - Filter deregistration arrived; splice out
    mas_forward_msg       - Forward msg to proper destination
    mas_relay_msg_to_mac  - Relay this message back to the MAC
    mas_next_op_stack_push- Push navigational info that gets us back to MAS
    mas_cli_msg_to_mac - Sends the response message to MAC and at the same time
                         it retains the MAC information in mib_arg stack.
*****************************************************************************/

/*****************************************************************************
  mas_info_request
*****************************************************************************/
uns32 mas_info_request(MAB_MSG* msg)
{
    MAS_TBL*      inst;
    MAS_FLTR*     match_fltr;
    MAS_ROW_REC*  tbl_rec; 
    NCSMIB_ARG*   mib_req;
    NCS_BOOL      next_case = FALSE;
    NCS_BOOL      move_case = FALSE;
    NCS_BOOL      wildcard_req = FALSE;
    uns32         status; 
    m_MAS_LK_INIT;

    m_MAB_DBG_TRACE("\nmas_info_request():entered.");

    inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    if(inst == NULL)
    {
        m_LOG_MAB_NO_CB("mas_info_request()"); 
        m_MMGR_FREE_MAB_MSG(msg); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
    }

    m_MAS_LK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_LOCKED,&inst->lock);

    mib_req = msg->data.data.snmp;

    /* Sync the message to the Standby */ 
    m_MAS_RE_MIB_ARG_SYNC(inst, msg, mib_req->i_tbl_id); 

    switch(mib_req->i_op)
    {
        case NCSMIB_OP_REQ_NEXT:
        case NCSMIB_OP_REQ_NEXTROW:
            next_case = TRUE;
        break;

        case NCSMIB_OP_REQ_MOVEROW:
            move_case = TRUE;
        break;

        case NCSMIB_OP_REQ_CLI:
            if (mib_req->req.info.cli_req.i_wild_card == TRUE)
                wildcard_req = TRUE;
        break;

        default:
        break;
    }

    m_MAB_DBG_TRACE2_MAS_IN(inst,msg);

    if((tbl_rec = mas_row_rec_find(inst, mib_req->i_tbl_id)) == NULL)
    {
        /* Tell the MAC that we don't know about the owner of this table id */
        m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_FIND_REQ_TBL_FAILED, 
                              inst->vrid, mib_req->i_tbl_id);

        switch(msg->data.data.snmp->i_op)
        {
            case NCSMIB_OP_REQ_SETROW  :
            case NCSMIB_OP_REQ_TESTROW :
            case NCSMIB_OP_REQ_REMOVEROWS:
            case NCSMIB_OP_REQ_SETALLROWS:
                m_MMGR_FREE_BUFR_LIST(mib_req->req.info.setrow_req.i_usrbuf);
                mib_req->req.info.setrow_req.i_usrbuf = NULL;
            break;

            case NCSMIB_OP_REQ_CLI:
                m_MMGR_FREE_BUFR_LIST(mib_req->req.info.cli_req.i_usrbuf);
                mib_req->req.info.cli_req.i_usrbuf = NULL;
            break;

            default:
            break;
        }

        if (mib_req->i_op == NCSMIB_OP_REQ_CLI)
            mib_req->i_op = NCSMIB_OP_RSP_CLI_DONE;  /* say done with the CLI-REQ */
        else
            mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);

        mib_req->rsp.i_status = NCSCC_RC_NO_SUCH_TBL;

        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        
        /* sync done message to standby */ 
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

        if (mas_relay_msg_to_mac(msg,inst,TRUE) != NCSCC_RC_SUCCESS)
        {
            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_SNT_NO_TBL_RSP_FAILED,  
                            mib_req->i_tbl_id); 
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_SNT_NO_TBL_RSP, mib_req->i_tbl_id);
        return NCSCC_RC_SUCCESS;
    }

    /* MAS found the table-record */
    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_FIND_REQ_TBL_SUCCESS, 
                          inst->vrid, mib_req->i_tbl_id);

    if(move_case == TRUE)
    {
        NCS_SE*       se;
        uns8*        stream;

        if(tbl_rec->dfltr_regd != TRUE)
        {
            /* return failure to the originating OAC */
            mib_req->rsp.info.moverow_rsp.i_move_to = mib_req->req.info.moverow_req.i_move_to;
            mib_req->rsp.info.moverow_rsp.i_usrbuf = mib_req->req.info.moverow_req.i_usrbuf;
            mib_req->req.info.moverow_req.i_usrbuf = NULL;

            mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);
            mib_req->rsp.i_status = NCSCC_RC_FAILURE;

            /* release the locks */ 
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            
            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

            /* it also ralays msgs to oac... */
            if(mas_relay_msg_to_mac(msg,inst,TRUE) != NCSCC_RC_SUCCESS)
            {
                m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_DEF_FLTR_NOT_REGISTERED, 
                        mib_req->i_tbl_id);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            return NCSCC_RC_SUCCESS;
        } /* if(tbl_rec->dfltr_regd != TRUE) */

        /* push BACKTO stack element */
        if ((se = ncsstack_push(&msg->data.data.snmp->stack, NCS_SE_TYPE_BACKTO,
                                (uns8)sizeof(NCS_SE_BACKTO))) == NULL) 
        {
            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

            mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);
            mib_req->rsp.i_status = NCSCC_RC_FAILURE;
            
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

            mas_relay_msg_to_mac(msg,inst,TRUE); 

            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_PUSH_FISE_FAILED);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        /* push the MAS address, so that MAS receives MOVE_ROW response */ 
        stream = m_NCSSTACK_SPACE(se);
        mds_st_encode_mds_dest( &stream,&inst->my_vcard);
        ncs_encode_16bit( &stream,NCSMDS_SVC_ID_MAS);
        ncs_encode_16bit( &stream,inst->vrid);

        /* forward the MOVEROW request the asked OAA */ 
        {
            MDS_HDL  tx_mds_hdl = inst->mds_hdl;
            MDS_DEST   to_vcard = msg->data.data.snmp->req.info.moverow_req.i_move_to;

            /* release the locks */ 
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            
            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

            /* forward */
            if (mas_forward_msg(msg, MAB_OAC_REQ_HDLR, tx_mds_hdl,
                                to_vcard, NCSMDS_SVC_ID_OAC, TRUE) != NCSCC_RC_SUCCESS)
            {
                /* return failure to the originating OAC */
                se = ncsstack_pop(&msg->data.data.snmp->stack);  

                mib_req->rsp.info.moverow_rsp.i_move_to = mib_req->req.info.moverow_req.i_move_to;
                mib_req->rsp.info.moverow_rsp.i_usrbuf = mib_req->req.info.moverow_req.i_usrbuf;
                mib_req->req.info.moverow_req.i_usrbuf = NULL;
                mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);
                mib_req->rsp.i_status = NCSCC_RC_FAILURE;
                /* it also ralays msgs to oac... */
                if(mas_relay_msg_to_mac(msg,inst,TRUE) != NCSCC_RC_SUCCESS)
                {
                    m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_MOVEROW_MAC_SND_FAILED, 
                                      mib_req->i_tbl_id);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
                m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_MOVEROW_OAC_SND_FAILED, 
                                  mib_req->i_tbl_id);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            } /* forwardes the move-row request to OAA */ 

            /* Enjoy the success of sending MOVEROW */ 
            m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_FRWD_MOVEROW_MIB_REQ_TO_OAC); 
            
        }/* forwardes the move-row request to OAA */ 
        return NCSCC_RC_SUCCESS;  
    } /* if(move_case == TRUE) */ /* completed MOVEROW request handling */ 

    /* If it is a  wild-card request, then get the first fltr */
    if (wildcard_req == TRUE)
        match_fltr = tbl_rec->fltr_list;
    else
        match_fltr = mas_locate_dst_oac(inst, tbl_rec->fltr_list,
                                     (uns32*)mib_req->i_idx.i_inst_ids,
                                     mib_req->i_idx.i_inst_len,next_case);
    
    /* no filter match for this request */ 
    if (match_fltr == NULL)
    {
        /* No Default/Staging area for this particular table */ 
        if(tbl_rec->dfltr_regd == TRUE)
        {
            MDS_HDL  tx_mds_hdl = inst->mds_hdl;
            MDS_DEST   to_vcard = tbl_rec->dfltr.vcard;

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

            /* send the message to the staging area OAA */ 
            if(mas_forward_msg(msg,MAB_OAC_DEF_REQ_HDLR,tx_mds_hdl,to_vcard,NCSMDS_SVC_ID_OAC,TRUE) == NCSCC_RC_FAILURE)
            {
#if (NCS_MDS == 1)
                if (tbl_rec->dfltr.fltr_ids.active_fltr != NULL)
                {
                    MAB_LM_EVT        mle;
                    mle.i_args      = NULL;
                    mle.i_event     = MAS_MAB_MDS_FWD_FAILED;
                    mle.i_usr_key   = inst->hm_hdl;
                    mle.i_vrid      = inst->vrid;
                    mle.i_which_mab = NCSMAB_MAS;
                    inst->lm_cbfnc(&mle);

                    ncsmib_arg_free_resources(mib_req,TRUE);
                    m_MMGR_FREE_MAB_MSG(msg);
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_NO_OAC_FRWD);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
                else
                {
                    status = mas_inform_maa(msg, inst, mib_req, NCSCC_RC_INV_SPECIFIC_VAL); 

                    /* frreeing the message and resources to be taken care */
                    return status; 
                }
#else
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_NO_OAC_FRWD);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
#endif
            }
            else
            {
                m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_FRWD_MIB_REQ_TO_OAC);
            }

            m_MAB_DBG_TRACE("\nmas_info_request():left.");
            return NCSCC_RC_SUCCESS;
        } /* if(tbl_rec->dfltr_regd == TRUE) */

        /* Tell the MAC that we don't know about the owner of this object/row instance */

        /* all we can fill out is just the param_id value...
         * we have no idea if it's suppose to be an int or an oct string...
         * unless what we have is a SET/TEST REQ...
         */
        m_LOG_MAB_HDLN_II(NCSFL_SEV_ERROR, MAB_HDLN_MAS_FIND_OAC_FAILED, 
                          msg->data.data.snmp->i_tbl_id, msg->data.data.snmp->i_op);
        switch(msg->data.data.snmp->i_op)
        {
            case NCSMIB_OP_REQ_SETROW  :
            case NCSMIB_OP_REQ_TESTROW :
            case NCSMIB_OP_REQ_REMOVEROWS:
            case NCSMIB_OP_REQ_SETALLROWS:
                m_MMGR_FREE_BUFR_LIST(mib_req->req.info.setrow_req.i_usrbuf);
                mib_req->req.info.setrow_req.i_usrbuf = NULL;
            break;

            case NCSMIB_OP_REQ_CLI:
                m_MMGR_FREE_BUFR_LIST(mib_req->req.info.cli_req.i_usrbuf);
                mib_req->req.info.cli_req.i_usrbuf = NULL;
            break;

            case NCSMIB_OP_REQ_GET :
                memset(&mib_req->rsp.info.get_rsp.i_param_val,0,sizeof(NCSMIB_PARAM_VAL));
                mib_req->rsp.info.get_rsp.i_param_val.i_param_id = mib_req->req.info.get_req.i_param_id;
            break;

            case NCSMIB_OP_REQ_NEXT:
                memset(&mib_req->rsp.info.next_rsp.i_param_val,0,sizeof(NCSMIB_PARAM_VAL));
                mib_req->rsp.info.next_rsp.i_param_val.i_param_id = mib_req->req.info.next_req.i_param_id;
            break;

            case NCSMIB_OP_REQ_SET :
            case NCSMIB_OP_REQ_TEST:
                memset(&mib_req->rsp.info.set_rsp.i_param_val,0,sizeof(NCSMIB_PARAM_VAL));
                mib_req->rsp.info.set_rsp.i_param_val.i_param_id = mib_req->req.info.set_req.i_param_val.i_param_id;
            break;

            default:
            break;
        }

        if (wildcard_req == TRUE)
            mib_req->i_op = NCSMIB_OP_RSP_CLI_DONE;
        else
            mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);
            
        mib_req->rsp.i_status = NCSCC_RC_NO_INSTANCE;

        /* sync done message to standby */ 
        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

        /* finally inform the sad news to MAA */ 
        if(mas_relay_msg_to_mac(msg,inst,TRUE) != NCSCC_RC_SUCCESS)
        {
            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, mib_req->i_tbl_id);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_SNT_NO_INST_RSP);
        return NCSCC_RC_SUCCESS;
    } /* if (match_fltr == NULL) */

    /* found the destination OAC to fwd the request */ 
    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAS_FIND_OAC_SUCCESS);
    if ((next_case == TRUE) || (wildcard_req == TRUE))
    {
        /* get the fltr-id for the active anchor in the matched filter */
        if (match_fltr->fltr_ids.active_fltr == NULL)
        {
            /* application in ACTIVE role did not register this filter */ 
            MAB_LM_EVT        mle;
            mle.i_args      = NULL;
            mle.i_event     = MAS_MAB_MDS_FWD_FAILED;
            mle.i_usr_key   = inst->hm_hdl;
            mle.i_vrid      = inst->vrid;
            mle.i_which_mab = NCSMAB_MAS;
            inst->lm_cbfnc(&mle);

            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

            /* RESPONSE to MAA is missing */ 

            ncsmib_arg_free_resources(mib_req,TRUE);
            m_MMGR_FREE_MAB_MSG(msg);
            m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_NO_OAC_FRWD);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        if (match_fltr->next != NULL)
        {
            /* push this filter-id into the stack of the MIB request */ 
            if (mas_next_op_stack_push(inst, &mib_req->stack, 
                           match_fltr->fltr_ids.active_fltr->fltr_id) != NCSCC_RC_SUCCESS)
            {
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_PUSH_FISE_FAILED);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                ncsmib_arg_free_resources(mib_req,TRUE);
                m_MMGR_FREE_MAB_MSG(msg);

                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
        }
    } /* pushed the filter-id for the GETNEXT and CLI CE requests */

    /* forward the message to OAC */  
    {
        MDS_HDL  tx_mds_hdl = inst->mds_hdl;
        MDS_DEST   to_vcard = match_fltr->vcard;
        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);

        /* sync done message to standby */ 
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_req->i_tbl_id);

        /* send the message to the OAA */ 
        if(mas_forward_msg(msg,MAB_OAC_REQ_HDLR,tx_mds_hdl,to_vcard,NCSMDS_SVC_ID_OAC,TRUE) != NCSCC_RC_SUCCESS)
        {
#if (NCS_MDS == 1)
            if (match_fltr->fltr_ids.active_fltr != NULL)
            {
                MAB_LM_EVT        mle;
                mle.i_args      = NULL;
                mle.i_event     = MAS_MAB_MDS_FWD_FAILED;
                mle.i_usr_key   = inst->hm_hdl;
                mle.i_vrid      = inst->vrid;
                mle.i_which_mab = NCSMAB_MAS;
                inst->lm_cbfnc(&mle);

                ncsmib_arg_free_resources(mib_req,TRUE);
                m_MMGR_FREE_MAB_MSG(msg);
                m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_NO_OAC_FRWD);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            else
            {
                status = mas_inform_maa(msg, inst, mib_req, NCSCC_RC_INV_SPECIFIC_VAL); 

                /* freeing the message and resources to be taken care */
                return status; 
            }
#else
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_NO_OAC_FRWD);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
#endif
        }
        else
        {
            char addr_str[255] = {0};
            
            m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_FRWD_MIB_REQ_TO_OAC);
            
            /* convert the MDS_DEST into a string */
            if (m_NCS_NODE_ID_FROM_MDS_DEST(match_fltr->vcard) == 0)
               sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(match_fltr->vcard));
            else
               sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%lu",
                        m_NCS_NODE_ID_FROM_MDS_DEST(match_fltr->vcard), 0, (long)(match_fltr->vcard));

            /* now log the message */
            ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_NO_CB, MAB_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_DEBUG,
                       NCSFL_TYPE_TIC, MAB_HDLN_MAS_FRWD_MIB_REQ_TO_THIS_OAC, addr_str);
        }
    }/* forward the message to OAC */

    m_MAB_DBG_TRACE("\nmas_info_request():left.");
    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  mas_info_response
*****************************************************************************/
uns32 mas_info_response(MAB_MSG* msg)
{
    MAS_TBL*       inst;  
    MAS_FLTR*      match_fltr;
    MAS_FLTR*      rsp_fltr;
    MAS_ROW_REC*   tbl_rec; 
    NCSMIB_ARG*    mib_rsp;
    uns32          fltr_id;
    uns32*         inst_ids;
    uns32          inst_len;
    NCS_SE*        se;
    uns8*          stream;
    MDS_DEST       dst_vcard;
    uns16          dst_svc_id  = 0;
    uns16          vrid        = 0;
    NCS_BOOL       failed      = FALSE;

    MDS_HDL   tx_mds_hdl;
    MDS_DEST   to_vcard;

    m_MAS_LK_INIT;

    m_MAB_DBG_TRACE("\nmas_info_response():entered.");

    inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    if(inst == NULL)
    {
        m_LOG_MAB_NO_CB("mas_info_response()"); 
        m_MMGR_FREE_MAB_MSG(msg); 
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
    }

    m_MAS_LK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_LOCKED,&inst->lock);

    memset(&dst_vcard, 0, sizeof(dst_vcard));
    mib_rsp = msg->data.data.snmp;

    /* send the message to Standby MAS */ 
    m_MAS_RE_MIB_ARG_SYNC(inst, msg, mib_rsp->i_tbl_id); 

    if (mib_rsp->i_op == NCSMIB_OP_RSP_MOVEROW)
    {
        if(mib_rsp->rsp.i_status != NCSCC_RC_SUCCESS)
        {
            /* sync done message to standby */ 
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

            m_LOG_MAB_HDLN_I(NCSFL_SEV_ERROR, MAB_HDLN_MAS_MOVEROW_RSP_FAILED, 
                           mib_rsp->rsp.i_status);
            failed = TRUE;
            if((se = ncsstack_pop(&mib_rsp->stack)) == NULL)
            {
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_POP_SE_FAILED);
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }

            stream = m_NCSSTACK_SPACE(se);
            if(se->type == NCS_SE_TYPE_BACKTO)
            {
                mds_st_decode_mds_dest(&stream, &dst_vcard);
                dst_svc_id = ncs_decode_16bit(&stream);
                vrid       = ncs_decode_16bit(&stream);
            }
            else
            {
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_MOVEROW_POP_XSE);
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
            }

            /* KCQ: I'm paranoid ;-]  */
            if((inst->vrid != vrid) || (dst_svc_id != NCSMDS_SVC_ID_OAC))
            {
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_POP_XSE);
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
            }

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);

            if(mas_forward_msg(msg,MAB_OAC_RSP_HDLR,
                           inst->mds_hdl,dst_vcard,
                           (SS_SVC_ID)dst_svc_id,FALSE) == NCSCC_RC_FAILURE)
            {
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_MAS_ERR_OAA_SND_FAILED);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }

            return NCSCC_RC_SUCCESS;
        } /* if(mib_rsp->rsp.i_status != NCSCC_RC_SUCCESS) */

        if(failed == FALSE)
        {
            m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_MOVEROW_RSP_SUCCESS);
            if((tbl_rec = mas_row_rec_find(inst, mib_rsp->i_tbl_id)) == NULL)
            {
                m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_FIND_REQ_TBL_FAILED, 
                                      inst->vrid, mib_rsp->i_tbl_id);

                /* sync done message to standby */ 
                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }

            m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_FIND_REQ_TBL_SUCCESS, 
                                  inst->vrid, mib_rsp->i_tbl_id);

            if((match_fltr = mas_locate_dst_oac(inst, tbl_rec->fltr_list, 
                                                (uns32*)mib_rsp->i_idx.i_inst_ids,
                                                mib_rsp->i_idx.i_inst_len,FALSE )) == NULL)
            {
                /* notify the AKE and forward failure to the originator */
                MAB_LM_EVT        mle;
                MAB_LM_FLTR_NULL  fn;
                fn.i_fltr_id    = 0;
                memset(&fn.i_vcard, 0, sizeof(fn.i_vcard));
                mle.i_args      = (NCSCONTEXT)&fn;
                mle.i_event     = MAS_FLTR_MRRSP_NO_FLTR;
                mle.i_usr_key   = inst->hm_hdl;
                mle.i_vrid      = inst->vrid;
                mle.i_which_mab = NCSMAB_MAS;
                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_MRRSP_NO_FLTR);
                inst->lm_cbfnc(&mle);
                mib_rsp->rsp.i_status = NCSCC_RC_FAILURE;
            }

            if(match_fltr->fltr.is_move_row_fltr != TRUE)
            {
                MAB_LM_EVT        mle;
                MAB_LM_FLTR_NULL  fn;
                fn.i_fltr_id    = 0;
                memset(&fn.i_vcard, 0, sizeof(fn.i_vcard));
                mle.i_args      = (NCSCONTEXT)&fn;
                mle.i_event     = MAS_FLTR_MRRSP_NO_FLTR;
                mle.i_usr_key   = inst->hm_hdl;
                mle.i_vrid      = inst->vrid;
                mle.i_which_mab = NCSMAB_MAS;
                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_MRRSP_NO_FLTR);
                inst->lm_cbfnc(&mle);
                mib_rsp->rsp.i_status = NCSCC_RC_FAILURE;
            }
        } /* if (failed == FALSE) */

        if((se = ncsstack_pop(&mib_rsp->stack)) == NULL)
        {
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_MOVEROW_POP_XSE);

            /* sync done message to standby */ 
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        stream = m_NCSSTACK_SPACE(se);
        if(se->type == NCS_SE_TYPE_BACKTO)
        {
            mds_st_decode_mds_dest(&stream, &dst_vcard);
            dst_svc_id = ncs_decode_16bit(&stream);
            vrid       = ncs_decode_16bit(&stream);
        }
        else
        {
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_MOVEROW_POP_XSE);

            /* sync done message to standby */ 
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
        }

        /* KCQ: I'm paranoid ;-]  */
        if((inst->vrid != vrid) || (dst_svc_id != NCSMDS_SVC_ID_OAC))
        {
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_POP_XSE);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            
            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
        }

        if(mas_forward_msg(msg,MAB_OAC_RSP_HDLR, inst->mds_hdl,dst_vcard,
                           (SS_SVC_ID)dst_svc_id,FALSE) == NCSCC_RC_FAILURE)
        {
            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, NCSCC_RC_FAILURE);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            
            /* sync done message to standby */ 
            m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        
        /* sync done message to standby */ 
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

        m_MAB_DBG_TRACE("\nmas_info_response():left.");
        return NCSCC_RC_SUCCESS;
    } /* if(mib_rsp->i_op == NCSMIB_OP_RSP_MOVEROW) */

    /* if the respose for GETNEXT/CLI */ 
    if((se = ncsstack_pop(&mib_rsp->stack)) == NULL)
    {
        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_POP_SE_FAILED);

        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

        /* sync done message to standby */ 
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }   

    stream = m_NCSSTACK_SPACE(se);
    if(se->type == NCS_SE_TYPE_FILTER_ID)
    {
        fltr_id = ncs_decode_32bit(&stream);
        m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAS_POP_FISE_SUCCESS);
    }
    else
    {
        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_POP_FISE_FAILED);

        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

        /* sync done message to standby */ 
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
    }

    if((tbl_rec = mas_row_rec_find(inst, mib_rsp->i_tbl_id)) == NULL)
    {
        m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                    MAB_MAS_FIND_REQ_TBL_FAILED, 
                        inst->vrid, mib_rsp->i_tbl_id);

        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        
        /* sync done message to standby */ 
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    
    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_FIND_REQ_TBL_SUCCESS, 
                inst->vrid, mib_rsp->i_tbl_id);

    if((match_fltr = mas_fltr_find(inst,&(tbl_rec->fltr_list),
                                   fltr_id,msg->fr_anc,msg->fr_card)) == NULL)
    {
        m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_FLTR_FIND_FAILED,
                         mib_rsp->i_tbl_id, fltr_id, -1); 

        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

        /* sync done message to standby */ 
        m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_FLTR_FIND_SUCCESS,
                 mib_rsp->i_tbl_id, fltr_id, -1); 

    /* sync done message to standby */ 
    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    m_MAS_RE_MIB_ARG_SYNC_DONE(inst, msg, mib_rsp->i_tbl_id);

    /* CLI response for the wild-card request, forward the response to MAC
    * and make a CLI request and send it to the next instance of OAC.
    */
    if (mib_rsp->i_op == NCSMIB_OP_RSP_CLI)
    { 
        if (mib_rsp->rsp.info.cli_rsp.o_partial != TRUE)
        {
            /* Done with the processing of wild_card request for this filter */
            match_fltr->wild_card = TRUE;

            /* Get the next instance filter to which the wild_card request is required to
             * send.
             */
            match_fltr = mas_get_wild_card_fltr(&(tbl_rec->fltr_list));

            /* If it is a LAST/Complete response for the WC request made and all the 
             * instances are processed then send CLI_DONE response to MAC.
             */
            if (match_fltr == NULL)
            {
                /* Reset the wild_card flag of all the filters */
                mas_reset_wild_card_flg(&(tbl_rec->fltr_list));

                mib_rsp->i_op = NCSMIB_OP_RSP_CLI_DONE;

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                if (mas_relay_msg_to_mac(msg, inst, FALSE) != NCSCC_RC_SUCCESS)
                {
                    m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, NCSCC_RC_FAILURE);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO,MAB_HDLN_MAS_FRWD_NI_RSP_TO_MAC);
            }
            else 
            {
                /* Send the message to MAC such that it retains the MAC info in the msg stack
                 * and the msg is not freed up 
                 */
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);

                mib_rsp->rsp.info.cli_rsp.o_partial = TRUE;

                if (mas_cli_msg_to_mac(msg, inst) != NCSCC_RC_SUCCESS)
                {
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
                }

                /* For the wild-card request, still filters exists for processing.
                  The request to the next instance will be only sent when it is the 
                  last/complete response from the OAC */
                if (mas_next_op_stack_push(inst,&mib_rsp->stack,
                                 match_fltr->fltr_ids.active_fltr->fltr_id) != NCSCC_RC_SUCCESS)
                {
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_PUSH_FISE_FAILED);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                mib_rsp->i_op = NCSMIB_OP_REQ_CLI;
                mib_rsp->req.info.cli_req.i_cmnd_id = mib_rsp->rsp.info.cli_rsp.i_cmnd_id; 
                tx_mds_hdl = inst->mds_hdl;
                to_vcard = match_fltr->vcard;
                if (mas_forward_msg(msg,MAB_OAC_REQ_HDLR,tx_mds_hdl,
                                   to_vcard,NCSMDS_SVC_ID_OAC,FALSE) == NCSCC_RC_FAILURE)
                {
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_NO_OAC_FRWD);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO,MAB_HDLN_MAS_FRWD_MIB_REQ_TO_OAC);
            }
        }
        else /* A partial response arrived, so just forward it to MAC */
        {
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);

            if (mas_relay_msg_to_mac(msg, inst, FALSE) != NCSCC_RC_SUCCESS)
            {
                m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, NCSCC_RC_FAILURE);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }

            m_LOG_MAB_HEADLINE(NCSFL_SEV_INFO,MAB_HDLN_MAS_FRWD_NI_RSP_TO_MAC);
        }
    }  /* NCSMIB_OP_RSP_CLI */
    else /* Request is not of CLI type */
    {
        switch(mib_rsp->rsp.i_status)
        {
            /************************[   NO_SUCH_NAME CASE    ]*********************************/
            case NCSCC_RC_NO_INSTANCE:
                m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_RCV_NI_MIB_RSP);
                if (match_fltr->next == NULL)
                {
                    /* we are done... sure failure... tell the MAC... */
                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    if (mas_relay_msg_to_mac(msg,inst,FALSE) != NCSCC_RC_SUCCESS)
                    {
                        m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, NCSCC_RC_FAILURE);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAS_FRWD_NI_RSP_TO_MAC);
                }
                else 
                {
                    /* we still have other filters to visit */
                    match_fltr = match_fltr->next;
                    if(mas_next_op_stack_push(inst,&mib_rsp->stack,
                                            match_fltr->fltr_ids.active_fltr->fltr_id) != NCSCC_RC_SUCCESS)
                    {
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_PUSH_FISE_FAILED);
                        m_MAS_UNLK(&inst->lock);
                        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }

                    switch(mib_rsp->i_op)
                    {
                        case NCSMIB_OP_RSP_NEXT:
                            mib_rsp->i_op =   NCSMIB_OP_REQ_NEXT;
                            mib_rsp->req.info.next_req.i_param_id = mib_rsp->rsp.info.next_rsp.i_param_val.i_param_id;
                        break;

                        case NCSMIB_OP_RSP_NEXTROW:
                            mib_rsp->i_op = NCSMIB_OP_REQ_NEXTROW;
                        break;

                        default:
                            m_MAS_UNLK(&inst->lock);
                            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }

                    tx_mds_hdl = inst->mds_hdl;
                    to_vcard = match_fltr->vcard;
                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    if (mas_forward_msg(msg,MAB_OAC_REQ_HDLR,tx_mds_hdl,
                                        to_vcard,NCSMDS_SVC_ID_OAC,FALSE) == NCSCC_RC_FAILURE)
                    {
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_NO_OAC_FRWD);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAS_FRWD_MIB_REQ_TO_OAC);
                }
            break;
            /************************[   NO_SUCH_NAME CASE ENDS   ]*********************************/

            /****************************[   SUCCESS CASE   ]***********************************/
            case NCSCC_RC_SUCCESS:
                m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_RCV_OK_MIB_RSP);

                /* must find the initial match filter for the specified index */
                switch(mib_rsp->i_op)
                {
                    case NCSMIB_OP_RSP_NEXT:
                        inst_ids = (uns32*)mib_rsp->rsp.info.next_rsp.i_next.i_inst_ids;
                        inst_len = mib_rsp->rsp.info.next_rsp.i_next.i_inst_len;
                    break;

                    case NCSMIB_OP_RSP_NEXTROW:
                        inst_ids = (uns32*)mib_rsp->rsp.info.nextrow_rsp.i_next.i_inst_ids;
                        inst_len = mib_rsp->rsp.info.nextrow_rsp.i_next.i_inst_len;
                    break;

                    default:
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_RCV_X_MIB_RSP);
                        m_MAS_UNLK(&inst->lock);
                        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                rsp_fltr = mas_locate_dst_oac(inst, tbl_rec->fltr_list,inst_ids,inst_len,TRUE);
                if (rsp_fltr == NULL)
                {
                    m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_FLTR_FIND_FAILED,
                    mib_rsp->i_tbl_id, 0, -1); 
                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_FLTR_FIND_SUCCESS,
                mib_rsp->i_tbl_id, 0, rsp_fltr->fltr.type); 

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                if((fltr_id == rsp_fltr->fltr_ids.active_fltr->fltr_id) &&
                    (memcmp(&msg->fr_card, &rsp_fltr->vcard, sizeof(msg->fr_card)) == 0))
                {
                    /* good news... it's a real success... tell the MAC... */
                    if(mas_relay_msg_to_mac(msg,inst,FALSE) != NCSCC_RC_SUCCESS)
                    {
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_NO_OK_FRWD_TO_MAC);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_FRWD_OK_RSP_TO_MAC);
                }
                else if(rsp_fltr == match_fltr->next)
                {
                    /* good news... we got the right answer after all... */
                    if(mas_relay_msg_to_mac(msg,inst,FALSE) != NCSCC_RC_SUCCESS)
                    {
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_NO_OK_FRWD_TO_MAC);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_FRWD_OK_RSP_TO_MAC);
                }
                else
                {
                    if(match_fltr->next == NULL)
                    {
                        /* we are done... there's nothing else left... tell the MAC... */
                        mib_rsp->rsp.i_status = NCSCC_RC_NO_INSTANCE;
                        if(mas_relay_msg_to_mac(msg,inst,FALSE) != NCSCC_RC_SUCCESS)
                        {
                            m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, NCSCC_RC_FAILURE);
                            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                        }
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_FRWD_NI_RSP_TO_MAC);

                        m_MAB_DBG_TRACE("\nmas_info_response():left.");
                        return NCSCC_RC_SUCCESS;          
                    }

                    /* it's not over yet... */
                    /* push FILTER_ID stack element */
                    /* we still have other filters to visit */
                    match_fltr = match_fltr->next;
                    if(mas_next_op_stack_push(inst,&mib_rsp->stack,
                            match_fltr->fltr_ids.active_fltr->fltr_id) != NCSCC_RC_SUCCESS)
                    {
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_PUSH_FISE_FAILED);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }

                    switch(mib_rsp->i_op)
                    {
                        case NCSMIB_OP_RSP_NEXT:
                            mib_rsp->i_op =   NCSMIB_OP_REQ_NEXT;
                            mib_rsp->req.info.next_req.i_param_id = mib_rsp->rsp.info.next_rsp.i_param_val.i_param_id;
                        break;

                        case NCSMIB_OP_RSP_NEXTROW:
                            mib_rsp->i_op = NCSMIB_OP_REQ_NEXTROW;
                        break;

                        default:
                            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }

                    if(mas_forward_msg(msg,MAB_OAC_REQ_HDLR,inst->mds_hdl,
                                       match_fltr->vcard,NCSMDS_SVC_ID_OAC,FALSE) != NCSCC_RC_SUCCESS)
                    {
                        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_NO_OAC_FRWD);
                        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                    m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAS_FRWD_MIB_REQ_TO_OAC);
                }
            break;
            /****************************[   SUCCESS CASE ENDS   ]***********************************/

            /************************[   NO_SUCH_OBJECT CASE    ]*********************************/      
            case NCSCC_RC_NO_OBJECT:
            case NCSCC_RC_FAILURE:
            case NCSCC_RC_REQ_TIMOUT:
            case NCSCC_RC_INV_SPECIFIC_VAL:
            case NCSCC_RC_INV_VAL:
            case NCSCC_RC_NO_SUCH_TBL:
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                if(mas_relay_msg_to_mac(msg,inst,FALSE) != NCSCC_RC_SUCCESS)
                {
                    m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR,MAB_MAS_ERR_NO_NI_FRWD_TO_MAC, NCSCC_RC_FAILURE);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
                m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG,MAB_HDLN_MAS_FRWD_NI_RSP_TO_MAC);
            break;
            /************************[   NO_SUCH_OBJECT CASE ENDS   ]*********************************/


            default:
                m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_RCV_XSTAT_MIB_RSP);
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }  /* End if switch statement */
    }  /* REQUEST is not of CLI type */ 

    m_MAB_DBG_TRACE("\nmas_info_response():left.");
    return NCSCC_RC_SUCCESS;
}
  

/*****************************************************************************
  mas_info_register
*****************************************************************************/
uns32 mas_info_register(MAB_MSG* msg)
{
    MAS_TBL*               inst;
    MAS_REG*               reg_req  = &msg->data.data.reg;
    MAS_ROW_REC*           tbl_rec;
    MAS_ROW_REC*           same_as_tbl_rec;
    MAS_FLTR*              new_fltr;
    MAS_FLTR*              tmp_fltr;
    MAS_FLTR               *fltr; 
    MAS_FLTR_FNC           match_fnc;
    NCS_BOOL                do_range = FALSE; /* SMM do initing early if possible */
    NCS_BOOL                exact_filter = FALSE; 
    uns32                   status = NCSCC_RC_FAILURE; 
    uns32                  o_fltr_id = 0; 
    V_DEST_RL              fr_oaa_role = 0;


    m_MAS_LK_INIT;

    m_MAB_DBG_TRACE("\nmas_info_register():entered.");

    inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    if(inst == NULL)
    {
        m_MMGR_FREE_MAB_MSG(msg);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
    }

    m_MAS_LK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_LOCKED,&inst->lock);

    /* send the message to STANDBY MAS */ 
    m_MAS_RE_REG_UNREG_SYNC(inst, msg, reg_req->tbl_id); 


    /* get the role of this anchor value */ 
    status = mas_get_crd_role(inst->mds_hdl, msg->fr_card, msg->fr_anc, &fr_oaa_role);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* return FAILURE */  /* Should inform OAA  -- TBD */ 
        /* now log msg->fr_anc */
        ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_ANC, MAB_FC_HDLN,
                    NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                    "TF", (double)msg->fr_anc);
        /* log the received filter */  
        m_LOG_MAB_FLTR_DATA(NCSFL_SEV_NOTICE, inst->vrid, reg_req->tbl_id, reg_req->fltr_id, &reg_req->fltr);
        mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        m_MMGR_FREE_MAB_MSG(msg);
        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    /* log the received filter */  
    m_LOG_MAB_FLTR_DATA(NCSFL_SEV_DEBUG, inst->vrid, reg_req->tbl_id, reg_req->fltr_id, &reg_req->fltr);

    /* set the verification function pointer based on the filter type */ 
    switch(reg_req->fltr.type)
    {
        case NCSMAB_FLTR_SAME_AS:
            match_fnc = mas_classify_same_as;
        break;

        case NCSMAB_FLTR_ANY:
            match_fnc = mas_classify_any;
        break;

        case NCSMAB_FLTR_RANGE:
            match_fnc = mas_classify_range;
            do_range = TRUE;
        break;

        case NCSMAB_FLTR_DEFAULT:
            match_fnc = NULL;
        break;

        case NCSMAB_FLTR_EXACT:
            match_fnc = mas_classify_exact;
            exact_filter = TRUE; 
        break;

        case NCSMAB_FLTR_SCALAR:
        default:
        {
            MAB_LM_EVT mle;
            mle.i_args      = (NCSCONTEXT)reg_req->fltr.type;
            mle.i_event     = MAS_FLTR_TYPE_REG_UNSUPP;
            mle.i_usr_key   = inst->hm_hdl;
            mle.i_vrid      = inst->vrid;
            mle.i_which_mab = NCSMAB_MAS;
            m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FTYPE_REG_UNSUPP);
            inst->lm_cbfnc(&mle);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);
            
            /* Inform STANDBY MAS about SYNC DONE */ 

            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_NEW_MAS_FLTR_FAILED);
            m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_RCV_X_FLTR_REG_REQ);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
    }/* endof filter type checking */ 

    if((tbl_rec = mas_row_rec_find(inst,reg_req->tbl_id)) == NULL)
    {
        /* add a new table -id in the hash table, for this filter */ 
        tbl_rec = m_MMGR_ALLOC_MAS_ROW_REC;
        if(tbl_rec == NULL)
        {
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_ROW_REC_ALLOC_FAILED,
                            "mas_info_register()");
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr); 
            m_MMGR_FREE_MAB_MSG(msg);

            /* Inform STANDBY MAS about SYNC DONE */ 

            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        memset(tbl_rec,0,sizeof(MAS_ROW_REC));
        tbl_rec->tbl_id = reg_req->tbl_id;
        tbl_rec->ss_id = msg->fr_svc; 
        tbl_rec->dfltr_regd = FALSE;

        m_MAB_DBG_TRACE2_MAS_TR_ALLOC(tbl_rec);

        /* add the new table into the HASH */ 
        mas_row_rec_put(inst,tbl_rec);
        m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_NEW_TBL_ROW_REC_SUCCESS);
    }
    else
    {
        /* if this table is trying to be owned by somebody else? */ 
        if(tbl_rec->ss_id != msg->fr_svc)
        {
            MAB_LM_EVT        mle;
            MAB_LM_FLTR_SS_MM fsmm;
            fsmm.i_tbl_ss_id  = tbl_rec->ss_id;
            fsmm.i_fltr_ss_id = msg->fr_svc;
            mle.i_args        = (NCSCONTEXT)&fsmm;
            mle.i_event       = MAS_FLTR_REG_SS_MISMATCH;
            mle.i_usr_key     = inst->hm_hdl;
            mle.i_vrid        = inst->vrid;
            mle.i_which_mab   = NCSMAB_MAS;
            m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_REG_SS_MM);
            inst->lm_cbfnc(&mle);

            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr); 
            m_MMGR_FREE_MAB_MSG(msg);
            m_LOG_MAB_HEADLINE(NCSFL_SEV_CRITICAL, MAB_NEW_MAS_FLTR_FAILED);
            
            /* Inform STANDBY MAS about SYNC DONE */ 

            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
    }

    /* Default filter addition */ 
    if(reg_req->fltr.type == NCSMAB_FLTR_DEFAULT)
    {
        /* below if is added to find duplicate fltr registration when vdest is different */
        if ((tbl_rec->dfltr_regd == TRUE)&&
                        (memcmp(&tbl_rec->dfltr.vcard,&msg->fr_card, sizeof(msg->fr_card) != 0)))
        {
            int8         addr_str[200]={0};
            int8         *str_ptr = addr_str;

            /* log the failure*/
            m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST);
            sprintf(addr_str, " For Env-Id: %d, Table-Id :%d, ", inst->vrid, tbl_rec->tbl_id);
            str_ptr = str_ptr + strlen(addr_str);
            if (m_NCS_NODE_ID_FROM_MDS_DEST(tbl_rec->dfltr.vcard) == 0)
                sprintf(str_ptr, "Existing VDEST:%d, ",
                        m_MDS_GET_VDEST_ID_FROM_MDS_DEST(tbl_rec->dfltr.vcard));
            else
                    sprintf(str_ptr, "Existing ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%lu",
                            m_NCS_NODE_ID_FROM_MDS_DEST(tbl_rec->dfltr.vcard), 0,
                            (long)(tbl_rec->dfltr.vcard));
            
            str_ptr = addr_str;
            str_ptr = str_ptr + strlen(addr_str);

            if (m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card) == 0)
                sprintf(str_ptr, "Request From VDEST:%d, ",
                        m_MDS_GET_VDEST_ID_FROM_MDS_DEST(msg->fr_card));
            else
                sprintf(str_ptr, "Request From ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%lu",
                        m_NCS_NODE_ID_FROM_MDS_DEST(msg->fr_card), 0,
                        (long)(msg->fr_card));

            /* now log the OAA address whose filters are getting wiped off */
            ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_NO_CB, MAB_FC_HDLN,
                       NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE,
                       NCSFL_TYPE_TIC, MAB_HDLN_MAS_DUP_DEF_FLTR, addr_str);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
        if (((tbl_rec->fltr_list != NULL) && (tbl_rec->fltr_list->fltr.type != NCSMAB_FLTR_RANGE)) ||
           ((tbl_rec->fltr_list != NULL) && (tbl_rec->fltr_list->fltr.type != NCSMAB_FLTR_EXACT)))
        {
            MAB_LM_EVT          mle;
            MAB_LM_FLTR_TYPE_MM ftm;
            ftm.i_xst_type  = tbl_rec->fltr_list->fltr.type;
            ftm.i_new_type  = reg_req->fltr.type;
            mle.i_args      = (NCSCONTEXT)&ftm;
            mle.i_event     = MAS_FLTR_TYPE_REG_MISMATCH;
            mle.i_usr_key   = inst->hm_hdl;
            mle.i_vrid      = inst->vrid;
            mle.i_which_mab = NCSMAB_MAS;
            m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FTYPE_REG_MM);
            inst->lm_cbfnc(&mle);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);

            /* Inform STANDBY MAS about SYNC DONE */ 
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        /* make sure that default filter is not already registered for this Anchor value */   
        status = mab_fltrid_list_get(&tbl_rec->dfltr.fltr_ids, msg->fr_anc, &o_fltr_id);
        if ((tbl_rec->dfltr_regd == TRUE) && (status == NCSCC_RC_SUCCESS)
#if (NCS_MAS_RED == 1)
            && (inst->red.ha_state != NCS_APP_AMF_HA_STATE_STANDBY)
#endif       
            )
        {
            MAB_LM_EVT            mle;
            MAB_LM_FLTR_XST       fx;
            fx.i_fltr_id    = 0;
            fx.i_vcard      = tbl_rec->dfltr.vcard;
            mle.i_args      = (NCSCONTEXT)&fx;
            mle.i_event     = MAS_FLTR_REG_EXIST;
            mle.i_usr_key   = inst->hm_hdl;
            mle.i_vrid      = inst->vrid;
            mle.i_which_mab = NCSMAB_MAS;
            m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_REG_EXIST);
            inst->lm_cbfnc(&mle);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

            m_MMGR_FREE_MAB_MSG(msg);
            /*  Inform STANDBY MAS about SYNC DONE */ 
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

#if (NCS_MAS_RED == 1)
        if ((tbl_rec->dfltr_regd == TRUE) && (status == NCSCC_RC_SUCCESS) &&
            (inst->red.ha_state == NCS_APP_AMF_HA_STATE_STANDBY))
        {
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);

            /* Inform STANDBY MAS about SYNC DONE */ 
            
            return NCSCC_RC_SUCCESS;
        }
#endif

        /* Update the default filter OAA details */  
        tbl_rec->dfltr_regd = TRUE;
        tbl_rec->dfltr.vcard = msg->fr_card;
        status = mab_fltrid_list_add(&tbl_rec->dfltr.fltr_ids, msg->fr_anc, fr_oaa_role, 1); 
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the error */ 
            if (status == NCSCC_RC_DUPLICATE_FLTR)  
            {
                m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST); 
            }
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);
            
            /* Inform STANDBY MAS about SYNC DONE */ 

            return status; 
        }
        m_MAB_DBG_TRACE2_MAS_DF_OP(inst,msg,tbl_rec,MFM_CREATE);
    } /* if(reg_req->fltr.type == NCSMAB_FLTR_DEFAULT) */
    else /* for the filters other than DEFAULT */
    {
        /* create the new filter */ 
        new_fltr = mas_fltr_create(inst, &(reg_req->fltr), msg->fr_card, fr_oaa_role,
                           msg->fr_anc, reg_req->fltr_id, match_fnc);
        if (new_fltr == NULL)
        {
            /* problem in creating the new filter, log the error */ 
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr); 
            m_MMGR_FREE_MAB_MSG(msg);
            m_LOG_MAB_HEADLINE(NCSFL_SEV_CRITICAL, MAB_NEW_MAS_FLTR_FAILED);
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        } /* failure in creating the new filter */ 

        /* log that filter creation is success, not yet added to the existing list */ 
        m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_NEW_MAS_FLTR_SUCCESS);

        if(do_range)
        {
            /* process the RANGE filter */ 
            NCSMAB_FLTR* mf;
            MAS_FLTR*   mas_fltr;
            int         min_res;
            int         max_res;
            /* Is this the first range filter? */  
            if(tbl_rec->fltr_list != NULL)
            {              
                /* filter types must match and filter range offsets must match also */
                mf = &tbl_rec->fltr_list->fltr;

                if(mf->type != new_fltr->fltr.type)
                {
                MAB_LM_EVT          mle;
                MAB_LM_FLTR_TYPE_MM tmm;
                tmm.i_xst_type    = mf->type;
                tmm.i_new_type    = new_fltr->fltr.type;
                mle.i_args        = (NCSCONTEXT)&tmm;
                mle.i_event       = MAS_FLTR_TYPE_REG_MISMATCH;
                mle.i_usr_key     = inst->hm_hdl;
                mle.i_vrid        = inst->vrid;
                mle.i_which_mab   = NCSMAB_MAS;
                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FTYPE_REG_MM);
                inst->lm_cbfnc(&mle);

                    /* free the new filter */  
                    m_MAS_FLTR_FREE(new_fltr);

                    /* free the received message */  
                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                    m_MMGR_FREE_MAB_MSG(msg);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
                else if((mf->fltr.range.i_bgn_idx != new_fltr->fltr.fltr.range.i_bgn_idx) ||
                (mf->fltr.range.i_idx_len != new_fltr->fltr.fltr.range.i_idx_len))
                {
                MAB_LM_EVT          mle;
                MAB_LM_FLTR_SIZE_MM smm;
                smm.i_xst_bgn_idx = mf->fltr.range.i_bgn_idx;
                smm.i_new_bgn_idx = new_fltr->fltr.fltr.range.i_bgn_idx;
                smm.i_xst_idx_len = mf->fltr.range.i_idx_len;
                smm.i_new_idx_len = new_fltr->fltr.fltr.range.i_idx_len;
                mle.i_args        = (NCSCONTEXT)&smm;
                mle.i_event       = MAS_FLTR_SIZE_REG_MISMATCH;
                mle.i_usr_key     = inst->hm_hdl;
                mle.i_vrid        = inst->vrid;
                mle.i_which_mab   = NCSMAB_MAS;
                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FSIZE_REG_MM);
                inst->lm_cbfnc(&mle);

                    /* free the just created filter */  
                    m_MAS_FLTR_FREE(new_fltr);

                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    m_MMGR_FREE_MAB_MSG(msg);

                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                /* see if there's already a filter with the given fltr_id/vcard pair registered */
                fltr = mas_fltr_find(inst,&tbl_rec->fltr_list, reg_req->fltr_id,
                                     msg->fr_anc,new_fltr->vcard); 
                if((fltr != NULL) 
#if (NCS_MAS_RED == 1)           
                    && (inst->red.ha_state != NCS_APP_AMF_HA_STATE_STANDBY)
#endif           
                )
                {
                    MAB_LM_EVT      mle;
                    MAB_LM_FLTR_XST fx;
                    fx.i_fltr_id     = reg_req->fltr_id;
                    fx.i_vcard       = new_fltr->vcard;
                    mle.i_args       = (NCSCONTEXT)&fx;
                    mle.i_event      = MAS_FLTR_REG_EXIST;
                    mle.i_usr_key    = inst->hm_hdl;
                    mle.i_vrid       = inst->vrid;
                    mle.i_which_mab  = NCSMAB_MAS;
                    m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST);
                    m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                tbl_rec->tbl_id, fltr, new_fltr);
                    inst->lm_cbfnc(&mle);

                    /* free the just created filter */  
                    m_MAS_FLTR_FREE(new_fltr);

                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                    m_MMGR_FREE_MAB_MSG(msg);

                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

#if (NCS_MAS_RED == 1)
                /* verify in the stand-by process */ 
                if((fltr != NULL) && (inst->red.ha_state == NCS_APP_AMF_HA_STATE_STANDBY)) 
                {
                    m_MAS_FLTR_FREE(new_fltr);

                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                    m_MMGR_FREE_MAB_MSG(msg);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return NCSCC_RC_SUCCESS;
                }    
#endif        

                /* find the insertion point and check for overlapping 
                and identical ranges for range filters 
                */
                for(mas_fltr = tbl_rec->fltr_list; mas_fltr != NULL; mas_fltr = mas_fltr->next)
                {
                    /* do identical filter check first */
                    m_MAB_FLTR_CMP(min_res,
                             mas_fltr->fltr.fltr.range.i_min_idx_fltr,
                             new_fltr->fltr.fltr.range.i_min_idx_fltr,
                             new_fltr->fltr.fltr.range.i_idx_len);

                    if(min_res == 0)
                    {
                        m_MAB_FLTR_CMP(max_res, mas_fltr->fltr.fltr.range.i_max_idx_fltr,
                                  new_fltr->fltr.fltr.range.i_max_idx_fltr, new_fltr->fltr.fltr.range.i_idx_len);
                        if(max_res == 0)
                        {
                            if(new_fltr->fltr.is_move_row_fltr != mas_fltr->fltr.is_move_row_fltr)
                            {
                                MAB_LM_EVT        mle;
                                MAB_LM_FLTR_OVRLP fo;
                                fo.i_xst_fltr   = mas_fltr;
                                fo.i_new_fltr   = new_fltr;
                                mle.i_args      = (NCSCONTEXT)&fo;
                                mle.i_event     = MAS_FLTR_REG_OVERLAP;
                                mle.i_usr_key   = inst->hm_hdl;
                                mle.i_vrid      = inst->vrid;
                                mle.i_which_mab = NCSMAB_MAS;
                                m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_OVRLAP_FLTR);
                                m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                            tbl_rec->tbl_id, mas_fltr, new_fltr);           
                                inst->lm_cbfnc(&mle);

                                m_MAS_FLTR_FREE(new_fltr);

                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                m_MMGR_FREE_MAB_MSG(msg);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                            }

                            if(memcmp(&mas_fltr->vcard, &new_fltr->vcard, sizeof(mas_fltr->vcard)) != 0)
                            {
                                MAB_LM_EVT        mle;
                                MAB_LM_FLTR_OVRLP fo;
                                fo.i_xst_fltr   = mas_fltr;
                                fo.i_new_fltr   = new_fltr;
                                mle.i_args      = (NCSCONTEXT)&fo;
                                mle.i_event     = MAS_FLTR_REG_OVERLAP;
                                mle.i_usr_key   = inst->hm_hdl;
                                mle.i_vrid      = inst->vrid;
                                mle.i_which_mab = NCSMAB_MAS;
                                m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_OVRLAP_FLTR);
                                m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                            tbl_rec->tbl_id, mas_fltr, new_fltr);
                                inst->lm_cbfnc(&mle);

                                m_MAS_FLTR_FREE(new_fltr);

                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                m_MMGR_FREE_MAB_MSG(msg);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                            }

                            /* MAS has the same filter information from the same VDEST.  
                             * Move the anchor, fltr-id association from the just created filter
                             * to the existing filter in MAS. (No move, just create a new)
                             */
                            /* we have an identical filter here */
                            status = mab_fltrid_list_add(&mas_fltr->fltr_ids, msg->fr_anc, fr_oaa_role, reg_req->fltr_id);
                            if (status != NCSCC_RC_SUCCESS)
                            {
                                MAB_LM_EVT            mle;
                                MAB_LM_FLTR_XST       fx;
                                fx.i_fltr_id    = 0;
                                fx.i_vcard      = tbl_rec->dfltr.vcard;
                                mle.i_args      = (NCSCONTEXT)&fx;
                                mle.i_event     = MAS_FLTR_REG_EXIST;
                                mle.i_usr_key   = inst->hm_hdl;
                                mle.i_vrid      = inst->vrid;
                                mle.i_which_mab = NCSMAB_MAS;
                                m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST);
                                m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                            tbl_rec->tbl_id, mas_fltr, new_fltr);
                                inst->lm_cbfnc(&mle);

                                m_MAS_FLTR_FREE(new_fltr);

                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                m_MMGR_FREE_MAB_MSG(msg);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                            }

                            m_MAS_FLTR_FREE(new_fltr);
                            m_MAB_DBG_TRACE2_MAS_MF_OP(inst,mas_fltr,tbl_rec->tbl_id,MFM_MODIFY);

                            /* Sync Done message to the Standby */ 
                            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                            m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                            m_MAS_UNLK(&inst->lock);
                            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                            m_MAB_DBG_TRACE("\nmas_info_register():left.");
                            return NCSCC_RC_SUCCESS;
                        }/*  if(max_res == 0) */
                    }/*  if(min_res == 0) */

                    /* do overlapping range check */

                    m_MAB_FLTR_CMP(min_res,
                             mas_fltr->fltr.fltr.range.i_max_idx_fltr,
                             new_fltr->fltr.fltr.range.i_min_idx_fltr,
                             new_fltr->fltr.fltr.range.i_idx_len);
                    if(min_res == 0)
                    {
                        if(new_fltr->fltr.is_move_row_fltr == TRUE)
                        {
                            if(mas_fltr->fltr.is_move_row_fltr == TRUE)
                            {
                                mas_fltr->vcard = new_fltr->vcard;
                                status = mab_fltrid_list_add(&mas_fltr->fltr_ids, msg->fr_anc, 
                                                             fr_oaa_role, reg_req->fltr_id);
                                if (status != NCSCC_RC_SUCCESS)
                                {
                                    m_MAS_FLTR_FREE(new_fltr);

                                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                    m_MMGR_FREE_MAB_MSG(msg);

                                    m_MAS_UNLK(&inst->lock);
                                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                    return NCSCC_RC_FAILURE; 
                                }

                                m_MAS_FLTR_FREE(new_fltr);
                                m_MAB_DBG_TRACE2_MAS_MF_OP(inst,mas_fltr,tbl_rec->tbl_id,MFM_MODIFY);

                                /* Sync Done message to the Standby */ 
                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                m_MAB_DBG_TRACE("\nmas_info_register():left.");
                                return NCSCC_RC_SUCCESS;
                            }
                        }
                        {
                            MAB_LM_EVT        mle;
                            MAB_LM_FLTR_OVRLP fo;
                            fo.i_xst_fltr   = mas_fltr;
                            fo.i_new_fltr   = new_fltr;
                            mle.i_args      = (NCSCONTEXT)&fo;
                            mle.i_event     = MAS_FLTR_REG_OVERLAP;
                            mle.i_usr_key   = inst->hm_hdl;
                            mle.i_vrid      = inst->vrid;
                            mle.i_which_mab = NCSMAB_MAS;
                            m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_OVRLAP_FLTR);
                            m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                        tbl_rec->tbl_id, mas_fltr, new_fltr);
                            inst->lm_cbfnc(&mle);

                            m_MAS_FLTR_FREE(new_fltr);

                            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                            mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                            m_MMGR_FREE_MAB_MSG(msg);

                            m_MAS_UNLK(&inst->lock);
                            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                        }
                    } /* if(min_res == 0) */
                    else if(min_res > 0)  
                    {
                        /* this case can turn out to be a good one when 
                        we just looked at the first filter
                        in the list and the new filter needs to go before this filter
                        */
                        if(mas_fltr == tbl_rec->fltr_list)
                        {
                            m_MAB_FLTR_CMP(max_res, new_fltr->fltr.fltr.range.i_max_idx_fltr,
                                         mas_fltr->fltr.fltr.range.i_min_idx_fltr, new_fltr->fltr.fltr.range.i_idx_len);           

                            if(max_res >= 0)
                            {
                                MAB_LM_EVT        mle;
                                MAB_LM_FLTR_OVRLP fo;
                                fo.i_xst_fltr   = mas_fltr;
                                fo.i_new_fltr   = new_fltr;
                                mle.i_args      = (NCSCONTEXT)&fo;
                                mle.i_event     = MAS_FLTR_REG_OVERLAP;
                                mle.i_usr_key   = inst->hm_hdl;
                                mle.i_vrid      = inst->vrid;
                                mle.i_which_mab = NCSMAB_MAS;
                                m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_OVRLAP_FLTR);
                                m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                            tbl_rec->tbl_id, mas_fltr, new_fltr);
                                inst->lm_cbfnc(&mle);

                                m_MAS_FLTR_FREE(new_fltr);

                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                m_MMGR_FREE_MAB_MSG(msg);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);

                                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                            }
                            else if(max_res < 0)
                            {
                                /* this is an ugly fix to handle insersion before the first filter */
                                tmp_fltr = tbl_rec->fltr_list;
                                tbl_rec->fltr_list = 0;
                                mas_fltr_put(&(tbl_rec->fltr_list),new_fltr);
                                new_fltr->next = tmp_fltr;

                                m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);

                                /* Sync Done message to the Standby */ 
                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                m_MAB_DBG_TRACE("\nmas_info_register():left.");
                                return NCSCC_RC_SUCCESS;
                            }
                        }
                        else /* if(mas_fltr == tbl_rec->fltr_list) */
                        {
                            MAB_LM_EVT        mle;
                            MAB_LM_FLTR_OVRLP fo;
                            fo.i_xst_fltr   = mas_fltr;
                            fo.i_new_fltr   = new_fltr;
                            mle.i_args      = (NCSCONTEXT)&fo;
                            mle.i_event     = MAS_FLTR_REG_OVERLAP;
                            mle.i_usr_key   = inst->hm_hdl;
                            mle.i_vrid      = inst->vrid;
                            mle.i_which_mab = NCSMAB_MAS;
                            m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_OVRLAP_FLTR);
                            m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                        tbl_rec->tbl_id, mas_fltr, new_fltr);
                            inst->lm_cbfnc(&mle);

                            m_MAS_FLTR_FREE(new_fltr);

                            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                            mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                            m_MMGR_FREE_MAB_MSG(msg);

                            m_MAS_UNLK(&inst->lock);
                            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                        }
                    } /* else if(min_res > 0) */ 
                    else if(min_res < 0)
                    {
                        if(mas_fltr->next != NULL)
                        {
                            m_MAB_FLTR_CMP(max_res, new_fltr->fltr.fltr.range.i_max_idx_fltr,
                                           mas_fltr->next->fltr.fltr.range.i_min_idx_fltr,
                                           new_fltr->fltr.fltr.range.i_idx_len);
                            if(max_res == 0)
                            {
                                if(new_fltr->fltr.is_move_row_fltr == TRUE)
                                {
                                    if(mas_fltr->next->fltr.is_move_row_fltr == TRUE)
                                    {
                                        mas_fltr->next->vcard = new_fltr->vcard;
                                        status = mab_fltrid_list_add(&mas_fltr->next->fltr_ids, 
                                                        msg->fr_anc, fr_oaa_role, reg_req->fltr_id);
                                        if (status != NCSCC_RC_SUCCESS)
                                        {
                                            m_MAS_FLTR_FREE(new_fltr);

                                            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                            mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                            m_MMGR_FREE_MAB_MSG(msg);

                                            m_MAS_UNLK(&inst->lock);
                                            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                            return NCSCC_RC_FAILURE; 
                                        }

                                        m_MAS_FLTR_FREE(new_fltr);

                                        m_MAB_DBG_TRACE2_MAS_MF_OP(inst,mas_fltr->next,tbl_rec->tbl_id,MFM_MODIFY);

                                        /* Sync Done message to the Standby */ 
                                        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                        m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                                        m_MAS_UNLK(&inst->lock);
                                        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                        m_MAB_DBG_TRACE("\nmas_info_register():left.");
                                        return NCSCC_RC_SUCCESS;
                                    }
                                }
                                {
                                    MAB_LM_EVT        mle;
                                    MAB_LM_FLTR_OVRLP fo;
                                    fo.i_xst_fltr   = mas_fltr;
                                    fo.i_new_fltr   = new_fltr;
                                    mle.i_args      = (NCSCONTEXT)&fo;
                                    mle.i_event     = MAS_FLTR_REG_OVERLAP;
                                    mle.i_usr_key   = inst->hm_hdl;
                                    mle.i_vrid      = inst->vrid;
                                    mle.i_which_mab = NCSMAB_MAS;
                                    m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_OVRLAP_FLTR);
                                    m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, 
                                                                tbl_rec->tbl_id, mas_fltr->next, new_fltr);
                                    inst->lm_cbfnc(&mle);

                                    m_MAS_FLTR_FREE(new_fltr);

                                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                    m_MMGR_FREE_MAB_MSG(msg);

                                    m_MAS_UNLK(&inst->lock);
                                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                                }
                            }
                            else if(max_res > 0)
                                continue;
                            else if(max_res < 0)
                            {
                                mas_fltr_put(&mas_fltr,new_fltr);

                                m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);

                                /* Sync Done message to the Standby */ 
                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                m_MAB_DBG_TRACE("\nmas_info_register():left.");
                                return NCSCC_RC_SUCCESS;
                            }
                        }/* if(mas_fltr->next != NULL) */ 
                        else 
                        {
                            mas_fltr_put(&mas_fltr,new_fltr);

                            m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);

                            /* Sync Done message to the Standby */ 
                            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                            m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                            m_MAS_UNLK(&inst->lock);
                            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                            m_MAB_DBG_TRACE("\nmas_info_register():left.");
                            return NCSCC_RC_SUCCESS;
                        }
                    } /* else if(min_res < 0) */ 
                }/* for(mas_fltr = tbl_rec->fltr_list; mas_fltr != NULL; mas_fltr = mas_fltr->next) */
            } /* if(tbl_rec->fltr_list != NULL) */
            else
            {
                /* this is the first range filter */ 
                mas_fltr_put(&(tbl_rec->fltr_list),new_fltr);
                m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);
            }
            /*******[ FREE STUFF FROM RANGE HERE ]*************/
        } /* if(do_range) */
        else if (exact_filter == TRUE)
        {
            /* process the exact filter */
            status = mas_exact_fltr_process(msg, inst, tbl_rec, new_fltr, 
                                    reg_req->fltr_id, fr_oaa_role);
            if (status != NCSCC_RC_SUCCESS)
            {
                /* log the error */
                if (status == NCSCC_RC_DUPLICATE_FLTR)  
                {
                    m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST); 
                    m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, tbl_rec->tbl_id, tbl_rec->fltr_list, new_fltr);  
                }
                m_MAS_FLTR_FREE(new_fltr);

                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                m_MMGR_FREE_MAB_MSG(msg);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return status; 
            }
        }
        else  /* for all the other kind of filters */ 
        {
            /* do a check for identical filters first */
            {
                NCS_BOOL     fltr_matched = FALSE;
                MAS_FLTR*   cur_fltr = tbl_rec->fltr_list;

                if(cur_fltr != NULL)
                {
                    if(reg_req->fltr.type == cur_fltr->fltr.type)
                    {
                        switch(reg_req->fltr.type)
                        {
                            case NCSMAB_FLTR_SAME_AS:
                                if(cur_fltr->fltr.fltr.same_as.i_table_id == new_fltr->fltr.fltr.same_as.i_table_id)
                                    fltr_matched = TRUE;
                            break;
                            case NCSMAB_FLTR_ANY:
                                fltr_matched = TRUE;
                            break;
                            default:
                            break;
                        }

                        if(fltr_matched == TRUE)
                        {
                            if(memcmp(&cur_fltr->vcard, &new_fltr->vcard, sizeof(new_fltr->vcard)) == 0)
                            {
#if(NCS_MAS_RED == 1)
                                o_fltr_id = 0; 
                                status = mab_fltrid_list_get(&cur_fltr->fltr_ids, msg->fr_anc, &o_fltr_id);
                                if ((status == NCSCC_RC_SUCCESS) &&
                                (o_fltr_id == reg_req->fltr_id) &&
                                (inst->red.ha_state == NCS_APP_AMF_HA_STATE_STANDBY))
                                {
                                m_MAS_FLTR_FREE(new_fltr);
                                /* We are a backup and we already have this filter */
                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                m_MMGR_FREE_MAB_MSG(msg);
                                m_MAB_DBG_TRACE("\nmas_info_register():left.");
                                return NCSCC_RC_SUCCESS;
                                }
#endif
                                /* add the filter-id to anchor value mapping to this filter */ 
                                status = mab_fltrid_list_add(&cur_fltr->fltr_ids, msg->fr_anc, fr_oaa_role, reg_req->fltr_id);
                                if (status != NCSCC_RC_SUCCESS)
                                {
                                    /* log the failure */ 
                                    if (status == NCSCC_RC_DUPLICATE_FLTR)  
                                    {
                                        m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST); 
                                        m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, tbl_rec->tbl_id, tbl_rec->fltr_list, new_fltr);  
                                    }
                                    m_MAS_FLTR_FREE(new_fltr);

                                    m_MAS_UNLK(&inst->lock);
                                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                    mas_mab_fltr_indices_cleanup(&msg->data.data.reg.fltr);
                                    m_MMGR_FREE_MAB_MSG(msg);

                                    /* Inform STANDBY MAS about SYNC DONE */ 

                                    return NCSCC_RC_FAILURE; 
                                }

                                m_MAS_FLTR_FREE(new_fltr);

                                m_MAB_DBG_TRACE2_MAS_MF_OP(inst,cur_fltr,tbl_rec->tbl_id,MFM_MODIFY);

                                /* Sync Done message to the Standby */ 
                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                m_MAB_DBG_TRACE("\nmas_info_register():left.");
                                return NCSCC_RC_SUCCESS;
                            }
                        }
                    }
                }
            }

            if((tbl_rec->fltr_list != NULL) && (reg_req->fltr.type != NCSMAB_FLTR_SAME_AS))
            {
                /* ignore this filter, because there could be only one here... */
                /* log the failure */
                m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST); 
                m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, tbl_rec->tbl_id, tbl_rec->fltr_list, new_fltr);  
                m_MAS_FLTR_FREE(new_fltr);

                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                m_MMGR_FREE_MAB_MSG(msg);
                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                m_MAB_DBG_TRACE("\nmas_info_register():left.");
                return NCSCC_RC_SUCCESS;
            }
            else
            {
                if(reg_req->fltr.type == NCSMAB_FLTR_SAME_AS)
                {
                    if((same_as_tbl_rec = mas_row_rec_find(inst,reg_req->fltr.fltr.same_as.i_table_id)) != NULL)
                    {
                        if(same_as_tbl_rec->fltr_list != NULL)
                        {
                            /* to prevent any chance of a filtering loop,
                            disallow SAME_AS filter point to other SAME_AS filters
                            */
                            if(same_as_tbl_rec->fltr_list->fltr.type == NCSMAB_FLTR_SAME_AS)
                            {
                                MAB_LM_EVT            mle;
                                MAB_LM_FLTR_INVAL_SA  fsant;
                                fsant.i_new_fltr_id = reg_req->fltr_id;
                                fsant.i_new_tbl_id  = reg_req->tbl_id;
                                fsant.i_sa_tbl_id   = reg_req->fltr.fltr.same_as.i_table_id;
                                fsant.i_type        = NCSMAB_SA_LOOP;
                                mle.i_args          = (NCSCONTEXT)&fsant;
                                mle.i_event         = MAS_FLTR_REG_INVALID_SA;
                                mle.i_usr_key       = inst->hm_hdl;
                                mle.i_vrid          = inst->vrid;
                                mle.i_which_mab     = NCSMAB_MAS;
                                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_REG_INV_SA);
                                inst->lm_cbfnc(&mle);
                                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                                m_MAS_FLTR_FREE(new_fltr);
                                m_MMGR_FREE_MAB_MSG(msg);
                                m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                                                      MAB_MAS_FLTR_SA_BAD_DST_TBL, inst->vrid, 
                                                      reg_req->fltr.fltr.same_as.i_table_id);
                                m_MAS_UNLK(&inst->lock);
                                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                            }              
                        }
                        /* We expect that when the same_as_tbl_rec is created,
                        it will contain range filters !!!*/
                    }

                    if(tbl_rec->fltr_list != NULL)
                    {
                        if(tbl_rec->fltr_list->fltr.fltr.same_as.i_table_id == new_fltr->fltr.fltr.same_as.i_table_id)
                        {
                            tbl_rec->fltr_list->ref_cnt++;

                            /* log the failure */
                            m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST); 
                             m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid, tbl_rec->tbl_id, tbl_rec->fltr_list, new_fltr);  

                            /* free the newly created filter */
                            m_MAS_FLTR_FREE(new_fltr);
                        }
                    }
                    else
                    {
                        new_fltr->ref_cnt = 1;
                        mas_fltr_put(&(tbl_rec->fltr_list),new_fltr);
                        m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);
                    }
                }
                else
                {
                    mas_fltr_put(&(tbl_rec->fltr_list),new_fltr);
                    m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);
                }
            }
        } /* end of other filter processing */
    } /* end of non default filter processing */


    /* Sync Done message to the Standby */ 
    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, tbl_rec->tbl_id);

    m_MAS_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
    m_MAB_DBG_TRACE("\nmas_info_register():left.");
    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  mas_info_unregister
*****************************************************************************/

uns32 mas_info_unregister(MAB_MSG* msg)
{
    MAS_TBL*     inst;
    MAS_UNREG*   unreg_req;
    MAS_ROW_REC* tbl_rec; 
    MAS_FLTR*    fltr        = NULL;
    NCS_BOOL      do_del_fltr = TRUE;
    MAS_FLTR     *rem_fltr_ids = NULL;  
    uns32        status; 
    uns32        table_id;
    m_MAS_LK_INIT;

    m_MAB_DBG_TRACE("\nmas_info_unregister():entered.");

    inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    if(inst == NULL)
    {
        m_LOG_MAB_NO_CB("mas_info_unregister()"); 
        m_MAB_DBG_TRACE("\nmas_info_unregister():left.");
        m_MMGR_FREE_MAB_MSG(msg);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);            
    }

    m_MAS_LK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_LOCKED,&inst->lock);

    unreg_req = &msg->data.data.unreg;

    /* send the message to STANDBY MAS */ 
    m_MAS_RE_REG_UNREG_SYNC(inst, msg, unreg_req->tbl_id); 

    /* search the table in the hash table */ 
    if((tbl_rec = mas_row_rec_find(inst,unreg_req->tbl_id)) == NULL)
    {
        m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                    MAB_MAS_FIND_REQ_TBL_FAILED, inst->vrid, unreg_req->tbl_id);
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        m_MMGR_FREE_MAB_MSG(msg);
        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

     table_id = tbl_rec->tbl_id;
 
    if(tbl_rec->ss_id != msg->fr_svc)
    {
        MAB_LM_EVT          mle;
        MAB_LM_FLTR_SS_MM   fsmm;
        fsmm.i_tbl_ss_id  = tbl_rec->ss_id;
        fsmm.i_fltr_ss_id = msg->fr_svc;
        mle.i_args        = (NCSCONTEXT)&fsmm;
        mle.i_event       = MAS_FLTR_UNREG_SS_MISMATCH;
        mle.i_usr_key     = inst->hm_hdl;
        mle.i_vrid        = inst->vrid;
        mle.i_which_mab   = NCSMAB_MAS;
        m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_UNREG_SS_MM);
        inst->lm_cbfnc(&mle);

        m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_FLTR_RMV_FAILED,
                               unreg_req->tbl_id, unreg_req->fltr_id, -1); 
        ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
        m_MMGR_FREE_MAB_MSG(msg);
        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_FIND_REQ_TBL_SUCCESS, 
                          inst->vrid, unreg_req->tbl_id);
    /* KCQ: we are unregistering the default filter here */
    if(unreg_req->fltr_id == 1)
    {
        if(tbl_rec->dfltr_regd == FALSE)
        {
            MAB_LM_EVT        mle;
            MAB_LM_FLTR_NULL  fn;
            fn.i_fltr_id    = unreg_req->fltr_id;
            fn.i_vcard      = msg->fr_card;
            mle.i_args      = (NCSCONTEXT)&fn;
            mle.i_event     = MAS_FLTR_UNREG_NO_FLTR;
            mle.i_usr_key   = inst->hm_hdl;
            mle.i_vrid      = inst->vrid;
            mle.i_which_mab = NCSMAB_MAS;
            m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_UNREG_NO_FLTR);
            inst->lm_cbfnc(&mle);

            m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, 
                                   MAB_MAS_FLTR_RMV_FAILED, unreg_req->tbl_id, 
                                   unreg_req->fltr_id, -1); 

            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);

            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }
        else
        {
            m_MAB_DBG_TRACE2_MAS_DF_OP(inst,msg,tbl_rec,MFM_DESTROY);

            /* delete the anchor, fltr-id mapping */ 
            status = mab_fltrid_list_del(&tbl_rec->dfltr.fltr_ids, msg->fr_anc);
            if (status != NCSCC_RC_SUCCESS)
            {
                /* log the error */ 
                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                m_MMGR_FREE_MAB_MSG(msg);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            /* if there are no filter-ids associated */
            if ((tbl_rec->dfltr.fltr_ids.active_fltr == NULL)&& 
            (tbl_rec->dfltr.fltr_ids.fltr_id_list == NULL))
            {
                tbl_rec->dfltr_regd = FALSE;
                memset(&tbl_rec->dfltr.vcard, 0, sizeof(tbl_rec->dfltr.vcard));
            }

            if((tbl_rec->dfltr_regd == FALSE) && (tbl_rec->fltr_list == NULL))
            {
                if((tbl_rec = mas_row_rec_rmv(inst,unreg_req->tbl_id)) == NULL)
                {
                    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                    MAB_MAS_RMV_ROW_REC_FAILED, 
                    inst->vrid, 
                    unreg_req->tbl_id);
                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    m_MMGR_FREE_MAB_MSG(msg);
                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG, MAB_MAS_RMV_ROW_REC_SUCCESS, 
                                      inst->vrid, unreg_req->tbl_id);

                m_MAB_DBG_TRACE2_MAS_TR_DALLOC(tbl_rec);

                m_MMGR_FREE_MAS_ROW_REC(tbl_rec);
            }
        }
    }/* if(unreg_req->fltr_id == 1) */
    else
    {
        if (tbl_rec->fltr_list == NULL)  /* added this condition to fix a crash (Bug Ids: 57844, 57845, 57986) */ 
        {
            m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_RMV_ROW_REC_FAILED, 
            inst->vrid, unreg_req->tbl_id);
            ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
            m_MMGR_FREE_MAB_MSG(msg);
            m_MAS_UNLK(&inst->lock);
            m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
        }

        if ((tbl_rec->fltr_list->fltr.type == NCSMAB_FLTR_RANGE)||
            (tbl_rec->fltr_list->fltr.type == NCSMAB_FLTR_EXACT))
        {
            MAS_FLTR* tmp;

            tmp = mas_fltr_find(inst,&(tbl_rec->fltr_list),unreg_req->fltr_id,msg->fr_anc,msg->fr_card);
            /* bug no: 10430  start */
            if (tmp == NULL)
            {
                MAB_LM_EVT        mle;
                MAB_LM_FLTR_NULL  fn;
                fn.i_fltr_id    = unreg_req->fltr_id;
                fn.i_vcard      = msg->fr_card;
                mle.i_args      = (NCSCONTEXT)&fn;
                mle.i_event     = MAS_FLTR_UNREG_NO_FLTR;
                mle.i_usr_key   = inst->hm_hdl;
                mle.i_vrid      = inst->vrid;
                mle.i_which_mab = NCSMAB_MAS;
                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_UNREG_NO_FLTR);
                inst->lm_cbfnc(&mle);

                m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, MAB_MAS_FLTR_RMV_FAILED,
                unreg_req->tbl_id, unreg_req->fltr_id, tbl_rec->fltr_list->fltr.type); 

                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                m_MMGR_FREE_MAB_MSG(msg);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
            }
            /* bug no: 10430  end */
            /* delete this fltr-id, anchor */
            rem_fltr_ids = tmp; 
        }
        else if ((tbl_rec->fltr_list->fltr.type == NCSMAB_FLTR_ANY) ||
                 (tbl_rec->fltr_list->fltr.type == NCSMAB_FLTR_SAME_AS))
        {
            rem_fltr_ids = tbl_rec->fltr_list; 
        }

        /* remove the anchor, fltr-id pair from the filter */
        status = mab_fltrid_list_del(&rem_fltr_ids->fltr_ids, msg->fr_anc);
        if ((rem_fltr_ids->fltr_ids.active_fltr != NULL)&&
            (rem_fltr_ids->fltr_ids.fltr_id_list != NULL))
        {
            /* if the other filter id is still there, we don't delete the filter */
            do_del_fltr = FALSE;
        }

        /* some special processing is required for SAME_AS filter */  
        if(tbl_rec->fltr_list->fltr.type == NCSMAB_FLTR_SAME_AS)
        {
            if ((rem_fltr_ids->fltr_ids.active_fltr == NULL) &&
                (rem_fltr_ids->fltr_ids.fltr_id_list == NULL))
                    tbl_rec->fltr_list->ref_cnt--;

            if(tbl_rec->fltr_list->ref_cnt != 0)
                do_del_fltr = FALSE;
        }

        if(do_del_fltr == TRUE)
        {
            if(tbl_rec->fltr_list != NULL)
            {
                if((fltr = mas_fltr_rmv(&(tbl_rec->fltr_list), rem_fltr_ids)) == NULL)
                {
                    MAB_LM_EVT        mle;
                    MAB_LM_FLTR_NULL  fn;
                    fn.i_fltr_id    = unreg_req->fltr_id;
                    fn.i_vcard      = msg->fr_card;
                    mle.i_args      = (NCSCONTEXT)&fn;
                    mle.i_event     = MAS_FLTR_UNREG_NO_FLTR;
                    mle.i_usr_key   = inst->hm_hdl;
                    mle.i_vrid      = inst->vrid;
                    mle.i_which_mab = NCSMAB_MAS;
                    m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_UNREG_NO_FLTR);
                    inst->lm_cbfnc(&mle);

                    m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR, 
                    MAB_MAS_FLTR_RMV_FAILED, unreg_req->tbl_id, 
                    unreg_req->fltr_id, -1); 

                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    m_MMGR_FREE_MAB_MSG(msg);

                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
                m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
                MAB_MAS_FLTR_RMV_SUCCESS, unreg_req->tbl_id, 
                unreg_req->fltr_id, fltr->fltr.type); 
            }    

            if ((fltr->fltr.type == NCSMAB_FLTR_RANGE) ||
                (fltr->fltr.type == NCSMAB_FLTR_EXACT))
            {
                if(fltr->fltr.is_move_row_fltr != TRUE)
                mas_mab_fltr_indices_cleanup(&fltr->fltr); 
                else if(tbl_rec->dfltr_regd != TRUE)
                {
                MAB_LM_EVT        mle;
                MAB_LM_FLTR_NULL  fn;
                fn.i_fltr_id    = unreg_req->fltr_id;
                fn.i_vcard      = msg->fr_card;
                mle.i_args      = (NCSCONTEXT)&fn;
                mle.i_event     = MAS_FLTR_UNREG_NO_FLTR;
                mle.i_usr_key   = inst->hm_hdl;
                mle.i_vrid      = inst->vrid;
                mle.i_which_mab = NCSMAB_MAS;
                m_LOG_MAB_EVT(NCSFL_SEV_ERROR,MAB_EV_LM_MAS_FLTR_UNREG_NO_FLTR);
                inst->lm_cbfnc(&mle);

                m_LOG_MAB_FLTR_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                MAB_MAS_FLTR_RMV_FAILED, unreg_req->tbl_id, 
                unreg_req->fltr_id, -1); 

                ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                m_MMGR_FREE_MAB_MSG(msg);

                m_MAS_UNLK(&inst->lock);
                m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }
                else
                { 
                    /* need to free index... */
                    MAB_MSG idx_msg;
                    uns32   code;

                    idx_msg.op = MAB_OAC_FIR_HDLR;
                    idx_msg.vrid = inst->vrid;
                    memset(&idx_msg.fr_card, 0, sizeof(idx_msg.fr_card));
                    idx_msg.fr_svc = 0;
                    idx_msg.data.data.idx_free.fltr_type = fltr->fltr.type;
                    idx_msg.data.data.idx_free.idx_tbl_id = tbl_rec->tbl_id;
                    if (idx_msg.data.data.idx_free.fltr_type == NCSMAB_FLTR_RANGE)
                    {
                        memcpy(&idx_msg.data.data.idx_free.idx_free_data.range_idx_free,
                        &fltr->fltr.fltr.range,sizeof(NCSMAB_RANGE));
                    }
                    else if (idx_msg.data.data.idx_free.fltr_type == NCSMAB_FLTR_EXACT)
                    {
                        memcpy(&idx_msg.data.data.idx_free.idx_free_data.exact_idx_free,
                        &fltr->fltr.fltr.exact,sizeof(NCSMAB_EXACT));
                    }

#if (NCS_MAS_RED == 1)
                    if (inst->red.ha_state == NCS_APP_AMF_HA_STATE_ACTIVE)
                    {
                        code = mab_mds_snd(inst->mds_hdl, &idx_msg, NCSMDS_SVC_ID_MAS,
                        NCSMDS_SVC_ID_OAC, tbl_rec->dfltr.vcard);
                    }
                    else
                        code = NCSCC_RC_SUCCESS;
#else
                    code = mab_mds_snd(inst->mds_hdl, &idx_msg, NCSMDS_SVC_ID_MAS,
                    NCSMDS_SVC_ID_OAC, tbl_rec->dfltr.vcard);
#endif
                    mas_mab_fltr_indices_cleanup(&fltr->fltr); 

                    if(code != NCSCC_RC_SUCCESS)
                    {
                        dummy = m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                    }
                }
            } /* if(fltr->fltr.type == NCSMAB_FLTR_RANGE) */ 
            m_MAB_DBG_TRACE2_MAS_MF_OP(inst,fltr,tbl_rec->tbl_id,MFM_DESTROY);

            m_MAS_FLTR_FREE(fltr);

            /* if there are no filters in the table, 
            * free the table record from the hash table 
            */  
            if((tbl_rec->fltr_list == NULL) && (tbl_rec->dfltr_regd != TRUE))
            {
                if((tbl_rec = mas_row_rec_rmv(inst,unreg_req->tbl_id)) == NULL)
                {
                    m_LOG_MAB_TBL_DETAILS(NCSFL_LC_HEADLINE, NCSFL_SEV_ERROR,
                                          MAB_MAS_RMV_ROW_REC_FAILED, 
                                          inst->vrid, unreg_req->tbl_id);
                    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
                    m_MMGR_FREE_MAB_MSG(msg);
                    m_MAS_UNLK(&inst->lock);
                    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
                    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
                }

                m_LOG_MAB_TBL_DETAILS(NCSFL_LC_DATA, NCSFL_SEV_DEBUG,
                                      MAB_MAS_RMV_ROW_REC_SUCCESS, 
                                      inst->vrid, unreg_req->tbl_id);
                m_MAB_DBG_TRACE2_MAS_TR_DALLOC(tbl_rec);

                m_MMGR_FREE_MAS_ROW_REC(tbl_rec);
            }
        } /* if(do_del_fltr == TRUE) */
    } /* else if(unreg_req->fltr_id == 1) */ 

    /* give the handle */ 
    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

    /* Sync Done message to the Standby */ 
    m_MAS_RE_REG_UNREG_SYNC_DONE(inst, msg, table_id);

    m_MAS_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);

    m_MAB_DBG_TRACE("\nmas_info_unregister():left.");
    return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  mas_forward_msg
*****************************************************************************/

uns32 mas_forward_msg(MAB_MSG* msg, MAB_SVC_OP msg_op, MDS_HDL  mds_hdl, 
                      MDS_DEST to_vcard, SS_SVC_ID to_svc_id, NCS_BOOL is_req)
{
  uns32 code;

  m_MAB_DBG_TRACE("\nmas_forward_msg():entered.");

  msg->op = msg_op;
  memset(&msg->fr_card, 0, sizeof(msg->fr_card));
  msg->fr_svc = 0;
  code = mab_mds_snd(mds_hdl, msg, NCSMDS_SVC_ID_MAS, to_svc_id, to_vcard);

  if(code != NCSCC_RC_SUCCESS)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

    switch(msg->data.data.snmp->i_op)
    {
      case NCSMIB_OP_REQ_SETROW  :
      case NCSMIB_OP_REQ_TESTROW :
      case NCSMIB_OP_REQ_SETALLROWS:
      case NCSMIB_OP_REQ_REMOVEROWS:
        m_MMGR_FREE_BUFR_LIST(msg->data.data.snmp->req.info.setrow_req.i_usrbuf);
        msg->data.data.snmp->req.info.setrow_req.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_GETROW :
      case NCSMIB_OP_RSP_SETROW :
      case NCSMIB_OP_RSP_TESTROW:
      case NCSMIB_OP_RSP_SETALLROWS:
      case NCSMIB_OP_RSP_REMOVEROWS:
        m_MMGR_FREE_BUFR_LIST(msg->data.data.snmp->rsp.info.getrow_rsp.i_usrbuf);
        msg->data.data.snmp->rsp.info.getrow_rsp.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_NEXTROW :
        m_MMGR_FREE_BUFR_LIST(msg->data.data.snmp->rsp.info.nextrow_rsp.i_usrbuf);
        msg->data.data.snmp->rsp.info.nextrow_rsp.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_MOVEROW :
        m_MMGR_FREE_BUFR_LIST(msg->data.data.snmp->rsp.info.moverow_rsp.i_usrbuf);
        msg->data.data.snmp->rsp.info.moverow_rsp.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_REQ_CLI:
        m_MMGR_FREE_BUFR_LIST(msg->data.data.snmp->req.info.cli_req.i_usrbuf);
        msg->data.data.snmp->req.info.cli_req.i_usrbuf = NULL;
        break;

      case NCSMIB_OP_RSP_CLI:
      case NCSMIB_OP_RSP_CLI_DONE:
        m_MMGR_FREE_BUFR_LIST(msg->data.data.snmp->rsp.info.cli_rsp.o_answer);
        msg->data.data.snmp->rsp.info.cli_rsp.o_answer = NULL;
        break;

      default:
        break;
    }

  if(ncsmib_arg_free_resources(msg->data.data.snmp,is_req) != NCSCC_RC_SUCCESS)
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);

  m_MMGR_FREE_MAB_MSG(msg);

  m_MAB_DBG_TRACE("\nmas_forward_msg():left.");
  
  return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
                       mas_cli_msg_to_mac
*****************************************************************************/
uns32  mas_cli_msg_to_mac(MAB_MSG *msg, MAS_TBL *inst)
{
  NCSMIB_ARG*  mib_rsp = msg->data.data.snmp;
  NCS_SE*      se;
  uns8*        stream;
  MAB_SVC_OP   mso;
  MDS_DEST     dst_vcard;
  uns16        dst_svc_id = 0;
  uns16        vrid = 0;


  m_MAB_DBG_TRACE("\nmas_cli_msg_to_mac():entered.");

  /* Get the MAC related information from the NCSMIB_ARG stack */
  if ((se = ncsstack_pop(&mib_rsp->stack)) == NULL)
  {
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);              
  }

  stream = m_NCSSTACK_SPACE(se);

  if (se->type == NCS_SE_TYPE_BACKTO)
  {
     mds_st_decode_mds_dest(&stream, &dst_vcard);
     dst_svc_id = ncs_decode_16bit(&stream);
     vrid       = ncs_decode_16bit(&stream);
  }
  else
  {
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
  }
  
  /* KCQ: I'm paranoid ;-]  */
  if (inst->vrid != vrid)
  {
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
  }
 
  if (dst_svc_id != NCSMDS_SVC_ID_MAC)
  {
     m_MAB_DBG_TRACE("\nmas_cli_msg_to_mac():not a MAC svc id.");
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
  }
  else
     mso = MAB_MAC_RSP_HDLR;

  /* KCQ: for now all messages going to a MAC are
     responses. It won't be so when traps are implemented
   */
  msg->op = mso;
  memset(&msg->fr_card, 0, sizeof(msg->fr_card));
  msg->fr_svc = 0;

  if (mab_mds_snd(inst->mds_hdl, msg, NCSMDS_SVC_ID_MAS, (SS_SVC_ID)dst_svc_id, dst_vcard)
      != NCSCC_RC_SUCCESS)
  {
     return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }

  /* Free the USRBUF contents of response */
  m_MMGR_FREE_BUFR_LIST(mib_rsp->rsp.info.cli_rsp.o_answer);
  mib_rsp->rsp.info.cli_rsp.o_answer = NULL;

  /* Since the same message need to send once again to the next 
   * OAC as a CLI-REQ, so retain the MAC info in the NCSMIB_ARG stack.
   */
  {
     NCS_SE* se;     
     uns8*   stream;

     /* Push the directions to get back...  */
     if ((se = ncsstack_push(&mib_rsp->stack,
                             NCS_SE_TYPE_BACKTO,
                             (uns8)sizeof(NCS_SE_BACKTO))) == NULL)
     {
        m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR, MAB_HDLN_MAS_PUSH_FISE_FAILED);
        m_MAS_UNLK(&inst->lock);
        m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
     }

     stream = m_NCSSTACK_SPACE(se);

     mds_st_encode_mds_dest( &stream, &dst_vcard);
     ncs_encode_16bit( &stream,dst_svc_id);
     ncs_encode_16bit( &stream,vrid);
  }

  m_MAB_DBG_TRACE("\nmas_cli_msg_to_mac():left.");
   
  return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  mas_relay_msg_to_mac
*****************************************************************************/
uns32 mas_relay_msg_to_mac(MAB_MSG* msg, MAS_TBL* inst,NCS_BOOL is_req)
{
  NCSMIB_ARG*   mib_rsp = msg->data.data.snmp;
  NCS_SE*       se;
  uns8*         stream;
  MAB_SVC_OP    mso;
  MDS_DEST      dst_vcard;
  uns16         dst_svc_id = 0;
  uns16         vrid = 0;

  m_MAB_DBG_TRACE("\nmas_relay_msg_to_mac():entered.");

  if((se = ncsstack_pop(&mib_rsp->stack)) == NULL)
  {
    m_LOG_MAB_HEADLINE(NCSFL_SEV_ERROR,MAB_HDLN_MAS_POP_SE_FAILED);
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);              
  }

  stream = m_NCSSTACK_SPACE(se);

  if(se->type == NCS_SE_TYPE_BACKTO)
  {
    mds_st_decode_mds_dest(&stream, &dst_vcard);
    dst_svc_id = ncs_decode_16bit(&stream);
    vrid       = ncs_decode_16bit(&stream);
  }
  else
  {
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
  }
  
  /* KCQ: I'm paranoid ;-]  */
  if(inst->vrid != vrid)
  {
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);             
  }
  
  switch(dst_svc_id)
  {
  case NCSMDS_SVC_ID_MAC:
    mso = MAB_MAC_RSP_HDLR;
    break;

  case NCSMDS_SVC_ID_OAC:
    mso = MAB_OAC_RSP_HDLR;
    break;
    
  default:
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);    
  }
  
  /* KCQ: for now all messages going to a MAC are
     responses. It won't be so when traps are implemented
   */
  if(mas_forward_msg(msg,mso,
    inst->mds_hdl, dst_vcard,
    (SS_SVC_ID)dst_svc_id,is_req) == NCSCC_RC_FAILURE)
  {
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
  }
  else
  {
    m_MAB_DBG_TRACE("\nmas_relay_msg_to_mac():left.");
    return NCSCC_RC_SUCCESS;
  }
  
  m_MAB_DBG_TRACE("\nmas_relay_msg_to_mac():left.");
  return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  mas_next_op_stack_push
*****************************************************************************/

uns32 mas_next_op_stack_push(MAS_TBL* inst,NCS_STACK* stack,uns32 fltr_id)
  {
  NCS_SE* se;
  uns8*  stream;

  /* push FILTER_ID stack element */

  if ((se = ncsstack_push( stack, 
    NCS_SE_TYPE_FILTER_ID,
    (uns8)sizeof(NCS_SE_FILTER_ID))) == NULL) 
    {
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_PUSH_FISE_SUCCESS);

  stream = m_NCSSTACK_SPACE(se);

  ncs_encode_32bit( &stream,fltr_id);

  /* push BACKTO stack element, so that the dst OAC can send the response back to us */

  if ((se = ncsstack_push( stack, 
    NCS_SE_TYPE_BACKTO,
    (uns8)sizeof(NCS_SE_BACKTO))) == NULL) 
    {
    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
  m_LOG_MAB_HEADLINE(NCSFL_SEV_DEBUG, MAB_HDLN_MAS_PUSH_BTSE_SUCCESS);

  stream = m_NCSSTACK_SPACE(se);

  mds_st_encode_mds_dest( &stream, &inst->my_vcard);
  ncs_encode_16bit( &stream,NCSMDS_SVC_ID_MAS);
  ncs_encode_16bit( &stream,inst->vrid);

  return NCSCC_RC_SUCCESS;
  }

static uns32
mas_exact_fltr_process(MAB_MSG      *msg,
                       MAS_TBL      *inst, 
                       MAS_ROW_REC  *tbl_rec, 
                       MAS_FLTR     *new_fltr,
                       uns32        i_fltr_id,
                       V_DEST_RL    oaa_role)
{
    NCSMAB_FLTR *mf;
    MAS_FLTR    *fltr; 
    MAS_FLTR    *mas_fltr;
    int32       res = 0; 
    MAS_FLTR    *prev_fltr = NULL; 
    uns32       status; 
    NCS_BOOL    duplicate = FALSE;
    uns32       size;
    
    /* find an appropriate place to add into the list */
    if (tbl_rec->fltr_list == NULL)
    {
        /* add the first filter into the list */ 
        mas_fltr_put(&(tbl_rec->fltr_list),new_fltr);
        m_MAB_DBG_TRACE2_MAS_MF_OP(inst,new_fltr,tbl_rec->tbl_id,MFM_CREATE);
        /* log the event that a new exact filter is added successfully */ 
        return NCSCC_RC_SUCCESS; 
    }

    /* make sure that all the filters are of same type */ 
    mf = &tbl_rec->fltr_list->fltr;
    if ((mf->type != new_fltr->fltr.type)||
        (mf->fltr.exact.i_bgn_idx != new_fltr->fltr.fltr.exact.i_bgn_idx))
    {
       /* inform the almighty and return the error */ 
       mas_exact_fltr_almighty_inform(inst, tbl_rec, new_fltr, MAS_FLTR_TYPE_REG_MISMATCH);
       return NCSCC_RC_FAILURE;
    }
    
    /* see if there's already a filter with the given fltr_id/vcard pair registered */
    fltr = mas_fltr_find(inst,&tbl_rec->fltr_list, i_fltr_id,
                          msg->fr_anc,new_fltr->vcard); 
    if ((fltr != NULL) 
#if (NCS_MAS_RED == 1)           
       && (inst->red.ha_state != NCS_APP_AMF_HA_STATE_STANDBY)
#endif           
           )
    {
       mas_exact_fltr_almighty_inform(inst, tbl_rec, new_fltr, MAS_FLTR_REG_EXIST);
       return NCSCC_RC_FAILURE;
    }

/* Following is a cut and paste, I am not sure why this is required...  */     
#if (NCS_MAS_RED == 1)
    if ((fltr != NULL) &&
        (inst->red.ha_state == NCS_APP_AMF_HA_STATE_STANDBY))
    {
        return NCSCC_RC_SUCCESS;
    }    
#endif        

    /* find the right place to insert this filter */ 
    prev_fltr = NULL; 
    for(mas_fltr = tbl_rec->fltr_list; mas_fltr != NULL; mas_fltr = mas_fltr->next)
    {
        /* from here start finding the best place to fit, 
         * and do this till the length of the existing filter is same 
         */ 
        /* find the best fit in this category */ 
        if (mas_fltr->fltr.fltr.exact.i_idx_len<=new_fltr->fltr.fltr.exact.i_idx_len)
            size = mas_fltr->fltr.fltr.exact.i_idx_len; 
        else 
            size = new_fltr->fltr.fltr.exact.i_idx_len; 
        m_MAB_FLTR_CMP(res,
                     mas_fltr->fltr.fltr.exact.i_exact_idx,
                     new_fltr->fltr.fltr.exact.i_exact_idx, size); 

        if (res == 0)
        {
            /* if the lengths are same */
            if (mas_fltr->fltr.fltr.exact.i_idx_len == new_fltr->fltr.fltr.exact.i_idx_len)
            {
                /* log the duplicate filter */
                duplicate = TRUE;
                break;
            }
            else if (new_fltr->fltr.fltr.exact.i_idx_len < mas_fltr->fltr.fltr.exact.i_idx_len)
            {
                /* insert after prev_fltr */
                break;
            }
        }

        if (res > 0)
        {
            /* insert after prev_fltr */
            break;
        }
        
        /* go to the next filter in the loop */
        prev_fltr = mas_fltr; 

    } /* for  all the MAS filters of this table */ 

    if ((res == 0) && (duplicate == TRUE))
    {
        if ((new_fltr->fltr.is_move_row_fltr != mas_fltr->fltr.is_move_row_fltr)||
            (memcmp(&mas_fltr->vcard, &new_fltr->vcard, sizeof(mas_fltr->vcard)) != 0))
        {
            mas_exact_fltr_almighty_inform(inst, tbl_rec, new_fltr, MAS_FLTR_REG_OVERLAP);
            /* log the error */
            m_LOG_MAB_EVT(NCSFL_SEV_NOTICE,MAB_EV_LM_MAS_FLTR_REG_EXIST); 
            m_LOG_MAB_OVERLAPPING_FLTRS(NCSFL_SEV_NOTICE, inst->vrid,
                                        tbl_rec->tbl_id, mas_fltr, new_fltr);  
            return NCSCC_RC_FAILURE; 
        }

        /* add to the filter list */
        status = mab_fltrid_list_add(&mas_fltr->fltr_ids, msg->fr_anc, oaa_role, i_fltr_id);
        if (status != NCSCC_RC_SUCCESS)
        {
            /* log the failure */
            return NCSCC_RC_FAILURE; 
        }
        m_MAS_FLTR_FREE(new_fltr);
        m_MAB_DBG_TRACE2_MAS_MF_OP(inst,mas_fltr,tbl_rec->tbl_id,MFM_MODIFY);
        return NCSCC_RC_SUCCESS;
    }
    
    if ((prev_fltr == NULL) && (mas_fltr == tbl_rec->fltr_list))
    {
        new_fltr->next = tbl_rec->fltr_list; 
        tbl_rec->fltr_list = new_fltr; 
    }
    else     
    {
        mas_fltr_put(&prev_fltr, new_fltr);
    }

    /* log the event that a new exact filter is added successfully */ 
    return NCSCC_RC_SUCCESS; 
}

static uns32
mas_exact_fltr_almighty_inform(MAS_TBL      *inst, 
                              MAS_ROW_REC  *tbl_rec, 
                              MAS_FLTR     *new_fltr, 
                              uns32        event_type)
{
    /* log the error */ 
    m_LOG_MAB_ERROR_I(NCSFL_SEV_ERROR, MAB_MAS_ERR_INFORM, 
                      event_type);

    /* log the filter details */ 
    m_LOG_MAB_FLTR_DATA(NCSFL_SEV_DEBUG, inst->vrid, tbl_rec->tbl_id, -1, &new_fltr->fltr);

    return NCSCC_RC_SUCCESS; 
}

/* to get the filter-id for a given anchor value */ 
static uns32 
mab_fltrid_list_get(MAS_FLTR_IDS    *fltr_list,
                    MAB_ANCHOR      i_anchor, 
                    uns32           *o_fltr_id) 
{
    uns32   status = NCSCC_RC_FAILURE; 
    MAB_FLTR_ANCHOR_NODE *i_list = NULL;

    if (fltr_list == NULL)
        return NCSCC_RC_FAILURE;

    if (fltr_list->active_fltr != NULL)
    {
        if (memcmp(&fltr_list->active_fltr->anchor, &i_anchor, sizeof(MAB_ANCHOR)) == 0)
        {
            *o_fltr_id = fltr_list->active_fltr->fltr_id;
            return NCSCC_RC_SUCCESS;
        }
    }

    i_list = fltr_list->fltr_id_list;
    while (i_list)
    {
        if (memcmp(&i_list->anchor, &i_anchor, sizeof(MAB_ANCHOR)) == 0)
        {
            *o_fltr_id = i_list->fltr_id;
            status = NCSCC_RC_SUCCESS;
            break; 
        }
        i_list = i_list->next; 
    }
    return status; 
}

/* to add this filtr_id and anchor value pair */  
static uns32   
mab_fltrid_list_add(MAS_FLTR_IDS    *fltr_ids,
                    MAB_ANCHOR      i_anchor, 
                    V_DEST_RL       oaa_role,
                    uns32           i_fltr_id)
{
    MAB_FLTR_ANCHOR_NODE    *new_anchor = NULL; 
    uns32                   dummy = 0; 

    uns32   status = NCSCC_RC_FAILURE; 

    if (fltr_ids == NULL)
        return NCSCC_RC_FAILURE; 

    /* is this fltr-id from an active OAA */
    if (oaa_role == V_DEST_RL_ACTIVE)
    {
        /* there can be only one active fltr-id for this filter */ 
        if (fltr_ids->active_fltr != NULL) 
            return NCSCC_RC_DUPLICATE_FLTR; 
            /*return NCSCC_RC_FAILURE; */ 

        /* allocate the memory for the new node */ 
        fltr_ids->active_fltr = m_MMGR_ALLOC_FLTR_ANCHOR_NODE;
        if (fltr_ids->active_fltr == NULL)
        {
            /* return the malloc failure */ 
            m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_MAB_FLTR_ANCHOR_NODE_ALLOC_FAILED,
                                           "mab_fltrid_list_add()");

            return NCSCC_RC_OUT_OF_MEM; 
        }
        memset(fltr_ids->active_fltr, 0, sizeof(MAB_FLTR_ANCHOR_NODE)); 

        /* copy the data */ 
        memcpy(&fltr_ids->active_fltr->anchor, &i_anchor, sizeof(MAB_ANCHOR)); 
        fltr_ids->active_fltr->fltr_id = i_fltr_id;

        return NCSCC_RC_SUCCESS;
    }
    else
    {
        /* see if it is already present in the list */ 
        status = mab_fltrid_list_get(fltr_ids, i_anchor, &dummy); 
        if (((status == NCSCC_RC_SUCCESS)&&(dummy != i_fltr_id)) ||
            (status != NCSCC_RC_SUCCESS))
        {
            /* allocate the memory for the new node */ 
            new_anchor = m_MMGR_ALLOC_FLTR_ANCHOR_NODE;
            if (new_anchor == NULL)
            {
                /* return the malloc failure */ 
                m_LOG_MAB_MEMFAIL(NCSFL_SEV_CRITICAL, MAB_MF_MAS_MAB_FLTR_ANCHOR_NODE_ALLOC_FAILED,
                                               "mab_fltrid_list_add()");

                return NCSCC_RC_OUT_OF_MEM; 
            }
            memset(new_anchor, 0, sizeof(MAB_FLTR_ANCHOR_NODE)); 

            /* copy the data */ 
            memcpy(&new_anchor->anchor, &i_anchor, sizeof(MAB_ANCHOR)); 
            new_anchor->fltr_id = i_fltr_id;
            
            /* add to the list in the front */ 
            if (fltr_ids->fltr_id_list == NULL)
                fltr_ids->fltr_id_list = new_anchor; 
            else
            {
                /* make the new node as the first */
                new_anchor->next = fltr_ids->fltr_id_list; 
                fltr_ids->fltr_id_list = new_anchor;
            }
        }
    }
    return NCSCC_RC_SUCCESS; 
}

/* delete this anchor from the list */
static uns32   
mab_fltrid_list_del(MAS_FLTR_IDS    *fltr_list,
                    MAB_ANCHOR      i_anchor)
{
    MAB_FLTR_ANCHOR_NODE    *del_me; 
    MAB_FLTR_ANCHOR_NODE    *prev = NULL; 

    if (fltr_list == NULL)
        return NCSCC_RC_FAILURE; 
     
    if ((fltr_list->active_fltr != NULL) && 
        (memcmp(&fltr_list->active_fltr->anchor, &i_anchor, sizeof(MAB_ANCHOR)) == 0))
    {
        m_MMGR_FREE_FLTR_ANCHOR_NODE(fltr_list->active_fltr); 
        fltr_list->active_fltr = NULL; 
        return NCSCC_RC_SUCCESS; 
    }
   
    del_me = fltr_list->fltr_id_list;  
    while (del_me)
    {
        if (memcmp(&del_me->anchor, &i_anchor, sizeof(MAB_ANCHOR)) == 0)
            break; 
        prev = del_me; 
        del_me = del_me->next; 
    }

    if (del_me == NULL)
    {
        /* anchor value not found */ 
        return NCSCC_RC_FAILURE; 
    }

    /* delink the node from the list */ 
    if (prev == NULL)
    {
        /* first node to be deleted from the list */ 
        fltr_list->fltr_id_list = del_me->next; 
    }
    else
    {
        prev->next = del_me->next; 
    }

    /* free the delinked node */ 
    m_MMGR_FREE_FLTR_ANCHOR_NODE(del_me); 
    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_fltr_ids_adjust                                        * 
*                                                                            *
*  Description:   modifies the filter list based on the anchor values and    * 
*                 role.                                                      *
*                                                                            *
*  Arguments:     NCSMDS_CALLBACK_INFO *cbinfo - Information from MDS        * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 NCSCC_RC_FAILURE - Some inconsistency in adjusting the     *
*                                    mappings                                *
\****************************************************************************/
static uns32
mas_fltr_ids_adjust(MAS_FLTR_IDS    *fltr_list,
                    MAB_ANCHOR      i_anchor, 
                    V_DEST_RL       i_role)
{
    MAB_FLTR_ANCHOR_NODE *tmp_node; 
    MAB_FLTR_ANCHOR_NODE *prev; 
    
    switch(i_role) 
    {
        case SA_AMF_HA_ACTIVE:
        {
            /* printf("\nmas_fltr_ids_adjust(): SA_AMF_HA_ACTIVE"); */
            prev = NULL; 
            tmp_node = fltr_list->fltr_id_list; 
            while (tmp_node)
            {
                if (tmp_node->anchor == i_anchor)
                {
                    /* delete this node from the list and add to the active_fltr_id list */ 
                    if (prev != NULL)
                        prev->next = tmp_node->next;
                    else
                        fltr_list->fltr_id_list = tmp_node->next; 

                    if (fltr_list->active_fltr != NULL)
                    {
                        /* add the active_filter node in the begining of the other filter slist */ 
                        fltr_list->active_fltr->next = fltr_list->fltr_id_list;
                        fltr_list->fltr_id_list = fltr_list->active_fltr; 
                        fltr_list->active_fltr = NULL; 
                    }

                    /* update this as the active filter */ 
                    fltr_list->active_fltr  = tmp_node; 
                    fltr_list->active_fltr->next = NULL; 
                    return NCSCC_RC_SUCCESS; 
                }
                prev = tmp_node; 
                tmp_node = tmp_node->next; 
            }
        }
        break; 
        case SA_AMF_HA_QUIESCED:
        {
            if (fltr_list->active_fltr != NULL)
            {
                /* add the active_filter node in the begining of the other filter slist */ 
                fltr_list->active_fltr->next = fltr_list->fltr_id_list;
                fltr_list->fltr_id_list = fltr_list->active_fltr; 
                fltr_list->active_fltr = NULL; 
                return NCSCC_RC_SUCCESS; 
            }
        }
        break; 
        case SA_AMF_HA_STANDBY:
        {
            /* printf("\nmas_fltr_ids_adjust(): SA_AMF_HA_STANDBY"); */
            /* by this time, quiesced state processing should have been done */ 
            if (fltr_list->active_fltr == NULL)
                return NCSCC_RC_SUCCESS; 
        }
        break; 
        default: 
            return NCSCC_RC_FAILURE; 
    } /* end of switch() */

    return NCSCC_RC_FAILURE; 
}

/* inform the MAA with the given MIB_ARG and error_code */
static uns32
mas_inform_maa(MAB_MSG *msg, MAS_TBL *inst, NCSMIB_ARG *mib_req, uns32 error_code) 
{
    NCS_BOOL      wildcard_req = FALSE;

    /* set the response info from the request */
    switch(mib_req->i_op)
    {
        case NCSMIB_OP_REQ_SETROW  :
        case NCSMIB_OP_REQ_TESTROW :
        case NCSMIB_OP_REQ_SETALLROWS:
        case NCSMIB_OP_REQ_REMOVEROWS:
        mib_req->rsp.info.setrow_rsp.i_usrbuf = mib_req->req.info.setrow_req.i_usrbuf;
        mib_req->req.info.setrow_req.i_usrbuf = NULL;
        break;

        case NCSMIB_OP_REQ_GET :
        memset(&mib_req->rsp.info.get_rsp.i_param_val,0,sizeof(NCSMIB_PARAM_VAL));
        mib_req->rsp.info.get_rsp.i_param_val.i_param_id = mib_req->req.info.get_req.i_param_id;
        break;
        
        case NCSMIB_OP_REQ_NEXT:
        memset(&mib_req->rsp.info.next_rsp.i_param_val,0,sizeof(NCSMIB_PARAM_VAL));
        mib_req->rsp.info.next_rsp.i_param_val.i_param_id = mib_req->req.info.next_req.i_param_id;
        break;
        
        case NCSMIB_OP_REQ_SET :
        case NCSMIB_OP_REQ_TEST:
        memset(&mib_req->rsp.info.set_rsp.i_param_val,0,sizeof(NCSMIB_PARAM_VAL));
        mib_req->rsp.info.set_rsp.i_param_val.i_param_id = mib_req->req.info.set_req.i_param_val.i_param_id;
        break;
        
        case NCSMIB_OP_REQ_CLI:
        if (mib_req->req.info.cli_req.i_wild_card == TRUE)
            wildcard_req = TRUE;
        break;

        default:
        break;
    }

    /* set the appropriate request */
    if (wildcard_req == TRUE)
     mib_req->i_op = NCSMIB_OP_RSP_CLI_DONE;
    else
     mib_req->i_op = m_NCSMIB_REQ_TO_RSP(mib_req->i_op);

    mib_req->rsp.i_status = error_code;

    if(mas_relay_msg_to_mac(msg,inst,TRUE) != NCSCC_RC_SUCCESS)
    {
        /* TBD - LOGGING */
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    return NCSCC_RC_SUCCESS; 
}

/****************************************************************************\
*  Name:          mas_oaa_down_process                                       * 
*                                                                            *
*  Description:   Cleans up the filters registerd by this OAA                * 
*                                                                            *
*  Arguments:     MAB_MSG  *msg - Contains the MDS_DEST of the dead OAA      * 
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 NCSCC_RC_FAILURE - something went wrong                    *
\****************************************************************************/
/* STANDBY MAS will call this function only from the MBCSv callback */ 
static uns32
mas_oaa_down_process(MAB_MSG *msg)
{
    uns32       status; 
    MAS_TBL* inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

    m_MAB_DBG_TRACE("\nmas_oaa_down_process():entered.");

    /* log the function entry API */ 
    if (inst == NULL)
        return NCSCC_RC_FAILURE;            
        
    /* take the lock */ 
    m_MAS_LK_INIT;
    m_MAS_LK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_LOCKED,&inst->lock);

    /* update the standby about this OAA Down */ 
    /* m_MAS_RE_OAA_DOWN_SYNC(inst, msg); */
    
    /* delete the filters of OAA from the MAS table records */                 
    status = mas_flush_fltrs(inst, msg->fr_card, msg->fr_anc, msg->fr_svc);

    /* log that filter flushing is done for this OAA */ 

    /* give the handle before it is freed */
    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

    /* update the standby about this OAA Down */ 
    /* m_MAS_RE_OAA_DOWN_SYNC_DONE(inst, msg); */

    /* release the lock */ 
    m_MAS_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
    
    m_MAB_DBG_TRACE("\nmas_oaa_down_process():left.");
    return status; 
}

/****************************************************************************\
*  Name:          mas_oaa_role_chg_process                                   * 
*                                                                            *
*  Description:   Processes the state change of a OAA                        * 
*                                                                            *
*  Arguments:     MAB_MSG *msg - contains the new role, MDS_DEST of OAA,     * 
*                                whose state has changed recently            *
*                                                                            * 
*  Returns:       NCSCC_RC_SUCCESS - Message was posted successfully         *
*                 NCSCC_RC_FAILURE - somethind went wrong while adjust the   *
*                                    mapping.                                * 
\****************************************************************************/
static uns32
mas_oaa_role_chg_process(MAB_MSG *msg)
{
    uns32       status, i;
    MAS_FLTR    *prev_fltr, *fltr;
    MAS_ROW_REC *tbl_rec; 
    MAS_TBL* inst = (MAS_TBL*)m_MAS_VALIDATE_HDL(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));

    m_MAB_DBG_TRACE("\nmas_oaa_role_chg_process():entered.");

    /* log the function entry API */ 
    if (inst == NULL)
        return NCSCC_RC_FAILURE;            
        
    /* take the lock */ 
    m_MAS_LK_INIT;
    m_MAS_LK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_LOCKED,&inst->lock);

    /* for all the buckets in MAS */ 
    for(i = 0;i < MAB_MIB_ID_HASH_TBL_SIZE;i++)
    {
        /* search for all the tables in a particular bucket */
        for(tbl_rec = inst->hash[i];tbl_rec != NULL; tbl_rec = tbl_rec->next)
        {
            /* search by SS-ID, is this the subsystem went down?  */
            if(tbl_rec->ss_id == msg->fr_svc)
            {
                /* if there is a deafault filter for this table, see is it from the same OAA */ 
                if (tbl_rec->dfltr_regd == TRUE)
                {
                    /* see if this OAA has registered a Default filter */ 
                    if(memcmp(&tbl_rec->dfltr.vcard, &msg->fr_card, sizeof(msg->fr_card)) == 0)
                    {
                        /* adjust filter-id's list */ 
                        status = mas_fltr_ids_adjust(&tbl_rec->dfltr.fltr_ids, msg->fr_anc, msg->fr_role); 
                        if (status != NCSCC_RC_FAILURE)
                        {
                            /* log the error */ 
                            /* continue cleaning up the other fitlers of the OAA in other tables */ 
                        }
                    }
                }
                
                prev_fltr = NULL;
                for(fltr = tbl_rec->fltr_list;fltr != NULL;)
                {
                    if(memcmp(&fltr->vcard, &msg->fr_card, sizeof(fltr->vcard)) == 0)
                    {
                        status= mas_fltr_ids_adjust(&fltr->fltr_ids, msg->fr_anc, msg->fr_role); 
                        if (status != NCSCC_RC_FAILURE)
                        {
                            /* log the error */ 
                            /* continue cleaning up the other fitlers of the OAA in other tables */ 
                        }
                    } 
                    prev_fltr = fltr; 
                    fltr = fltr->next;
                } /* for(fltr = tbl_rec->fltr_list;fltr != NULL;) */
            }
        } /* for(tbl_rec = inst->hash[i];tbl_rec != NULL;...) */
    }/* for all the buckets in the hash table */

    /* release the lock */ 
    m_MAS_UNLK(&inst->lock);
    m_LOG_MAB_LOCK(MAB_LK_MAS_UNLOCKED,&inst->lock);
    
    ncshm_give_hdl(NCS_PTR_TO_INT32_CAST(msg->yr_hdl));
    m_MAB_DBG_TRACE("\nmas_oaa_down_process():left.");
    return NCSCC_RC_SUCCESS; 
}

#endif /* (NCS_MAB == 1) */

