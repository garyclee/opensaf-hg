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
 */


#include <stdio.h>
extern "C"{
#include <opensaf/ncsgl_defs.h>
#include <opensaf/ncs_osprm.h>
#include <opensaf/ncs_main_papi.h>
#include <opensaf/ncs_svd.h>
#include <opensaf/ncssysfpool.h>
#include <opensaf/ncssysf_def.h>
#include <opensaf/ncssysf_tsk.h>
#include <opensaf/ncs_trap.h>
#include <opensaf/ncs_edu_pub.h>
/* #include "avsv_mapi.h" */
}

/* SAF header files */
#include <saAis.h>
#include <saEvt.h>

const int SUD_SAF_RELEASE_CODE_VERSION = 'B';
const int SUD_SAF_MAJOR_VERSION = 1;
const int SUD_SAF_MINOR_VERSION = 1;

const SaTimeT SUD_SYNC_CALL_TIMEOUT = 1000000000; /*1 seconds.*/

const int SUD_MAX_EVENT_SIZE = 1024;

void trap_filter_callback(SaEvtSubscriptionIdT sub_id,
    SaEvtEventHandleT event_hdl, const SaSizeT event_data_size);

void decoded_data_print(NCS_TRAP *decoded_data_ptr);
uns32 decoded_data_free(NCS_TRAP *decoded_data_ptr);
int main (void)
{
    SaEvtCallbacksT reg_callback_set;
    SaVersionT version;
    SaAisErrorT retval;
    SaEvtHandleT evt_init_handle;
    uns32 tmp_ctr;
    char *pargv[9];
    char pargv_buffs[9][100];

   for (tmp_ctr=0; tmp_ctr<9; tmp_ctr++)
      pargv[tmp_ctr] = pargv_buffs[tmp_ctr];

   sprintf(pargv[0],"NONE");
   sprintf(pargv[1],"CLUSTER_ID=%d",1);
   sprintf(pargv[2],"SHELF_ID=%d",0);
   sprintf(pargv[3],"SLOT_ID=%d",1);
   sprintf(pargv[4],"NODE_ID=%d",1);
   sprintf(pargv[5],"PCON_ID=%d", 1);
   sprintf(pargv[6],"MDS_IFINDEX=%d",1);
   strcpy(pargv[7],"HUB=n");
   strcpy(pargv[8],"EDSV=y");

    /* initiliase the Environment */
    ncs_agents_startup(9,pargv);

    if(leap_env_init() != NCSCC_RC_SUCCESS)
        return 1;

    reg_callback_set.saEvtChannelOpenCallback  = NULL;
    reg_callback_set.saEvtEventDeliverCallback = trap_filter_callback;
    version.releaseCode = SUD_SAF_RELEASE_CODE_VERSION;
    version.majorVersion = SUD_SAF_MAJOR_VERSION;
    version.minorVersion = SUD_SAF_MINOR_VERSION;
    retval = saEvtInitialize(&evt_init_handle, &reg_callback_set, &version);

    if (SA_AIS_OK == retval)
        printf("\nEDSv init SUCCESS");
    else
	{
        printf("\nEDSv init FAILED cause %d", retval);
		exit(1);
	}

    SaEvtChannelOpenFlagsT   chan_open_flags = 0;
    SaEvtChannelHandleT evt_chan_handle;
    SaNameT event_channel_name;

    event_channel_name.length = strlen(m_SNMP_EDA_EVT_CHANNEL_NAME);
    m_NCS_MEMCPY(event_channel_name.value, m_SNMP_EDA_EVT_CHANNEL_NAME, 
                event_channel_name.length);

    chan_open_flags = SA_EVT_CHANNEL_SUBSCRIBER;

    retval = saEvtChannelOpen(evt_init_handle, &event_channel_name,
                           chan_open_flags, SUD_SYNC_CALL_TIMEOUT, &evt_chan_handle);

    if (SA_AIS_OK == retval)
        printf("\nEDSv open chan success");
    else
	{
        printf("\nEDSv open chan FAILED cause %d", retval);
		exit(1);
	}

    SaEvtEventFilterArrayT filter_array;
    SaEvtSubscriptionIdT subscription_id = 1;
    filter_array.filtersNumber = 0;
    filter_array.filters = NULL;

    retval = saEvtEventSubscribe(evt_chan_handle, &filter_array, 
                              subscription_id);

    if (SA_AIS_OK == retval)
        printf("\nEDSv subscribe success");
    else
	{
        printf("\nEDSv subscribe FAILED cause %d", retval);
		exit(1);
	}

    /*Get the selection object.*/
    SaSelectionObjectT sa_sel_obj;
    NCS_SEL_OBJ_SET wait_sel_obj;
    NCS_SEL_OBJ ncs_sel_obj;
    retval = saEvtSelectionObjectGet(evt_init_handle, &sa_sel_obj);

    if (SA_AIS_OK == retval)
       printf("\nGot SA selection object");
    else
	{
       printf("\nFailed to get SA selection object cause %d\n", retval);
	   exit(1);
	}

    /* Reset the wait select objects */
    m_NCS_SEL_OBJ_ZERO(&wait_sel_obj);

    /* derive the OpenSAF fd for SA selection object */
    m_SET_FD_IN_SEL_OBJ(sa_sel_obj, ncs_sel_obj);
    /*Set the selection object to wait on */
    m_NCS_SEL_OBJ_SET(ncs_sel_obj, &wait_sel_obj);

    while (m_NCS_SEL_OBJ_SELECT
        (ncs_sel_obj, &wait_sel_obj, NULL, NULL, NULL) == NCSCC_RC_SUCCESS)
    {
        /* Process any callbacks. This blocks until we get signal */
        if (m_NCS_SEL_OBJ_ISSET(ncs_sel_obj, &wait_sel_obj))
        {
            /* Dispatch one pending callback at a time. Doing
            SA_DISPATCH_ALL would require sync'ing for this thread */
            retval = saEvtDispatch(evt_init_handle, SA_DISPATCH_ONE);
            if (SA_AIS_OK != retval)
            {
                printf("\nError cause %d occurred during EDSv dispatch", retval);
            }

        }

        /* Again set ckpt select object to wait for another callback */
        m_NCS_SEL_OBJ_SET(ncs_sel_obj, &wait_sel_obj);

    }

    leap_env_destroy();
}

