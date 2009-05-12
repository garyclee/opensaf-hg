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


#include "ifsv.h"
/*****************************************************************************
FILE NAME: IFSV_CIF.C
DESCRIPTION: Interface Agent Routines.
******************************************************************************/

#define m_IFMIB_STR_TO_PARAM(param,str) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = strlen(str); \
   param->info.i_oct = (uns8 *)str; \
}


/* Get the next record of the interface for the given key */
uns32 ifsv_ifrec_getnext(IFSV_CB *cb, uns32 ifkey, 
                          NCS_IFSV_INTF_REC *ifmib_rec)
{
   IFSV_INTF_DATA *intf_data = NULL;
   IFSV_INTF_REC *intf_rec;

   if(ifkey == 0)
   {
      /* Get the first node */
      intf_rec = (IFSV_INTF_REC *)ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)NULL);
   }
   else
   {
      intf_rec = (IFSV_INTF_REC *)ncs_patricia_tree_getnext(&cb->if_tbl, (uns8*)&ifkey);
   }

   if(intf_rec == NULL)
      return NCSCC_RC_NO_INSTANCE;

   intf_data = &intf_rec->intf_data;

   /* Got the record, fill the ifmib_rec */
   ifmib_rec->if_index = intf_data->if_index;
   ifmib_rec->if_info = intf_data->if_info;
   ifmib_rec->spt_info = intf_data->spt_info;

   return NCSCC_RC_SUCCESS;
}

/* Function to get the interface record & Fill it in  NCS_IFSV_INTF_REC*/
uns32 ifsv_ifrec_get(IFSV_CB *cb, uns32 ifkey, 
                          NCS_IFSV_INTF_REC *ifmib_rec)
{
   IFSV_INTF_DATA *intf_data = NULL;

   intf_data = ifsv_intf_rec_find(ifkey, cb);

   if(intf_data == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Got the record, fill the ifmib_rec */
   ifmib_rec->if_index = intf_data->if_index;
   ifmib_rec->if_info = intf_data->if_info;
   ifmib_rec->spt_info = intf_data->spt_info;

   return NCSCC_RC_SUCCESS;
}

uns32 ifsv_ifkey_from_instid(const uns32* i_inst_ids, uns32 i_inst_len
                                    , NCS_IFSV_IFINDEX *ifkey)
{

   if(i_inst_len != IFSV_IFINDEX_INST_LEN)
      return NCSCC_RC_NO_INSTANCE;

   *ifkey = *i_inst_ids;

   return NCSCC_RC_SUCCESS;
}

/******************************************************************************
  Function :  ifentry_get
  
  Purpose  :  Get function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get request
                                  will be serviced

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ifentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   IFSV_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFINDEX  ifkey = 0;
   
   /* Get the CB pointer from the CB handle */
   (cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key)) == 0 ? (cb = 
      ncshm_take_hdl(NCS_SERVICE_ID_IFND, arg->i_mib_key)):(cb = cb);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Validate instance ID and get Key from Inst ID */
   rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len, 
      &ifkey);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   /* Reset the global if_rec stored in CB */
   memset(&cb->ifmib_rec, 0, sizeof(NCS_IFSV_INTF_REC));

   /* Get the record for the given ifKey */
   rc = ifsv_ifrec_get(cb, ifkey, &cb->ifmib_rec);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = &cb->ifmib_rec;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ifentry_next
  
  Purpose  :  Get Next function for Interface MIB 
 
  Input    :  NCSCONTEXT hdl,     - IFSV_CB pointer
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get next 
                                  request will be serviced.

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ifentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                   NCSCONTEXT *data, uns32* next_inst_id,
                                      uns32 *next_inst_id_len)
{
   IFSV_CB           *cb;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFINDEX  ifkey = 0;

   /* Get the CB pointer from the CB handle */
   (cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key)) == 0 ? (cb = 
      ncshm_take_hdl(NCS_SERVICE_ID_IFND, arg->i_mib_key)):(cb = cb);

   if(cb == NULL)
      return NCSCC_RC_FAILURE;

   if(arg->i_idx.i_inst_len)
   {
      /* Validate instance ID and get Key from Inst ID */
      rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len, 
         &ifkey);

      if(rc != NCSCC_RC_SUCCESS)
      {
         /* RSR:TBD Log the error */
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }

   }

   /* Reset the global if_rec stored in CB */
   memset(&cb->ifmib_rec, 0, sizeof(NCS_IFSV_INTF_REC));

   /* Get the record for the given ifKey */
   rc = ifsv_ifrec_getnext(cb, ifkey, &cb->ifmib_rec);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = &cb->ifmib_rec;

   /* Populate next instance ID */
   *next_inst_id_len = IFSV_IFINDEX_INST_LEN;
   next_inst_id[0] = cb->ifmib_rec.if_index;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


