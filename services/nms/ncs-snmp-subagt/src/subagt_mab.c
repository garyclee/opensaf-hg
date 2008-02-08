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

  .....................................................................  
  
  DESCRIPTION: This file describes the routines for the conversion of 
                AGENTX PDU into NCSMIB_ARG structure. 
                - snmpsubagt_mab_mib_param_fill
                    This routine Converts the Agentx PDU into NCSMIB_ARG.
                    
                snmpsubagt_mab_mac_msg_send
                    This routine sends the NCSMIB_ARG to MAB as a SYNC
                    request. 
                    
                snmpsubagt_mab_param_type_get
                    This routine maps the AGENTX parameter type to the 
                    MAB supported type. 
                    
                snmpsubagt_mab_index_extract
                    This routine extract the index from the AGENTX Varbind.
                    
                snmpsubagt_mab_oid_compose
                    This routine composes the AGENTX OID from the NCSMIB_ARG.

                smidump_mibarg_resp_process
                    This routine gives the address of the NCSMIB_ARG Param val
                    to the NET-SNMP Library. 
*****************************************************************************/
#include "subagt.h"
#include "subagt_mab.h"

/* Globals and static function 
used only in this file - only for Logging */ 
typedef struct snmpsa_log_tbl_info
{ 
    uns32   req_type; 
    uns32   table;
}SNMPSA_LOG_TBL_INFO; 


typedef struct snmpsa_log_param_info
{ 
   uns32 param_id; 
   uns32 fmt_id; 
   uns32 i_len; 
   uns32   status; 
}SNMPSA_LOG_PARAM_INFO; 

typedef struct snmpsa_log_octet_info
{ 
      uns8* octet_info; 
}SNMPSA_LOG_OCTETS; 

typedef struct snmpsa_log_int_info
{
       uns32 int_info; 
}SNMPSA_LOG_INT_INFO; 
 
static void snmpsa_log_ncsmib_arg(NCSMIB_ARG *mib_arg); 

static uns32
snmpsubagt_mab_getnext_column_process(NCSMIB_ARG        *io_mib_arg, 
                                      NCSMEM_AID        *ma, 
                                      uns8              *space, 
                                      uns32             space_size,
                                      uns32             i_time_val,
                                      uns32             i_tbl_id, 
                                      uns32             cur_column, 
                                      struct variable   *obj_details, 
                                      uns32             num_of_objs,
                                      NCS_BOOL          is_sparse_table); 

static uns32
snmpsubagt_mab_getnext_column(uns32             cur_column, 
                              struct variable   *obj_details, 
                              uns32             num_objs);

/******************************************************************************
 *  Name:          snmpsubagt_mab_mib_param_fill
 *  Description:   To compose the NCSMIB_ARG from AGENTX request 
 *                   
 *  Arguments:    NCSMIB_PARAM_VAL *io_param_val - Destination NCSMIB_ARG(i/o)
 *                 NCSMIB_PARAM_ID  - Parameter ID (i)
 *                 NCSMIB_FMAT_ID   - Parameter type (i) 
 *                 void *i_set_val  - Set parameter value (i)
 *                 uns32 i_set_val_len - Set parameter value length (i)
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32
snmpsubagt_mab_mib_param_fill(NCSMIB_PARAM_VAL  *io_param_val,
                            NCSMIB_PARAM_ID     i_param_id, 
                            uns32               i_param_type,
                            void                *i_set_val, 
                            uns32               i_set_val_len, 
                            uns8                *counter64_in_octs)
{
    uns32 status  = NCSCC_RC_SUCCESS;
    long long   llong = 0; 
    U64         *pCounter64 = NULL;  
    
    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_PARAM_FILL);

    if (io_param_val == NULL)
    {
        /* log the error */
        return NCSCC_RC_FAILURE; 
    }

    /* update the param id */
    io_param_val->i_param_id = i_param_id;

    /* update the format id */
    io_param_val->i_fmat_id  = snmpsubagt_mab_param_type_get(i_param_type);

    /* update the length */
    io_param_val->i_length = i_set_val_len;

    /* update the value */
    switch(io_param_val->i_fmat_id)
    {
        /* add all the other formats like OID, COUNTER64 -- TBD Mahesh */
        case NCSMIB_FMAT_INT:
            io_param_val->info.i_int = *((uns32*)(i_set_val));
            break;

        case NCSMIB_FMAT_OCT:
            if (i_param_type == ASN_COUNTER64)
            {
                /* convert the given long long into octet string */
#if 0
                printU64(counter64_in_octs, (U64*)(i_set_val)); /* net-snmp api, int64.h */
#endif
                pCounter64 = (U64*)(i_set_val);
                llong = pCounter64->high;
                llong = ((llong << 32)|(pCounter64->low));
                m_NCS_OS_HTONLL_P(counter64_in_octs, llong); 
                io_param_val->info.i_oct =  counter64_in_octs; 
                io_param_val->i_length = 8;
            }
            else if (i_param_type == ASN_OBJECT_ID)
            {
                /* take a temporary array, into which we can buffer the encoded OIDs */ 
                uns8    oid_in_nw_order[128*4]; 
                uns8    *temp_oid = oid_in_nw_order;
                uns32   *temp_oid_str=(uns32*)i_set_val;
                uns8    *temp_oid_in_nw_order;
                uns32   i; 

                temp_oid_in_nw_order = malloc(i_set_val_len); 
                if (temp_oid_in_nw_order == NULL)
                    return NCSCC_RC_OUT_OF_MEM;

                m_NCS_MEMSET(oid_in_nw_order, 0, sizeof(oid_in_nw_order)); 
                memset(temp_oid_in_nw_order, 0, i_set_val_len); 

                /* encode the data in network order */ 
                for (i=0; i<(i_set_val_len/4); i++)
                    ncs_encode_32bit(&temp_oid, temp_oid_str[i]);

                memcpy(temp_oid_in_nw_order, oid_in_nw_order, i_set_val_len);     
                io_param_val->info.i_oct = temp_oid_in_nw_order;
            }
            else
            {
                 io_param_val->info.i_oct = (uns8*)(i_set_val);
            }
            break;

        default:
           status = NCSCC_RC_FAILURE;
           break;
    }
    
    return status;
}

