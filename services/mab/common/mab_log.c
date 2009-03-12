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


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:    log_mab_headline()
                                        log_mab_svc_prvdr()
                                        log_mab_lock()
                                        log_mab_memfail()
                                        log_mab_evt()
                                        log_mab_api()
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#if (NCS_MAB == 1)

#include "mab.h"

static int32 gl_mab_dts_bind_cntr;

/* log the OAA address and the filter information */ 
static uns32 
log_mab_oaa_and_fltr_data(uns8      sev,
                        uns32       env_id, 
                        uns32       table_id, 
                        uns8        exist,
                        MAS_FLTR    *fltr); 
/*****************************************************************************

  mab_log_bind

  DESCRIPTION: Function is used for binding with DTS.

*****************************************************************************/

uns32 mab_log_bind()
{
    if (gl_mab_dts_bind_cntr == 0)
    {
        NCS_DTSV_RQ        reg;
        memset(&reg, 0, sizeof(NCS_DTSV_RQ)); 

        reg.i_op = NCS_DTSV_OP_BIND;
        reg.info.bind_svc.svc_id = NCS_SERVICE_ID_MAB;
        /* fill version no. */
        reg.info.bind_svc.version = MASV_LOG_VERSION;
        /* fill svc_name */
        strcpy(reg.info.bind_svc.svc_name, "MAB");

        if (ncs_dtsv_su_req(&reg) == NCSCC_RC_FAILURE)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }
    gl_mab_dts_bind_cntr++; 
    return NCSCC_RC_SUCCESS;
}

/*****************************************************************************

  mab_log_unbind

  DESCRIPTION: Function is used for unbinding with DTS.

*****************************************************************************/

uns32 mab_log_unbind()
{
    gl_mab_dts_bind_cntr --; 

    if (gl_mab_dts_bind_cntr == 0)
    {
        NCS_DTSV_RQ        dereg;

        memset(&dereg, 0, sizeof(NCS_DTSV_RQ)); 
        
        dereg.i_op = NCS_DTSV_OP_UNBIND;
        dereg.info.unbind_svc.svc_id = NCS_SERVICE_ID_MAB;
        if (ncs_dtsv_su_req(&dereg) == NCSCC_RC_FAILURE)
            return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
    }

    return NCSCC_RC_SUCCESS;
}
#if (MAB_LOG == 1)
uns32 log_mab_svc_prvdr_evt(uns8 sev, uns32 id, uns32 svc_id, 
                            MDS_DEST addr, uns32 new_state, uns32 anchor)
{
    char addr_str[255] = {0}; 
    
    /* convert the MDS_DEST into a string */ 
    if (m_NCS_NODE_ID_FROM_MDS_DEST(addr) == 0)
        sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(addr));
    else
        sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%llu",
                m_NCS_NODE_ID_FROM_MDS_DEST(addr), 0, (addr)); 

    /* now log the message */ 
    return ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_SVC_PRVDR_EVT, 
                      MAB_FC_SVC_PRVDR_FLEX, NCSFL_LC_SVC_PRVDR, sev, 
                      NCSFL_TYPE_TICLLL, id, addr_str, svc_id, new_state, anchor);
}

uns32 log_mab_svc_prvdr_msg(uns8 sev, uns32 id, uns32 svc_id, 
                            MDS_DEST addr, uns32 msg_type)
{
    char addr_str[255] = {0}; 
    
    /* convert the MDS_DEST into a string */ 
    if (m_NCS_NODE_ID_FROM_MDS_DEST(addr) == 0)
        sprintf(addr_str, "VDEST:%d",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(addr));
    else
        sprintf(addr_str, "ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%llu",
                m_NCS_NODE_ID_FROM_MDS_DEST(addr), 0, (addr)); 

    /* now log the message */ 
    return ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_SVC_PRVDR_MSG, 
                      MAB_FC_SVC_PRVDR_FLEX, NCSFL_LC_SVC_PRVDR, sev, 
                      NCSFL_TYPE_TICLL, id, addr_str, svc_id, msg_type);
}

