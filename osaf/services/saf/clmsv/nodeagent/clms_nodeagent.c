#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>


#include <ncs_main_papi.h>
#include <ncssysf_ipc.h>
#include <mds_papi.h>
#include <ncs_hdl_pub.h>
#include <ncsencdec_pub.h>
#include <ncs_mda_pvt.h>
#include <ncs_util.h>
#include <logtrace.h>
#include <saClm.h>
#include "clmsv_msg.h"
#include "clmsv_enc_dec.h"


/*TBD : Add nodeaddress as well for future use*/


typedef struct node_detail_t {
	SaUint32T  node_id;
        SaNameT node_name;
}NODE_INFO;

#define CLMNA_MDS_SUB_PART_VERSION   1
#define CLMS_NODEUP_WAIT_TIME 1000
#define CLMNA_SVC_PVT_SUBPART_VERSION  1
#define CLMNA_WRT_CLMS_SUBPART_VER_AT_MIN_MSG_FMT 1
#define CLMNA_WRT_CLMS_SUBPART_VER_AT_MAX_MSG_FMT 1
#define CLMNA_WRT_CLMS_SUBPART_VER_RANGE             \
        (CLMNA_WRT_CLMS_SUBPART_VER_AT_MAX_MSG_FMT - \
         CLMNA_WRT_CLMS_SUBPART_VER_AT_MIN_MSG_FMT + 1)

/*msg format version for CLMNA subpart version 1 */
static MDS_CLIENT_MSG_FORMAT_VER
	CLMNA_WRT_CLMS_MSG_FMT_ARRAY[CLMNA_WRT_CLMS_SUBPART_VER_RANGE] = {1};


/* CLMS CLMNA sync params */
int clms_sync_awaited;
NCS_SEL_OBJ clms_sync_sel;
MDS_HDL  mds_hdl;
NODE_INFO * node;
MDS_DEST clms_mds_dest; /* CLMS absolute/virtual address */

SaSelectionObjectT selectionObject;

static uns32 clmna_mds_enc(struct ncsmds_callback_info *info);
static uns32 clmna_mds_callback(struct ncsmds_callback_info *info);

static uns32 clmna_mds_cpy(struct ncsmds_callback_info *info)
{
        return NCSCC_RC_SUCCESS;
}

static uns32 clmna_mds_dec(struct ncsmds_callback_info *info)
{
	return NCSCC_RC_SUCCESS;
}

static uns32 clmna_mds_dec_flat(struct ncsmds_callback_info *info)
{
	return NCSCC_RC_SUCCESS;
}
static uns32 clmna_mds_rcv(struct ncsmds_callback_info *mds_cb_info)
{
	return NCSCC_RC_SUCCESS;
}