/******************************************************************************
 *  Name:          snmpsubagt_mab_mac_msg_send
 *
 *  Description:   To Send the message to the MAB as a SYNC request.  
 * 
 *  Arguments:     NCSMIB_ARG   *io_mib_arg - NCSMIB_ARG to be sent to MAB
 *                 uns32        i_table_id  - Table-ID
 *                 uns32        *i_inst     - Instance details
 *                 uns32        i_inst_len  - Instance Length
 *                 uns32        i_param_id  - Paramter ID
 *                 uns32        i_param_type - Type of the parameter 
 *                 uns32        i_req_type  - Type of the request
 *                 void         *i_set_val  - Set Value
 *                 uns32        i_set_val_len - Set value length
 *                 uns32        i_time_val  - Time to wait for the response
 *                NCSMEM_AID    *ma - to strore the response details
 *                 struct       *variable   - Object properties in this table
 *                                            from MAB 
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
/* Function to send the message to MAC */
uns32
snmpsubagt_mab_mac_msg_send(NCSMIB_ARG   *io_mib_arg,
                           uns32        i_table_id,
                           uns32        *i_inst,
                           uns32        i_inst_len,
                           uns32        i_param_id, 
                           uns32        i_param_type,
                           uns32        i_req_type,
                           void         *i_set_val, 
                           uns32        i_set_val_len,
                           uns32        i_time_val,
                           NCSMEM_AID   *ma, 
                           uns8         *space, 
                           uns32        space_size,
                           struct       variable *obj_details, 
                           uns32        obj_count,
                           NCS_BOOL     is_sparse_table)
{
    uns32       status = NCSCC_RC_SUCCESS;
    uns8        counter64_in_octs[I64CHARSZ+1] = {0};  /* I64CHARSZ, defined in net-snmp package */
#if 0
    NCSSA_CB    *cb = m_SNMPSUBAGT_CB_GET;
    SaAisErrorT saf_status = SA_AIS_OK;
#endif
    uns8        *temp_oid_ptr = NULL;

    /* Log the Function Entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_MAB_MAC_MSG_SEND);

    /* validate the inputs */
    if (io_mib_arg == NULL)
    {
        return NCSCC_RC_FAILURE; 
    }

    /* set the NCSMIB_ARG */
    m_NCS_MEMSET(io_mib_arg, 0, sizeof(NCSMIB_ARG));
    ncsmib_init(io_mib_arg);

    /* Fill in the NCSMIB_ARG */
    io_mib_arg->i_op = i_req_type;
    io_mib_arg->i_tbl_id = i_table_id;
    io_mib_arg->i_rsp_fnc = NULL;
    io_mib_arg->i_idx.i_inst_ids = i_inst;
    io_mib_arg->i_idx.i_inst_len = i_inst_len;

    /* set the param details based on the type of the request */
    switch(i_req_type)
    {
        case NCSMIB_OP_REQ_GET:
               io_mib_arg->req.info.get_req.i_param_id = i_param_id;
               break;
        case NCSMIB_OP_REQ_NEXT:
               io_mib_arg->req.info.next_req.i_param_id = i_param_id;
               break;
        case NCSMIB_OP_REQ_SET:
        case NCSMIB_OP_REQ_TEST:
               /* update the param */
               status = snmpsubagt_mab_mib_param_fill(
                       &io_mib_arg->req.info.set_req.i_param_val,
                       i_param_id,
                       i_param_type,
                       i_set_val, i_set_val_len,counter64_in_octs);
               if (status != NCSCC_RC_SUCCESS)
                   return status; 
               if (i_param_type == ASN_OBJECT_ID)
               {
                  /* set the address of the temporary pointer to be freed */
                  temp_oid_ptr = (uns8 *)io_mib_arg->req.info.set_req.i_param_val.info.i_oct;
               }
               break;
        default:
            return NCSCC_RC_FAILURE;
    }

    /* send the request to MAB */
    /* Fill in the Key  */
    io_mib_arg->i_mib_key = m_SUBAGT_MAC_HDL_GET; 
    io_mib_arg->i_usr_key = m_SUBAGT_MAC_HDL_GET; 

   #if 0 
    if ((is_sparse_table == TRUE) && (i_req_type == NCSMIB_OP_REQ_NEXT))
    {
        /* Disable the health monitoring */
        saf_status = saAmfHealthcheckStop(cb->amfHandle,
                                    &cb->compName,
                                    &cb->healthCheckKey);
        if (saf_status != SA_AIS_OK)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_HLTH_CHK_STOP_FAILED,
                                    saf_status, 0, 0);
        }
        else
        {
           cb->healthCheckStarted = FALSE;

           /* log that health check started */
           m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_HLTH_CHECK_STOPPED);
        }
    }
   #endif

    /* LOG MIBARG Data */ 
    snmpsa_log_ncsmib_arg(io_mib_arg); 
    status = ncsmib_sync_request(io_mib_arg, 
                                ncsmac_mib_request, 
                                i_time_val, ma);
    /* after processing the request, free the memory */
    if (temp_oid_ptr != NULL)
    {
       free(temp_oid_ptr);
       temp_oid_ptr = NULL;
    }
    
    /* LOG MIBARG Data after receiving response. */ 
    snmpsa_log_ncsmib_arg(io_mib_arg); 

    if ((status == NCSCC_RC_SUCCESS) &&
        (i_req_type == NCSMIB_OP_REQ_NEXT) && 
        ((io_mib_arg->rsp.i_status == NCSCC_RC_NO_INSTANCE) || (io_mib_arg->rsp.i_status == NCSCC_RC_FAILURE)))
    {
         status = snmpsubagt_mab_getnext_column_process(io_mib_arg, ma, space, space_size, i_time_val, 
                                                i_table_id, i_param_id, obj_details,obj_count,is_sparse_table);
    }
   
   #if 0 
    if ((is_sparse_table == TRUE) && (i_req_type == NCSMIB_OP_REQ_NEXT))
    {
        /* start the healthcheck */
        saf_status = saAmfHealthcheckStart(cb->amfHandle,
                                      &cb->compName,
                                      &cb->healthCheckKey,
                                      cb->invocationType,
                                      cb->recommendedRecovery);
        if (saf_status != SA_AIS_OK)
        {
            /* log the error */
            m_SNMPSUBAGT_ERROR_LOG(SNMPSUBAGT_AMF_HLTH_CHK_STRT_FAILED,
                                   cb->amfHandle,saf_status, 0);
        }
        else
        {
           cb->healthCheckStarted = TRUE;

           /* log that health check started */
           m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_AMF_HLTH_CHECK_STARTED);
        }
    }
   #endif
    return status;
}