uns32 log_mab_fltr_data(uns8 sev, uns32 env_id, uns32 tbl_id, 
                        uns32 fltr_id, NCSMAB_FLTR *fltr_data) 
{
    NCSFL_MEM   index; 

    if (fltr_data == NULL)
        return NCSCC_RC_SUCCESS; 

     switch(fltr_data->type)
     {
         case NCSMAB_FLTR_RANGE:
          ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_RANGE, 
                           MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                           "TILLLLL", MAB_MAS_RCV_RNG_FLTR_REG_REQ, 
                           env_id, tbl_id, fltr_id, 
                           fltr_data->fltr.range.i_idx_len,  
                           fltr_data->fltr.range.i_bgn_idx); 
          
          memset(&index, 0, sizeof(NCSFL_MEM)); 
          index.len = (uns16)(fltr_data->fltr.range.i_idx_len * sizeof(fltr_data->fltr.range.i_idx_len)); 

          index.addr = index.dump = (char*)fltr_data->fltr.range.i_min_idx_fltr; 
          ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_RANGE_INDEX, 
                     MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                     "TID",MAB_MAS_RCV_RNG_FLTR_MIN_IDX, index);                 

          index.addr = index.dump = (char*)fltr_data->fltr.range.i_max_idx_fltr; 
          ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_RANGE_INDEX, 
                     MAB_FC_TBL_DET, NCSFL_LC_DATA,sev,
                     "TID",MAB_MAS_RCV_RNG_FLTR_MAX_IDX, index);                 
         break; 
         case NCSMAB_FLTR_EXACT:
          ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_EXACT, 
                           MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                           "TILLLLL", MAB_MAS_RCV_EXACT_FLTR_REG_REQ, 
                           env_id, tbl_id, fltr_id, 
                           fltr_data->fltr.exact.i_idx_len,  
                           fltr_data->fltr.exact.i_bgn_idx); 
          
          memset(&index, 0, sizeof(NCSFL_MEM)); 
          index.len = (uns16)(fltr_data->fltr.exact.i_idx_len * sizeof(fltr_data->fltr.exact.i_idx_len)); 

          index.addr = index.dump = (char*)fltr_data->fltr.exact.i_exact_idx; 
          ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_EXACT_INDEX, 
                     MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                     "TID",MAB_MAS_RCV_EXACT_FLTR_IDX, index);                 
         break; 
         case NCSMAB_FLTR_ANY:
         return ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_ANY, 
                           MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                           NCSFL_TYPE_TILLL, MAB_MAS_RCV_ANY_FLTR_REG_REQ, 
                           env_id, tbl_id, fltr_id); 
         break; 
         case NCSMAB_FLTR_SAME_AS:
         return ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_SA, 
                           MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                           NCSFL_TYPE_TILLLL, MAB_MAS_RCV_SA_FLTR_REG_REQ, 
                           env_id, tbl_id, fltr_id,
                           fltr_data->fltr.same_as.i_table_id); 
         break; 
         case NCSMAB_FLTR_DEFAULT:
         return ncs_logmsg(NCS_SERVICE_ID_MAB, MAB_LID_FLTR_DEF, 
                           MAB_FC_TBL_DET, NCSFL_LC_DATA, sev,
                           NCSFL_TYPE_TILLL, MAB_MAS_RCV_DEF_FLTR_REG_REQ, 
                           env_id, tbl_id, fltr_id); 
         break;
         case NCSMAB_FLTR_SCALAR:
         default:
         break; 
     }
     return NCSCC_RC_SUCCESS; 
}

static uns32 
log_mab_oaa_and_fltr_data(uns8      sev,
                        uns32       env_id, 
                        uns32       table_id, 
                        uns8        exist,
                        MAS_FLTR    *fltr) 
{
    int8    addr_str[128]= {0}; 
    int8    addr_str1[1024]= {0}; /* to log other filter-id , anchor value mappings */ 
    int8    *ancfid = addr_str1;
    
    /* log the existing filter, and from whom it has been received */ 
    if (m_NCS_NODE_ID_FROM_MDS_DEST(fltr->vcard) == 0)
       sprintf(addr_str, exist == 1?"Existing VDEST: %d":"New VDEST: %d", 
               m_MDS_GET_VDEST_ID_FROM_MDS_DEST(fltr->vcard));
    else
       sprintf(addr_str, exist == 1?"Existing ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%llu"
                                   :"New ADEST:node_id:%d, v1.pad16:%d, v1.vcard:%llu",
                m_NCS_NODE_ID_FROM_MDS_DEST(fltr->vcard), 0, 
                (fltr->vcard));
       
    ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_NO_CB, MAB_FC_HDLN,
               NCSFL_LC_HEADLINE, sev,
               NCSFL_TYPE_TIC, MAB_HDLN_MAS_OAA_ADDR, addr_str);

    /* log the filter data */ 
    log_mab_fltr_data(sev, env_id, table_id,  
                      (fltr->fltr_ids.active_fltr != NULL)?fltr->fltr_ids.active_fltr->fltr_id:0,  
                      &fltr->fltr); 
    ancfid += sprintf(ancfid,"Other Filter-Id Anchor Mapping list:  "); 
    if (!fltr->fltr_ids.fltr_id_list) sprintf(ancfid,"Empty");
    while(fltr->fltr_ids.fltr_id_list)
    {
        ancfid += sprintf(ancfid,"%llu:%u ",
                          fltr->fltr_ids.fltr_id_list->anchor,
                          fltr->fltr_ids.fltr_id_list->fltr_id);
        fltr->fltr_ids.fltr_id_list = fltr->fltr_ids.fltr_id_list->next;
    }
    ncs_logmsg(NCS_SERVICE_ID_MAB,  MAB_LID_NO_CB, MAB_FC_HDLN,
               NCSFL_LC_HEADLINE, sev,
               NCSFL_TYPE_TIC, MAB_HDLN_MAS_OAA_ADDR, addr_str1);

    return NCSCC_RC_SUCCESS; 
}

uns32 log_overlapping_fltrs(uns8        sev,
                            uns32       env_id, 
                            uns32       table_id, 
                            MAS_FLTR    *exst_fltr, 
                            MAS_FLTR    *new_fltr)
{
    /* validate the input pointers */ 
    if ((exst_fltr == NULL)||(new_fltr == NULL))
        return NCSCC_RC_FAILURE; 
        
    /* log the existing filter information */ 
    log_mab_oaa_and_fltr_data(sev, env_id, table_id, 1, exst_fltr);

    /* log the new filter, and from whom it has been received */ 
    log_mab_oaa_and_fltr_data(sev, env_id, table_id, 0, new_fltr);

    return NCSCC_RC_SUCCESS; 
}
#endif
#endif



