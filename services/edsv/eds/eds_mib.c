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

  DESCRIPTION: This file contains routines for 
                         - Interaction with MAB(Mib Access Broker Module)
                         - Registration of edsv mib tables & filters with OAC 
                         - OAC callbacks for EDSV MIB requests.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */


#include "eds.h"


/****************************************************************************
 * Name          : edsv_mab_register
 *
 * Description   : This function registers edsv mib tables with MAB.
 *                 
 *
 * Arguments     : eds_cb * - Pointer to the EDS Control Block.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 edsv_mab_register(EDS_CB *cb)
{
   NCSOAC_SS_ARG            edsv_oac_arg;
   NCSMIB_TBL_ID            tbl_id;
   uns32 rc = NCSCC_RC_SUCCESS;

   /* Register for all tables */

   for (tbl_id  = NCSMIB_TBL_EDSV_BASE;
        tbl_id <= NCSMIB_TBL_EDSV_CHAN_TBL;
        tbl_id ++)
   {
      /* Log Message # Register for table Id */
       m_NCS_MEMSET(&edsv_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

       edsv_oac_arg.i_oac_hdl                = cb->mab_hdl;
       edsv_oac_arg.info.tbl_owned.i_ss_id   = NCS_SERVICE_ID_EDS;
       edsv_oac_arg.info.tbl_owned.i_mib_req = edsv_mab_request;
       edsv_oac_arg.info.tbl_owned.i_mib_key = cb->my_hdl; /* Check this.TBD */
       edsv_oac_arg.i_op         = NCSOAC_SS_OP_TBL_OWNED;
       edsv_oac_arg.i_tbl_id     = tbl_id;

     if((rc =ncsoac_ss(&edsv_oac_arg)) != NCSCC_RC_SUCCESS)
     {
        /* Log Error */
        m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,tbl_id);
        m_NCS_CONS_PRINTF("oac register failed \n"); 
        return NCSCC_RC_FAILURE ;
     }
     else
     {
       /* Log Success */
       m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,tbl_id);
       m_NCS_CONS_PRINTF("oac register success \n");
     }

      /* Register the Filter for each table. */

      edsv_oac_arg.i_op = NCSOAC_SS_OP_ROW_OWNED;
      edsv_oac_arg.info.row_owned.i_fltr.type = NCSMAB_FLTR_ANY;
      edsv_oac_arg.info.row_owned.i_fltr.fltr.any.meaningless = 0x123;

      if((rc =ncsoac_ss(&edsv_oac_arg)) != NCSCC_RC_SUCCESS)
      { 
        m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,tbl_id);
        return NCSCC_RC_FAILURE;
      }
     switch (tbl_id)
      {
       case NCSMIB_TBL_EDSV_SCALAR:

                 cb->row_handle.scalar_tbl_row_hdl = 
                    (uns32)edsv_oac_arg.info.row_owned.o_row_hdl;
                 m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,cb->row_handle.scalar_tbl_row_hdl,__FILE__,__LINE__,tbl_id);

                 break ;

       case NCSMIB_TBL_EDSV_CHAN_TBL:

                 cb->row_handle.chan_tbl_row_hdl = 
                    (uns32)edsv_oac_arg.info.row_owned.o_row_hdl;
                 m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,cb->row_handle.chan_tbl_row_hdl,__FILE__,__LINE__,tbl_id);

                 break ;
       case NCSMIB_TBL_EDSV_TRAPOBJ_TBL:
       case NCSMIB_TBL_EDSV_TRAP_TBL:
       /* Change this later accordingly, while supporting trap.TBD */
                 break ;  
       default :
              /* Log it. */
                 m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,tbl_id);
                 return rc=NCSCC_RC_FAILURE;

     } /* End of switch tbl_id */

   } /* end for tbl_id loop. Registration of all tables */

    return rc ;
}