/******************************************************************************
 *  Name:          snmpsubagt_mab_param_type_get
 *
 *  Description:   This routine converts Agentx supported data types to the
 *                  MAB supported data type. 
 * 
 *  Arguments:     i_param_type     - Agentx Supported type
 *
 *  Returns:       NCSMIB_FMAT_ID   - if everything is OK, appropriate
 *                                      type is returned. 
 *                                    if there is no match found, 
                                        0 is returned
 *  NOTE: 
 *****************************************************************************/
NCSMIB_FMAT_ID
snmpsubagt_mab_param_type_get(uns32 i_param_type)
{
    NCSMIB_FMAT_ID   mab_param_type;

    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_PARAM_TYPE_GET);

    /* get the appropriate MAB supported type */
    switch(i_param_type)
    {
        case ASN_BOOLEAN:
        /* Universal Integer based types */
        case ASN_INTEGER:
        /* Application Integer based types */
        case ASN_COUNTER:
        case ASN_GAUGE:
        /*case ASN_UNSIGNED: */ /* same as of GAUGE */
        case ASN_TIMETICKS:
            mab_param_type = NCSMIB_FMAT_INT;
            break;
        /* Universal types */
        case ASN_OCTET_STR:
        case ASN_BIT_STR:
        case ASN_OBJECT_ID:
        /* Application types */
        case ASN_IPADDRESS:
        case ASN_OPAQUE:
            mab_param_type = NCSMIB_FMAT_OCT;
            break;

        case ASN_COUNTER64:
            mab_param_type = NCSMIB_FMAT_OCT;
            break;
        
        default:
            /* Log that, this type is not supported in the MAB */
                mab_param_type = 0;
                break;
    }
    return mab_param_type;
}

/******************************************************************************
 *  Name:          snmpsubagt_mab_index_extract
 *
 *  Description:   To extract the index from the OID 
 * 
 *  Arguments:     oid    *i_name   - input OID from the Agentx request
 *                 uns32  i_name_len - input OID length 
 *                 uns32  base_oid_len - base oid length 
 *                 uns32  *o_instance  - extracted instance will be stored here
 *                 uns32  *o_instance_len - instance length 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *
 *  NOTE: This API is used in case of old_api helper of NET-SNMP only. 
 *****************************************************************************/