static uns32 clmna_mds_svc_evt(struct ncsmds_callback_info *mds_cb_info)
{

	TRACE_ENTER2("%d",mds_cb_info->info.svc_evt.i_change);

	switch(mds_cb_info->info.svc_evt.i_change) {
	case NCSMDS_NEW_ACTIVE:
        case NCSMDS_UP:
		switch(mds_cb_info->info.svc_evt.i_svc_id){
		TRACE("svc_id %d",mds_cb_info->info.svc_evt.i_svc_id);
		case NCSMDS_SVC_ID_CLMS:
			clms_mds_dest = mds_cb_info->info.svc_evt.i_dest;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	TRACE_LEAVE();
	return NCSCC_RC_SUCCESS;
}

static uns32 clmna_mds_enc_flat(struct ncsmds_callback_info *info)
{
        /* Modify the MDS_INFO to populate enc */
        info->info.enc = info->info.enc_flat;
        /* Invoke the regular mds_enc routine  */
        return clmna_mds_enc(info);
}


static uns32 clmna_mds_enc(struct ncsmds_callback_info *info)
{
        CLMSV_MSG *msg;
        NCS_UBAID *uba;
        uns8 *p8;
        uns32 total_bytes = 0;

	TRACE_ENTER();

	MDS_CLIENT_MSG_FORMAT_VER msg_fmt_version;

        msg_fmt_version = m_NCS_ENC_MSG_FMT_GET(info->info.enc.i_rem_svc_pvt_ver,
                                                CLMNA_WRT_CLMS_SUBPART_VER_AT_MIN_MSG_FMT,
                                                CLMNA_WRT_CLMS_SUBPART_VER_AT_MAX_MSG_FMT, CLMNA_WRT_CLMS_MSG_FMT_ARRAY);
        if (0 == msg_fmt_version) {
                TRACE("Wrong msg_fmt_version!!\n");
                TRACE_LEAVE();
                return NCSCC_RC_FAILURE;
        }

        info->info.enc.o_msg_fmt_ver = msg_fmt_version;


	msg = (CLMSV_MSG *)info->info.enc.i_msg;
        uba = info->info.enc.io_uba;

        TRACE_2("msgtype: %d", msg->evt_type);

        if (uba == NULL) {
		TRACE("uba == NULL\n");
                return NCSCC_RC_FAILURE;
        }

        /** encode the type of message **/
        p8 = ncs_enc_reserve_space(uba, 4);
        if (!p8) {
                TRACE("NULL pointer");
                return NCSCC_RC_FAILURE;
        }
        ncs_encode_32bit(&p8, msg->evt_type);
        ncs_enc_claim_space(uba, 4);
        total_bytes += 4;


	if (CLMSV_CLMA_TO_CLMS_API_MSG == msg->evt_type) {
		
		/* encode the nodeupinfo*/
		p8 = ncs_enc_reserve_space(uba, 4);
		if (!p8) {
                	TRACE("NULL pointer");
			TRACE_LEAVE();
                	return NCSCC_RC_FAILURE;
        	}
		ncs_encode_32bit(&p8, msg->info.api_info.type);
                ncs_enc_claim_space(uba, 4);
                total_bytes += 4;

		TRACE_2("api_info.type: %d\n", msg->info.api_info.type);

		if (msg->info.api_info.type == CLMSV_NODE_UP_MSG){
			 p8 = ncs_enc_reserve_space(uba, 4);
	                if (!p8) {
        	                TRACE("NULL pointer");
                	        TRACE_LEAVE();
                        	return NCSCC_RC_FAILURE;
                	}

			ncs_encode_32bit(&p8, msg->info.api_info.param.nodeup_info.node_id);
			ncs_enc_claim_space(uba, 4);
	        	total_bytes += 4;
			total_bytes += encodeSaNameT(uba,&(msg->info.api_info.param.nodeup_info.node_name));
		}
	}
	
	TRACE_LEAVE();      		 
        return NCSCC_RC_SUCCESS;
}


static uns32 clmna_mds_callback(struct ncsmds_callback_info *info)
{
	uns32 rc;
	TRACE_ENTER();
                
        static NCSMDS_CALLBACK_API cb_set[MDS_CALLBACK_SVC_MAX] = {
		clmna_mds_cpy,   /* MDS_CALLBACK_COPY      0 */
                clmna_mds_enc,   /* MDS_CALLBACK_ENC       1 */
                clmna_mds_dec,   /* MDS_CALLBACK_DEC       2 */
                clmna_mds_enc_flat,      /* MDS_CALLBACK_ENC_FLAT  3 */
                clmna_mds_dec_flat,      /* MDS_CALLBACK_DEC_FLAT  4 */
                clmna_mds_rcv,   /* MDS_CALLBACK_RECEIVE   5 */
                clmna_mds_svc_evt 
        };
	
	if(info->i_op <= MDS_CALLBACK_SVC_EVENT){
		 rc = (*cb_set[info->i_op]) (info);
                if (rc != NCSCC_RC_SUCCESS)
                        TRACE("MDS_CALLBACK_SVC_EVENT not in range");

		TRACE_LEAVE();
                return rc; 
        } else {
		TRACE_LEAVE();
                return NCSCC_RC_SUCCESS;
	}
}



uns32 clmna_mds_init(void)
{
        NCSADA_INFO ada_info;
        NCSMDS_INFO mds_info;
        uns32 rc = NCSCC_RC_SUCCESS;
	MDS_SVC_ID svc = NCSMDS_SVC_ID_CLMS;


	TRACE_ENTER();

    /** Create the ADEST for CLMNA and get the pwe hdl**/
        memset(&ada_info, '\0', sizeof(ada_info));
        ada_info.req = NCSADA_GET_HDLS;

        if (NCSCC_RC_SUCCESS != (rc = ncsada_api(&ada_info))) {
                TRACE("NCSADA_GET_HDLS failed, rc = %d", rc);
		TRACE_LEAVE();
                return NCSCC_RC_FAILURE;
        }

    /** Store the info obtained from MDS ADEST creation  **/
        mds_hdl = ada_info.info.adest_get_hdls.o_mds_pwe1_hdl;

    /** Now install into mds **/
        memset(&mds_info, '\0', sizeof(NCSMDS_INFO));
        mds_info.i_mds_hdl = mds_hdl;
        mds_info.i_svc_id = NCSMDS_SVC_ID_CLMNA;
        mds_info.i_op = MDS_INSTALL;

        mds_info.info.svc_install.i_yr_svc_hdl = 0;
        mds_info.info.svc_install.i_install_scope = NCSMDS_SCOPE_NONE;  /* PWE scope */
        mds_info.info.svc_install.i_svc_cb = clmna_mds_callback; /* callback */  
        mds_info.info.svc_install.i_mds_q_ownership = FALSE;    /* CLMNA doesn't own the mds queue */
        mds_info.info.svc_install.i_mds_svc_pvt_ver = CLMNA_SVC_PVT_SUBPART_VERSION;

        if ((rc = ncsmds_api(&mds_info)) != NCSCC_RC_SUCCESS) {
                TRACE("mds api call failed");
		TRACE_LEAVE();
                return NCSCC_RC_FAILURE;
        }

	/* Now subscribe for events that will be generated by MDS */
        memset(&mds_info, '\0', sizeof(NCSMDS_INFO));

        mds_info.i_mds_hdl = mds_hdl;
        mds_info.i_svc_id = NCSMDS_SVC_ID_CLMNA;
        mds_info.i_op = MDS_SUBSCRIBE;

        mds_info.info.svc_subscribe.i_scope = NCSMDS_SCOPE_NONE;
        mds_info.info.svc_subscribe.i_num_svcs = 1;
        mds_info.info.svc_subscribe.i_svc_ids = &svc;

        rc = ncsmds_api(&mds_info);
        if (rc != NCSCC_RC_SUCCESS) {
                TRACE("mds api call failed");
		TRACE_LEAVE();
                return rc;
        }


	TRACE_LEAVE();
	return rc;
}

void clmna_node_get_info(void)
{
	FILE *fp;
	TRACE_ENTER();

	fp = fopen("/etc/opensaf/node_name", "r");
        if (fp == NULL){
                TRACE("Error: can't open /etc/opensaf/ node_name file\n");
		assert(0);
        }
        fscanf(fp,"%s",node->node_name.value);
	node->node_name.length = strlen((char *)node->node_name.value);
        TRACE("%s\n",node->node_name.value);
        fclose(fp);

	/*Get the node_id*/
        fp = fopen("/var/lib/opensaf/node_id","r");
        if (fp == NULL){
                TRACE("Error: can't open node_id file\n");
		assert(0);
        }
        fscanf(fp,"%x",&node->node_id);
        TRACE("%d\n",node->node_id);
        fclose(fp);
	
	TRACE_LEAVE();

}

uns32 clmna_mds_msg_sync_send(CLMSV_MSG *i_msg, uns32 timeout)
{
        NCSMDS_INFO mds_info;
        uns32 rc = NCSCC_RC_SUCCESS;


        assert(i_msg != NULL);

        memset(&mds_info, '\0', sizeof(NCSMDS_INFO));
        mds_info.i_mds_hdl = mds_hdl;
        mds_info.i_svc_id = NCSMDS_SVC_ID_CLMNA;
        mds_info.i_op = MDS_SEND;
                 
        /* Fill the send structure */
        mds_info.info.svc_send.i_msg = (NCSCONTEXT)i_msg;
        mds_info.info.svc_send.i_to_svc = NCSMDS_SVC_ID_CLMS;
        mds_info.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
        mds_info.info.svc_send.i_priority = MDS_SEND_PRIORITY_MEDIUM;     /* fixme? */
        /* fill the sub send rsp strcuture */
        mds_info.info.svc_send.info.sndrsp.i_time_to_wait = timeout;    /* timeto wait in 10ms FIX!!! */
        mds_info.info.svc_send.info.sndrsp.i_to_dest = clms_mds_dest;

        /* send the message */
        if (NCSCC_RC_SUCCESS != (rc = ncsmds_api(&mds_info))) 
		TRACE("mds send failed \n");

        return rc;
}


int main()
{
	uns32 rc = NCSCC_RC_SUCCESS;
	CLMSV_MSG msg;
	struct pollfd fds[1];
	int ret;

	node = (NODE_INFO *)malloc(sizeof(NODE_INFO));	

	/*Get the node details*/
	clmna_node_get_info();

	if ((rc = ncs_agents_startup(0, 0)) != NCSCC_RC_SUCCESS) {
        	TRACE("ncs_agents_startup FAILED");
	}

	
	/*Register with mds*/
	rc = clmna_mds_init();
	if ( rc != NCSCC_RC_SUCCESS)
		TRACE("MDS Initialization failed\n");

	if (nid_notify("CLMNA", rc, NULL) != NCSCC_RC_SUCCESS) {
		TRACE("nid_notify failed");
	}

	fds[0].fd = (int) selectionObject;
        fds[0].events = POLLIN;

	while (1){
                ret = poll(fds, 1, 1000);
		if (fds[0].revents & POLLIN){
			msg.evt_type =  CLMSV_CLMA_TO_CLMS_API_MSG;
	                msg.info.api_info.type = CLMSV_NODE_UP_MSG;
                	msg.info.api_info.param.nodeup_info.node_id = node->node_id;
                	msg.info.api_info.param.nodeup_info.node_name =  node->node_name;
                	TRACE("Sending msg to CLMS");
                	rc = clmna_mds_msg_sync_send(&msg,CLMS_NODEUP_WAIT_TIME);
                	if (rc != NCSCC_RC_SUCCESS)
                        	TRACE("Send msg to clms failed\n");
			else
				break;
		}
        }
	return 0;
}



__attribute__ ((constructor))
static void clmna_init_constructor(void)
{
        char *value;

        /* Initialize trace system first of all so we can see what is going. */
        if ((value = getenv("CLMNA_TRACE_PATHNAME")) != NULL) {
                if (logtrace_init("clmna", value) != 0) {
                        syslog(LOG_WARNING, "LOG lib: logtrace_init FAILED, tracing disabled...");
                        return;
                }

                /* Do not care about categories now, get all */
                trace_category_set(CATEGORY_ALL);
        }
}
