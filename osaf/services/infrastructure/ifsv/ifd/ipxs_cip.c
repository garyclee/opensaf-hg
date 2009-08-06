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
#include "ifsvipxs.h"
#include "ipxs_mem.h"

NCS_IPV4_ADDR addr[255] = {0};
NCS_IP_ADDR  addrNet;
/*****************************************************************************
FILE NAME: IPXS_CIP.C
DESCRIPTION: Configuration (MIB Instrumentation) routines for IPXS.
             This file is having instrumentation routines for:
               IP - Table
               IFIP - Table
******************************************************************************/

static uns32 ipxs_ipkey_from_instance_id(IPXS_IP_KEY *key, 
                                    NCS_IP_ADDR_TYPE *type,
                                    const uns32 *inst_id, const uns32 inst_len);

static void ipxs_instance_id_from_ipkey(IPXS_IP_KEY *key, NCS_IP_ADDR_TYPE type,
                                                      uns32 *inst_id, uns32 *inst_len);

static uns32 ipxs_iprec_getnext(IPXS_CB *cb, IPXS_IP_KEY *ipkeyNet, 
                            NCS_IP_ADDR_TYPE type, IPXS_IP_INFO **ip_info);


#define m_IFSV_IP_INTEGER_TO_PARAM(param,val) \
{\
   param->i_fmat_id = NCSMIB_FMAT_INT; \
   param->i_length = sizeof(uns32); \
   param->info.i_int = m_NCS_OS_HTONL(val); \
}