uns32
snmpsubagt_mab_index_extract(oid    *i_name,
                            uns32   i_name_len,
                            oid     *base_oid,
                            uns32   base_oid_len,
                            uns32   *o_instance,
                            uns32   *o_instance_len)
{
    int32 i=0;

    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_INDEX_EXTRACT);

    /* do the inut and output validations */
    if ((i_name == NULL) ||
        (i_name_len == 0) ||
        (base_oid == NULL) ||
        (base_oid_len == 0) ||
        (o_instance == NULL) ||
        (o_instance_len == 0))
    {
        return NCSCC_RC_FAILURE; 
    }

    /* compare whether the base-oid is matching in the 
     * user given name(oid)
     */
    if (base_oid_len > i_name_len)
    {
        /* there will be nothing to extract */
        return NCSCC_RC_SUCCESS; 
    }

    for (i=0; i<base_oid_len; i++)
    {
        if (i_name[i] != base_oid[i])
        {
            *o_instance = 0; 
            *o_instance_len = 0; 
            return NCSCC_RC_SUCCESS; 
        }
    }

    /* initialize the instance and length to 0 */
    *o_instance = 0; 
    *o_instance_len = 0; 

    /* get rid of the base oid, extract the instance from rmaining oid */
    for (i=0; i < (int32)(i_name_len - (base_oid_len + 1)); i++)
    {
        /* extract the instance from oid; skip the column id */
        o_instance[i] = i_name[base_oid_len+i+1];
    }
    
    /* set the instance length */
    *o_instance_len = (uns32)i;

    return NCSCC_RC_SUCCESS;
}

/******************************************************************************
 *  Name:          snmpsubagt_mab_oid_compose
 *
 *  Description:   To compose the new OID to be sent back to the NetSNMP 
 *                  library, as part of servicing the GETNEXT request
 * 
 *  Arguments:     oid             *io_oid - OID to be sent to NETSNMP
                   size_t         *io_oid_length - OID length 
                   uns32         i_base_oid_len - Base OID Length 
                   NCSMIB_NEXT_RSP    *i_next_rsp - GET Next Response 
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  NOTE: 
 *****************************************************************************/
uns32    
snmpsubagt_mab_oid_compose(oid             *io_oid,
                        size_t             *io_oid_length, 
                        oid                *i_base_oid, 
                        uns32             i_base_oid_len, 
                        NCSMIB_NEXT_RSP    *i_next_rsp)
{
    uns32    i = 0; 
    uns32   new_inst_len = 0;
    uns32   inst_len = 0;

    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_OID_COMPOSE); 

    /* input validation */
    if ((io_oid == NULL) ||
        (io_oid_length == NULL) ||
        (i_next_rsp == NULL))
    {
        return NCSCC_RC_FAILURE; 
    }

    if (i_next_rsp->i_next.i_inst_len == 0)
    {
        inst_len = 1; 
    }
    else
    {
        inst_len = i_next_rsp->i_next.i_inst_len; 
    }

    /* Get the new instance length */
    new_inst_len = (i_base_oid_len + 1 + inst_len);

    /* for re-allocation for the new index */
    if (new_inst_len > MAX_OID_LEN)    
    {
        /* if the user given OID's length is less than MAX_OID_LEN, io_oid is an array.  
         * No reallocation can be done in this case. 
         * For more information on this, please refer to the snmp_set_var_objid() definition 
         * in snmp_api.c file
         */ 
        /* log a headline for this specific case */
        return NCSCC_RC_OUT_OF_MEM; 
    }    

    /* modify the ioutput length if it is not same */
    *io_oid_length = new_inst_len;

    m_NCS_MEMSET(io_oid, 0, (*io_oid_length)*(sizeof(oid)));
    
    /* we have enough memory, compose the new instance */
    m_NCS_OS_MEMCPY(io_oid, i_base_oid, i_base_oid_len*sizeof(oid));
    
    /* 1. Move to update the column id */
    io_oid = io_oid+i_base_oid_len/*+1*/;
    
    /* 2. Append the Column id */
    *io_oid = (oid)i_next_rsp->i_param_val.i_param_id;
    io_oid++;

    /* 3. update the instance */
    *io_oid = 0; 
    for(i = 0; i < i_next_rsp->i_next.i_inst_len; i++)
    {
        *io_oid = (oid)(*(i_next_rsp->i_next.i_inst_ids));
        io_oid++; 
        i_next_rsp->i_next.i_inst_ids++;
    }
    /* Everything went fine */
    return NCSCC_RC_SUCCESS; 
}

/******************************************************************************
 *  Name:          snmpsubagt_mab_mibarg_resp_process
 *
 *  Description:   To give the address of the data to be populated into 
 *                  the Agentx response
 * 
 *  Arguments:     NCSMIB_PARAM_VAL - address of the param info to be returned
 *                                      to the NETSNMP Library
 *                 io_var_len       - Length of the data  
 *
 *  Returns:       NCSMIB_PARAM_VAL - Address of the param val info 
 *                  NULL            - Something went wrong
 *  NOTE: 
 *****************************************************************************/
