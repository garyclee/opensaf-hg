#include <saImmOm.h>
#include <saImmOi.h>

#include "clms.h"


void clms_all_node_rattr_update(void);
SaAisErrorT clms_node_ccb_comp_cb(CcbUtilOperationData_t *opdata);
uns32 clms_imm_node_unlock(CLMS_CLUSTER_NODE * nodeop);
uns32 clms_imm_node_lock(CLMS_CLUSTER_NODE * nodeop);
uns32 clms_imm_node_shutdown(CLMS_CLUSTER_NODE * nodeop);
static void clms_lock_send_start_cbk(CLMS_CLUSTER_NODE * nodeop);
static void clms_timer_ipc_send(SaClmNodeIdT  node_id);
static uns32 clms_lock_send_no_start_cbk(CLMS_CLUSTER_NODE * nodeop);

static SaVersionT immVersion = { 'A', 2, 1 };

/**
* Initialize the track response patricia tree for the node
* @param[in] node node to initialize trackresponse tree
*/
static void clms_trackresp_patricia_init(CLMS_CLUSTER_NODE * node)
{
	NCS_PATRICIA_PARAMS trackresp_param;
	TRACE_ENTER();

	memset(&trackresp_param,0, sizeof(NCS_PATRICIA_PARAMS));
	trackresp_param.key_size = sizeof(SaInvocationT);

	if(NCSCC_RC_SUCCESS != ncs_patricia_tree_init(&node->trackresp, &trackresp_param)){
		LOG_ER("patricia tree init failed for node's trackresp tree");
		assert(0);
	}

	TRACE_LEAVE();
}

/**
 * Become object and class implementer. 
 * @param[in] _cb
 * @return void*
 */
static void *imm_impl_set(void *_cb)
{
	SaAisErrorT rc;
	CLMS_CB *cb = (CLMS_CB *)_cb;

	TRACE_ENTER();

	if ((rc = immutil_saImmOiImplementerSet(cb->immOiHandle, IMPLEMENTER_NAME)) != SA_AIS_OK){
		LOG_ER("saImmOiImplementerSet failed %u", rc);
		exit(EXIT_FAILURE);
	}

	if ((rc = saImmOiClassImplementerSet(cb->immOiHandle, "SaClmNode")) != SA_AIS_OK){
		LOG_ER("saImmOiClassImplementerSet failed  for class SaClmNode%u", rc);
		exit(EXIT_FAILURE);
	}

	if ((rc = saImmOiClassImplementerSet(cb->immOiHandle, "SaClmCluster")) != SA_AIS_OK){
		LOG_ER("saImmOiClassImplementerSet failed  for class SaClmCluster%u", rc);
		exit(EXIT_FAILURE);
	}

	TRACE_LEAVE();
}
/**
* Lookup the client db to find if any subscribers exist for the START step
* @return NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
*/
uns32 clms_chk_sub_start(void)
{

        CLMS_CLIENT_INFO *rec = NULL;
	uns32 client_id = 0;

	TRACE_ENTER();

        while(NULL != (rec =  clms_client_getnext_by_id(client_id))){
		client_id = rec->client_id;

                if(rec->track_flags & (SA_TRACK_START_STEP))
                        return NCSCC_RC_SUCCESS;
        }

	TRACE_LEAVE();

        return NCSCC_RC_FAILURE;
}


/**
 * Become object and class implementer, non-blocking.
 * @param cb
 */
