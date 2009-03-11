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
  
  DESCRIPTION: This file describes the routines/Macros for the 
               NCS SNMP SubAgent. 
  ***************************************************************************/ 
#include "subagt.h"

/******************************************************************************
 *  Name:          snmpsubagt_dla_initialize
 *  Description:   To intialize the session with Disributed Loggin Agent
 *                  - Register the canned strings and the formats 
 *                  - Register the Service Id
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note:
 ******************************************************************************/
uns32 
snmpsubagt_dla_initialize(NCS_SERVICE_ID i_svc_id)
{
    NCS_DTSV_RQ                 reg_info; 
    uns32                       status = NCSCC_RC_FAILURE; 

    /* Level the ground */
    memset(&reg_info, 0, sizeof(NCS_DTSV_RQ));
    
    /* register the NCS SNMP SubAgent */
    reg_info.i_op = NCS_DTSV_OP_BIND;
    reg_info.info.bind_svc.svc_id = i_svc_id;
    /* fill version no. */
    reg_info.info.bind_svc.version = SNMPSUBAGT_LOG_VERSION;
    /* fill svc_name */
    m_NCS_STRCPY(reg_info.info.bind_svc.svc_name, "SUBAGT");

    status = ncs_dtsv_su_req(&reg_info);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the error */ 
        m_NCS_SNMPSUBAGT_DBG_SINK(status);
    }
    
    return status; 
}

/******************************************************************************
 *  Name:          snmpsubagt_dla_finalize
 *  Description:   To Fintialize the session with Disributed Loggin Agent
 *                  - Register the canned strings and the formats 
 *                  - Register the Service Id
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note:
 ******************************************************************************/
uns32 
snmpsubagt_dla_finalize(NCS_SERVICE_ID i_svc_id)
{
    NCS_DTSV_RQ                 reg_info; 
    uns32                       status = NCSCC_RC_FAILURE; 

    memset(&reg_info, 0, sizeof(NCS_DTSV_RQ));
   
 
    /* Unbind the NCS SNMP SubAgent */
    reg_info.i_op = NCS_DTSV_OP_UNBIND;
    reg_info.info.unbind_svc.svc_id = i_svc_id;
    status = ncs_dtsv_su_req(&reg_info);
    if (status != NCSCC_RC_SUCCESS)
    {
        /* lgo the error */ 
        m_NCS_SNMPSUBAGT_DBG_SINK(status);
    }

    return status; 
}

/* Logging Routines */
/******************************************************************************
 *  Name:          snmpsubagt_headline_log
 *  Description:   To log the headline
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note: <TBD Header Mahesh>
 ******************************************************************************/
void snmpsubagt_log_headline (uns8 hdln_id)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_HDLN; 

    ncs_logmsg(svc_id, (uns8)fmt_id,(uns8)SNMPSUBAGT_FS_HDLN, NCSFL_LC_HEADLINE,NCSFL_SEV_INFO, NCSFL_TYPE_TI, hdln_id);   
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_memfailure_log
 *  Description:   To log the memory failures 
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note: <TBD Header Mahesh>
 ******************************************************************************/

void snmpsubagt_log_mem( uns8 index)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_MEM; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_MEM, NCSFL_LC_HEADLINE,NCSFL_SEV_INFO, NCSFL_TYPE_TI,index);   
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_errors_log
 *  Description:   To log the error 
 *
 *  Arguments:     
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note: <Header TBD>
 ******************************************************************************/
void 
snmpsubagt_log_errors(uns8 index, uns32 info1, uns32 info2, uns32 info3)
{
#if (NCS_SNMPSUBAGT_LOG == 1) 
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_ERRORS; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_ERRORS, NCSFL_LC_HEADLINE,NCSFL_SEV_NOTICE, NCSFL_TYPE_TILLL,index,info1,info2,info3);   
#endif
    return; 
}


/******************************************************************************
 *  Name:          snmpsubagt_log_func_entry
 *  Description:   To log the function entry 
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note: 
 ******************************************************************************/
void snmpsubagt_log_func_entry ( uns8 index)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_FUNC_ENTRY; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_FUNC_ENTRY, NCSFL_LC_HEADLINE,NCSFL_SEV_INFO, NCSFL_TYPE_TI,index);   
#endif
    return; 
}


/******************************************************************************
 *  Name:          snmpsubagt_log_data_dump
 *  Description:   To log the error 
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:      
 ******************************************************************************/
void 
snmpsubagt_log_data_dump(uns8 index, uns32 info1)
{

#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_DATA_DUMP; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_DATA_DUMP, NCSFL_LC_HEADLINE,NCSFL_SEV_INFO, NCSFL_TYPE_TIL,index,info1);   
#endif
    return; 
}