/******************************************************************************
  Function :  ipxs_iprec_getnext
  
  Purpose  :  Get the next record of the interface for the given key
******************************************************************************/
static uns32 ipxs_iprec_getnext(IPXS_CB *cb, IPXS_IP_KEY *ipkeyNet, 
                            NCS_IP_ADDR_TYPE type, IPXS_IP_INFO **ip_info)
{
   uns32 index = 0;
   IPXS_IP_NODE   *ip_node;

   /* Get the DB index from type */
   m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, type);
   
   if(index == IPXS_AFI_MAX)   /* This address type is not supported */
      return NCSCC_RC_NO_INSTANCE;

   if(!ipkeyNet)
   {
      /* Get the first node */
      ip_node = (IPXS_IP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ip_tbl[index].ptree, (uns8*)NULL);
   }
   else
   {
      ip_node = (IPXS_IP_NODE *)ncs_patricia_tree_getnext
                      (&cb->ip_tbl[index].ptree, (uns8*)ipkeyNet);
   }

   if(ip_node == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *ip_info = &ip_node->ipinfo;

   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ipxs_ipkey_from_instance_id
* 
* Purpose:  Prepares the IP prefix key from instance-ids for mtree.
*
* Input:    BGP_PFX_KEY *key,                       
*           uns32       *inst_id
*
* Notes:  
**************************************************************************/
static uns32 ipxs_ipkey_from_instance_id(IPXS_IP_KEY *key, 
                                    NCS_IP_ADDR_TYPE *type,
                                    const uns32 *inst_id, const uns32 inst_len)
{
   uns32 count = 0, temp = 0;

   memset(key, 0, sizeof(IPXS_IP_KEY));

   if(!inst_len)
   {
      return NCSCC_RC_SUCCESS;
   }

   *type = inst_id[0];

   switch(*type)
   {
   case NCS_IP_ADDR_TYPE_IPV4:
      if(inst_len != IPXS_IPENTRY_INST_MIN_LEN)
         return NCSCC_RC_NO_INSTANCE;

      for(count=1; count<5 ; count ++)
      {
         temp = (inst_id[count]) << ((5 - (count+1)) * 8);
         key->ip.v4 = key->ip.v4 | temp;
      }
      key->ip.v4 = m_NCS_OS_HTONL(key->ip.v4);
      break;
#if(HJ_IPV6 == 1)
   case HJ_IP_ADDR_TYPE_IPV6:
      if(inst_len != IPXS_IPENTRY_INST_MAX_LEN)
         return NCSCC_RC_NO_INSTANCE;
      /* Read the V6 address */
      for(count=1; count<NCS_IPV6_ADDR_UNS8_CNT+1; count++)
      {
         key->ip.v6.m_ipv6_addr[count-1] = (uns8)inst_id[count];
      }
      break;
#endif   /* HJ_IPV6 == 1 */
   default:
      break;
   }
   
   return NCSCC_RC_SUCCESS;
}

/**************************************************************************
* Function: ipxs_instance_id_from_ipkey
*
* Purpose:  Prepares instance-ids from the IP key.
*
* Input:    IPXS_IP_KEY *key,                       
*           uns32       *inst_id
*
* Returns:  Nothing
*
* Notes:  
**************************************************************************/
static void ipxs_instance_id_from_ipkey(IPXS_IP_KEY *key, NCS_IP_ADDR_TYPE type,
                                                 uns32 *inst_id, uns32 *inst_len)
{
   uns32  count = 0, ip_addr=0;

   inst_id[0] = type;

   switch(type)
   {
   case NCS_IP_ADDR_TYPE_IPV4:
      ip_addr = ntohl(key->ip.v4); 
      for(count = 1; count < 5; count ++)
      {
         inst_id[count] = (ip_addr >> ((4 - count) * 8)) & 0x000000ff;
      }
      *inst_len = IPXS_IPENTRY_INST_MIN_LEN;
      break;
#if(HJ_IPV6 == 1)
   case NCS_IP_ADDR_TYPE_IPV6:

      /* Read the V6 address */
      for(count=1; count<NCS_IPV6_ADDR_UNS8_CNT+1; count++)
      {
         inst_id[count] = key->ip.v6.m_ipv6_addr[count-1];
      }
      *inst_len = IPXS_IPENTRY_INST_MAX_LEN;
      break;
#endif
   default:
      break;
   }

}

/******************************************************************************
  Function :  ncsifsvipentry_get
  
  Purpose  :  Get function for IP MIB 
 
  Input    :  NCSCONTEXT hdl,     - IPXS CB handle
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get request
                                  will be serviced

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvipentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   IPXS_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   IPXS_IP_KEY       ipkeyNet;
   NCS_IP_ADDR_TYPE  type = 0;
   IPXS_IP_NODE      *ip_node = NULL;

   /* Validate instance ID and get Key from Inst ID */
   rc = ipxs_ipkey_from_instance_id(&ipkeyNet, &type, 
            arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      return rc;
   }

   /* Get the CB pointer from the CB handle */
   cb = (IPXS_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Get the record for the given ifKey */
   rc = ipxs_iprec_get(cb, &ipkeyNet, type, &ip_node, TRUE);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = (NCSCONTEXT)&ip_node->ipinfo;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvipentry_next
  
  Purpose  :  Get Next function for IP MIB 
 
  Input    :  NCSCONTEXT hdl,     - IPXS CB handle
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get next 
                                  request will be serviced.

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvipentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                       NCSCONTEXT *data, uns32* next_inst_id,
                       uns32 *next_inst_id_len)
{
   IPXS_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   IPXS_IP_KEY       ipkeyNet;
   NCS_IP_ADDR_TYPE  type = 0;
   IPXS_IP_INFO      *ip_info = NULL;

   /* Validate instance ID and get Key from Inst ID */
   rc = ipxs_ipkey_from_instance_id(&ipkeyNet, &type, 
            arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      return rc;
   }

   /* Get the CB pointer from the CB handle */
   cb = (IPXS_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Get the record for the given ifKey */
   if(arg->i_idx.i_inst_len != 0)
      rc = ipxs_iprec_getnext(cb,  &ipkeyNet, type, &ip_info);
   else
   {
      type = NCS_IP_ADDR_TYPE_IPV4;
      rc = ipxs_iprec_getnext(cb,  NULL, type, &ip_info);
   }

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = (NCSCONTEXT) ip_info;

   ipxs_instance_id_from_ipkey(&ip_info->keyNet, ip_info->addr.type,
                                           next_inst_id, next_inst_id_len);

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvipentry_set
  
  Purpose  :  Set function for IFSV interface table objects 
 
  Input    :  NCSCONTEXT hdl,            - CB handle
              NCSMIB_ARG *arg,           - MIB argument (input)
              NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
              NCS_BOOL test_flag         - set/test operation (input)
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvipentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   IPXS_CB           *cb;
   IPXS_IP_KEY       ipkeyNet;
   NCS_IP_ADDR_TYPE  type = 0;
   IPXS_IP_NODE      *ip_node = NULL;
   NCS_BOOL           val_same_flag = FALSE;
   NCS_BOOL           create_flag = FALSE;
   NCSMIB_SET_REQ     *i_set_req = &arg->req.info.set_req; 
   uns32             rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO temp_mib_req;

   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   memset(&ipkeyNet, 0, sizeof(IPXS_IP_KEY));

   /* Validate instance ID and get Key from Inst ID */
   rc = ipxs_ipkey_from_instance_id(&ipkeyNet, &type, 
            arg->i_idx.i_inst_ids, arg->i_idx.i_inst_len);
      if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      return rc;
   }

   /* Get the CB pointer from the CB handle */
   cb = (IPXS_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   
   if (test_flag && ((i_set_req->i_param_val.i_param_id == ncsIfsvIpEntRowStatus_ID)))
   {
     uns32 index = 0;

     /* Get the DB index from type */
     m_IPXS_DB_INDEX_FROM_ADDRTYPE_GET(index, type);

     if(index == IPXS_AFI_MAX)   /* This address type is not supported */
       {
         ncshm_give_hdl(cb->cb_hdl);
         return NCSCC_RC_NO_INSTANCE;
       }

     ip_node = (IPXS_IP_NODE *)ncs_patricia_tree_get(&cb->ip_tbl[index].ptree,
                                                      (uns8 *)&ipkeyNet);

      /*NCSMIB_ACCESS_READ_CREATE-END*/
   
      /* Skip the MIB validations during PSSv playback. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP;
         temp_mib_req.info.i_val_status_util_info.row_status =
                                      &(i_set_req->i_param_val.info.i_int);
         temp_mib_req.info.i_val_status_util_info.row = (NCSCONTEXT)(ip_node);

         /* call the mib routine handler */
         rc = ncsmiblib_process_req(&temp_mib_req);
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }
   }   



  /* Get the record for the given ifKey */
   rc = ipxs_iprec_get(cb, &ipkeyNet, type, &ip_node, test_flag);
  

   if(test_flag && 
      (rc == NCSCC_RC_NO_INSTANCE) && 
      (var_info->access == NCSMIB_ACCESS_READ_CREATE))
   {
       ncshm_give_hdl(cb->cb_hdl);
       return NCSCC_RC_SUCCESS;
   }

   /* Record not found, return from here */
   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   /* Do the row status validations */
   /* If the row status is up, don't allow to set the "set when down" objects */
   if((ip_node->ipinfo.status == NCS_ROW_ACTIVE) &&
      (var_info->set_when_down == TRUE))
   {
      /* Return error */
      ncshm_give_hdl(cb->cb_hdl);
      return NCSCC_RC_INV_SPECIFIC_VAL;
   }


   /* Validate row status */
   if (i_set_req->i_param_val.i_param_id == ncsIfsvIpEntRowStatus_ID)
   {
/*NCSMIB_ACCESS_READ_CREATE-START*/
      if ((i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_CREATE_GO) ||
          (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_CREATE_WAIT))
      {
         create_flag = TRUE;
         i_set_req->i_param_val.info.i_int = NCSMIB_ROWSTATUS_NOTREADY;
      }
      else
      {
/*NCSMIB_ACCESS_READ_CREATE-END*/

      /* Skip the MIB validations during PSSv playback. */
      if((arg->i_policy & NCSMIB_POLICY_PSS_BELIEVE_ME) != NCSMIB_POLICY_PSS_BELIEVE_ME)
      {
         temp_mib_req.req = NCSMIBLIB_REQ_VALIDATE_STATUS_UTIL_OP; 
         temp_mib_req.info.i_val_status_util_info.row_status =
                                      &(i_set_req->i_param_val.info.i_int);
         temp_mib_req.info.i_val_status_util_info.row = (NCSCONTEXT)&(ip_node->ipinfo);
      
         /* call the mib routine handler */ 
         if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
         {
            ncshm_give_hdl(cb->cb_hdl);
            return rc;
         }
      }
    }
   }

   /* Set the object */
   if(test_flag != TRUE)
   {
      memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = (NCSCONTEXT)&ip_node->ipinfo;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }

   if((!val_same_flag) &&
      (i_set_req->i_param_val.i_param_id == ncsIfsvIpEntRowStatus_ID))
   {
        if(((ip_node->ipinfo.if_index) && (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_ACTIVE)) || 
          (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_DESTROY))
          {  
             if(ipxs_ifip_record_get(&cb->ifip_tbl, ip_node->ipinfo.if_index) == NULL)
              {
                ipxs_ip_record_del(cb, &cb->ip_tbl[0], ip_node);
                ncshm_give_hdl(cb->cb_hdl);
                return NCSCC_RC_SUCCESS;
              }
              else
                rc = ipxs_ifd_ipinfo_process(cb, ip_node);
          }
          else if ((ip_node->ipinfo.if_index == 0) && (i_set_req->i_param_val.info.i_int == NCSMIB_ROWSTATUS_ACTIVE))
          {
            ipxs_ip_record_del(cb, &cb->ip_tbl[0], ip_node);
            ncshm_give_hdl(cb->cb_hdl);
            return NCSCC_RC_FAILURE;
          }
   }
   }
/*NCSMIB_ACCESS_READ_CREATE-START*/
   if (var_info->access == NCSMIB_ACCESS_READ_CREATE)
   {
      if (((i_set_req->i_param_val.i_param_id == ncsIfsvIpEntRowStatus_ID)
           && (create_flag != FALSE)))
       {
        if(ip_node->ipinfo.if_index)
           rc = ipxs_ifd_ipinfo_process(cb, ip_node);
       }
   }
   else
     rc =  NCSCC_RC_NO_INSTANCE;

/*NCSMIB_ACCESS_READ_CREATE-END */

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}