static uns32 ifsv_process_ifentry_set(IFSV_CB *cb, uns32 paramid)
{
   uns32             rc = NCSCC_RC_SUCCESS;
   IFSV_EVT          *evt;
   NCS_IFSV_BM       attr=0;
   IFSV_INTF_DATA    *intf_data = NULL;

   /* Get the MIB Record stored in Data Base */
   intf_data = ifsv_intf_rec_find(cb->ifmib_rec.if_index, cb);

   if(intf_data == NULL)
      return NCSCC_RC_FAILURE;

   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
      return NCSCC_RC_FAILURE;

   memset(evt, 0, sizeof(IFSV_EVT));
   evt->cb_hdl = cb->cb_hdl;

   switch(paramid)
   {
   case ifAdminStatus_ID:
      attr = attr | NCS_IFSV_IAM_ADMSTATE;
      intf_data->if_info.admin_state = cb->ifmib_rec.if_info.admin_state;
      break;
   default:
      /* Not a settable object */
      ifsv_evt_destroy (evt);
      return NCSCC_RC_FAILURE;
   }


   if(cb->comp_type == IFSV_COMP_IFD)
   {
      evt->type = IFD_EVT_INTF_CREATE;
      evt->info.ifd_evt.info.intf_create.if_attr = attr;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info = 
                                                  cb->ifmib_rec.spt_info;
      evt->info.ifd_evt.info.intf_create.intf_data.if_index = cb->ifmib_rec.if_index;
      evt->info.ifd_evt.info.intf_create.intf_data.if_info = cb->ifmib_rec.if_info;
      evt->info.ifd_evt.info.intf_create.intf_data.rec_type = NCS_IFSV_IF_INFO;

      evt->info.ifd_evt.info.intf_create.intf_data.originator = NCS_IFSV_EVT_ORIGN_IFD;
      evt->info.ifd_evt.info.intf_create.intf_data.originator_mds_destination = cb->my_dest;
      evt->info.ifd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFD;


   }
   else if(cb->comp_type == IFSV_COMP_IFND)
   {
      evt->type = IFND_EVT_INTF_CREATE;
      evt->info.ifnd_evt.info.intf_create.if_attr = attr;
      evt->info.ifnd_evt.info.intf_create.intf_data.spt_info = 
                                                  cb->ifmib_rec.spt_info;
      evt->info.ifnd_evt.info.intf_create.intf_data.if_info = 
                                                      cb->ifmib_rec.if_info;
      evt->info.ifnd_evt.info.intf_create.intf_data.rec_type = NCS_IFSV_IF_INFO;

      evt->info.ifnd_evt.info.intf_create.intf_data.originator = NCS_IFSV_EVT_ORIGN_IFND;
      evt->info.ifnd_evt.info.intf_create.intf_data.originator_mds_destination = cb->my_dest;
      evt->info.ifnd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFND;

      /* Send the event */
      rc = m_NCS_IPC_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL);
   }

   return rc;
}

/******************************************************************************
  Function :  ifentry_set
  
  Purpose  :  Set function for IFSV interface table objects 
 
  Input    :  NCSCONTEXT hdl,            - CB handle
              NCSMIB_ARG *arg,           - MIB argument (input)
              NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
              NCS_BOOL test_flag         - set/test operation (input)
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ifentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   IFSV_CB           *cb;
   NCS_BOOL           val_same_flag = FALSE;
   NCSMIB_SET_REQ     *i_set_req = &arg->req.info.set_req; 
   uns32             rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO temp_mib_req;
   uns32              ifkey=0;


      /* Get the CB pointer from the CB handle */
   (cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key)) == 0 ? (cb = 
      ncshm_take_hdl(NCS_SERVICE_ID_IFND, arg->i_mib_key)):(cb = cb);

   if(cb == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, 
                        arg->i_idx.i_inst_len, &ifkey);
   
   /* Get the record for the given ifKey */
   rc = ifsv_ifrec_get(cb, ifkey, &cb->ifmib_rec);

   /* Record not found, return from here */
   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   /* Set the object */
   if(test_flag != TRUE)
   {
      memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = (NCSCONTEXT)&cb->ifmib_rec;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }
   }

   if(val_same_flag != TRUE)
   {
      rc = ifsv_process_ifentry_set(cb, i_set_req->i_param_val.i_param_id);
   }

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}

uns32 ifentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_SUCCESS;


   switch(param->i_param_id)
   {
      case ifDescr_ID:
      {
         NCS_IFSV_INTF_REC *p_if_info = (NCS_IFSV_INTF_REC   *)data;
         m_IFMIB_STR_TO_PARAM(param, p_if_info->if_info.if_descr);
      }
      break;
/*
      case ifPhysAddress_ID:
      {
         NCS_IFSV_INTF_REC *p_if_info = (NCS_IFSV_INTF_REC   *)data;
         m_IFMIB_STR_TO_PARAM(param, p_if_info->if_info.phy_addr);
      }
      break;
*/

      default:
      {
         rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
      }

   }
   return rc;
}

uns32 ifentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