void clms_imm_impl_set(CLMS_CB *cb)
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	TRACE_ENTER();

	if (pthread_create(&thread, &attr, imm_impl_set, cb) != 0) {
		LOG_ER("pthread_create FAILED: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	TRACE_LEAVE();
}

/**
* Create the new node
* @param[in] name  DN name of the node to create
* @param[in] attrs node's attributes
* @return Cluster node
*/
CLMS_CLUSTER_NODE * clms_node_new(SaNameT *name,const SaImmAttrValuesT_2 ** attrs)
{
	CLMS_CLUSTER_NODE * node=NULL;
	const SaImmAttrValuesT_2 *attr;
	int i = 0;

	TRACE_ENTER();	
	
	/*If Cluster Node already exists,no need to calloc,just populate it */	
	if ((node = clms_node_get_by_name(name)) == NULL){
		node = (CLMS_CLUSTER_NODE *)malloc(sizeof(CLMS_CLUSTER_NODE));
		if (node == NULL)
		{
			LOG_ER("Calloc failed");
			goto done;
		}
	}
	
	memset(node,0,sizeof(CLMS_CLUSTER_NODE));
	/*Initialize some attributes of the node like*/
	memcpy(node->node_name.value,name->value,name->length);
	node->node_name.length=name->length;
	TRACE("nodename %s",node->node_name.value);
	
	while ((attr = attrs[i++]) != NULL) {
		void *value;

		if (attr->attrValuesNumber == 0)
			continue;

		value = attr->attrValues[0];
		TRACE("Inside the while loop attrname %s",attr->attrName);
		       
	       	if (!strcmp(attr->attrName, "saClmNodeAdminState")) {
			node->admin_state = *((SaUint32T *)value);
			
		} else if (!strcmp(attr->attrName, "saClmNodeLockCallbackTimeout")) {
			node->lck_cbk_timeout = *((SaTimeT *)value);
		       
		} else if (!strcmp(attr->attrName, "saClmNodeDisableReboot")) {
			node->disable_reboot = *((SaUint32T *)value);
		
		} else if (!strcmp(attr->attrName, "saClmNodeAddressFamily")) {
			node->node_addr.family = *((SaUint32T *)value);

		} else if (!strcmp(attr->attrName, "saClmNodeAddress")) {
			strcpy((char *)node->node_addr.value,*((char **)value));
			node->node_addr.length = (SaUint16T)strlen((char *)node->node_addr.value);

		} else if (!strcmp(attr->attrName, "saClmNodeEE")) {
			SaNameT *name = (SaNameT*)value;
			TRACE("saClmNodeEE attribute name's length %d",name->length);

			if(name->length != 0){
				memcpy(node->ee_name.value,name->value,name->length);
				node->ee_name.length = name->length;
			}
		}

  	}

	/*Init patricia trackresp for each node*/
	clms_trackresp_patricia_init(node);
	
done:
	TRACE_LEAVE();
 	return node;
}
/**
* This constructs the node db by reading the config attributes from immsv
*/
SaAisErrorT clms_node_create_config(void)
{
	 SaAisErrorT rc = SA_AIS_ERR_INVALID_PARAM;
	 uns32 rt;	
	 SaImmHandleT imm_om_hdl;
	 SaImmSearchHandleT search_hdl;
	 SaImmSearchParametersT_2 searchParam;
	 SaNameT dn;
	 SaImmAttrValuesT_2 **attributes;
	/*For time being changed to SaClmNode1 but shud be SaClmNode*/
	 const char *className="SaClmNode";

	TRACE_ENTER();

	 CLMS_CLUSTER_NODE * node = NULL;

	 (void) immutil_saImmOmInitialize(&imm_om_hdl, NULL, &immVersion);

	 searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	 searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	 searchParam.searchOneAttr.attrValue = &className;

	rc = immutil_saImmOmSearchInitialize_2(imm_om_hdl, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &search_hdl);
	
	if (rc != SA_AIS_OK)
	{
		LOG_ER("No Object of  SaClmNode Class was found");
		goto done1;
	} 

	while (immutil_saImmOmSearchNext_2(search_hdl, &dn, &attributes) == SA_AIS_OK)
	{
		TRACE("immutil_saImmOmSearchNext_2 Success");
		if ((rt = clms_node_dn_chk(&dn))!= NCSCC_RC_SUCCESS){
			TRACE("Node DN name is incorrect");
                        rc = SA_AIS_ERR_BAD_OPERATION;
                        goto done2;
                }
			
	 	if ((node = clms_node_new(&dn,(const SaImmAttrValuesT_2 **)attributes)) == NULL)
			goto done2;
		clms_node_add_to_model(node);
	
	}

	rc = SA_AIS_OK;
done2:  

 (void) immutil_saImmOmSearchFinalize(search_hdl);

done1:  
 (void) immutil_saImmOmFinalize(imm_om_hdl);
	TRACE_LEAVE();
	return rc;
}
/**
* This creates the cluster database by reading the cluster dn from immsv
*/
SaAisErrorT clms_cluster_config_get(void)
{
	 SaAisErrorT rc = SA_AIS_ERR_INVALID_PARAM;
	 uns32 rt = NCSCC_RC_SUCCESS;
	 SaImmHandleT imm_om_hdl;
	 SaImmSearchHandleT search_hdl;
	 SaImmSearchParametersT_2 searchParam;
	 SaNameT dn;
	 SaImmAttrValuesT_2 **attributes;
	 const char *className="SaClmCluster";

	TRACE_ENTER();	

	 (void) immutil_saImmOmInitialize(&imm_om_hdl, NULL, &immVersion);

	 searchParam.searchOneAttr.attrName = "SaImmAttrClassName";
	 searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	 searchParam.searchOneAttr.attrValue = &className;
	
	rc = immutil_saImmOmSearchInitialize_2(imm_om_hdl, NULL, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &search_hdl);

	if(rc != SA_AIS_OK)
	{
		LOG_ER("No Object of  SaClmCluster Class was found");
		goto done1;
	} 
	/*Multiple Cluster is not supported*/

	if ((immutil_saImmOmSearchNext_2(search_hdl, &dn, &attributes)) == SA_AIS_OK){
		TRACE("clms_cluster_config_get:%s",dn.value);
		if ((rt = clms_cluster_dn_chk(&dn)) != NCSCC_RC_SUCCESS){
			TRACE("clms_cluster_dn_chk failed");
			goto done2;
		}
		if (osaf_cluster == NULL){	
			osaf_cluster = calloc(1,sizeof(CLMS_CLUSTER_INFO));
			if (osaf_cluster == NULL){
				LOG_ER("osaf_cluster calloc failed");
				goto done2;
			}
		}
		memcpy(&osaf_cluster->name,&dn,sizeof(SaNameT));	

		osaf_cluster->num_nodes = 0;  /*Will be updated @runtime*/

		if(clms_cb->ha_state ==  SA_AMF_HA_ACTIVE){
			osaf_cluster->init_time = clms_get_SaTime();
		}
	}
	rc = SA_AIS_OK;
done2:

 (void) immutil_saImmOmSearchFinalize(search_hdl);

done1:
 (void) immutil_saImmOmFinalize(imm_om_hdl);
	TRACE_LEAVE();
	return rc;

}

/**
 * Retrieve the Cluster Node configuration from IMM using the
 * IMM-OM interface and initialize the corresponding information 
 * in the CLM control block. 
 */
SaAisErrorT clms_imm_activate(CLMS_CB *cb)
{	  
	SaAisErrorT rc = SA_AIS_OK;
 
	TRACE_ENTER();
        if (( rc = clms_cluster_config_get()) != SA_AIS_OK){
		LOG_ER("clms_cluster_config_get failed");
		exit(EXIT_FAILURE);
	}

	if ((rc = clms_node_create_config()) != SA_AIS_OK){
		LOG_ER("clms_node_create_config failed");
		exit(EXIT_FAILURE);
	}
	
        if (clms_cb->ha_state == SA_AMF_HA_ACTIVE) {
		 if ((rc = immutil_saImmOiImplementerSet(cb->immOiHandle, IMPLEMENTER_NAME))!=SA_AIS_OK){
			LOG_ER("saImmOiImplementerSet failed %u", rc);
			exit(EXIT_FAILURE);
		}

		if ((rc = saImmOiClassImplementerSet(cb->immOiHandle, "SaClmNode"))!=SA_AIS_OK){
			LOG_ER("saImmOiClassImplementerSet failed  for class SaClmNode%u", rc);
			exit(EXIT_FAILURE);
		}

		if ((rc = saImmOiClassImplementerSet(cb->immOiHandle, "SaClmCluster"))!=SA_AIS_OK){
			LOG_ER("saImmOiClassImplementerSet failed  for class SaClmCluster%u", rc);
			exit(EXIT_FAILURE);
		}
	 
		clms_all_node_rattr_update();
	}

	TRACE_LEAVE();
	return rc;

}


/** 
* Update IMMSv the runtime info of the node 
* @param[in] nd pointer to cluster node
*/
void clms_node_update_rattr(CLMS_CLUSTER_NODE * nd)
{
        SaImmAttrModificationT_2 attr_Mod[5];
        SaAisErrorT rc;
        SaImmAttrValueT attrUpdateValue[] =  { &nd->member};
        SaImmAttrValueT attrUpdateValue1[] = { &nd->node_id};
        SaImmAttrValueT attrUpdateValue2[] = { &nd->boot_time};
        SaImmAttrValueT attrUpdateValue3[] = { &nd->init_view};
        SaImmAttrValueT attrUpdateValue4[] = { &nd->admin_state};

        const SaImmAttrModificationT_2 *attrMods[] = {
				&attr_Mod[0],
				&attr_Mod[1],
				&attr_Mod[2],
				&attr_Mod[3],
				&attr_Mod[4],
				NULL};

	
	TRACE_ENTER(); 
                                                
        attr_Mod[0].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[0].modAttr.attrName = "saClmNodeIsMember";
        attr_Mod[0].modAttr.attrValuesNumber = 1;
        attr_Mod[0].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
        attr_Mod[0].modAttr.attrValues = attrUpdateValue;

        attr_Mod[1].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[1].modAttr.attrName = "saClmNodeID";
        attr_Mod[1].modAttr.attrValuesNumber = 1;
        attr_Mod[1].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
        attr_Mod[1].modAttr.attrValues = attrUpdateValue1;

        attr_Mod[2].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[2].modAttr.attrName = "saClmNodeBootTimeStamp";
        attr_Mod[2].modAttr.attrValuesNumber = 1;
        attr_Mod[2].modAttr.attrValueType = SA_IMM_ATTR_SATIMET;
        attr_Mod[2].modAttr.attrValues = attrUpdateValue2;

        attr_Mod[3].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[3].modAttr.attrName = "saClmNodeInitialViewNumber";
        attr_Mod[3].modAttr.attrValuesNumber = 1;
        attr_Mod[3].modAttr.attrValueType = SA_IMM_ATTR_SAUINT64T;
        attr_Mod[3].modAttr.attrValues = attrUpdateValue3;
                
        attr_Mod[4].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[4].modAttr.attrName = "saClmNodeAdminState";
        attr_Mod[4].modAttr.attrValuesNumber = 1;
        attr_Mod[4].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
        attr_Mod[4].modAttr.attrValues = attrUpdateValue4;

	int errorsAreFatal = immutilWrapperProfile.errorsAreFatal;
	immutilWrapperProfile.errorsAreFatal = 0;
        rc = immutil_saImmOiRtObjectUpdate_2(clms_cb->immOiHandle, &nd->node_name, attrMods);
	immutilWrapperProfile.errorsAreFatal = errorsAreFatal;
                                 
        if (rc != SA_AIS_OK){
                LOG_ER("saImmOiRtObjectUpdate FAILED %u, '%s'",
                        rc, nd->node_name.value);
	}
	
	TRACE_LEAVE();
}
 
/** 
* Update IMMSv the runtime info of all node 
*/
void clms_all_node_rattr_update(void)
{
	CLMS_CLUSTER_NODE * node = NULL;
	SaUint32T nodeid = 0;

	node = clms_node_getnext_by_id(nodeid);
	while(node != NULL){
		clms_node_update_rattr(node);
		node = clms_node_getnext_by_id(node->node_id);
	}

}

/** 
* Update IMMSv with the runtime info of the osaf cluster 
* @param[in] osaf_cluster pointer to CLM Cluster
*/
void clms_cluster_update_rattr(CLMS_CLUSTER_INFO *osaf_cluster)
{        
	SaImmAttrModificationT_2 attr_Mod[2];
        SaAisErrorT rc;
        SaImmAttrValueT attrUpdateValue[] = { &osaf_cluster->num_nodes};
        SaImmAttrValueT attrUpdateValue1[] = { &osaf_cluster->init_time};

        const SaImmAttrModificationT_2 *attrMods[] = {
				&attr_Mod[0],
				&attr_Mod[1],
				NULL};

	
	TRACE_ENTER(); 
                                                
        attr_Mod[0].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[0].modAttr.attrName = "saClmClusterNumNodes";
        attr_Mod[0].modAttr.attrValuesNumber = 1;
        attr_Mod[0].modAttr.attrValueType = SA_IMM_ATTR_SAUINT32T;
        attr_Mod[0].modAttr.attrValues = attrUpdateValue;

        attr_Mod[1].modType = SA_IMM_ATTR_VALUES_REPLACE;
        attr_Mod[1].modAttr.attrName = "saClmClusterInitTimestamp";
        attr_Mod[1].modAttr.attrValuesNumber = 1;
        attr_Mod[1].modAttr.attrValueType = SA_IMM_ATTR_SATIMET;
        attr_Mod[1].modAttr.attrValues = attrUpdateValue1;

	int errorsAreFatal = immutilWrapperProfile.errorsAreFatal;
	immutilWrapperProfile.errorsAreFatal = 0;
        rc = immutil_saImmOiRtObjectUpdate_2(clms_cb->immOiHandle, &osaf_cluster->name, attrMods);
	immutilWrapperProfile.errorsAreFatal = errorsAreFatal;
                                 
        if (rc != SA_AIS_OK){
                LOG_ER("saImmOiRtObjectUpdate FAILED %u, '%s'",
                        rc, osaf_cluster->name.value);
	}
	
	/* TBD: We need to handle a case where there's only one node,
	 this node is not a member yet, because after a cluster reset,
	when we came up, we could be in a locked state, which means,
 	we are not yet a member, which would also mean as per the definition
	of this particular attribute, it is the timestamp ofthe first "member"
	that joined the cluster.
	Hence, 
	- The above attribute update should happen under a condition that
	if our adminstate is not unlocked and we are member.
	- this timestamp or attribute updation should also happen when
	an IMM unlock for the self-node happens and we become member.*/

	TRACE_LEAVE();
}
/**
* Handles CLM Admin Operation.This is executed as an IMM callback.
* @param immOiHandle
* @param invocation
* @param objectName
* @param opId 
* @param params
*/
static void clms_imm_admin_op_callback (SaImmOiHandleT immOiHandle,
			SaInvocationT invocation,
			const SaNameT *objectName,
			SaImmAdminOperationIdT opId,
			const SaImmAdminOperationParamsT_2 **params)
{
	CLMS_CLUSTER_NODE * nodeop;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("Admin callback for nodename:%s, opId:%llu",objectName->value,opId);

	/*Lookup by the node_name and get the cluster node for CLM Admin oper*/
	nodeop = clms_node_get_by_name(objectName);
	if(nodeop == NULL) {
		LOG_ER("Admin Operation on invalid node_name"); 
		assert(0);
	}

	/* Don't proceed: already admin op is in progress on this node */		 	
	if (nodeop->admin_op != 0) {
		(void)immutil_saImmOiAdminOperationResult(immOiHandle, invocation, SA_AIS_ERR_BAD_OPERATION);
		goto done;
	}

	nodeop->curr_admin_inv = invocation;

	/*Check for admin oper for opId*/ 
	switch(opId){

	case SA_CLM_ADMIN_LOCK:
		nodeop->admin_op = IMM_LOCK;
		rc = clms_imm_node_lock(nodeop);
		if (rc != NCSCC_RC_SUCCESS){
			LOG_ER("clms_imm_node_lock failed");
			goto done;
		}

		break;
	case SA_CLM_ADMIN_UNLOCK:
		
		nodeop->admin_op = IMM_UNLOCK;
		rc = clms_imm_node_unlock(nodeop);
		if (rc != NCSCC_RC_SUCCESS){
                        LOG_ER("clms_imm_node_unlock failed");
                        goto done;
                }

		break;
	case SA_CLM_ADMIN_SHUTDOWN:
		nodeop->admin_op = IMM_SHUTDOWN;
		rc = clms_imm_node_shutdown(nodeop);
		if (rc != NCSCC_RC_SUCCESS){
                        LOG_ER("clms_imm_node_shutdown failed");
                        goto done;
                }
		break;	
	default:
 		TRACE("Admin operation not supported");
		assert(0);
	}

	clms_node_update_rattr(nodeop);
	clms_cluster_update_rattr(osaf_cluster);

	/*CheckPoint the node data*/
	ckpt_node_rec(nodeop);

	/*Checkpoint Cluster data*/
	ckpt_cluster_rec();

done:
	TRACE_LEAVE();

}

/**
* This forms the track response list for the start/validate step during admin operation
* @param[in] node          pointer to cluster node on which this list is to be created
* @param[client] client    client info to be added to list
*/
static void clms_create_track_resp_list(CLMS_CLUSTER_NODE *node,CLMS_CLIENT_INFO *client)
{
	CLMS_TRACK_INFO * trk = NULL;
	TRACE_ENTER();

	trk = (CLMS_TRACK_INFO *)calloc(1,sizeof(CLMS_TRACK_INFO));
        if(trk == NULL){
		LOG_ER("trk calloc failed");
                assert(0);
	}
	/*add to the tracklist of clmresp tracking*/
        trk->client_id =client->client_id;
        trk->client_dest = client->mds_dest;
        trk->client_id_net = client->client_id_net;
	trk->inv_id = client->inv_id;
        trk->pat_node.key_info =(uns8 *)&client->inv_id; /*Key Info as inv id*/

        if(ncs_patricia_tree_add(&node->trackresp,&trk->pat_node) != NCSCC_RC_SUCCESS){
        	LOG_ER("patricia tree add failed for CLMS_TRACK_INFO");
                assert(0);
	}

	TRACE_LEAVE();
}

/**
* This sends the track callback to its clients on the basis of the subscribed trackflags
* @param[in] cb      CLM CB
* @param[in] node    pointer to cluster node
* @param[in] step    CLM step for which to send the callback 
*/
void  clms_send_track(CLMS_CB *cb,CLMS_CLUSTER_NODE *node,SaClmChangeStepT step)
{
	CLMS_CLIENT_INFO *rec;
	uns32  client_id = 0; 
	SaClmClusterNotificationT_4 *notify_changes = NULL;
	SaClmClusterNotificationT_4 *notify_changes_only = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER2("step: %d",step);

	if (node->change == SA_CLM_NODE_LEFT)
		LOG_NO("%s LEFT, view number=%llu", node->node_name.value, node->init_view);
	else if (node->change == SA_CLM_NODE_JOINED)
		LOG_NO("%s JOINED, view number=%llu", node->node_name.value, node->init_view);
	else if (node->change == SA_CLM_NODE_SHUTDOWN)
		LOG_NO("%s SHUTDOWN, view number=%llu", node->node_name.value, node->init_view);

	/* Check for previous stale tracklist on this node and delete it.
           PLM doesnt allows multiple admin operations, hence this means
           PLM is enforcing its clients to forcibly-move to the next step
	   If it was validate, then we get abort. If we were in start,
	   we get completed.
           for the same admin operation during which a fail/switchover
           happened 
         */
	if(ncs_patricia_tree_size(&node->trackresp) != 0)
		clms_node_trackresplist_empty(node);

	notify_changes_only = clms_notbuffer_changes_only(step);
	notify_changes = clms_notbuffer_changes(step);

	while((rec = clms_client_getnext_by_id(client_id)) != NULL){
		client_id = rec->client_id;
		if (step == SA_CLM_CHANGE_START){		

			if(rec->track_flags & (SA_TRACK_START_STEP)){
		 		/*Create the inv-id*/
		 		rec->inv_id = m_CLMSV_PACK_INV((cb->curr_invid)++, node->node_id);
				TRACE("rec->inv_id %llu",rec->inv_id);

				if (rec->track_flags & SA_TRACK_CHANGES_ONLY)
		 			rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_START,notify_changes_only);
				else if (rec->track_flags & SA_TRACK_CHANGES)
					rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_START,notify_changes);

		 		if (rc != NCSCC_RC_SUCCESS){
					TRACE("Sending track callback failed for SA_CLM_CHANGE_START");
				}

				clms_create_track_resp_list(node,rec);
				
			}
       		} else if (step == SA_CLM_CHANGE_VALIDATE){
			if (rec->track_flags & (SA_TRACK_VALIDATE_STEP)){
				/*Create the inv-id*/
				rec->inv_id = m_CLMSV_PACK_INV((cb->curr_invid)++, node->node_id);

				if (rec->track_flags & SA_TRACK_CHANGES_ONLY)
		 			rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_VALIDATE,notify_changes_only);
				else if (rec->track_flags & SA_TRACK_CHANGES)
					rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_VALIDATE,notify_changes);

				if (rc != NCSCC_RC_SUCCESS){
					TRACE("Sending track callback failed for SA_CLM_CHANGE_VALIDATE");
				}
				
				clms_create_track_resp_list(node,rec);
				TRACE("patricia tree add succeeded for validate step");

			}
		} else if (step == SA_CLM_CHANGE_COMPLETED){

				rec->inv_id = 0; /*No resp for Completed Step*/

				if (rec->track_flags & SA_TRACK_CHANGES_ONLY)
		 			rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_COMPLETED,notify_changes_only);
				else if (rec->track_flags & SA_TRACK_CHANGES)
					rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_COMPLETED,notify_changes);

				if (rc != NCSCC_RC_SUCCESS){
					TRACE("Sending track callback failed for SA_CLM_CHANGE_COMPLETED");
				}

		} else if (step == SA_CLM_CHANGE_ABORTED){  /*no track response list is maintained*/

			if ((rec->track_flags & SA_TRACK_VALIDATE_STEP) && (node->admin_op == PLM)){
				 rec->inv_id = 0; /*No resp for Aborted Step*/

                                if (rec->track_flags & SA_TRACK_CHANGES_ONLY)
                                        rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_ABORTED,notify_changes_only);
                                else if (rec->track_flags & SA_TRACK_CHANGES)
                                        rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_ABORTED,notify_changes);

                                if (rc != NCSCC_RC_SUCCESS){
                                        TRACE("Sending track callback failed for SA_CLM_CHANGE_ABORTED");
				}

			}else if ((rec->track_flags & SA_TRACK_START_STEP) && ((node->admin_op != PLM) && (node->admin_op != 0))){
				rec->inv_id = 0; /*No resp for Aborted Step*/

                                if (rec->track_flags & SA_TRACK_CHANGES_ONLY)
                                        rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_ABORTED,notify_changes_only);
                                else if (rec->track_flags & SA_TRACK_CHANGES)
                                        rc = clms_prep_and_send_track(cb,node,rec,SA_CLM_CHANGE_ABORTED,notify_changes);

                                if (rc != NCSCC_RC_SUCCESS){
                                        TRACE("Sending track callback failed for SA_CLM_CHANGE_ABORTED");
                                }

			}
		
		}
	}
	TRACE_LEAVE();
}



