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

#if (NCS_IFSV_BOND_INTF == 1)

#include "ifd.h"
#include "ifsv_db.h"
#include "ifsv_ir_mapi.h"
#include "ifsv_mib_inc.h"


static
uns32 ifsv_bind_rec_get(IFSV_CB *cb, uns32 *bind_no_key,
                    IFSV_BIND_NODE  **ifsv_bind_node, NCS_BOOL test);
static
uns32 ifsv_ifd_bind_rec_delete(IFSV_CB *cb, uns32 bind_portnum );
static
uns32 ifsv_ifd_bind_rec_create(IFSV_CB *cb, IFSV_BIND_NODE *bind_node);


uns32  ncsifsvbindifentry_get(NCSCONTEXT cb, 
                                         NCSMIB_ARG *p_get_req, 
                                         NCSCONTEXT* data)
{
    IFSV_CB     *bind_ifsv_cb = (IFSV_CB *)cb;
    int32 bind_portnum;   
    NCS_IFSV_SPT  spt_info;
    IFSV_INTF_DATA *intf_data;
    NCS_IFSV_IFINDEX bind_ifindex;   


    bind_portnum = p_get_req->i_idx.i_inst_ids[0];

    spt_info.port   =  bind_portnum;
    spt_info.shelf  =  IFSV_BINDING_SHELF_ID;
    spt_info.slot    =  IFSV_BINDING_SLOT_ID;
    spt_info.type   =  NCS_IFSV_INTF_BINDING;   
    spt_info.subscr_scope   =  NCS_IFSV_SUBSCR_INT;   

    if(ifsv_get_ifindex_from_spt (&bind_ifindex, spt_info, bind_ifsv_cb) != NCSCC_RC_SUCCESS)
    {
        m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_get_ifindex_from_spt():ncsifsvbindifentry_get returned failure"," ");
        return NCSCC_RC_FAILURE;        
    }

    intf_data = ifsv_intf_rec_find (bind_ifindex, bind_ifsv_cb);    
   
    switch(p_get_req->req.info.get_req.i_param_id)
    {
           case ncsIfsvBindIfBindIfIndex_ID:
           case ncsIfsvBindIfMasterIfIndex_ID:
           case ncsIfsvBindIfSlaveIfIndex_ID:
           case ncsIfsvBindIfBindRowStatus_ID:
           {
              *data = (NCSCONTEXT)intf_data; 
           }
           break;

           default:
              return NCSCC_RC_INV_VAL;
     }
     if( data == NULL )
           return NCSCC_RC_FAILURE;
    return NCSCC_RC_SUCCESS;
}


uns32 ncsifsvbindifentry_extract(NCSMIB_PARAM_VAL *param, 
                                             NCSMIB_VAR_INFO  *var_info,
                                             NCSCONTEXT data,
                                             NCSCONTEXT buffer)
{
    IFSV_INTF_DATA *intf_data = (IFSV_INTF_DATA *)data;

    if(intf_data == NULL)
    {
        return NCSCC_RC_FAILURE;
    }

    switch(param->i_param_id)
    {
           case ncsIfsvBindIfBindPortNum_ID: 
           {
               param->i_fmat_id = NCSMIB_FMAT_INT; 
               param->i_length = sizeof(uns32); 
               param->info.i_int = intf_data->spt_info.port; 
           }
           break;

           case ncsIfsvBindIfBindIfIndex_ID:
           {
               param->i_fmat_id = NCSMIB_FMAT_INT; 
               param->i_length = sizeof(uns32); 
               param->info.i_int = intf_data->if_index; 
           }
           break;

           case ncsIfsvBindIfMasterIfIndex_ID:
           {
               param->i_fmat_id = NCSMIB_FMAT_INT; 
               param->i_length = sizeof(uns32); 
               param->info.i_int = intf_data->if_info.bind_master_ifindex; 
           }
           break;

           case ncsIfsvBindIfSlaveIfIndex_ID:
           {
               param->i_fmat_id = NCSMIB_FMAT_INT; 
               param->i_length = sizeof(uns32); 
               param->info.i_int = intf_data->if_info.bind_slave_ifindex; 
           }
           break;

           case ncsIfsvBindIfBindRowStatus_ID:
               param->i_fmat_id = NCSMIB_FMAT_INT; 
               param->i_length = sizeof(uns32); 
               if(intf_data->active)
               {
                  param->info.i_int = NCSMIB_ROWSTATUS_ACTIVE; 
               }
               else 
               {
                  param->info.i_int = NCSMIB_ROWSTATUS_NOTREADY;
               }
           break;
   }
   return NCSCC_RC_SUCCESS;
}