unsigned char*
snmpsubagt_mab_mibarg_resp_process(NCSMIB_PARAM_VAL    *i_rsp_param_val, 
                            size_t              *io_var_len, 
                            uns32               i_param_type)
{
    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_RESP_PROCESS);

    /* validate the input parameters */ 
    if ((i_rsp_param_val == NULL) ||
        (io_var_len == NULL))
    {
        return NULL;
    }
    
    /* fill in the response, based on the type of Object */
    switch(i_rsp_param_val->i_fmat_id)
    {
        /* Extend the other format ids -- TBD Mahesh */
        /* return the address of a global variable, danger */
        case NCSMIB_FMAT_INT:
        case NCSMIB_FMAT_BOOL:
            if (i_rsp_param_val->i_length == 0)
            {
                /* All the applications may not fill this value
                 * If this value is not filled, NET-SNMP library
                 * does not know how many bytes to read 
                 */    
                *io_var_len = 4; 
            }
            else
            {
                *io_var_len = i_rsp_param_val->i_length;
            }
            return (unsigned char *) &(i_rsp_param_val->info.i_int);
            
        case NCSMIB_FMAT_OCT:
            if (i_param_type == ASN_COUNTER64)
            {
                long long llong = 0;
                *io_var_len = sizeof(g_counter64);
                
                /* convert the data into 64 bit value */
                m_NCS_MEMSET(&g_counter64, 0, sizeof(g_counter64));
                llong = m_NCS_OS_NTOHLL_P(i_rsp_param_val->info.i_oct);
                g_counter64.low = (u_long)(llong)&(0xFFFFFFFF);
                g_counter64.high = (u_long)(llong>>32)&(0xFFFFFFFF);
#if 0
                m_NCS_MEMCPY(octs, i_rsp_param_val->info.i_oct, 8);
                if (read64(&g_counter64, octs) != 1)
                {
                    /* log the error */
                    m_NCS_MEMSET(&g_counter64, 0, sizeof(g_counter64));
                    return NULL;
                }
#endif
                return(unsigned char *)(&g_counter64);
            }
            else if (i_param_type == ASN_OBJECT_ID)
            {
                uns8    *temp; 
                uns32   *p_int; 
                uns32   i; 

                temp = (uns8 *)i_rsp_param_val->info.i_oct; 
                p_int = (uns32*)i_rsp_param_val->info.i_oct; 

                /* put in the host order */ 
                for (i = 0; i<(i_rsp_param_val->i_length/4); i++)
                    p_int[i] = ncs_decode_32bit(&temp);

                *io_var_len = i_rsp_param_val->i_length;
                return (unsigned char *) (i_rsp_param_val->info.i_oct);
            }
            else
            {
                *io_var_len = i_rsp_param_val->i_length;
                return (unsigned char *) (i_rsp_param_val->info.i_oct);
            }
            break;
            
        default:
        break;
    }
    return (unsigned char *)NULL;
}


/******************************************************************************
 *  Name:           snmpsa_log_ncsmibarg 
 *
 *  Description:    Helper function to LOG the NCS MIB ARG data  
 *                 
 * 
 *  Arguments:     NCSMIB_ARG 
 *                 
 *
 *  Returns:       
 *                  NULL      
 *  NOTE: 
 *****************************************************************************/