/**
* Prepares the CLMSV msg to send to clma
* @param[in] node	
* @param[in] client
* @param[in] step	
* @param[in] notification buffer
*/
uns32 clms_prep_and_send_track(CLMS_CB *cb,CLMS_CLUSTER_NODE * node,CLMS_CLIENT_INFO *client,SaClmChangeStepT step, SaClmClusterNotificationT_4 *notify)
{
	CLMSV_MSG *msg = NULL;
	uns32 rc = NCSCC_RC_SUCCESS;

	TRACE_ENTER();	

	msg = (CLMSV_MSG *)malloc(sizeof(CLMSV_MSG));
	if(!msg){
		LOG_ER("Malloc failed for CLMSV_MSG");
		assert(0);
	}

	/* stick the notification buffer into the message */
	memset(msg, 0, sizeof(CLMSV_MSG));

	/* Fill the msg */
	msg->evt_type = CLMSV_CLMS_TO_CLMA_CBK_MSG;
	msg->info.cbk_info.client_id = client->client_id; 
	msg->info.cbk_info.type = CLMSV_TRACK_CBK;
	msg->info.cbk_info.param.track.mem_num  = osaf_cluster->num_nodes;
	msg->info.cbk_info.param.track.err = SA_AIS_OK;
	msg->info.cbk_info.param.track.inv = client->inv_id;

	msg->info.cbk_info.param.track.root_cause_ent = (SaNameT *)malloc(sizeof(SaNameT));

	if (node->admin_op != PLM){	
		msg->info.cbk_info.param.track.root_cause_ent->length = node->node_name.length;
		memcpy(msg->info.cbk_info.param.track.root_cause_ent->value,node->node_name.value,node->node_name.length);
	}else{
		msg->info.cbk_info.param.track.root_cause_ent->length= node->ee_name.length;
		memcpy(msg->info.cbk_info.param.track.root_cause_ent->value,node->ee_name.value,node->ee_name.length);
	}

	msg->info.cbk_info.param.track.cor_ids= (SaNtfCorrelationIdsT *)malloc(sizeof(SaNtfCorrelationIdsT)); /*Not Supported as of now*/
	msg->info.cbk_info.param.track.step = step;

	if (step == SA_CLM_CHANGE_START)
		msg->info.cbk_info.param.track.time_super= node->lck_cbk_timeout;
	else
		msg->info.cbk_info.param.track.time_super= (SaTimeT)SA_TIME_UNKNOWN;

	msg->info.cbk_info.param.track.buf_info.viewNumber = node->init_view; 

	if (client->track_flags & SA_TRACK_CHANGES_ONLY)
		msg->info.cbk_info.param.track.buf_info.numberOfItems = clms_nodedb_lookup(0);
	else if (client->track_flags & SA_TRACK_CHANGES)
		msg->info.cbk_info.param.track.buf_info.numberOfItems = clms_nodedb_lookup(1);
	
	msg->info.cbk_info.param.track.buf_info.notification = (SaClmClusterNotificationT_4 *)malloc(msg->info.cbk_info.param.track.buf_info.numberOfItems*
												sizeof(SaClmClusterNotificationT_4));

	if(!msg->info.cbk_info.param.track.buf_info.notification){
		LOG_ER("Malloc failed for notification");
		assert(0);
	}		
	if(notify != NULL){
		memcpy(msg->info.cbk_info.param.track.buf_info.notification,notify,
				(msg->info.cbk_info.param.track.buf_info.numberOfItems *sizeof(SaClmClusterNotificationT_4)));
	}

	rc = clms_mds_msg_send(cb,msg,&client->mds_dest, NULL,MDS_SEND_PRIORITY_MEDIUM);

	if (rc != NCSCC_RC_SUCCESS){
		TRACE("callback msg send to clma  failed");
	}
	TRACE_LEAVE();
	return rc;
}
/**
* Handles the creation of the new Clm node.This is executed as an IMM callback
*/ 
static SaAisErrorT clms_imm_ccb_obj_create_callback (
			SaImmOiHandleT immOiHandle,
			SaImmOiCcbIdT ccbId,
			const SaImmClassNameT className,
			const SaNameT *parentName,
			const SaImmAttrValuesT_2 **attr
			)
{
	 
	SaAisErrorT rc = SA_AIS_OK;
	uns32 rt;
        CcbUtilCcbData_t *ccb_util_ccb_data;
        CcbUtilOperationData_t *operation = NULL;

        TRACE_ENTER2("CCB ID %llu, class %s, parent %s", ccbId, className, parentName->value);

        if ((ccb_util_ccb_data = ccbutil_getCcbData(ccbId)) != NULL) {
                int i = 0;
                const SaImmAttrValuesT_2 *attrValue;

                operation = ccbutil_ccbAddCreateOperation(ccb_util_ccb_data, className, parentName, attr);

		if(operation == NULL){
			LOG_ER("Failed to get CCB operation object for %llu", ccbId);
	                rc = SA_AIS_ERR_NO_MEMORY;
        	        goto done;
        	}

                /* Find the RDN attribute and store the object DN */
                while ((attrValue = attr[i++]) != NULL) {
			if (!strncmp(attrValue->attrName, "safCluster",10)){
				rc = SA_AIS_ERR_NOT_SUPPORTED;
                                goto done;
			}
			else if (!strncmp(attrValue->attrName, "safNode", 7)) {
				if (attrValue->attrValueType == SA_IMM_ATTR_SASTRINGT) {
	                               	SaStringT rdnVal = *((SaStringT *)attrValue->attrValues[0]);
					if ((parentName != NULL) && (parentName->length > 0)) {
                	                       	operation->objectName.length = (SaUint16T)sprintf((char *)operation->objectName.value,
                        	                                                      "%s,%s", rdnVal, parentName->value);
                               		} else {
						rc = SA_AIS_ERR_BAD_OPERATION;
                               		}
				 } 
				
				TRACE("Operation's object Name %s",operation->objectName.value);

                	}
		}
	}		

	if ((rt = clms_node_dn_chk(&operation->objectName))!= NCSCC_RC_SUCCESS){
		TRACE("Node DN name is incorrect");
                rc =SA_AIS_ERR_INVALID_PARAM;
	}

 done:
	TRACE_LEAVE();
        return rc;
}