uns32 ncsifsvbindifentry_next(NCSCONTEXT cb,
                       NCSMIB_ARG *p_next_req,
                       NCSCONTEXT* data,
                       uns32* o_next_inst_id,
                       uns32* o_next_inst_id_len) 
{
    IFSV_INTF_DATA *intf_data;
    int32 bind_portnum;   
    uns32 next_index;
   
  
   m_NCS_CONS_PRINTF("\n ncsifsvbindifentry_next:  Received SNMP NEXT request\n");

    if(( p_next_req->i_idx.i_inst_ids == NULL)&&( p_next_req->i_idx.i_inst_len == 0 ))
       bind_portnum = 0;
    else
      bind_portnum = p_next_req->i_idx.i_inst_ids[0];

    intf_data = ncsifsvbindgetnextindex(cb,bind_portnum,&next_index);  

    if(intf_data == NULL)
    {
       o_next_inst_id[0] = 0xff;
       *o_next_inst_id_len = 1;
       return NCSCC_RC_FAILURE;
    }

    switch(p_next_req->req.info.next_req.i_param_id)
    {
           case ncsIfsvBindIfBindPortNum_ID: 
           case ncsIfsvBindIfBindIfIndex_ID:
           case ncsIfsvBindIfMasterIfIndex_ID:
           case ncsIfsvBindIfSlaveIfIndex_ID:
           case ncsIfsvBindIfBindRowStatus_ID:
           {
              *data = intf_data; 
           }
           break;

        default:
                 printf("Invalid parm\n");
                 return NCSCC_RC_INV_VAL;
     }

     if( data == NULL )
        return NCSCC_RC_FAILURE;

   o_next_inst_id[0] = next_index;
   *o_next_inst_id_len = 1;

   return NCSCC_RC_SUCCESS;
}

IFSV_INTF_DATA* ncsifsvbindgetnextindex(IFSV_CB *bind_ifsv_cb,uns32 bind_portnum,uns32 *next_index) 
{
    NCS_IFSV_SPT  spt_info;
    uns32 bind_ifindex;
    IFSV_INTF_DATA *intf_data;

    /* Trying to get the first row */
    if(bind_portnum == 0) 
                bind_portnum = 1;   


    while(bind_portnum <= bind_ifsv_cb->bondif_max_portnum)
    {
           spt_info.port           =  bind_portnum;
           spt_info.shelf          =  IFSV_BINDING_SHELF_ID;
           spt_info.slot           =  IFSV_BINDING_SLOT_ID;
           spt_info.type           =  NCS_IFSV_INTF_BINDING;   
           spt_info.subscr_scope   =  NCS_IFSV_SUBSCR_INT;   

           if(ifsv_get_ifindex_from_spt (&bind_ifindex, spt_info, bind_ifsv_cb) != NCSCC_RC_SUCCESS)
           {
                m_IFD_LOG_STR_2_NORMAL(IFSV_LOG_FUNC_RET_FAIL,"ifsv_get_ifindex_from_spt():ncsifsvbindgetnextindex returned failure"," ");
                bind_portnum++;
                continue;           
           }
           else
           {
                intf_data = ifsv_intf_rec_find (bind_ifindex, bind_ifsv_cb);    
                *next_index = bind_portnum + 1;
                return intf_data;
           }
    }
    return NULL;
}
 