/******************************************************************************
 *  Name:          snmpsubagt_log_state
 *  Description:   To log the error 
 *
 *  Arguments:     
 *
 *  Returns:       
 ******************************************************************************/
void 
snmpsubagt_log_state(uns8 index, uns32 info1, uns32 info2)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
 
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_STATE; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_STATE,NCSFL_LC_HEADLINE,NCSFL_SEV_NOTICE, NCSFL_TYPE_TILL,index,info1,info2);   
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_log_memdump
 *  Description:   To log the error 
 *
 *  Arguments:     
 *
 *  Returns:     
 ******************************************************************************/
void 
snmpsubagt_log_memdump(uns8 index, NCSFL_MEM mem)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
 
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_MEMDUMP; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_MEMDUMP,NCSFL_LC_HEADLINE,NCSFL_SEV_INFO, NCSFL_TYPE_TID,index,mem);   
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_log_error_strs
 *  Description:   To log the error 
 *
 *  Arguments:     
 *
 *  Returns:    
 ******************************************************************************/
void 
snmpsubagt_log_error_strs(uns8 index,char *info1, uns32 info2, uns32 info3)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_ERROR_STRS; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_ERROR_STRS,
              NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TICLL,index,info1,info2,info3);  
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_log_intf_init_state
 *  Description:   To log the intf_init_state
 *
 *  Arguments:     NCS_SERVICE_ID - NCS SNMP SubAgent's Service ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note: <TBD Header Mahesh>
 ******************************************************************************/
void snmpsubagt_log_intf_init_state (uns8 intf_init_state_id)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_INTF_INIT_STATE; 

    ncs_logmsg(svc_id, (uns8)fmt_id,(uns8)SNMPSUBAGT_FS_INTF_INIT_STATE, NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TI, intf_init_state_id);   
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_log_table_id
 *  Description:   To log the table id
 *
 *  Arguments:     uns8 index, uns32 - Table ID
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 *  Note: <TBD Header Mahesh>
 ******************************************************************************/
void snmpsubagt_log_table_id (uns8 index, uns32 table_id)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_LOG_TABLE_ID; 

    ncs_logmsg(svc_id, (uns8)fmt_id,(uns8)SNMPSUBAGT_FS_LOG_TABLE_ID, NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TIL, index, table_id);   
#endif
    return; 
}

/******************************************************************************
 *  Name:          snmpsubagt_dbg_sink
 *  Description:   To log the error 
 *
 *  Arguments:     
 *
 *  Returns:       NCSCC_RC_SUCCESS   - everything is OK
 *                 NCSCC_RC_FAILURE   -  failure
 ******************************************************************************/
uns32 snmpsubagt_dbg_sink(uns32 l, char* f, uns32 code)
{
    m_NCS_CONS_PRINTF ("IN SNMPSUBAGT_DBG_SINK: line %d, file %s\n",l,f);
    return code;
}
           
/***************************************************************************** *
*  Name:          snmpsubagt_gen_str_log
*  Description:   To log the error string in the generated code
*
*  Arguments:     
*
*  Returns:    
***************************************************************************** */
void 
snmpsubagt_gen_str_log(char *info1)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_GEN_STR; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_GEN_STR,
               NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TIC,0,info1);  
#endif
    return; 
}
/******************************************************************************
*  Name:          snmpsubagt_gen_err_log
*  Description:   To log the error string and the error code in the generated
     code
*
*  Arguments:     
*
*  Returns:    
******************************************************************************/
void 
snmpsubagt_gen_err_log(char *info1, uns32 info2)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_GEN_ERR; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_GEN_ERR,
             NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TICL,0,info1, info2);  
#endif
    return; 
}
           
/******************************************************************************
*  Name:          snmpsubagt_gen_oid_log
*  Description:   To log the OID
*
*  Arguments:     
*
*  Returns:    
******************************************************************************/
void 
snmpsubagt_gen_oid_log(NCSFL_MEM info)
{
#if (NCS_SNMPSUBAGT_LOG == 1)
    NCS_SERVICE_ID svc_id = NCS_SERVICE_ID_SNMPSUBAGT; 
    SNMPSUBAGT_FMAT_ENUM fmt_id = SNMPSUBAGT_FMTID_GEN_OID_LOG; 

    ncs_logmsg(svc_id, (uns8)fmt_id, (uns8)SNMPSUBAGT_FS_GEN_OID_LOG,
               NCSFL_LC_HEADLINE, NCSFL_SEV_NOTICE, NCSFL_TYPE_TID, 0, info);  
#endif
    return; 
}
                         