/**
* Handles the deletion of the Clm node.This is executed as an IMM callback
*/ 
static SaAisErrorT clms_imm_ccb_obj_delete_callback (
			SaImmOiHandleT immOiHandle,
			SaImmOiCcbIdT ccbId,
			const SaNameT *objectName
			)
{
	SaAisErrorT rc = SA_AIS_OK;
        struct CcbUtilCcbData *ccb_util_ccb_data;
                
        TRACE_ENTER2("CCB ID %llu, %s", ccbId, objectName->value);

        if ((ccb_util_ccb_data = ccbutil_getCcbData(ccbId)) != NULL) {
                /* "memorize the request" */
                ccbutil_ccbAddDeleteOperation(ccb_util_ccb_data, objectName);
        } else {
                TRACE("Failed to get CCB object for %llu", ccbId);
                rc = SA_AIS_ERR_NO_MEMORY;
        }                               
	
	if (!strncmp((const char *)objectName->value,"safCluster=",11)){
		rc = SA_AIS_ERR_NOT_SUPPORTED;
	}

	TRACE_LEAVE();
                          
        return rc;

}

/**
* Handles the node's attribute value modification.This is executed as an IMM callback
*/ 
static SaAisErrorT clms_imm_ccb_obj_modify_callback (
			SaImmOiHandleT immOiHandle,
			SaImmOiCcbIdT ccbId,
			const SaNameT *objectName,
			const SaImmAttrModificationT_2 **attrMods
			)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	         
	TRACE_ENTER2("CCB ID %llu for object-name:%s ",ccbId,objectName->value);

	if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) != NULL) {
		/*memorize the modification request*/
		if (ccbutil_ccbAddModifyOperation(ccbUtilCcbData, objectName, attrMods) != 0){
			LOG_ER("Failed ccb object modify %s", objectName->value);
			rc = SA_AIS_ERR_BAD_OPERATION;
		}
	}else{
		LOG_ER("Failed to get CCB objectfor %llu", ccbId);
		rc = SA_AIS_ERR_NO_MEMORY;
	}

	TRACE_LEAVE();
	return rc;
}

