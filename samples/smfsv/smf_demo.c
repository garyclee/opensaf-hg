#include <stdio.h>
#include<malloc.h>
#include<string.h>
#include<sys/select.h>
#include <saSmf.h>

#define LABEL1 "init_callback"
#define LABEL2 "init"
#define LABEL3 "callback"
#define LABEL4 "all_pass"

void smfa_cbk (SaSmfHandleT smfHandle, SaInvocationT invocation, SaSmfCallbackScopeIdT scopeId,
		const SaNameT *objectName, SaSmfPhaseT phase, const SaSmfCallbackLabelT *callbackLabel,
		const SaStringT params)
{
	SaAisErrorT rc;
	SaUint8T label[callbackLabel->labelSize + 1];
	SaUint8T obj[objectName->length + 1];

	memcpy(label,callbackLabel->label,callbackLabel->labelSize);
	label[callbackLabel->labelSize] = 0;

	memcpy(obj,objectName->value,objectName->length);
	obj[objectName->length] = 0;
	printf("\nCbk received.hdl: %llu, inv_id: %llu, scope_id: %d, obj_name: %s, phase: %d, label: %s, params: %s\n",
	smfHandle,invocation,scopeId,obj,phase,label,params);
	sleep(1);
	rc = saSmfResponse(smfHandle,invocation,SA_AIS_OK);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfResponse FAILED.\n");
		return;
	}
	printf("\nsaSmfResponse successful.\n");
	sleep(1);
	return;
}

int main()
{
	SaSmfHandleT hdl;
	SaVersionT ver;
	SaSmfCallbacksT cbk;
	SaAisErrorT rc;
	SaSelectionObjectT sel_obj;
	SaSmfCallbackScopeIdT scope_1 = 1;
	SaSmfCallbackScopeIdT scope_2 = 2;
	SaSmfLabelFilterArrayT filter_arr;
	SaSmfLabelFilterT filter_label[2];
	SaSmfLabelFilterArrayT filter_arr_2;
	SaSmfLabelFilterT filter_label_2[2];
	fd_set read_fd;

	ver.releaseCode = 'A';
	ver.majorVersion = 1;
	ver.minorVersion = 1;
	
	cbk.saSmfCampaignCallback = smfa_cbk;

	rc = saSmfInitialize(&hdl,&cbk,&ver);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfInitialize FAILED.\n");
		return 0;
	}
	printf("\nsaSmfInitialize successful. Hdl: %llu.\n",hdl);
	sleep(1);
	
	rc = saSmfSelectionObjectGet(hdl,&sel_obj);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfSelectionObjectGet FAILED.\n");
		return 0;
	}
	printf("\nsaSmfSelectionObjectGet successful. sel_obj: %llu.\n",sel_obj);
	sleep(1);
	
	filter_label[0].filterType = SA_SMF_EXACT_FILTER;
	filter_label[0].filter.labelSize = strlen(LABEL1);
	filter_label[0].filter.label = (SaUint8T *)malloc(filter_label[0].filter.labelSize * sizeof(SaUint8T));
	memcpy(filter_label[0].filter.label,LABEL1,filter_label[0].filter.labelSize);
	
	filter_label[1].filterType = SA_SMF_PREFIX_FILTER;
	filter_label[1].filter.labelSize = strlen(LABEL2);
	filter_label[1].filter.label = (SaUint8T *)malloc(filter_label[1].filter.labelSize * sizeof(SaUint8T));
	memcpy(filter_label[1].filter.label,LABEL2,filter_label[1].filter.labelSize);

	filter_arr.filtersNumber = 2;
	filter_arr.filters = filter_label; 

	rc = saSmfCallbackScopeRegister(hdl,scope_1,&filter_arr);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfCallbackScopeRegister FAILED.\n");
		return 0;
	}
	printf("\nsaSmfCallbackScopeRegister successful. scope_id: %d, filter1: %s, filter2: %s\n",
	scope_1,LABEL1,LABEL2);
	sleep(1);

	filter_label_2[0].filterType = SA_SMF_SUFFIX_FILTER;
	filter_label_2[0].filter.labelSize = strlen(LABEL3);
	filter_label_2[0].filter.label = (SaUint8T *)malloc(filter_label_2[0].filter.labelSize * sizeof(SaUint8T));
	memcpy(filter_label_2[0].filter.label,LABEL3,filter_label_2[0].filter.labelSize);
	
	filter_label_2[1].filterType = SA_SMF_PASS_ALL_FILTER;
	filter_label_2[1].filter.labelSize = strlen(LABEL4);
	filter_label_2[1].filter.label = (SaUint8T *)malloc(filter_label_2[1].filter.labelSize * sizeof(SaUint8T));
	memcpy(filter_label_2[1].filter.label,LABEL4,filter_label_2[1].filter.labelSize);

	filter_arr_2.filtersNumber = 2;
	filter_arr_2.filters = filter_label_2; 
	
	rc = saSmfCallbackScopeRegister(hdl,scope_2,&filter_arr_2);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfCallbackScopeRegister FAILED.\n");
		return 0;
	}
	printf("\nsaSmfCallbackScopeRegister successful. scope_id: %d, filter1: %s, filter2: %s\n",
	scope_2,LABEL3,LABEL4);
	sleep(1);
	
	FD_ZERO(&read_fd);
	FD_SET(sel_obj, &read_fd);

	printf("\nWaiting on the fd to receive the cbk.\n");
	while (select(sel_obj+1 , &read_fd, 0, 0, NULL) > 0 ){
		if (FD_ISSET(sel_obj ,&read_fd)){
			rc = saSmfDispatch(hdl,SA_DISPATCH_BLOCKING);	
			if (SA_AIS_OK != rc){
				printf("\nsaSmfDispatch FAILED.\n");
				return 0;
			}
			FD_CLR(sel_obj , &read_fd);
			FD_SET(sel_obj, &read_fd);
		}
	}

	rc = saSmfCallbackScopeUnregister(hdl,scope_1);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfCallbackScopeUnregister FAILED.\n");
		return 0;
	}
	printf("\nScope id %u unregistered successfully.\n",scope_1);
	sleep(1);
	rc = saSmfCallbackScopeUnregister(hdl,scope_2);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfCallbackScopeUnregister FAILED.\n");
		return 0;
	}
	printf("\nScope id %u unregistered successfully.\n",scope_2);
	sleep(1);

	rc = saSmfFinalize(hdl);
	if (SA_AIS_OK != rc){
		printf("\nsaSmfFinalize FAILED.\n");
	}
	printf("\nsaSmfFinalize successful.\n");
	sleep(1);
	return 0;
}