void trap_filter_callback(SaEvtSubscriptionIdT sub_id,
    SaEvtEventHandleT event_hdl, const SaSizeT event_data_size)
{
    char event_data[SUD_MAX_EVENT_SIZE];
    SaSizeT buffer_size = SUD_MAX_EVENT_SIZE;
    SaAisErrorT retval;
    EDU_HDL edu_hdl;
    EDU_ERR error_code = EDU_NORMAL;
    uns32 data_len = (uns32)event_data_size;
    uns32 status = NCSCC_RC_SUCCESS;
    NCS_TRAP decoded_data; /* Note that the all data allocated within this structure MUST be freed */
    NCS_TRAP *decoded_data_ptr;

    m_NCS_MEMSET(event_data, 0, SUD_MAX_EVENT_SIZE);
    m_NCS_MEMSET(&decoded_data, 0, sizeof(decoded_data));

    printf("\nEvent size is %d", data_len);

    retval = saEvtEventDataGet(event_hdl, event_data, &buffer_size);

    if (SA_AIS_OK == retval)
        printf("\nEDSv DataGet() success. Size of data is %d", (uns32)buffer_size);
    else
	{
        printf("\nEDSv DataGet() FAILED cause %d", retval);
		exit(1);
	}
    
    m_NCS_MEMSET(&edu_hdl, 0, sizeof(edu_hdl));
    m_NCS_EDU_HDL_INIT(&edu_hdl);

    decoded_data_ptr = &decoded_data;

    m_NCS_EDU_TLV_EXEC(&edu_hdl, ncs_edp_ncs_trap, event_data,
        data_len, EDP_OP_TYPE_DEC, &decoded_data_ptr, &error_code);
    
    /* print the contets of decoded_data_ptr */
    decoded_data_print(decoded_data_ptr);

    /*Free the varbinds that OpenSAF allocated on our behalf.*/
    /*ncs_trap_eda_trap_varbinds_free();*/
    status = decoded_data_free(decoded_data_ptr);

}
void decoded_data_print(NCS_TRAP *decoded_data_ptr)
{
    uns32 i;
    uns32 count = 1;
    printf("\n=============Decoded Trap Data START===============\n");
    printf("\ni_trap_tbl_id = %d",decoded_data_ptr->i_trap_tbl_id);
    printf("\ni_trap_id     = %d",decoded_data_ptr->i_trap_id);
    printf("\ni_inform_mgr  = %d",decoded_data_ptr->i_inform_mgr);
    printf("\nTrap var Binds:\n");
    while(decoded_data_ptr->i_trap_vb != NULL)
    {
        printf("\nVar Bind %d :",count);
        printf("\n------------");
        printf("\ni_tbl_id  = %d",decoded_data_ptr->i_trap_vb->i_tbl_id);
        printf("\nParamVal  :");
        printf("\nParamId = %d  ",decoded_data_ptr->i_trap_vb->i_param_val.i_param_id);
        switch(decoded_data_ptr->i_trap_vb->i_param_val.i_fmat_id)
        {
            case 0: printf(" NULL"); /* NULL */
                    break;
            case 1: printf(" Integer"); /* NCSMIB_FMAT_INT */
                    printf("Value %d\n",decoded_data_ptr->i_trap_vb->i_param_val.info.i_int);
                    break;
            case 2: printf(" OctetStr");  /* NCSMIB_FMAT_OCT */
                    for (i = 0; i < decoded_data_ptr->i_trap_vb->i_param_val.i_length; i++)
                        printf(",0x%x ",decoded_data_ptr->i_trap_vb->i_param_val.info.i_oct[i]);
                        printf("\n");
                    break;
            case 3: printf(" Bool"); /* NCSMIB_FMAT_BOOL */
                    printf("Value %d\n",decoded_data_ptr->i_trap_vb->i_param_val.info.i_int);
                    break;
            default: break;
        }
        printf("Index Vals as int: ");
        if(decoded_data_ptr->i_trap_vb->i_idx.i_inst_ids == NULL)
            printf(" NULL\n");
        else
        {
            for(i=0; i<decoded_data_ptr->i_trap_vb->i_idx.i_inst_len; i++)
                printf(", %d",decoded_data_ptr->i_trap_vb->i_idx.i_inst_ids[i]);
            printf("\n");
        }
        printf("Index Vals as ascii: ");
        if(decoded_data_ptr->i_trap_vb->i_idx.i_inst_ids == NULL)
            printf(" NULL\n");
        else
        {
            for(i=0; i<decoded_data_ptr->i_trap_vb->i_idx.i_inst_len; i++)
                printf(", %c",decoded_data_ptr->i_trap_vb->i_idx.i_inst_ids[i]);
            printf("\n");
        }
        decoded_data_ptr->i_trap_vb = decoded_data_ptr->i_trap_vb->next_trap_varbind;
        count++;
        
    }
    printf("\n=============Decoded Trap Data END===============\n");
    printf("\n");
}
uns32 decoded_data_free(NCS_TRAP *decoded_data_ptr)
{
    NCS_TRAP_VARBIND     *prev, *current; 
    if(decoded_data_ptr->i_trap_vb == NULL)
    {
        return NCSCC_RC_SUCCESS;
    }
    current = decoded_data_ptr->i_trap_vb;
    while(current)
    {
        prev = current;
        switch(prev->i_param_val.i_fmat_id)
        {
            case 1: /* NCSMIB_FMAT_INT */
                    /* nothing to do */
                    break;
            case 3: /* NCSMIB_FMAT_BOOL */
                    /* nothing to do */
                    break; 
            case 2: /* NCSMIB_FMAT_OCT */
                    if (prev->i_param_val.info.i_oct != NULL)
                    {
                        m_NCS_MEM_FREE(prev->i_param_val.info.i_oct,
                                      NCS_MEM_REGION_PERSISTENT,
                                      NCS_SERVICE_ID_COMMON,2);
                    }
                    break;
            default: /* log the error */
                    break;
        }
        if (prev->i_idx.i_inst_ids != NULL)
        {
            /* free the index */
            m_NCS_MEM_FREE(prev->i_idx.i_inst_ids,
                         NCS_MEM_REGION_PERSISTENT,
                         NCS_SERVICE_ID_COMMON,4);
        }
        /* go to the next node */
        current = current->next_trap_varbind; 
        /* free the varbind now */
        m_MMGR_NCS_TRAP_VARBIND_FREE(prev);
    }
    return NCSCC_RC_SUCCESS;
}