static SaAisErrorT clms_imm_ccb_completed_callback (
			SaImmOiHandleT immOiHandle,
			SaImmOiCcbIdT ccbId
			)
{
	SaAisErrorT rc = SA_AIS_OK;
        CcbUtilOperationData_t *OperData = NULL;

	TRACE_ENTER2("CCB ID %llu", ccbId);


        /* "check that the sequence of change requests contained in the CCB is
           valid and that no errors will be generated when these changes
           are applied." */

        while ((OperData = ccbutil_getNextCcbOp(ccbId,OperData)) != NULL) {
                rc = clms_node_ccb_comp_cb(OperData);

                /* Get out at first error */
                if (rc != SA_AIS_OK)
                        break;
        }
	TRACE_LEAVE();

        return rc;
}


static void clms_imm_ccb_abort_callback (
			SaImmOiHandleT immOiHandle,
			SaImmOiCcbIdT ccbId
			)
{
	struct CcbUtilCcbData *ccbUtilCcbData;

	TRACE_ENTER2("CCB Id %llu",ccbId);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) != NULL)
		ccbutil_deleteCcbData(ccbUtilCcbData);
	else
		LOG_ER("Failed to find CCB object for %llu", ccbId);

	TRACE_LEAVE();
											  
}


SaAisErrorT clms_node_ccb_comp_modify(CcbUtilOperationData_t *opdata)
{
	 SaAisErrorT rc = SA_AIS_OK;
        const SaImmAttrModificationT_2 *attr_mod;
        int i = 0;

        TRACE_ENTER2("%s", opdata->objectName.value);

        while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
                const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
                void *value = attribute->attrValues[0];

                if (!strcmp(attribute->attrName, "saClmNodeDisableReboot")) {
                        SaBoolT val = *((SaBoolT *)value);
                        if (val > SA_TRUE) {
                                rc = SA_AIS_ERR_BAD_OPERATION;
                                goto done;
                        }
                } else if (!strcmp(attribute->attrName, "saClmNodeAddressFamily")) {
				TRACE("Modifictaion of saClmNodeAddressFamily  not allowed");
                                rc = SA_AIS_ERR_NOT_SUPPORTED;
                                goto done;
                } else if (!strcmp(attribute->attrName, "saClmNodeAddress")) {
				TRACE("Modifictaion of saClmNodeAddress  not allowed");
                                rc = SA_AIS_ERR_NOT_SUPPORTED;
                                goto done;
                } else if (!strcmp(attribute->attrName, "saClmNodeLockCallbackTimeout")) {
                        SaTimeT Timeout = *((SaTimeT *)value);
                        if (Timeout == 0) {
                                rc = SA_AIS_ERR_BAD_OPERATION;
                                goto done;
                        }
                } else if (!strcmp(attribute->attrName, "saClmNodeEE")) {

			if (clms_cb->reg_with_plm == SA_FALSE){	
				TRACE("saClmNodeEE1 attribute change doesn't apply when plm in not in model");
                                rc = SA_AIS_ERR_BAD_OPERATION;
                                goto done;
                        }else {
				TRACE("saClmNodeEE1 attribute change is not supported");
				rc = SA_AIS_ERR_NOT_SUPPORTED;
                                goto done;
			}
                } else {
                        TRACE("Unknown attribute %s", attribute->attrName);
                        rc = SA_AIS_ERR_BAD_OPERATION;
                        goto done;
                }
        }

 done:

	TRACE_LEAVE();
        return rc;


}



/**
 * This routine handles all CCB operations on SaClmNode objects.
 * @param[in] opdata  : Ccb Util Oper Data
 * @return SA_AIS_OK/SA_AIS_ERR_BAD_OPERATION 
 */
SaAisErrorT clms_node_ccb_comp_cb(CcbUtilOperationData_t *opdata)
{
        SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	uns32 rt = NCSCC_RC_SUCCESS;
        CLMS_CLUSTER_NODE *node;

        TRACE_ENTER2("'%s', %u", opdata->objectName.value, (unsigned int)opdata->ccbId);

        switch (opdata->operationType) {
        case CCBUTIL_CREATE:
		 if ((rt = clms_node_dn_chk(&opdata->objectName))!= NCSCC_RC_SUCCESS){
                        TRACE("Node DN name is incorrect");
                        goto done;
		}
                
                if ((node = clms_node_new(&opdata->objectName, opdata->param.create.attrValues)) == NULL) {
                        goto done;
                }
                opdata->userData = node;  /* Save for later use in apply */

                break;
        case CCBUTIL_MODIFY:
		if ((rc = clms_node_ccb_comp_modify(opdata)) != SA_AIS_OK)	
			goto done;
                break;
        case CCBUTIL_DELETE:
		/*lookup by objectName in the database*/
		if ((node = clms_node_get_by_name(&opdata->objectName)) == NULL)
			goto done;
		if (node->member == SA_TRUE){
			LOG_ER("Node %s is still a cluster member",opdata->objectName.value);
			goto done;
		}
		
		if (node->admin_state !=  SA_CLM_ADMIN_LOCKED) {
	        	LOG_ER("Node '%s' not locked", opdata->objectName.value);
			goto done;
        	}
                break;
        default:
                assert(0);
                break;
        }

	TRACE_LEAVE();
        return SA_AIS_OK;
 done:

	TRACE_LEAVE();
        return rc;
}