/****************************************************************************
 * Name          : edsv_mab_unregister
 * 
 * Description   : This function unregisters edsv mib tables with MAB.
 *                 
 *     
 * Arguments     : eds_cb * - Pointer to the EDS Control Block.
 *     
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/


uns32 edsv_mab_unregister(EDS_CB *cb)
{
   NCSOAC_SS_ARG            edsv_oac_arg;
   NCSMIB_TBL_ID            tbl_id;
   uns32 rc = NCSCC_RC_SUCCESS ;

   /* Unregister for all tables */

   for (tbl_id  = NCSMIB_TBL_EDSV_BASE;
        tbl_id <= NCSMIB_TBL_EDSV_CHAN_TBL;
        tbl_id ++)
   {
      m_NCS_MEMSET(&edsv_oac_arg, 0, sizeof(NCSOAC_SS_ARG));

     edsv_oac_arg.i_tbl_id = tbl_id;
     edsv_oac_arg.i_op = NCSOAC_SS_OP_ROW_GONE;
     edsv_oac_arg.i_oac_hdl = cb->mab_hdl;

     switch (tbl_id)
      {
       case NCSMIB_TBL_EDSV_SCALAR :
           if (cb->row_handle.scalar_tbl_row_hdl)
           {
              edsv_oac_arg.info.row_gone.i_row_hdl = 
                          cb->row_handle.scalar_tbl_row_hdl;

             if((rc = ncsoac_ss(&edsv_oac_arg)) != NCSCC_RC_SUCCESS)
             {
               /* Log Error */
                   m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,cb->row_handle.scalar_tbl_row_hdl);
                   return NCSCC_RC_FAILURE;
             }
             else
                   m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,cb->row_handle.scalar_tbl_row_hdl);

                 cb->row_handle.scalar_tbl_row_hdl = 0;

                 edsv_oac_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
                 edsv_oac_arg.info.tbl_gone.i_meaningless = (void *)0x1234;

                 if((rc = ncsoac_ss(&edsv_oac_arg)) != NCSCC_RC_SUCCESS)
                 {
                   m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,cb->row_handle.scalar_tbl_row_hdl);
                   return NCSCC_RC_FAILURE;
                 }
                 else
                   m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,cb->row_handle.scalar_tbl_row_hdl);
       
           }
           break ;

       case NCSMIB_TBL_EDSV_CHAN_TBL:
           if (cb->row_handle.chan_tbl_row_hdl)
           {
              edsv_oac_arg.info.row_gone.i_row_hdl = 
                          cb->row_handle.chan_tbl_row_hdl;

             if((rc = ncsoac_ss(&edsv_oac_arg)) != NCSCC_RC_SUCCESS)
             {
                   m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,cb->row_handle.chan_tbl_row_hdl);
                   return NCSCC_RC_FAILURE;
             }
             else 
                   m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,cb->row_handle.chan_tbl_row_hdl);


                 cb->row_handle.chan_tbl_row_hdl = 0;

                 edsv_oac_arg.i_op = NCSOAC_SS_OP_TBL_GONE;
                 edsv_oac_arg.info.tbl_gone.i_meaningless = (void *)0x1234;

                 if((rc = ncsoac_ss(&edsv_oac_arg)) != NCSCC_RC_SUCCESS)
                 {
                   m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,cb->row_handle.chan_tbl_row_hdl);
                   return NCSCC_RC_FAILURE;
                 }
                 else
                   m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,rc,__FILE__,__LINE__,cb->row_handle.chan_tbl_row_hdl);
           }
           break ;
       default :
            m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,tbl_id);
            break ;

     } /* End of switch statement */

   } /* end for tbl_id loop. UnRegistration of all tables */

   return rc ;
}