uns32 ncsifsvipentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   IPXS_IP_INFO *ipinfo;
   uns16  len=0;

   memset(&addrNet, 0, sizeof(addrNet));

   switch(param->i_param_id)
   {
   case ncsIfsvIpEntAddr_ID:
      ipinfo = (IPXS_IP_INFO *) data;
      if(!ipinfo)
          return NCSCC_RC_FAILURE;
      m_NCS_HTON_IP_ADDR(&ipinfo->addr, &addrNet);
      if(ipinfo->addr.type == NCS_IP_ADDR_TYPE_IPV4)
         len = 4; /* Please not that this is not a magic no */
      else if(ipinfo->addr.type == NCS_IP_ADDR_TYPE_IPV6)
         len = 16; /* Please not that this is not a magic no */
      else
         return NCSCC_RC_FAILURE;

      param->i_fmat_id = NCSMIB_FMAT_OCT;                  
      param->i_length  = len;   
      param->info.i_oct = ((uns8*)&addrNet.info);
      break;

   case ncsIfsvIpEntAddrMask_ID:
      ipinfo = (IPXS_IP_INFO *) data;
      if(!ipinfo)
          return NCSCC_RC_FAILURE;

      m_IFSV_IP_INTEGER_TO_PARAM(param, ipinfo->mask );
      break;

   default:
      rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
      break;
   }

   return rc;
}

