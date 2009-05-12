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


#include "ifd.h"
/*****************************************************************************
FILE NAME: IFSV_CIFMAP.C
DESCRIPTION: Interface Agent Routines.
******************************************************************************/

static uns32 ifsv_ifmaprec_getnext(IFSV_CB *cb, NCS_IFSV_SPT *spt, 
                                           NCS_IFSV_SPT_MAP **spt_map);

static uns32 ifsv_ifmaprec_get(IFSV_CB *cb, NCS_IFSV_SPT *spt, 
                          NCS_IFSV_SPT_MAP **spt_map);

static uns32 ifsv_ifspt_from_instid(const uns32* i_inst_ids, uns32 i_inst_len
                                    , NCS_IFSV_SPT *spt);

static uns32 ifsv_instid_from_ifspt(NCS_IFSV_SPT *i_spt, 
                              uns32 *o_inst_ids, 
                              uns32 *o_inst_len);

/* Get the next record of the interface for the given key */
static uns32 ifsv_ifmaprec_getnext(IFSV_CB *cb, NCS_IFSV_SPT *spt, 
                                           NCS_IFSV_SPT_MAP **spt_map)
{
   IFSV_SPT_REC *spt_rec;
   NCS_PATRICIA_TREE *p_tree = &cb->if_map_tbl;

   if(spt) 
   {
      spt_rec = (IFSV_SPT_REC *)ncs_patricia_tree_getnext(p_tree, (uns8*)spt);
   }
   else
   {
      /* Get the first node */
      spt_rec = (IFSV_SPT_REC *)ncs_patricia_tree_getnext(p_tree, (uns8*)NULL);
   }

   if(spt_rec == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *spt_map = &spt_rec->spt_map;

   return NCSCC_RC_SUCCESS;
}

/* Function to get the interface record & Fill it in  NCS_IFSV_INTF_REC*/
static uns32 ifsv_ifmaprec_get(IFSV_CB *cb, NCS_IFSV_SPT *spt, 
                          NCS_IFSV_SPT_MAP **spt_map)
{
   IFSV_SPT_REC *spt_rec;
   NCS_PATRICIA_TREE *p_tree = &cb->if_map_tbl;

   spt_rec = (IFSV_SPT_REC *)ncs_patricia_tree_get(p_tree, 
      (uns8 *)spt);

   if (!spt_rec)
   {
      *spt_map = NULL;
      return (NCSCC_RC_FAILURE);
   }
   *spt_map = &spt_rec->spt_map;
   return NCSCC_RC_SUCCESS;
}

static uns32 ifsv_ifspt_from_instid(const uns32* i_inst_ids, uns32 i_inst_len
                                    , NCS_IFSV_SPT *spt)
{

   if(i_inst_len != IFSV_IFMAPTBL_INST_LEN)
      return NCSCC_RC_NO_INSTANCE;

   spt->shelf = i_inst_ids[0];
   spt->slot  = i_inst_ids[1];
   /* embedding subslot changes */
   spt->subslot  = i_inst_ids[2];
   spt->port  = i_inst_ids[3];
   spt->type  = i_inst_ids[4];
   spt->subscr_scope = i_inst_ids[5];

   return NCSCC_RC_SUCCESS;
}


static uns32 ifsv_instid_from_ifspt(NCS_IFSV_SPT *i_spt, 
                              uns32 *o_inst_ids, 
                              uns32 *o_inst_len )
{

   *o_inst_len = IFSV_IFMAPTBL_INST_LEN;

   o_inst_ids[0] = i_spt->shelf;  
   o_inst_ids[1] = i_spt->slot; 
   /* embedding subslot changes */
   o_inst_ids[2] = i_spt->subslot;
   o_inst_ids[3] = i_spt->port; 
   o_inst_ids[4] = i_spt->type;  
   o_inst_ids[5] = i_spt->subscr_scope;  

   return NCSCC_RC_SUCCESS;
}


/******************************************************************************
  Function :  ncsifsvifmapentry_get
  
  Purpose  :  Get function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get request
                                  will be serviced

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifmapentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   IFSV_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_SPT      spt;
   NCS_IFSV_SPT_MAP  *spt_map=NULL;
   
   /* Get the CB pointer from the CB handle */
   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Validate instance ID and get Key from Inst ID */
   rc = ifsv_ifspt_from_instid(arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len, 
      &spt);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifspt_from_instid() returned failure,rc:",rc);
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   /* Get the record for the given ifKey */
   rc = ifsv_ifmaprec_get(cb, &spt, &spt_map);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifmaprec_get() returned failure,rc:",rc);
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = (NCSCONTEXT)spt_map;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvifmapentry_next
  
  Purpose  :  Get Next function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get next 
                                  request will be serviced.

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifmapentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                             NCSCONTEXT *data, uns32* next_inst_id,
                             uns32 *next_inst_id_len)
{
   IFSV_CB           *cb;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_SPT      spt;
   NCS_IFSV_SPT_MAP  *spt_map=NULL;
   IFSV_INTF_DATA    *rec_data=NULL;

   /* Get the CB pointer from the CB handle */
   cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);
   if (cb == IFSV_NULL)
      return NCSCC_RC_FAILURE; 
   if(arg->i_idx.i_inst_len == 0)
   {
      /* Get the first node */
      rc = ifsv_ifmaprec_getnext(cb, (NCS_IFSV_SPT*)NULL, &spt_map);
      if(rc != NCSCC_RC_SUCCESS)
      {
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifmaprec_getnext() returned failure,rc:",rc);
      }
   }
   else
   {

      /* Validate instance ID and get Key from Inst ID */
      rc = ifsv_ifspt_from_instid(arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len, 
         &spt);

      if(rc != NCSCC_RC_SUCCESS)
      {
         /* RSR:TBD Log the error */
         m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifspt_from_instid() returned failure,rc:",rc);
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }

      /* Get the record for the given ifKey */
      rc = ifsv_ifmaprec_getnext(cb, &spt, &spt_map);
      if(rc != NCSCC_RC_SUCCESS)
      {
      m_IFD_LOG_STR_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_ifmaprec_getnext() returned failure,rc:",rc);
      }
   }

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }
   if(arg->req.info.next_req.i_param_id == ncsIfsvIfMapEntryIfIndex_ID)
   {
     *data = (NCSCONTEXT)spt_map;
   }
   else
   if(arg->req.info.next_req.i_param_id == ncsIfsvIfMapEntryIfInfo_ID)
    {
        if ((rec_data = ifsv_intf_rec_find (spt_map->if_index, cb)) == NULL)
        {
           *data = NULL;
           m_IFSV_LOG_HEAD_LINE_NORMAL(ifsv_log_svc_id_from_comp_type(cb->comp_type),\
           IFSV_LOG_SPT_TO_INDEX_LOOKUP_FAILURE, spt_map->if_index, 0);
           return NCSCC_RC_FAILURE;
        }
        *data = (NCSCONTEXT *)&(rec_data->if_info);
    }

   /* Populate  next instance ID */
   ifsv_instid_from_ifspt(&spt_map->spt, next_inst_id, next_inst_id_len); 

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvifmapentry_set
  
  Purpose  :  Set function for IFSV interface table objects 
 
  Input    :  NCSCONTEXT hdl,            - CB handle
              NCSMIB_ARG *arg,           - MIB argument (input)
              NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
              NCS_BOOL test_flag         - set/test operation (input)
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifmapentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                            NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   return NCSCC_RC_FAILURE;
}

#define m_IFSV_STR_TO_PARAM(param,str) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = strlen(str); \
   param->info.i_oct = (uns8 *)str; \
}

uns32 ncsifsvifmapentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_FAILURE;

   if(param->i_param_id == ncsIfsvIfMapEntryIfInfo_ID)
   {
       NCS_IFSV_INTF_INFO *if_info = ( NCS_IFSV_INTF_INFO *)data;
       m_IFSV_STR_TO_PARAM(param,if_info->if_name);
       rc = NCSCC_RC_SUCCESS;
   }
   else
      rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);

   return rc;
}

uns32 ncsifsvifmapentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