/****************************************************************************
 * Name          : edsv_mab_request
 * 
 * Description   : This function is invoked for processing all EDSv MAB
 *                 requests.
 *                 
 *
 * Arguments     : NCSMIB_ARG * - Pointer to the NCSMIB_ARG structure.
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/
uns32 edsv_mab_request(NCSMIB_ARG *args)
{
   EDS_CB *eds_cb=NULL;
   EDSV_EDS_EVT *evt=NULL;
   uns32       rc = NCSCC_RC_SUCCESS; 
  if (args == NULL)
  { 
      m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      return NCSCC_RC_FAILURE;
  }
  evt = m_MMGR_ALLOC_EDSV_EDS_EVT;
  if (evt == EDSV_EDS_EVT_NULL)
  {
      m_LOG_EDSV_S(EDS_MEM_ALLOC_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
      return NCSCC_RC_FAILURE;
  }
  /* Check here, if i_mib_key is properly stored.Done.Remove thiscomment.TBD */
  eds_cb = (EDS_CB *)ncshm_take_hdl(NCS_SERVICE_ID_EDS, (uns32)(args->i_mib_key));

  if(eds_cb == NULL)
  {
      m_LOG_EDSV_S(EDS_CB_TAKE_HANDLE_FAILED,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
      return NCSCC_RC_FAILURE;
  }

  evt->cb_hdl = args->i_mib_key;
  evt->evt_type = EDSV_EVT_MIB_REQ; /* Check if this type definition clashes with someother fn handler */

  /* make a copy of the MIB request */
  evt->info.mib_req = ncsmib_memcopy(args);

  if (evt->info.mib_req == NULL)
  {
      m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
      ncshm_give_hdl(args->i_mib_key);
      m_MMGR_FREE_EDSV_EDS_EVT(evt);
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
      return NCSCC_RC_FAILURE;
  }

   if ((rc = m_NCS_IPC_SEND(&eds_cb->mbx,evt,NCS_IPC_PRIORITY_NORMAL))
            != NCSCC_RC_SUCCESS)
   {
      m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      m_EDSV_DEBUG_CONS_PRINTF("ipc send failed\n");
      ncshm_give_hdl(args->i_mib_key);
      m_MMGR_FREE_EDSV_EDS_EVT(evt);
      args->rsp.i_status = NCSCC_RC_NO_INSTANCE;
      args->i_op = m_NCSMIB_REQ_TO_RSP(args->i_op);
      args->i_rsp_fnc(args);
      return NCSCC_RC_FAILURE;
   }

   ncshm_give_hdl(args->i_mib_key);

   return NCSCC_RC_SUCCESS;
    
}/*End edsv_mab_request */


/****************************************************************************
 * Name          : edsv_table_register
 * 
 * Description   : This function registers all EDSv MIB tables.
 *                 
 * Arguments     : None.
 *     
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 edsv_table_register(void)
{
    uns32 rc = NCSCC_RC_SUCCESS ;
 
    if((rc = safevtscalarobject_tbl_reg()) != NCSCC_RC_SUCCESS)
    {
      m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return rc ;
    }
    else
    {
      m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,0);
    }
    if((rc = saevtchannelentry_tbl_reg()) != NCSCC_RC_SUCCESS)
    {
      m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,rc,__FILE__,__LINE__,0);
      return rc ;
    }
    else
    {
      m_LOG_EDSV_S(EDS_MIB_SUCCESS,NCSFL_LC_EDSV_INIT,NCSFL_SEV_INFO,1,__FILE__,__LINE__,0);
    }

    return rc;
}/* edsv_table_register */

/****************************************************************************
 * Name          : eds_retrieve_chan_table_index
 * 
 * Description   : This function registers all EDSv MIB tables.
 *                 
 * Arguments     : arg          : NCSMIB arg pointer 
 *                 channel_name : pointer to channel name
 *                 event_id     : pointer to event id 
 *
 * Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 *
 * Notes         : None.
 *****************************************************************************/

uns32 safevtscalarobject_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   EDS_CB *eds_cb=NULL;

   eds_cb=(EDS_CB *)cb;

   if (eds_cb == NULL)
   {
       m_LOG_EDSV_S(EDS_MIB_FAILURE,NCSFL_LC_EDSV_INIT,NCSFL_SEV_ERROR,0,__FILE__,__LINE__,0);
       return NCSCC_RC_NO_INSTANCE;
   }
   /* Check service state */
   if (eds_cb->scalar_objects.svc_state != RUNNING)
       return NCSCC_RC_NO_INSTANCE; 

   *data = (NCSCONTEXT)&eds_cb->scalar_objects;

    return NCSCC_RC_SUCCESS;

} /* End safevtscalarobject_get */

uns32 safevtscalarobject_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{
  uns32 rc = NCSCC_RC_SUCCESS;
  EDS_CB *eds_cb=NULL;
  uns32 svc_state=0;
 
  eds_cb=(EDS_CB *)cb;

  if (eds_cb == NULL)
      return NCSCC_RC_FAILURE;
  if(eds_cb->scalar_objects.svc_status == TRUE)
  {
    switch(arg->req.info.set_req.i_param_val.i_param_id)
    {
      case  safServiceState_ID: /* Only object with read-write access */

              if (test_flag == TRUE)
                  return NCSCC_RC_SUCCESS;
              
              svc_state = arg->req.info.set_req.i_param_val.info.i_int;
              
              if( (svc_state == RUNNING) || (svc_state == STOPPED))
              {
                  eds_cb->scalar_objects.svc_state = svc_state;
              }
              else 
                /* Incorrect value */
                return NCSCC_RC_NO_INSTANCE;
      break;

      default :
              return NCSCC_RC_INV_VAL; /* Invalid operation */
   }/*End Switch */
 }
 else
    return NCSCC_RC_INV_SPECIFIC_VAL;

  return rc;
}/* End safevtscalarobject_set */