uns32 ncsifsvipentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                          NCSMIB_SETROW_PARAM_VAL* params,
                          struct ncsmib_obj_info* obj_info,
                          NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}


/******************************************************************************
  Function :  ipxs_ifiprec_get
  
  Purpose  :  To get the IF-IP Record
******************************************************************************/
uns32 ipxs_ifiprec_get(IPXS_CB *cb, uns32 ifkeyNet,
                                     IPXS_IFIP_INFO **ifip_info)
{
   IPXS_IFIP_NODE   *ifip_node;

   ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_get(&cb->ifip_tbl, 
      (uns8 *)&ifkeyNet);

   if(ifip_node == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *ifip_info = &ifip_node->ifip_info;

   return NCSCC_RC_SUCCESS;
}


/******************************************************************************
  Function :  ipxs_ifiprec_getnext
  
  Purpose  :  Get the next record of the interface for the given key
******************************************************************************/
static uns32 ipxs_ifiprec_getnext(IPXS_CB *cb, uns32 ifkeyNet,
                                     IPXS_IFIP_INFO **ifip_info)
{
   IPXS_IFIP_NODE   *ifip_node;

   if(!ifkeyNet)
   {
      /* Get the first node */
      ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                          (&cb->ifip_tbl, (uns8*)NULL);
   }
   else
   {
      ifip_node = (IPXS_IFIP_NODE *)ncs_patricia_tree_getnext
                                          (&cb->ifip_tbl, (uns8*)&ifkeyNet);
   }

   if(ifip_node == NULL)
      return NCSCC_RC_NO_INSTANCE;

   *ifip_info = &ifip_node->ifip_info;

   return NCSCC_RC_SUCCESS;
}


/******************************************************************************
  Function :  ncsifsvifipentry_get
  
  Purpose  :  Get function for IP MIB 
 
  Input    :  NCSCONTEXT hdl,     - IPXS CB handle
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get request
                                  will be serviced

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifipentry_get(NCSCONTEXT hdl, NCSMIB_ARG *arg, NCSCONTEXT* data)
{
   IPXS_CB            *cb = NULL;
   uns32              rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFINDEX   ifkeyNet = 0;
   IPXS_IFIP_INFO     *ifip_info = NULL;

   /* Validate instance ID and get Key from Inst ID */
   rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, 
                           arg->i_idx.i_inst_len, &ifkeyNet);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      return rc;
   }

   /* Get the CB pointer from the CB handle */
   cb = (IPXS_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Get the record for the given ifKey */
   rc = ipxs_ifiprec_get(cb, ifkeyNet, &ifip_info);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = (NCSCONTEXT)ifip_info;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvifipentry_next
  
  Purpose  :  Get Next function for IP MIB 
 
  Input    :  NCSCONTEXT hdl,     - IPXS CB handle
              NCSMIB_ARG *arg,   - MIB argument (input)
              NCSCONTEXT* data   - pointer to the data from which get next 
                                  request will be serviced.

  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifipentry_next(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                            NCSCONTEXT *data, uns32* next_inst_id,
                            uns32 *next_inst_id_len)
{
   IPXS_CB           *cb = NULL;
   uns32             rc = NCSCC_RC_SUCCESS;
   NCS_IFSV_IFINDEX  ifkeyNet = 0;
   IPXS_IFIP_INFO    *ifip_info = NULL;

   if(arg->i_idx.i_inst_len)
   {
      /* Validate instance ID and get Key from Inst ID */
      rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, 
                           arg->i_idx.i_inst_len, &ifkeyNet);

      if(rc != NCSCC_RC_SUCCESS)
      {
         /* RSR:TBD Log the error */
         return rc;
      }
   }

   /* Get the CB pointer from the CB handle */
   cb = (IPXS_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   /* Get the record for the given ifKey */
   rc = ipxs_ifiprec_getnext(cb,  ifkeyNet, &ifip_info);

   if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      ncshm_give_hdl(cb->cb_hdl);
      return rc;
   }

   *data = (NCSCONTEXT) ifip_info;
   next_inst_id[0] = ifip_info->ifindexNet;
   *next_inst_id_len = IPXS_IFIPENTRY_INST_LEN;

   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


/******************************************************************************
  Function :  ncsifsvifipentry_set
  
  Purpose  :  Set function for IFSV interface table objects 
 
  Input    :  NCSCONTEXT hdl,            - CB handle
              NCSMIB_ARG *arg,           - MIB argument (input)
              NCSMIB_VAR_INFO* var_info, - Object properties/info(i)
              NCS_BOOL test_flag         - set/test operation (input)
    
  Returns  :  NCSCC_RC_SUCCESSS/NCSCC_RC_FAILURE
 
  Notes    : 
******************************************************************************/
uns32 ncsifsvifipentry_set(NCSCONTEXT hdl, NCSMIB_ARG *arg, 
                    NCSMIB_VAR_INFO *var_info, NCS_BOOL test_flag)
{
   IPXS_CB              *cb;
   NCS_IFSV_IFINDEX     ifkeyNet = 0;
   IPXS_IFIP_INFO       *ifip_info = NULL;
   NCS_BOOL             val_same_flag = FALSE;
   NCSMIB_SET_REQ       *i_set_req = &arg->req.info.set_req; 
   uns32                rc = NCSCC_RC_SUCCESS;
   NCSMIBLIB_REQ_INFO   temp_mib_req;

   memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

   /* Validate instance ID and get Key from Inst ID */
   rc = ifsv_ifkey_from_instid(arg->i_idx.i_inst_ids, 
                           arg->i_idx.i_inst_len, &ifkeyNet);

      if(rc != NCSCC_RC_SUCCESS)
   {
      /* RSR:TBD Log the error */
      return rc;
   }

   /* Get the CB pointer from the CB handle */
   cb = (IPXS_CB *) ncshm_take_hdl(NCS_SERVICE_ID_IFD, arg->i_mib_key);

   if(cb == NULL)
      return NCSCC_RC_NO_INSTANCE;
   
   /* Get the record for the given ifKey */
   rc = ipxs_ifiprec_get(cb, ifkeyNet, &ifip_info);

   /* Record not found, return from here */
   if(rc != NCSCC_RC_SUCCESS)
   {
      ncshm_give_hdl(cb->cb_hdl);
      /* RSR:TBD Log the error */
      return rc;
   }

   /* Set the object */
   if(test_flag != TRUE)
   {
      memset(&temp_mib_req, 0, sizeof(NCSMIBLIB_REQ_INFO));

      temp_mib_req.req = NCSMIBLIB_REQ_SET_UTIL_OP; 
      temp_mib_req.info.i_set_util_info.param = &(i_set_req->i_param_val);
      temp_mib_req.info.i_set_util_info.var_info = var_info;
      temp_mib_req.info.i_set_util_info.data = (NCSCONTEXT)ifip_info;
      temp_mib_req.info.i_set_util_info.same_value = &val_same_flag;

      /* call the mib routine handler */ 
      if((rc = ncsmiblib_process_req(&temp_mib_req)) != NCSCC_RC_SUCCESS) 
      {
         ncshm_give_hdl(cb->cb_hdl);
         return rc;
      }
   }

   if(!val_same_flag)
   {
      uns32 am=0;

      if(i_set_req->i_param_val.i_param_id == ncsIfsvIfIpIsV4Unnmbrd_ID)
      {
         am |= NCS_IPXS_IPAM_UNNMBD;

         if(ifip_info->is_v4_unnmbrd == NCS_SNMP_TRUE)
         {
            ipxs_ifd_proc_v4_unnmbrd(cb, ifip_info, ifip_info->is_v4_unnmbrd);
         }

      }

      /* Keep adding attrubutes here */
      
      rc = ipxs_ifd_ifip_info_bcast(cb, ifip_info, am);
   }
   
   ncshm_give_hdl(cb->cb_hdl);
   return rc;
}


uns32 ncsifsvifipentry_extract (NCSMIB_PARAM_VAL* param, NCSMIB_VAR_INFO* var_info,
                   NCSCONTEXT data, NCSCONTEXT buffer)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   IPXS_IFIP_INFO *ifip_info;
/*   NCS_IP_ADDR addrNet[255]; */
   uns16  len=0,i=0;
   uns32  ipaddr_count =0;
   uns32  count =0;
   
   memset(&addr, 0, sizeof(addr));

   switch(param->i_param_id)
   {
   case ncsIfsvIfIpAddr_ID:
     ifip_info = (IPXS_IFIP_INFO *) data;
     if(!ifip_info)
        return NCSCC_RC_FAILURE;

     if(!ifip_info->ipaddr_cnt)
     {
         len = 0;
     }
     else
     {
         for(i = 0; i < ifip_info->ipaddr_cnt; i++)
         {
/*           m_NCS_HTON_IP_ADDR(&ifip_info->ipaddr_list[i].ipaddr.ipaddr, &addrNet[i]); 
           addr[i]=addrNet[i].info.v4; */
           addr[i]=ifip_info->ipaddr_list[i].ipaddr.ipaddr.info.v4; 
         }
         if(ifip_info->ipaddr_list[0].ipaddr.ipaddr.type == NCS_IP_ADDR_TYPE_IPV4)
            len = ifip_info->ipaddr_cnt*sizeof(NCS_IPV4_ADDR); /* Please not that this is not a magic no */
         else if(ifip_info->ipaddr_list[0].ipaddr.ipaddr.type == NCS_IP_ADDR_TYPE_IPV6)
            len = 16; /* Please not that this is not a magic no */
         else
            return NCSCC_RC_FAILURE;
      }

      param->i_fmat_id = NCSMIB_FMAT_OCT;                  
      param->i_length  = len;   
      param->info.i_oct = ((uns8*)&addr);
      break;
   case ncsIfsvIfIpAddrCnt_ID:
     ifip_info = (IPXS_IFIP_INFO *) data;
     if(!ifip_info)
        return NCSCC_RC_FAILURE;

     if(!ifip_info->ipaddr_cnt)
     {
         len = 0;
     }
     else
     {
         len = sizeof(ipaddr_count);
         count = ifip_info->ipaddr_cnt;
         ipaddr_count = htonl(count);
     }
      param->i_fmat_id = NCSMIB_FMAT_INT;                  
      param->info.i_int = ipaddr_count;
      break;

   default:
      rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
      break;
  }

   return rc;
}

uns32 ncsifsvifipentry_setrow (NCSCONTEXT cb, NCSMIB_ARG* args,
                                     NCSMIB_SETROW_PARAM_VAL* params,
                                     struct ncsmib_obj_info* obj_info,
                                     NCS_BOOL testrow_flag)
{
   return NCSCC_RC_FAILURE;
}

uns32 ncsifsvifipentry_verify_instance(NCSMIB_ARG* args)
{
   return NCSCC_RC_SUCCESS;
}