uns32 ncsifsvbindifentry_setrow(NCSCONTEXT hdl, NCSMIB_ARG *arg,NCSMIB_SETROW_PARAM_VAL* params, NCSMIB_OBJ_INFO *var_info, NCS_BOOL test_flag)
{
   IFSV_CB                  *cb;
   NCSMEM_AID         mem_aid;
   NCSMIB_PARAM_VAL   param_val;
   NCSPARM_AID        rsp_pa;
   uns32             param_cnt = 0;
   uns32             i,rc;
    int32            master_ifindex = -1,slave_ifindex = -1;
   uns32           bind_portnum,bind_ifindex = 0;
   IFSV_EVT          *evt;
   uns8            space[1024];
   IFSV_INTF_DATA   *intf_data;
   IFSV_INTF_DATA   *slave_intf_data;
   USRBUF *ub = NULL;

   m_NCS_OS_MEMSET(&rsp_pa, 0, sizeof(NCSPARM_AID));
   ncsmem_aid_init(&mem_aid, space, 1024);


    (cb = ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key)) == 0 ? (cb = 
      ncshm_take_hdl(NCS_SERVICE_ID_IFND, arg->i_mib_key)):(cb = cb);

   if(cb == NULL)
   {
      return NCSCC_RC_FAILURE;
   }

   ub = m_MMGR_DITTO_BUFR(arg->rsp.info.setrow_rsp.i_usrbuf);
   if(ub == NULL)
     return NCSCC_RC_FAILURE;


  /* Get the master and slave if index. The slave index is optional */
  param_cnt = ncsparm_dec_init(&rsp_pa, ub);
  for(i =0; i < param_cnt; i++)
  {
     if( ncsparm_dec_parm( &rsp_pa, &param_val, &mem_aid) == NCSCC_RC_SUCCESS)
     {
        if(param_val.i_fmat_id == NCSMIB_FMAT_INT)
          {
           switch(param_val.i_param_id)
           {
               case ncsIfsvBindIfMasterIfIndex_ID:
                   master_ifindex = param_val.info.i_int;
               break;

               case ncsIfsvBindIfSlaveIfIndex_ID:
                    slave_ifindex = param_val.info.i_int;
               break;
           }      
       }
     }
  }    
  ncsparm_dec_done(&rsp_pa);

  if(master_ifindex < 0) 
  {
     ncshm_give_hdl(cb->cb_hdl);      
     return NCSCC_RC_FAILURE;
  }


   /* Get the index */
   bind_portnum = arg->i_idx.i_inst_ids[0];
   if(bind_portnum <= 0 || bind_portnum > cb->bondif_max_portnum)
   {
   ncshm_give_hdl(cb->cb_hdl);      
   return NCSCC_RC_FAILURE;
   }

   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return NCSCC_RC_FAILURE;
   }

      m_NCS_MEMSET(evt,0,sizeof(IFSV_EVT));
      evt->type = IFD_EVT_INTF_CREATE;
      evt->cb_hdl = arg->i_mib_key;
      evt->info.ifd_evt.info.intf_create.if_attr = (NCS_IFSV_IAM_CHNG_MASTER);
 
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.port  = bind_portnum;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.shelf = IFSV_BINDING_SHELF_ID;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.slot  = IFSV_BINDING_SLOT_ID;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.type  = NCS_IFSV_INTF_BINDING;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.subscr_scope = NCS_IFSV_SUBSCR_INT;

      if(ifsv_get_ifindex_from_spt (&bind_ifindex, evt->info.ifd_evt.info.intf_create.intf_data.spt_info, cb) == NCSCC_RC_SUCCESS)
         {
          evt->info.ifd_evt.info.intf_create.intf_data.if_index = bind_ifindex;
         }
   else
   {
          bind_ifindex = 0;
          evt->info.ifd_evt.info.intf_create.intf_data.if_index = 0;
   }

       /* Get the if_info of master ifindex and copy it to binding interface */
       intf_data = ifsv_intf_rec_find(master_ifindex, cb);
       if(intf_data == NULL)
       {
            ncshm_give_hdl(cb->cb_hdl);      
            return NCSCC_RC_FAILURE;
       }

       if(intf_data->spt_info.subscr_scope != NCS_IFSV_SUBSCR_EXT)
       {
       ncshm_give_hdl(cb->cb_hdl);      
       return NCSCC_RC_FAILURE;
       }

       /* We check the same stuff while adding it from application. This is to tell the user that he couldnot add the binding interface */
       /* Donot allow to bind more than one binding interface for a given master or slave */
       if((intf_data->if_info.bind_master_ifindex != 0) && (intf_data->if_info.bind_master_ifindex != bind_ifindex))
       {
            ncshm_give_hdl(cb->cb_hdl);      
            return NCSCC_RC_FAILURE;

       }

       if(slave_ifindex != 0)
       {
          slave_intf_data = ifsv_intf_rec_find(slave_ifindex, cb);
          if(slave_intf_data != NULL)
          {
             if(slave_intf_data->spt_info.subscr_scope != NCS_IFSV_SUBSCR_EXT)
             {
             ncshm_give_hdl(cb->cb_hdl);      
             return NCSCC_RC_FAILURE;
             }

             if((slave_intf_data->if_info.bind_slave_ifindex != 0) && (slave_intf_data->if_info.bind_slave_ifindex != bind_ifindex))
          {
                  ncshm_give_hdl(cb->cb_hdl);      
                  return NCSCC_RC_FAILURE;
          }
          }
          else
          {
                  ncshm_give_hdl(cb->cb_hdl);
                  return NCSCC_RC_FAILURE;
          }
       }


      m_NCS_MEMCPY((char *)&evt->info.ifd_evt.info.intf_create.intf_data.if_info,(char *)&intf_data->if_info, sizeof(NCS_IFSV_INTF_INFO));

   evt->info.ifd_evt.info.intf_create.intf_data.if_info.bind_master_ifindex = master_ifindex;

      if(slave_ifindex >= 0) {
      evt->info.ifd_evt.info.intf_create.intf_data.if_info.bind_slave_ifindex = slave_ifindex;
         }
      evt->info.ifd_evt.info.intf_create.intf_data.rec_type = NCS_IFSV_IF_INFO;
      evt->info.ifd_evt.info.intf_create.intf_data.originator = NCS_IFSV_EVT_ORIGN_IFD;
      evt->info.ifd_evt.info.intf_create.intf_data.originator_mds_destination = cb->my_dest;
      evt->info.ifd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFD;   


     /* Send the event */
      rc = m_NCS_IPC_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL);

      ncshm_give_hdl(cb->cb_hdl);      

      return rc;
}