SaAisErrorT clms_node_ccb_apply_modify(CcbUtilOperationData_t *opdata)
{
  	SaAisErrorT rc = SA_AIS_OK;
        const SaImmAttrModificationT_2 *attr_mod;
        CLMS_CLUSTER_NODE *node = NULL;
        int i = 0;

        TRACE_ENTER2("%s", opdata->objectName.value);

	node = clms_node_get_by_name(&opdata->objectName);
	assert(node != NULL);

        while ((attr_mod = opdata->param.modify.attrMods[i++]) != NULL) {
                const SaImmAttrValuesT_2 *attribute = &attr_mod->modAttr;
                void *value = attribute->attrValues[0];

                if (!strcmp(attribute->attrName, "saClmNodeDisableReboot")) {
			node->disable_reboot = *((SaBoolT *)value);
		
                } else if (!strcmp(attribute->attrName, "saClmNodeLockCallbackTimeout")) {
                        node->lck_cbk_timeout  = *((SaTimeT *)value);
		}
		
        }
	node->stat_change = SA_TRUE;
	node->change = SA_CLM_NODE_RECONFIGURED;	
	node->admin_op = IMM_RECONFIGURED;
	/*Send track callback with saClmClusterChangesT= SA_CLM_NODE_RECONFIGURED*/
	clms_send_track(clms_cb,node,SA_CLM_CHANGE_COMPLETED);
	/*Clear admin_op and stat_change*/
	node->admin_op = 0;
	node->stat_change = SA_FALSE; 

	/*Send Notification with ntfid as SA_CLM_NTFID_NODE_RECONFIG*/
	rc = clms_node_reconfigured_ntf(clms_cb,node);
	if(rc != SA_AIS_OK) {
		TRACE("clms_node_reconfigured_ntf failed %u", rc);
	}

	TRACE_LEAVE();
        return rc;

} 



SaAisErrorT clms_node_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
        CLMS_CLUSTER_NODE *node;
	CLMS_CKPT_REC ckpt;
	SaNameT eename;
	SaNameT *entityNames = NULL;
	SaUint32T i = 0,entityNamesNumber = 0;

        
        TRACE_ENTER2("%s, %llu", opdata->objectName.value, opdata->ccbId);
        
        switch (opdata->operationType) {
        case CCBUTIL_CREATE:
                clms_node_add_to_model(opdata->userData);
                /*clms_cluster_update_rattr(osaf_cluster);*/
		/*Checkpointing also need to be done*/
		/* Add to the plm entity group*/
		entityNamesNumber = ncs_patricia_tree_size(&clms_cb->nodes_db);

		memset(&eename, 0, sizeof(SaNameT));
		entityNames = (SaNameT*)malloc(sizeof(SaNameT)*entityNamesNumber);
        	node = clms_node_get_by_eename(&eename);

        	while(node != NULL) {
                	entityNames[i].length = node->ee_name.length;
                	(void)memcpy(entityNames[i].value, node->ee_name.value, entityNames[i].length);
                	node = clms_node_get_by_eename(&node->ee_name);
                	i++;
        	}
		#ifdef ENABLE_AIS_PLM
        	rc = saPlmEntityGroupAdd(clms_cb->ent_group_hdl, entityNames, entityNamesNumber, SA_PLM_GROUP_SINGLE_ENTITY);
        	if(rc != SA_AIS_OK) {
                	LOG_ER("saPlmEntityGroupAdd FAILED rc = %d", rc);
                	return rc;
        	}
		#endif

		/* Need to discuss - if need to send Clmtrack callback with node 
		join from here as node create might not imply node member*/		
	        if (clms_cb->ha_state == SA_AMF_HA_ACTIVE) {

	                memset(&ckpt, 0, sizeof(CLMS_CKPT_REC));
	                ckpt.header.type = CLMS_CKPT_NODE_REC;
	                ckpt.header.num_ckpt_records = 1;
	                ckpt.header.data_len = 1;
			prepare_ckpt_node(&ckpt.param.node_csync_rec, opdata->userData);

	                rc = clms_send_async_update(clms_cb, &ckpt, NCS_MBCSV_ACT_ADD);
	                if (rc != NCSCC_RC_SUCCESS)
	                        TRACE("send_async_update FAILED rc = %u",(unsigned int)rc);
		}
		
                break;
        case CCBUTIL_MODIFY:
                rc = clms_node_ccb_apply_modify(opdata); 
		if ( rc != SA_AIS_OK) 
                	goto done;

                if (clms_cb->ha_state == SA_AMF_HA_ACTIVE) {

                        memset(&ckpt, 0, sizeof(CLMS_CKPT_REC));
                        ckpt.header.type = CLMS_CKPT_NODE_CONFIG_REC;
                        ckpt.header.num_ckpt_records = 1;
                        ckpt.header.data_len = 1;

			node = clms_node_get_by_name(&opdata->objectName);
                        prepare_ckpt_config_node(&ckpt.param.node_config_rec, node);

                        rc = clms_send_async_update(clms_cb, &ckpt, NCS_MBCSV_ACT_ADD);
                        if (rc != NCSCC_RC_SUCCESS)
                                TRACE("send_async_update FAILED rc = %u",(unsigned int)rc);
                }

                break;
        case CCBUTIL_DELETE:
		if ((node = clms_node_get_by_name(&opdata->objectName)) == NULL)
                	goto done;
		clms_node_delete(node,0);
		clms_node_delete(node,1);
		clms_node_delete(node,2);
		clms_cluster_update_rattr(osaf_cluster);
		entityNamesNumber = (SaUint32T)ncs_patricia_tree_size(&clms_cb->nodes_db);
	
		/*Delete it from the plm entity group*/
		memset(&eename, 0, sizeof(SaNameT));
                entityNames = (SaNameT*)malloc(sizeof(SaNameT)*entityNamesNumber);
                node = clms_node_get_by_eename(&eename);

                while(node != NULL) {
                        entityNames[i].length = node->ee_name.length;
                        (void)memcpy(entityNames[i].value, node->ee_name.value, entityNames[i].length);
                        node = clms_node_get_by_eename(&node->ee_name);
                        i++;
                }
		#ifdef ENABLE_AIS_PLM

                rc = saPlmEntityGroupAdd(clms_cb->ent_group_hdl, entityNames, entityNamesNumber, SA_PLM_GROUP_SINGLE_ENTITY);
                if(rc != SA_AIS_OK) {
                        LOG_ER("saPlmEntityGroupAdd FAILED rc = %d", rc);
                        return rc;
                }
		#endif
	
		/*Checkpointing also need to be done*/
                if (clms_cb->ha_state == SA_AMF_HA_ACTIVE) {

                        memset(&ckpt, 0, sizeof(CLMS_CKPT_REC));
                        ckpt.header.type = CLMS_CKPT_NODE_DEL_REC;
                        ckpt.header.num_ckpt_records = 1;
                        ckpt.header.data_len = 1;

                        ckpt.param.node_del_rec.node_name.length = opdata->objectName.length;
			(void)memcpy(ckpt.param.node_del_rec.node_name.value, opdata->objectName.value,
                     		     opdata->objectName.length);
 
                        rc = clms_send_async_update(clms_cb, &ckpt, NCS_MBCSV_ACT_ADD);
                        if (rc != NCSCC_RC_SUCCESS)
                                TRACE("send_async_update FAILED rc = %u",(unsigned int)rc);
                }

                break;
        default:
                assert(0);
                break;
        }

	TRACE_LEAVE();
        return SA_AIS_OK;
 done:
	TRACE_LEAVE();
        return rc;
}

static SaAisErrorT clms_imm_ccb_apply_callback (
                        SaImmOiHandleT immOiHandle,
                        SaImmOiCcbIdT ccbId
                        )
{
        CcbUtilOperationData_t *opdata = NULL;
        SaAisErrorT rc = SA_AIS_OK;

        TRACE_ENTER2("CCB ID %llu", ccbId);

        while ((opdata = ccbutil_getNextCcbOp(ccbId, opdata)) != NULL) {
                        rc = clms_node_ccb_apply_cb(opdata);
                 if (rc != SA_AIS_OK)
                        break;

        }

	TRACE_LEAVE();
        return rc;

}