uns32 safevtscalarobject_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{

  /* Ideally this fn should not be invoked. Check this. TBD */
  return NCSCC_RC_FAILURE;
 
}/*End safevtscalarobject_next */

uns32 safevtscalarobject_extract(NCSMIB_PARAM_VAL *param,
                                    NCSMIB_VAR_INFO *var_info,
                                    NCSCONTEXT data, NCSCONTEXT buffer)
{
  uns32 rc = NCSCC_RC_SUCCESS;

  EDS_MIB_SCALAR  *scalar = (EDS_MIB_SCALAR *)data;

      if (scalar == EDS_MIB_SCALAR_NULL)
      {
         /* The row was not found */
         return NCSCC_RC_NO_INSTANCE;
      }

     if (var_info->param_id == safSpecVersion_ID)
     {
         param->i_fmat_id = NCSMIB_FMAT_OCT;
         param->i_length  = scalar->version.length;
         memcpy((uns8 *)buffer,scalar->version.value,param->i_length);
         param->info.i_oct     = (uns8*)buffer;
     }
     else if (var_info->param_id == safAgentVendor_ID)
     {
          param->i_fmat_id = NCSMIB_FMAT_OCT;
          param->i_length  = scalar->vendor.length;
          memcpy((uns8 *)buffer,scalar->vendor.value,param->i_length);
          param->info.i_oct     = (uns8*)buffer;
     }
     else if (var_info->param_id == safServiceStartEnabled_ID)
     {
          param->info.i_int = (scalar->svc_status == 1)?1:2;
          param->i_length = 4;
          param->i_fmat_id = NCSMIB_FMAT_INT;
     }
     else
     {
        /* For the standard object types just call the MIBLIB 
         * utility routine for standard object types 
         */
       return ncsmiblib_get_obj_val(param, var_info, data, buffer);
     }
     
     return rc;
}/* End safevtscalarobject_extract */

uns32 safevtscalarobject_setrow(NCSCONTEXT cb, NCSMIB_ARG *args,
                     NCSMIB_SETROW_PARAM_VAL *params,
                     struct ncsmib_obj_info *obj_info,
                     NCS_BOOL flag)
{
 
 return NCSCC_RC_FAILURE;
}/*end safevtscalarobject_setrow */

uns32 saevtchannelentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,
                         NCSMIB_VAR_INFO* var_info, NCS_BOOL test_flag)
{

  /* No object in this table is read-write */
  return NCSCC_RC_FAILURE;
} /* saevtchannelentry_set */

EDS_WORKLIST *get_channel_from_worklist(EDS_CB *cb, SaNameT chan_name)
{
  EDS_WORKLIST *wp=NULL;

  wp = cb->eds_work_list;
  if(wp == NULL)
     return wp; /*No channels yet */
 
  /* Loop through the list for a matching channel name */

  while(wp)
  {
    if(wp->cname_len == chan_name.length) /* Compare channel length */
    {
   /*   len = chan_name.length; */

      /* Compare channel name */
      if(m_NCS_OS_MEMCMP(wp->cname,chan_name.value,chan_name.length) == 0)
          return wp; /* match found */
    }
    wp=wp->next;
  }/*End while */

 /* if we reached here, no channel by that name */
  return ((EDS_WORKLIST *)NULL);
}/*End get_channel_from_worklist */

uns32 saevtchannelentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg,
                                  NCSCONTEXT* data)
{
   EDS_CB *eds_cb = NULL;
   SaNameT chan_name;
   uns32 i = 0;
   EDS_WORKLIST *wp=NULL;

   eds_cb = (EDS_CB *)cb;
   if (eds_cb == NULL)
       return ( NCSCC_RC_FAILURE );

   m_NCS_MEMSET(&chan_name, '\0', sizeof(SaNameT));

   /* Get channel name instance, from the instant identifiers */
   chan_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];

   if(chan_name.length > SA_MAX_NAME_LENGTH)
      return NCSCC_RC_NO_INSTANCE;

   for(i = 0; i < chan_name.length; i++)
   {
      chan_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
   }

   /* Search for this channel in EDS channel worklist */
   wp=get_channel_from_worklist(eds_cb,chan_name);

   if(wp == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *data = (NCSCONTEXT)wp;

  return NCSCC_RC_SUCCESS;
}/* saevtchannelentry_get */