uns32 ncsifsvbindifentry_set(NCSCONTEXT hdl, 
                               NCSMIB_ARG *arg, 
                               NCSMIB_VAR_INFO *var_info,
                               NCS_BOOL test_flag)
{
   NCS_BOOL            val_same_flag = FALSE;
   NCSMIB_SET_REQ      *i_set_req = &arg->req.info.set_req; 
   uns32               rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO  temp_mib_req;
   uns8             create_flag = FALSE;
   IFSV_BIND_NODE  *ifsv_bind_node;
   IFSV_CB         *ifsv_bind_cb;
   uns32           bind_portnum;/* the key to search the Ifd DataBase */

   /* Get the CB pointer from the CB handle */
   ifsv_bind_cb = (IFSV_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);



   if(ifsv_bind_cb == NULL)
   {
      return NCSCC_RC_NO_INSTANCE;
   }

   m_NCS_CONS_PRINTF("\nncsifsvbindifentry:  Received SNMP SET request\n");
   /* Pretty print the contents of NCSMIB_ARG */
   ncsmib_pp(arg); 

   m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO)); 
  
   bind_portnum = arg->i_idx.i_inst_ids[0];


   /* Validate row status */
   if(test_flag &&
      (i_set_req->i_param_val.i_param_id == ncsIfsvBindIfBindRowStatus_ID))
   {
      uns32  rc = NCSCC_RC_SUCCESS;


      ifsv_bind_node = (IFSV_BIND_NODE *)ncs_patricia_tree_get(&ifsv_bind_cb->ifsv_bind_mib_rec,
                                                            (uns8 *)&bind_portnum);
      /* Skip the MIB validations during PSSv playback. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
         temp_mib_req.info.i_val_status_util_info.row_status =
                                      &(i_set_req->i_param_val.info.i_int);
         temp_mib_req.info.i_val_status_util_info.row = (NCSCONTEXT)(ifsv_bind_node);

         /* call the mib routine handler */
         if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
         {
            ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
            return rc;
         }
      }
   }


   /* Get the record for the given ifKey */
   rc = ifsv_bind_rec_get(ifsv_bind_cb, &bind_portnum, &ifsv_bind_node, test_flag);

   if(test_flag &&
      (rc == NCSCC_RC_NO_INSTANCE) &&
      (var_info->access == NCSMIB_ACCESS_READ_CREATE))
   {
       ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
       return NCSCC_RC_SUCCESS;
   }
   /* Record not found, return from here */
   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
      return rc;
   }
   /* Do the row status validations */
   /* If the row status is up, don't allow to set the "set when down" objects */
   if((ifsv_bind_node->bind_info.status == NCS_ROW_ACTIVE) &&
      (var_info->set_when_down == TRUE))
   {
      /* Return error */
      ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
      return NCSCC_RC_INV_SPECIFIC_VAL;
   }

   /* Validate row status */
   if ( i_set_req->i_param_val.i_param_id == ncsIfsvBindIfBindRowStatus_ID ) 
   {
/*NCSMIB_ACCESS_READ_CREATE-START*/
      if ((i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_CREATE_GO) ||
          ((i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_ACTIVE)&& 
          (ifsv_bind_node->bind_info.status == NCSMIB_ROWSTATUS_CREATE_WAIT) ))
      {
         create_flag = TRUE;
      }
   }

   /* All the tests have been done now set the value */
   /* Set the object */
   if(test_flag != TRUE)
   {
      m_NCS_OS_MEMSET(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP;
      temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = (NCSCONTEXT)&ifsv_bind_node->bind_info;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS)
      {
         ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
         return rc;
      }

      if((!val_same_flag) &&
         (i_set_req->i_param_val.i_param_id == ncsIfsvBindIfBindRowStatus_ID))
      {
          if( i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_DESTROY )
          {
             if( ifsv_bind_node->bind_info.bind_port_no )
             {
                rc = ifsv_ifd_bind_rec_delete(ifsv_bind_cb, ifsv_bind_node->bind_info.bind_port_no);
             }
             else
                return NCSCC_RC_FAILURE;
          }
      }
   }