/**
* Add to the patricia tree
* @param[in] node pointer to cluster node
*/
void clms_node_add_to_model(CLMS_CLUSTER_NODE *node)
{

	TRACE_ENTER();
	CLMS_CLUSTER_NODE * tmp_node = NULL;

	 assert(node != NULL);

	/*If the Node is already added to patricia tree;then ignore it */
	if ((tmp_node = clms_node_get_by_eename(&node->ee_name)) == NULL){	
		if (clms_cb->reg_with_plm == SA_TRUE){
			if(clms_node_add(node,1) != NCSCC_RC_SUCCESS){
				assert(0);
			}
		}
	}

	 /*If Cluster Node already is already added to patricia tree;then ignore it  */
        if ((tmp_node = clms_node_get_by_name(&node->node_name)) == NULL){
		TRACE("Not in patricia tree");

		if (clms_node_add(node,2) != NCSCC_RC_SUCCESS){
			if(clms_node_delete(node,1) != NCSCC_RC_SUCCESS)
				TRACE("patricia tree deleted failed for eename");
			
                	assert(0);
        	}
        }

	TRACE_LEAVE();
}	

static const SaImmOiCallbacksT_2 callbacks = {
        .saImmOiAdminOperationCallback = clms_imm_admin_op_callback,
        .saImmOiCcbAbortCallback = clms_imm_ccb_abort_callback,
        .saImmOiCcbApplyCallback = clms_imm_ccb_apply_callback,
        .saImmOiCcbCompletedCallback = clms_imm_ccb_completed_callback,
        .saImmOiCcbObjectCreateCallback = clms_imm_ccb_obj_create_callback,
        .saImmOiCcbObjectDeleteCallback = clms_imm_ccb_obj_delete_callback,
        .saImmOiCcbObjectModifyCallback = clms_imm_ccb_obj_modify_callback,
        .saImmOiRtAttrUpdateCallback = NULL
};


/**
 * Initialize the OI interface and get a selection object. 
 * @param cb
 * 
 * @return SaAisErrorT
 */
uns32 clms_imm_init(CLMS_CB *cb)
{
        uns32 rc = NCSCC_RC_SUCCESS;
        SaAisErrorT ais_rc = SA_AIS_OK;

        TRACE_ENTER();
        if ((ais_rc = immutil_saImmOiInitialize_2(&cb->immOiHandle, &callbacks, &immVersion))!=SA_AIS_OK){
                LOG_ER("saImmOiInitialize_2 failed %u", rc);
                rc = NCSCC_RC_FAILURE;
                goto done;
        }

        if ((ais_rc = immutil_saImmOiSelectionObjectGet(cb->immOiHandle, &cb->imm_sel_obj))!=SA_AIS_OK){
                LOG_ER("saImmOiSelectionObjectGet failed %u", rc);
                rc = NCSCC_RC_FAILURE;
                goto done;
        }

        TRACE_LEAVE();

done:
        return rc;
}
/**
*  Signal handler on lock timer expiry.
*/
void clms_lock_timer_exp(int signo, siginfo_t *info, void *context)
{
	CLMS_CLUSTER_NODE * node = NULL;
	TRACE_ENTER();

	 /* acquire cb write lock */
	/*m_NCS_LOCK(&clms_cb->lock, NCS_LOCK_WRITE);*/

	node = (CLMS_CLUSTER_NODE *)info->si_value.sival_ptr;

	TRACE("node on which lock was performed %s",node->node_name.value);

	clms_timer_ipc_send(node->node_id);

	/*m_NCS_UNLOCK(&clms_cb->lock, NCS_LOCK_WRITE);*/
	
	
	TRACE_LEAVE();

}
/**
* Send the timer expiry event to the mailbox
* @param[in] node_id  timer expired for admin operation on  node_id 
*/
static void clms_timer_ipc_send(SaClmNodeIdT  node_id)
{
	CLMSV_CLMS_EVT *clmsv_evt;
	uns32 rc = NCSCC_RC_SUCCESS;
	TRACE_ENTER();

	/** allocate an CLMSV_CLMS_EVENT now **/
        if (NULL == (clmsv_evt = calloc(1, sizeof(CLMSV_CLMS_EVT)))) {
                LOG_ER("memory alloc FAILED");
		assert(0);
	}
        

	memset(clmsv_evt,0,sizeof(CLMSV_CLMS_EVT));
        clmsv_evt->type = CLMSV_CLMS_NODE_LOCK_TMR_EXP;
	clmsv_evt->info.tmr_info.node_id = node_id;

	TRACE("clmsv_evt.type %d,node_id %d",clmsv_evt->type,clmsv_evt->info.tmr_info.node_id);

	rc = m_NCS_IPC_SEND(&clms_cb->mbx,clmsv_evt,MDS_SEND_PRIORITY_VERY_HIGH);
	if (rc != NCSCC_RC_SUCCESS){
        	TRACE("IPC send failed %d", rc);
		free(clmsv_evt);
	}

	TRACE_LEAVE();	
}

/**
* This sends the track callback when there are no start step subscribers  
*/
static uns32 clms_lock_send_no_start_cbk(CLMS_CLUSTER_NODE * nodeop)
{
	uns32 rc = NCSCC_RC_SUCCESS; 

	TRACE_ENTER();
	nodeop->change = SA_CLM_NODE_LEFT;
        nodeop->member = SA_FALSE;
        nodeop->stat_change = SA_TRUE;
        nodeop->admin_state = SA_CLM_ADMIN_LOCKED;
        --(osaf_cluster->num_nodes);

        clms_send_track(clms_cb,nodeop,SA_CLM_CHANGE_COMPLETED);

        /*Clear admin_op and stat_change*/
	nodeop->admin_op = 0;
        nodeop->stat_change = SA_FALSE;

        (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_OK);

         /* Send node leave notification*/
         rc = clms_node_admin_state_change_ntf(clms_cb,nodeop,SA_CLM_ADMIN_LOCKED);
         if(rc != NCSCC_RC_SUCCESS) {
        	 TRACE("clms_node_exit_ntf failed %u", rc);
                 goto done;
	}

	rc = clms_node_exit_ntf(clms_cb,nodeop);
	if(rc != NCSCC_RC_SUCCESS) {
        	TRACE("clms_node_exit_ntf failed %u", rc);
                goto done;
	}

done:
	TRACE_LEAVE();
	return rc;

}


/**
* Performs the admin lock operation on the node
*/
uns32 clms_imm_node_lock(CLMS_CLUSTER_NODE * nodeop)
{

        uns32 rc = NCSCC_RC_SUCCESS;
	TRACE_ENTER2("Node name: %s to lock",nodeop->node_name.value);
	
	if(nodeop->admin_state == SA_CLM_ADMIN_LOCKED){
		LOG_ER("Node is already locked");
		nodeop->admin_op = 0;
		rc = NCSCC_RC_FAILURE;
                (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_ERR_NO_OP);
		goto done;
	}

	if (nodeop->node_id == clms_cb->node_id){
		LOG_ER("Lock on active node not allowed");
		nodeop->admin_op = 0;
		rc = NCSCC_RC_FAILURE;
                (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_ERR_NOT_SUPPORTED);
		goto done;
	}

	

        if(nodeop->member == TRUE){

                /*No Subscribers for start step*/
                nodeop->stat_change = SA_TRUE;
                rc = clms_chk_sub_start();

                if (rc == NCSCC_RC_SUCCESS){
			
			TRACE("Clients exist for the start step");
			clms_lock_send_start_cbk(nodeop);
			
                 }else {
			TRACE("No Clients for start step");
			rc = clms_lock_send_no_start_cbk(nodeop);
			if (rc != NCSCC_RC_SUCCESS){
				TRACE("clms_lock_send_no_start_cbk failed");
				goto done;
			}
                }
        } else {
		TRACE("Node is not a member node");
                nodeop->admin_state = SA_CLM_ADMIN_LOCKED;
		nodeop->admin_op = 0;
		nodeop->stat_change = SA_FALSE;	
                (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_OK);

                /*Send Notification*/
		 rc = clms_node_admin_state_change_ntf(clms_cb,nodeop,SA_CLM_ADMIN_LOCKED);
                if(rc != NCSCC_RC_SUCCESS) {
                        TRACE("clms_node_admin_state_change_ntf failed %u", rc);
                        goto done;
                }

        }
done:
	TRACE_LEAVE();
        return rc;
}