static void snmpsa_log_ncsmib_arg (NCSMIB_ARG* io_mib_arg)
{
        NCSFL_MEM              log_mem_dump; 
        SNMPSA_LOG_TBL_INFO    tbl_info; 
        SNMPSA_LOG_PARAM_INFO  param_info; 
        SNMPSA_LOG_OCTETS      oct_info; 
        SNMPSA_LOG_INT_INFO    int_info;
        NCSMIB_PARAM_VAL       l_param_val;
        

        m_NCS_MEMSET(&tbl_info, 0, sizeof(SNMPSA_LOG_TBL_INFO));  
        m_NCS_MEMSET(&param_info, 0, sizeof(SNMPSA_LOG_PARAM_INFO)); 
        m_NCS_MEMSET(&log_mem_dump,   0, sizeof(NCSFL_MEM));
        m_NCS_MEMSET(&oct_info,       0, sizeof(SNMPSA_LOG_OCTETS));
        m_NCS_MEMSET(&int_info,       0, sizeof(SNMPSA_LOG_INT_INFO));
        m_NCS_MEMSET(&l_param_val,    0, sizeof(NCSMIB_PARAM_VAL)); 

        /* LOG Table and Object Data */ 
        tbl_info.req_type = io_mib_arg->i_op; 
        tbl_info.table    = io_mib_arg->i_tbl_id; 
        log_mem_dump.len  = sizeof(SNMPSA_LOG_TBL_INFO); 
        log_mem_dump.dump = log_mem_dump.addr = (char *)&tbl_info;   
        m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_TBL_DUMP, log_mem_dump);
  
        /* LOG Instance Ids */ 
        if (io_mib_arg->i_idx.i_inst_len != 0) 
        {
            m_NCS_MEMSET(&log_mem_dump, 0, sizeof(log_mem_dump));  
            if (io_mib_arg->i_op == NCSMIB_OP_RSP_NEXT)
            {
               log_mem_dump.len = (io_mib_arg->rsp.info.next_rsp.i_next.i_inst_len)*sizeof(uns32);  
               log_mem_dump.dump = log_mem_dump.addr = (char *)io_mib_arg->rsp.info.next_rsp.i_next.i_inst_ids;  
            }
            else
            {
               log_mem_dump.len = (io_mib_arg->i_idx.i_inst_len)*sizeof(uns32);  
               log_mem_dump.dump = log_mem_dump.addr = (char *)io_mib_arg->i_idx.i_inst_ids;  
            }
            m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_INSTID_DUMP, log_mem_dump);
        }

        /* LOG parameter id's */ 
       m_NCS_MEMSET(&log_mem_dump, 0, sizeof(log_mem_dump));
       switch(io_mib_arg->i_op)
       {
          case  NCSMIB_OP_REQ_GET:
                param_info.param_id = (uns32)io_mib_arg->req.info.get_req.i_param_id;
          break; 

          case NCSMIB_OP_RSP_GET:
                param_info.param_id = (uns32)io_mib_arg->rsp.info.get_rsp.i_param_val.i_param_id;
                param_info.fmt_id =(uns32) io_mib_arg->rsp.info.get_rsp.i_param_val.i_fmat_id;
                param_info.i_len = (uns32)io_mib_arg->rsp.info.get_rsp.i_param_val.i_length;  
                m_NCS_MEMCPY(&l_param_val, &io_mib_arg->rsp.info.get_rsp.i_param_val, sizeof(NCSMIB_PARAM_VAL));
                param_info.status   = io_mib_arg->rsp.i_status; 
          break; 

          case  NCSMIB_OP_REQ_NEXT:
                param_info.param_id = (uns32)io_mib_arg->req.info.next_req.i_param_id;
          break; 
       
          case NCSMIB_OP_RSP_NEXT:
                param_info.param_id = (uns32)io_mib_arg->rsp.info.next_rsp.i_param_val.i_param_id;
                param_info.fmt_id =(uns32) io_mib_arg->rsp.info.next_rsp.i_param_val.i_fmat_id;
                param_info.i_len = (uns32)io_mib_arg->rsp.info.next_rsp.i_param_val.i_length;  
                m_NCS_MEMCPY(&l_param_val, &io_mib_arg->rsp.info.next_rsp.i_param_val, sizeof(NCSMIB_PARAM_VAL));
                param_info.status   = io_mib_arg->rsp.i_status; 
          break; 
 
          case  NCSMIB_OP_REQ_SET:
          case  NCSMIB_OP_REQ_TEST:
                param_info.param_id = (uns32)io_mib_arg->req.info.set_req.i_param_val.i_param_id;
                param_info.fmt_id =(uns32) io_mib_arg->req.info.set_req.i_param_val.i_fmat_id;
                param_info.i_len = (uns32)io_mib_arg->req.info.set_req.i_param_val.i_length;  
                m_NCS_MEMCPY(&l_param_val, &io_mib_arg->req.info.set_req.i_param_val, sizeof(NCSMIB_PARAM_VAL));
          break;
          
          case  NCSMIB_OP_RSP_SET:
          case  NCSMIB_OP_RSP_TEST:
                param_info.param_id = (uns32)io_mib_arg->rsp.info.set_rsp.i_param_val.i_param_id;
                param_info.fmt_id =(uns32) io_mib_arg->rsp.info.set_rsp.i_param_val.i_fmat_id;
                param_info.i_len = (uns32)io_mib_arg->rsp.info.set_rsp.i_param_val.i_length;  
                m_NCS_MEMCPY(&l_param_val, &io_mib_arg->rsp.info.set_rsp.i_param_val, sizeof(NCSMIB_PARAM_VAL));
                param_info.status   = io_mib_arg->rsp.i_status; 
          break;
  
  
          default:
                 param_info.param_id = 0xFFFF; /* strange value indicating error */ 
          break; 
      }

        log_mem_dump.len = sizeof(SNMPSA_LOG_PARAM_INFO);   
        log_mem_dump.dump = log_mem_dump.addr = (char *)&param_info;  
        m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_PARAM_INFO_DUMP, log_mem_dump);

        /* Dump INT param value */ 

    if (l_param_val.i_fmat_id == NCSMIB_FMAT_INT)
      {
        int_info.int_info = (uns32)l_param_val.info.i_int; 
        log_mem_dump.len = sizeof(SNMPSA_LOG_INT_INFO);   
        log_mem_dump.dump = log_mem_dump.addr = (char *)&int_info.int_info;  
        m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_INT_INFO_DUMP, log_mem_dump);
      } 
        
    if (l_param_val.i_fmat_id == NCSMIB_FMAT_OCT)
    { 
       oct_info.octet_info = (uns8 *)l_param_val.info.i_oct; 
       log_mem_dump.len = (uns32)l_param_val.i_length;   
       log_mem_dump.dump = log_mem_dump.addr = (char *)oct_info.octet_info;  
       m_SNMPSUBAGT_MEMDUMP_LOG(SNMPSUBAGT_NCSMIB_ARG_OCT_INFO_DUMP, log_mem_dump);
    } 

    return; 
 
} 