/*NCSMIB_ACCESS_READ_CREATE-START*/
   /* If the corresponding entry is not exists, and if the object is of
      read-create type then create a fsv_bind_node entry. */
   if ( test_flag != TRUE )
   {
       if (var_info->access == NCSMIB_ACCESS_READ_CREATE)
       {
          if ( (i_set_req->i_param_val.i_param_id != ncsIfsvBindIfBindRowStatus_ID) ||
             ( (i_set_req->i_param_val.i_param_id == ncsIfsvBindIfBindRowStatus_ID) 
             & (create_flag != FALSE) ) )
           {
              if(ifsv_bind_node->bind_info.bind_port_no)
              {
                 rc = ifsv_ifd_bind_rec_create(ifsv_bind_cb, ifsv_bind_node);
                 if(rc != NCSCC_RC_SUCCESS)
                 {
                    if(ncs_patricia_tree_del
                       (&ifsv_bind_cb->ifsv_bind_mib_rec, (NCSCONTEXT)ifsv_bind_node) == NCSCC_RC_SUCCESS)
                    {
                       m_MMGR_FREE_IFSV_BIND_NODE(ifsv_bind_node);
                    }
                    else
                    {
                       ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
                       return NCSCC_RC_FAILURE;
                    }
                 }
              }
           
           }
       }
       else
         rc =  NCSCC_RC_NO_INSTANCE;
   }

  /*NCSMIB_ACCESS_READ_CREATE-END */

   ncshm_give_hdl(ifsv_bind_cb->cb_hdl);
   return rc;
}


/*
** FUNCTION NAME :: ifsv_bind_rec_get
**
** This function Adds/Creates a new IFSV BIND MIB Reord, incase it doenst exist
** O/W it keeps on Adding the Attributes to IFSV_BIND_INFO structure
*/