/**
* Perform the admin unlock operation on the node                                        
*/
uns32 clms_imm_node_unlock(CLMS_CLUSTER_NODE * nodeop)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	TRACE_ENTER2("Node name %s to unlock",nodeop->node_name.value);
	
	if (nodeop->admin_state == SA_CLM_ADMIN_UNLOCKED){
		LOG_ER("Node is already in an unlocked state");
		nodeop->admin_op = 0;
		(void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_ERR_NO_OP);
		rc = NCSCC_RC_FAILURE;
		goto done;
	}

	if (((nodeop->admin_state == SA_CLM_ADMIN_LOCKED)||
			    (nodeop->admin_state == SA_CLM_ADMIN_SHUTTING_DOWN))){

		if (clms_cb->reg_with_plm == SA_FALSE){
			if(nodeop->nodeup == SA_TRUE){

				/* nodeup true  ==> cluster member*/
        			nodeop->member = SA_TRUE;
        			nodeop->admin_state = SA_CLM_ADMIN_UNLOCKED;
                		nodeop->boot_time = clms_get_SaTime();
        			nodeop->init_view =++(clms_cb->cluster_view_num);
        			nodeop->stat_change = SA_TRUE;
        			nodeop->change = SA_CLM_NODE_JOINED;
				++(osaf_cluster->num_nodes);
				
				/*Send Callback to its clienst*/
				clms_send_track(clms_cb,nodeop,SA_CLM_CHANGE_COMPLETED);

				/*Send node join notification*/
				rc = clms_node_join_ntf(clms_cb,nodeop);
				if(rc != NCSCC_RC_SUCCESS) {
					TRACE("clms_node_join_ntf failed %u", rc);
                			goto done;
				}
			}
		}else {
			#ifdef ENABLE_AIS_PLM
			if((nodeop->ee_red_state == SA_PLM_READINESS_IN_SERVICE) && 
							(nodeop->nodeup == SA_TRUE)){
				/* SA_PLM_READINESS_IN_SERVICE ==> cluster member*/
                                nodeop->member = SA_TRUE;
                                nodeop->admin_state = SA_CLM_ADMIN_UNLOCKED;
                		nodeop->boot_time = clms_get_SaTime();
                                nodeop->init_view =++(clms_cb->cluster_view_num);
                                nodeop->stat_change = SA_TRUE;
                                nodeop->change = SA_CLM_NODE_JOINED;
                                ++(osaf_cluster->num_nodes);

				/*Send Callback to its clients*/
                                clms_send_track(clms_cb,nodeop,SA_CLM_CHANGE_COMPLETED);

				/*Send node join notification*/
				rc = clms_node_join_ntf(clms_cb,nodeop);
				if(rc != NCSCC_RC_SUCCESS) {
					TRACE("clms_node_join_ntf failed %u", rc);
                			goto done;
				}


			}else if(nodeop->ee_red_state != SA_PLM_READINESS_IN_SERVICE){

				nodeop->member = SA_FALSE;
				nodeop->admin_state = SA_CLM_ADMIN_UNLOCKED;

			}	
			#endif
		}
	}
	/*Clear stat_change and admin_op after sending the callback*/
	nodeop->stat_change = SA_FALSE;
	nodeop->admin_op = 0;

	/* Send node join notification*/
        (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_OK);
        rc = clms_node_admin_state_change_ntf(clms_cb,nodeop,SA_CLM_ADMIN_UNLOCKED);
        if(rc != NCSCC_RC_SUCCESS) {
        	TRACE("clms_node_admin_state_change_ntf failed %u", rc);
                goto done;
        }
done:
	TRACE_LEAVE();
	return rc;
}

/**
* Perform the admin shutdown operation on the node
*/
uns32 clms_imm_node_shutdown(CLMS_CLUSTER_NODE * nodeop)
{
	uns32 rc = NCSCC_RC_SUCCESS;
	TRACE_ENTER();
	
	if ((nodeop->admin_state == SA_CLM_ADMIN_LOCKED) || (nodeop->admin_state == SA_CLM_ADMIN_SHUTTING_DOWN)){
		LOG_ER("Node is already in locked or shutting down state");
		nodeop->admin_op = 0;
		rc = NCSCC_RC_FAILURE;
		(void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_ERR_NO_OP);
		goto done;
	}

	if (nodeop->node_id == clms_cb->node_id){
		LOG_ER("Shutdown of an active node not allowed");
		nodeop->admin_op = 0;
		rc = NCSCC_RC_FAILURE;
                (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_ERR_NOT_SUPPORTED);
		goto done;
	}

	if ((nodeop->member == SA_FALSE) && (nodeop->admin_state == SA_CLM_ADMIN_UNLOCKED)){

        	nodeop->admin_state = SA_CLM_ADMIN_LOCKED;
		nodeop->admin_op = 0;
                (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_OK);

	}else {

                rc = clms_chk_sub_start();
                if (rc == NCSCC_RC_SUCCESS){
			nodeop->admin_state = SA_CLM_ADMIN_SHUTTING_DOWN;
                	nodeop->stat_change = SA_TRUE;
			nodeop->change = SA_CLM_NODE_SHUTDOWN;
                	clms_send_track(clms_cb,nodeop,SA_CLM_CHANGE_START);
                }else {
                	nodeop->admin_state = SA_CLM_ADMIN_LOCKED;
                        nodeop->stat_change = SA_TRUE;
                        nodeop->change = SA_CLM_NODE_SHUTDOWN;
                        --(osaf_cluster->num_nodes);

                        clms_send_track(clms_cb,nodeop,SA_CLM_CHANGE_COMPLETED);

			/*Clear Admin_op and stat_change*/
			nodeop->admin_op = 0;
			nodeop->stat_change = SA_FALSE;

                        (void)immutil_saImmOiAdminOperationResult(clms_cb->immOiHandle, nodeop->curr_admin_inv, SA_AIS_OK);

                        rc = clms_node_admin_state_change_ntf(clms_cb,nodeop,SA_CLM_NODE_SHUTDOWN);
                        if(rc != NCSCC_RC_SUCCESS) {
                        	TRACE("clms_node_admin_state_change_ntf failed %u", rc);
                        }
		}
	}
done:
	TRACE_LEAVE();
	return rc;
}


/**
* Start the timer for admin lock operation
*/
static void clms_lock_send_start_cbk(CLMS_CLUSTER_NODE * nodeop)
{
	struct itimerspec timer;
        struct sigevent signal_spec;
        struct sigaction act;
	
        TRACE_ENTER2("Node name: %s to lock",nodeop->node_name.value);

        signal_spec.sigev_notify = SIGEV_SIGNAL;
        signal_spec.sigev_signo = SIGALRM;
        signal_spec.sigev_value.sival_ptr = nodeop;

        act.sa_sigaction= clms_lock_timer_exp;
        act.sa_flags = SA_SIGINFO;

	nodeop->change = SA_CLM_NODE_LEFT;
        nodeop->stat_change = SA_TRUE;
	
	clms_send_track(clms_cb,nodeop,SA_CLM_CHANGE_START);
	if(sigaction(SIGALRM,&act,NULL) != 0){
        	TRACE("Sigaction failed");
		assert(0);
	}
	/*Create the lock Callbck timer*/
	if((timer_create(CLOCK_REALTIME, &signal_spec, &nodeop->lock_timerid)) != 0){
        	TRACE("Creating Lock Timer failed");
		assert(0);
	}
	TRACE("Timer creation successful");	

	/* Assumed that the timer expiration value be specified in nanoseconds*/
	timer.it_value.tv_sec = (__time_t)(nodeop->lck_cbk_timeout/sec_to_nanosec);
	timer.it_value.tv_nsec = (long int)(nodeop->lck_cbk_timeout % sec_to_nanosec); 
	timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_nsec = 0;

        if((timer_settime(nodeop->lock_timerid,0,&timer,NULL)) != 0){
        	TRACE("Setting lock timer value failed");
		assert(0);
	}
	TRACE("Timer started");
	TRACE_LEAVE();
}