/*****************************************************************************
 * Function:    snmpsubagt_given_oid_validate
 *
 * Parameters:
 *      *vp      (I)     Pointer to variable entry that points here.
 *      *name    (I/O)   Input name requested, output name found.
 *      *length  (I/O)   Length of input and output oid's.
 *       exact   (I)     TRUE if an exact match was requested.
 *      *var_len (O)     Length of variable or 0 if function returned.
 *    (**write_method)   Hook to name a write method (UNUSED).
 *      
 * Returns:
 *    NCSCC_RC_SUCCESS    If vp->name matches name (accounting for exact bit).
 *    NCSCC_RC_FAILURE    Otherwise,
 *
 *
 * Check whether variable (vp) matches name.
 * This function is borrowed from util_funcs.c from NET-SNMP distribution. 
 * This avoids including a private (util_funcs.h) file from the distribution. 
 * NOTE: This function is used only in case of old_api.c helpers. 
 ********************************************************************************/
uns32
snmpsubagt_given_oid_validate(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN];
    int             result;

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    newname[vp->namelen] = 0;
    result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
    if ((exact && (result != 0)) || (!exact && (result >= 0)))
        return (NCSCC_RC_FAILURE);
    memcpy((char *) name, (char *) newname,
           ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;

    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */
    return (NCSCC_RC_SUCCESS);
}

/*****************************************************************************
 * Function:    snmpsubagt_mab_error_code_map
 *
 * Parameters:
 *      mab_err_code    Error code as understood by the MAB 
 *      
 * Returns:
 *    error             Error code as understood by the NET-SNMP 
 *
 ********************************************************************************/
unsigned char
snmpsubagt_mab_error_code_map(uns32 mab_err_code, uns32 req_type)
{
    unsigned char error = SNMP_ERR_NOERROR;   

    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_ERROR_CODE_MAP);

   /* set the error code and return/continue on to the next varbind?? */
   switch (mab_err_code)
    {
       case NCSCC_RC_NO_OBJECT:
       case NCSCC_RC_NO_SUCH_TBL:
            if ((req_type == MODE_SET_RESERVE1)|| (req_type == MODE_SET_ACTION))
               error = SNMP_ERR_NOTWRITABLE;
            else
               error = SNMP_NOSUCHOBJECT;
       break;

       case NCSCC_RC_NO_INSTANCE:
       case NCSCC_RC_NOSUCHNAME:
            if ((req_type == MODE_SET_RESERVE1) || (req_type == MODE_SET_ACTION))
               error = SNMP_ERR_NOCREATION; 
            else /* get or get-next */
               error = SNMP_NOSUCHINSTANCE; 
       break; 

       case NCSCC_RC_NOT_WRITABLE:
            error = SNMP_ERR_NOTWRITABLE;  
       break;

       case NCSCC_RC_NO_CREATION:
            error = SNMP_ERR_NOCREATION;
       break;

       case NCSCC_RC_INCONSISTENT_NAME:
            error = SNMP_ERR_INCONSISTENTNAME;
       break;

       case NCSCC_RC_INV_VAL:
            error = SNMP_ERR_WRONGVALUE; 
       break; 
            
       case NCSCC_RC_INV_SPECIFIC_VAL:
            error = SNMP_ERR_INCONSISTENTVALUE;  
       break; 

       case NCSCC_RC_NO_ACCESS:
            error = SNMP_ERR_NOACCESS;  
       break; 

       case NCSCC_RC_REQ_TIMOUT:        
       case NCSCC_RC_OUT_OF_MEM:
       case NCSCC_RC_RESOURCE_UNAVAILABLE:
            if ((req_type == MODE_SET_RESERVE1) || (req_type == MODE_SET_ACTION))
               error = SNMP_ERR_RESOURCEUNAVAILABLE;
            else
               error = SNMP_ERR_GENERR;
       break;

       default:
            error = SNMP_ERR_GENERR;
       break;
    } /* end of switch */
    return error; 
}


/*****************************************************************************
 * Function:    snmpsubagt_mab_unregister_mib
 *              Unregisters a MIB with the Agent 
 * 
 *
 * Parameters: netsnmp_handler_registration *reginfo
 *              Contains the information about  the OID registered with the 
 *              Agent
 *      
 * Returns:
 * 
 * NOTE:        This routine will have to be deleted, once we migrate to 5.1.2. 
 *              In NET-SNMP-5.1.2 release, it is availble as 
 *              netsnmp_unregister_handler().  
 *
 ********************************************************************************/
int32   
snmpsubagt_mab_unregister_mib(netsnmp_handler_registration *reginfo)
{
    /* Log the function entry */
    m_SNMPSUBAGT_FUNC_ENTRY_LOG(SNMPSUBAGT_FUNC_ENTRY_UNREGISTER_MIB);

    return unregister_mib_context(reginfo->rootoid, reginfo->rootoid_len,
                                  reginfo->priority, reginfo->range_subid, 
                                  reginfo->range_ubound, reginfo->contextName);
}