uns32 ifsv_bind_rec_get(IFSV_CB *cb, uns32 *bind_no_key,
                    IFSV_BIND_NODE  **ifsv_bind_node, NCS_BOOL test)
{
   IFSV_BIND_INFO *bind_info;



   *ifsv_bind_node = (IFSV_BIND_NODE *)ncs_patricia_tree_get(&cb->ifsv_bind_mib_rec,
      (uns8 *)bind_no_key);


   if(test && (*ifsv_bind_node == NULL))
      return NCSCC_RC_NO_INSTANCE;

   if(*ifsv_bind_node == NULL)
   {
      /* Create the new node */
      *ifsv_bind_node = m_MMGR_ALLOC_IFSV_BIND_NODE;
      if(!*ifsv_bind_node)
      {
         m_IFA_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL,0);
         return NCSCC_RC_FAILURE;
      }

      m_NCS_OS_MEMSET(*ifsv_bind_node, 0 , sizeof(IFSV_BIND_NODE));

      bind_info = &((*ifsv_bind_node)->bind_info);

      bind_info->bind_port_no = *bind_no_key;

      bind_info->status = NCSMIB_ROWSTATUS_NOTREADY;

      (*ifsv_bind_node)->pat_node.key_info = (uns8*)&bind_info->bind_port_no;

      if(ncs_patricia_tree_add (&cb->ifsv_bind_mib_rec, (NCSCONTEXT)*ifsv_bind_node) != NCSCC_RC_SUCCESS)
      {
         m_MMGR_FREE_IFSV_BIND_NODE(*ifsv_bind_node);
         return NCSCC_RC_FAILURE;
      }
      else
      {
          /*
           LOG Failure
          */
      }
   }

   return NCSCC_RC_SUCCESS;
}

uns32 ifsv_ifd_bind_rec_create(IFSV_CB *cb, IFSV_BIND_NODE *ifsv_bind_node)
{
   IFSV_INTF_DATA   *intf_data;
   IFSV_EVT          *evt;
   uns32 master_ifindex = ifsv_bind_node->bind_info.master_ifindex;
   uns32 bind_portnum;
   uns32 bind_ifindex;
   uns32 slave_ifindex = ifsv_bind_node->bind_info.slave_ifindex;
   uns32 rc;

  if(master_ifindex < 0)
  {
     ncshm_give_hdl(cb->cb_hdl);
     return NCSCC_RC_FAILURE;
  }

   /* Get the index */
   bind_portnum = ifsv_bind_node->bind_info.bind_port_no;


   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
      m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
      return NCSCC_RC_FAILURE;
   }

      m_NCS_MEMSET(evt,0,sizeof(IFSV_EVT));
      evt->type = IFD_EVT_INTF_CREATE;
      evt->cb_hdl = cb->cb_hdl;
      evt->info.ifd_evt.info.intf_create.if_attr = (NCS_IFSV_IAM_CHNG_MASTER);

      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.port  = bind_portnum;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.shelf = IFSV_BINDING_SHELF_ID;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.slot  = IFSV_BINDING_SLOT_ID;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.type  = NCS_IFSV_INTF_BINDING;
      evt->info.ifd_evt.info.intf_create.intf_data.spt_info.subscr_scope = NCS_IFSV_SUBSCR_INT;

      if(ifsv_get_ifindex_from_spt (&bind_ifindex, evt->info.ifd_evt.info.intf_create.intf_data.spt_info, cb) == NCSCC_RC_SUCCESS)
         {
          evt->info.ifd_evt.info.intf_create.intf_data.if_index = bind_ifindex;
         }
   else
   {
          bind_ifindex = 0;
          evt->info.ifd_evt.info.intf_create.intf_data.if_index = 0;
   }

       /* Get the if_info of master ifindex and copy it to binding interface */
       intf_data = ifsv_intf_rec_find(master_ifindex, cb);
       if(intf_data == NULL)
       {
            ncshm_give_hdl(cb->cb_hdl);
            return NCSCC_RC_FAILURE;
       }

       /* We check the same stuff while adding it from application. This is to tell the user that he couldnot add the binding interface */
       /* Donot allow to bind more than one binding interface for a given master or slave */
       if((intf_data->if_info.bind_master_ifindex != 0) && (intf_data->if_info.bind_master_ifindex != bind_ifindex))
       {
            ncshm_give_hdl(cb->cb_hdl);
            return NCSCC_RC_FAILURE;

       }

       if(slave_ifindex != 0)
       {
          intf_data = ifsv_intf_rec_find(slave_ifindex, cb);
          if(intf_data == NULL)
          {
               ncshm_give_hdl(cb->cb_hdl);
               return NCSCC_RC_FAILURE;
          }
          else
          {
             if((intf_data->if_info.bind_slave_ifindex != 0) && (intf_data->if_info.bind_slave_ifindex != bind_ifindex))
             {
                  ncshm_give_hdl(cb->cb_hdl);
                  return NCSCC_RC_FAILURE;
             }
          }
       }


      m_NCS_MEMCPY((char *)&evt->info.ifd_evt.info.intf_create.intf_data.if_info,(char *)&intf_data->if_info, sizeof(NCS_IFSV_INTF_INFO));

   evt->info.ifd_evt.info.intf_create.intf_data.if_info.bind_master_ifindex = master_ifindex;

      if(slave_ifindex >= 0) {
      evt->info.ifd_evt.info.intf_create.intf_data.if_info.bind_slave_ifindex = slave_ifindex;
         }
      evt->info.ifd_evt.info.intf_create.intf_data.rec_type = NCS_IFSV_IF_INFO;
      evt->info.ifd_evt.info.intf_create.intf_data.originator = NCS_IFSV_EVT_ORIGN_IFD;
      evt->info.ifd_evt.info.intf_create.intf_data.originator_mds_destination = cb->my_dest;
      evt->info.ifd_evt.info.intf_create.intf_data.current_owner = NCS_IFSV_OWNER_IFD;


     /* Send the event */
      rc = m_NCS_IPC_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL);

      ncshm_give_hdl(cb->cb_hdl);

      return rc;
}