uns32 saevtchannelentry_extract(NCSMIB_PARAM_VAL* param,
                                NCSMIB_VAR_INFO* var_info, NCSCONTEXT data,
                                NCSCONTEXT buffer)
{
   EDS_CHAN_TBL  *chan_row=NULL;
   EDS_WORKLIST  *wp = (EDS_WORKLIST *)data;

   if (wp ==  NULL)
       return NCSCC_RC_FAILURE;
   else
       chan_row = &wp->chan_row;
       

   if (chan_row == NULL)
   {
      /* The row was not found */
      return NCSCC_RC_NO_INSTANCE;
   }

   /* This table has the standard object types so just call the MIBLIB 
    */
   switch(param->i_param_id)
   {
      case saEvtChannelCreationTimeStamp_ID:
           /* Convert SaSizeT to SNMP VarBind compatible form */
           m_EDSV_UNS64_TO_VB(param,buffer,chan_row->create_time);
           break;
      default :
           if ((var_info != NULL) && (var_info->offset != 0))
                return ncsmiblib_get_obj_val(param, var_info, data, buffer);
           else
                return NCSCC_RC_NO_OBJECT;

   }/*End Switch */

 return NCSCC_RC_SUCCESS;
}/* saevtchannelentry_extract */

uns32 saevtchannelentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg,
                           NCSCONTEXT* data, uns32* next_inst_id,
                           uns32 *next_inst_id_len)
{
   EDS_CB *eds_cb = NULL;
   SaNameT chan_name;
   uns32 i=0;
   EDS_WORKLIST *wp_next=NULL;
   EDS_CNAME_REC *cn;  
 
   eds_cb = (EDS_CB *)cb;
   if (eds_cb == NULL)
       return ( NCSCC_RC_FAILURE );

   m_NCS_MEMSET(&chan_name, '\0', sizeof(SaNameT));

   /* Get channel name instance, from the instant identifiers */
   if (arg->i_idx.i_inst_len != 0)
   {

      chan_name.length = (SaUint16T)arg->i_idx.i_inst_ids[0];
      for(i = 0; i < chan_name.length; i++)
      {
         chan_name.value[i] = (uns8)(arg->i_idx.i_inst_ids[i+1]);
      }
     
      if(chan_name.length > SA_MAX_NAME_LENGTH)
         return NCSCC_RC_NO_INSTANCE;
      
      chan_name.length = m_NCS_OS_HTONS(chan_name.length);

       /* Get the record pointer from the patricia tree */
       cn    =  (EDS_CNAME_REC *)ncs_patricia_tree_getnext(&eds_cb->eds_cname_list,
                                                           (uns8*)&chan_name);
       /* Get channel info from worklist */
       /* Get the next channel record. Its an ordered list */
       if(cn)     
          wp_next = cn->wp_rec;
   }
   else
   {  
      /* Get the record pointer from the patricia tree */
      
      cn =  (EDS_CNAME_REC *)ncs_patricia_tree_getnext(&eds_cb->eds_cname_list,
                                                        (uns8*)NULL);
      /*If num instances is 0, return the first channel. Check this.TBD  */
      if(cn)
        wp_next = cn->wp_rec; 
   }

   if(wp_next == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Now fill the next instance id from the next channel name */
   next_inst_id[0] = wp_next->cname_len;

   for(i = 0; i < next_inst_id[0]; i++)
   {
      next_inst_id[i + 1] = (uns32)(wp_next->cname[i]);
   }
   *next_inst_id_len = wp_next->cname_len+1;

   *data = (NCSCONTEXT)wp_next;

   return NCSCC_RC_SUCCESS;

}/*End saevtchannelentry_next */

uns32 saevtchannelentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                             NCSMIB_SETROW_PARAM_VAL* params,
                             struct ncsmib_obj_info* obj_info,
                             NCS_BOOL testrow_flag)
{

   return NCSCC_RC_FAILURE;

}/*End saevtchannelentry_setrow */