/*******************************************************************************
  Function : snmpsubagt_mab_getnext_column_process 
  
  Purpose  :  To get the response for the next accessible column in the table
 
  Input    :  NCSMIB_ARG        - vehicle to carry the data
              NCSMEM_AID        - Aid to the vehicle
              space             -   - do -
              space_size        -   - do -
              i_time_val        -  Time to wait for the response from MASv
              NCSMIB_TBL_ID    - Table ID
              uns32 cur_column - Current column id
              obj_details       - Details of the columns in this table
              num_of_objs       - Total number of columns
              is_sparse_table   - Is this a sparse table?? 
    
  Returns  :  NCSCC_RC_SUCCESS  - Everything went fine       
              NCSCC_RC_FAILURE  - Something went wrong/send failed
*******************************************************************************/
static uns32
snmpsubagt_mab_getnext_column_process(NCSMIB_ARG        *io_mib_arg, 
                                      NCSMEM_AID        *ma, 
                                      uns8              *space, 
                                      uns32             space_size,
                                      uns32             i_time_val,
                                      uns32             i_tbl_id, 
                                      uns32             cur_column, 
                                      struct variable   *obj_details, 
                                      uns32             num_of_objs,
                                      NCS_BOOL          is_sparse_table)          
{
    uns32   next_column = 0;     
    uns32   still_columns = TRUE;
    uns32   ret_code = NCSCC_RC_FAILURE; 

    /* there are some more columns in this table */
    while (still_columns)
    {
        /* get the next column from the var_info */ 
        if ((next_column = snmpsubagt_mab_getnext_column(cur_column, 
                                          obj_details, num_of_objs)) == 0)
            break;
        /* clean up the vehicle */        
        m_NCS_MEMSET(io_mib_arg, 0, sizeof(NCSMIB_ARG));    
        ncsmib_init(io_mib_arg);
        
        /* clean up the temporary data required in getting the response to io_mib_arg */
        m_NCS_MEMSET(space, 0, space_size);
        ncsmem_aid_init(ma, space, space_size);
        
        /* ask the required */
        io_mib_arg->i_op = NCSMIB_OP_REQ_NEXT;
        io_mib_arg->i_tbl_id = i_tbl_id; 
        io_mib_arg->i_mib_key = m_SUBAGT_MAC_HDL_GET; 
        io_mib_arg->i_usr_key = m_SUBAGT_MAC_HDL_GET; 
        io_mib_arg->req.info.next_req.i_param_id = next_column; 
        io_mib_arg->i_idx.i_inst_len = 0; 
        io_mib_arg->i_idx.i_inst_ids = NULL; 

        m_SNMPSUBAGT_HEADLINE_LOG(SNMPSUBAGT_JUMP_TO_NEXT_COLUMN);

        /* LOG MIBARG Response before sending request. */ 
        snmpsa_log_ncsmib_arg(io_mib_arg); 

        /* send the getnext request on the new column to get the first instance */ 
        ret_code = ncsmib_sync_request(io_mib_arg, 
                                ncsmac_mib_request, 
                                i_time_val, ma);
        
        /* LOG MIBARG Response after receiving response. */ 
        snmpsa_log_ncsmib_arg(io_mib_arg); 

        /* if the response from application is success, then break */
        if ((ret_code != NCSCC_RC_SUCCESS) ||
            (io_mib_arg->rsp.i_status == NCSCC_RC_SUCCESS))

            break;
         
        /* if the response is no such instance and this is not a sparse table */ 
        /* return the no such instance/next column to zero */
        if ((io_mib_arg->rsp.i_status != NCSCC_RC_SUCCESS) && (is_sparse_table == FALSE))
        {
            next_column = 0;
            break;
        }
        else if ((io_mib_arg->rsp.i_status != NCSCC_RC_SUCCESS) && (is_sparse_table == TRUE))   
        {
            /* make this as current column and send the GETNEXT request */
            cur_column = next_column; 
        }
     }

     return ret_code; 
}

/****************************************************************************
  Function : snmpsubagt_mab_getnext_column
  
  Purpose  :  To get the next accessible column from varinfo
 
  Input    :  uns32 cur_column - Current column id
              stuct variable *  - Information about the objects in the table
              num_objectis - Number of objects/columns in the table 
    
  Returns  :  next column id for which the data is given
              0 - if there is no next column/no such instance 
 
  Notes    :  
****************************************************************************/
static uns32
snmpsubagt_mab_getnext_column(uns32             cur_column, 
                              struct variable   *obj_details, 
                              uns32             num_objs)
{
    uns32   next_column = 0; 
    uns32   i = 0;     

    for (i = 0; i <num_objs; i++)
    {
        if (obj_details[i].magic > cur_column)
        {
            /* check for the access permissions */
            if ((obj_details[i].acl == RONLY) || (obj_details[i].acl == RWRITE))
#if 0
                /* Can not get these values from the SMIDUMP tool */
                /* MIBLIB shall do this check */
               ((obj_details[i].status == NCSMIB_OBJ_CURRENT) ||
                (obj_details[i].status == NCSMIB_OBJ_DEPRECATE)))
#endif
            {
                next_column = obj_details[i].magic;
                break;
            }
        }
    }
    return next_column; 
}