uns32 ifsv_ifd_bind_rec_delete(IFSV_CB *cb, uns32 bind_portnum )
{
   IFSV_EVT          *evt;
   IFSV_BIND_NODE *ifsv_bind_node;

   evt = m_MMGR_ALLOC_IFSV_EVT;
   if (evt == IFSV_NULL)
   {
     m_IFD_LOG_SYS_CALL_FAIL(IFSV_LOG_MEM_ALLOC_FAIL, 0);
     return NCSCC_RC_FAILURE;
   }
   m_NCS_MEMSET(evt,0,sizeof(IFSV_EVT));
   evt->type = IFD_EVT_INTF_DESTROY;
   evt->cb_hdl = cb->cb_hdl;
   evt->info.ifd_evt.info.intf_destroy.orign = NCS_IFSV_EVT_ORIGN_IFD;
   evt->info.ifd_evt.info.intf_destroy.own_dest =  cb->my_dest;
   
   /* not filling scope here */
   evt->info.ifd_evt.info.intf_destroy.spt_type.port  = bind_portnum;
   evt->info.ifd_evt.info.intf_destroy.spt_type.shelf = IFSV_BINDING_SHELF_ID;
   evt->info.ifd_evt.info.intf_destroy.spt_type.slot  = IFSV_BINDING_SLOT_ID;
   evt->info.ifd_evt.info.intf_destroy.spt_type.type  = NCS_IFSV_INTF_BINDING;
   evt->info.ifd_evt.info.intf_destroy.spt_type.subscr_scope = NCS_IFSV_SUBSCR_INT;
   
   /* Send the event */
   m_NCS_IPC_SEND(&cb->mbx, evt, NCS_IPC_PRIORITY_NORMAL);
   
/* Delete the ifsv_bind_node Node Created in from ifsv_bind_mib_rec patricia tree */

   ifsv_bind_node = (IFSV_BIND_NODE *)ncs_patricia_tree_get(&cb->ifsv_bind_mib_rec,
      (uns8 *)&bind_portnum);

    if(ncs_patricia_tree_del (&cb->ifsv_bind_mib_rec, (NCSCONTEXT)ifsv_bind_node) == NCSCC_RC_SUCCESS)
    {
        m_MMGR_FREE_IFSV_BIND_NODE(ifsv_bind_node);
    }
    else
    {
        ncshm_give_hdl(cb->cb_hdl);
        return NCSCC_RC_FAILURE;
    }


   ncshm_give_hdl(cb->cb_hdl);
   return NCSCC_RC_SUCCESS;
 
}

#endif
