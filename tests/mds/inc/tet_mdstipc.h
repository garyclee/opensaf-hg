#ifndef _TET_MDSTIPC_H
#define _TET_MDSTIPC_H

#include "ncssysf_tsk.h"
#include "ncssysf_def.h"

typedef struct tet_task{
  NCS_OS_CB      entry;
  void *         arg;
  char *         name;
  int            prio;
  unsigned int   stack_size;
  NCSCONTEXT     t_handle;     
}TET_TASK;

typedef struct tet_subscr{
  NCSMDS_SCOPE_TYPE  scope;
  uint8_t               num_svcs;
  MDS_SVC_ID         *svc_ids;
  EVT_FLTR           evt_map;
}TET_SUBSCR;
typedef struct tet_eventinfo{
  MDS_SVC_ID         ur_svc_id;
  NCSMDS_CHG         event;
  MDS_DEST           dest;
  V_DEST_QA          anc;
  V_DEST_RL          role;
  NODE_ID            node_id;
  PW_ENV_ID          pwe_id;
  MDS_SVC_ID         svc_id;
  MDS_SVC_PVT_SUB_PART_VER         rem_svc_pvt_ver; 
  MDS_HDL                          svc_pwe_hdl;
}TET_EVENT_INFO;
typedef struct tet_svc{
  NCSMDS_CALLBACK_API svc_cb;
  MDS_CLIENT_HDL      yr_svc_hdl;/*the decider*/
  NCSMDS_SCOPE_TYPE   install_scope;
  NCS_BOOL            mds_q_ownership;
  /* return values */
  MDS_DEST            dest;    /*can be Absolute or Virtual*/
  V_DEST_QA           anc;    /*can b on multiple Vdests*/
  NCS_SEL_OBJ         sel_obj;
  /*added later*/
  TET_SUBSCR          subscr; 
  TET_EVENT_INFO      svcevt[100];/*holds 100 subscribed services event info*/
  int                 subscr_count;
  MDS_SVC_ID          svc_id;
  MDS_SVC_PVT_SUB_PART_VER   mds_svc_pvt_ver;
  TET_TASK            task; /* FIX THIS can b multiple tasks */
  NCS_BOOL            got_msg;/* when will u fill this*/
}TET_SVC;

typedef struct tet_pwe{
  MDS_HDL     mds_dest_hdl; /* can be mds_adest_hdl or mds_vdest_hdl */
  PW_ENV_ID   pwe_id;
  /* to b passed to the Service API's*/
  MDS_HDL  mds_pwe_hdl;
}TET_PWE;

typedef struct tet_adest{
  
  /*return values*/
  MDS_DEST    adest;
  /*to be passed to Service API's*/
  MDS_HDL  mds_pwe1_hdl;
  MDS_HDL  mds_adest_hdl;
  /*added by me: should support multiple*/
  TET_SVC     svc[600];     /*change the array size*/
  int         svc_count;   /* how many services r installed in this ADESt */
  TET_PWE     pwe[2000];
  int         pwe_count;   /* how many pwes r installed in this ADEST */
}TET_ADEST;

typedef struct tet_vdest{
  NCS_BOOL       persistent;
  NCS_VDEST_TYPE policy;
  SaNameT        name;
  /*return values*/
  MDS_DEST       vdest;
  /*to be passed to Service API's*/
  MDS_HDL     mds_pwe1_hdl;
  MDS_HDL     mds_vdest_hdl;
  /*added by me: should support multiple */
  TET_SVC        svc[600];  /*Stress : change it to 2500 from 1000*/
  int            svc_count;  /* how many services r installed in this VDEST */
  TET_PWE        pwe[2000];
  int            pwe_count;  /* how many pwes r installed in this VDEST */
}TET_VDEST;

typedef struct mds_tet_msg{
#define TET_MSG_SIZE_MAX  45875200
#define TET_MSG_SIZE_MIN  3000
  uns32 send_len;
  uns32 recvd_len;
  /*char *send_data; this is best approach*/
  /*char *recvd_data;this is best approach*/
  char  send_data[TET_MSG_SIZE_MIN];
  char  recvd_data[TET_MSG_SIZE_MIN];
#if 0
  char  send_data[TET_MSG_SIZE_MAX];
  char  recvd_data[TET_MSG_SIZE_MAX];
#endif
}TET_MDS_MSG;

/*this is for callback receive*/

typedef struct tet_mds_recvd_msg_info{
  
  MDS_SVC_ID             yr_svc_id; /*the service id , the MDS is talking to: 
                                      should b identical to the one specified
                                      by the user at the time of service 
                                      installation*/
  MDS_CLIENT_HDL         yr_svc_hdl; /*its service handle*/
  NCSCONTEXT             msg;       /* message to be delivered*/
  NCS_BOOL               rsp_reqd;  /* TRUE if send is awaiting a response */
  MDS_SYNC_SND_CTXT      msg_ctxt;  /* Valid only if "i_rsp_expected == TRUE"*/
  MDS_DEST               fr_dest;   /* MDS destination on which sending 
                                       service resides   */
  V_DEST_QA              fr_anc;    /* From address anchor in case of VDEST */
  MDS_SVC_ID             fr_svc_id; /* Sender service id   */            
  MDS_DEST               to_dest;   /* MDS destination on which Receiving 
                                       service resides */
  MDS_SVC_ID             to_svc_id; /*Receiver service id*/
  MDS_SEND_PRIORITY_TYPE priority; 
  NODE_ID                node_id;   /*nodeid on which the sender service 
                                      resides */
  MDS_CLIENT_MSG_FORMAT_VER msg_fmt_ver;
  MDS_DIRECT_BUFF        buff;
  uint16_t                  len;
}TET_MDS_RECVD_MSG_INFO;

/********************* GLOBAL variables ********************/
TET_ADEST              gl_tet_adest;
TET_VDEST              gl_tet_vdest[4];/*change it to 6 to run VDS Redundancy: 101 for Stress*/
NCSADA_INFO            ada_info;
NCSVDA_INFO            vda_info;
NCSMDS_INFO            svc_to_mds_info;
TET_EVENT_INFO         gl_event_data;
TET_SVC                gl_tet_svc;
TET_MDS_RECVD_MSG_INFO gl_rcvdmsginfo,gl_direct_rcvmsginfo;
int                    gl_vdest_indx;
MDS_DIRECT_BUFF        direct_buff;

/*Callback Failure*/
int                    gl_COPY_CB_FAIL;
int                    gl_SYS_EVENT_CB_FAIL;
int                    gl_ENC_CB_FAIL;
int                    gl_DEC_CB_FAIL;
int                    gl_ENC_FLAT_CB_FAIL;
int                    gl_DEC_FLAT_CB_FAIL;
int                    gl_RECEIVE_CB_FAIL;
int                    gl_COPY_CB_FAIL;


uns32 ncs_encode_16bit(uint8_t **,uns32);
uint16_t ncs_decode_16bit(uint8_t **);

uns32 tet_mds_svc_callback(NCSMDS_CALLBACK_INFO *);
/******************MDS call back routines *********************************/
uns32 tet_mds_cb_cpy         (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_enc         (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_dec         (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_enc_flat    (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_dec_flat    (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_rcv         (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_svc_event      (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_sys_event      (NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_quiesced_ack(NCSMDS_CALLBACK_INFO *);
uns32 tet_mds_cb_direct_rcv  (NCSMDS_CALLBACK_INFO *);
/*uns32 tet_mds_svc_max   (NCSMDS_CALLBACK_INFO *mds_to_svc_info);*/
/**********************************************************************/
uns32 tet_initialize_setup(NCS_BOOL);
uns32 tet_cleanup_setup(void);

/***************  USER DEFINED WRAPPERS FOR MDS DEST APIs     *************/

uns32 adest_get_handle(void);
uns32 create_pwe_on_adest(MDS_HDL,PW_ENV_ID);
uns32 destroy_pwe_on_adest(MDS_HDL);
uns32 create_vdest(NCS_VDEST_TYPE,MDS_DEST);
uns32 create_named_vdest(NCS_BOOL,NCS_VDEST_TYPE,char *);
MDS_DEST vdest_lookup(char *);
uns32 vdest_change_role(MDS_DEST ,V_DEST_RL);
uns32 destroy_vdest(MDS_DEST);
uns32 destroy_named_vdest(NCS_BOOL,MDS_DEST,char *);
uns32 create_pwe_on_vdest(MDS_HDL,PW_ENV_ID);
uns32 destroy_pwe_on_vdest(MDS_HDL);


/**************     USER DEFINED WRAPPERS FOR MDS SERVICE APIs **************/

uns32 tet_create_task(NCS_OS_CB ,NCSCONTEXT );
int is_adest_sel_obj_found(int );
int is_sel_obj_found(int);
int is_vdest_sel_obj_found(int ,int );
void tet_mds_free_msg(NCSCONTEXT msg_to_be_freed);

uns32 mds_service_install(MDS_HDL ,MDS_SVC_ID ,MDS_SVC_PVT_SUB_PART_VER ,NCSMDS_SCOPE_TYPE,NCS_BOOL,NCS_BOOL);
uns32 mds_service_subscribe(MDS_HDL,MDS_SVC_ID ,NCSMDS_SCOPE_TYPE ,uint8_t ,
                            MDS_SVC_ID *);
uns32 mds_service_redundant_subscribe(MDS_HDL ,MDS_SVC_ID ,
                                      NCSMDS_SCOPE_TYPE ,uint8_t ,MDS_SVC_ID *);
uns32 mds_service_system_subscribe(MDS_HDL ,MDS_SVC_ID ,EVT_FLTR );
uns32 mds_service_cancel_subscription(MDS_HDL ,MDS_SVC_ID ,uint8_t ,
                                      MDS_SVC_ID *);
uns32 mds_service_uninstall(MDS_HDL ,MDS_SVC_ID);

uns32 mds_send_message(MDS_HDL ,MDS_SVC_ID ,NCSCONTEXT ,MDS_SVC_ID ,
                       MDS_SEND_PRIORITY_TYPE ,MDS_SENDTYPES,MDS_DEST ,uns32 ,
                       MDS_DIRECT_BUFF ,uint16_t ,MDS_SYNC_SND_CTXT ,
                       NCSMDS_SCOPE_TYPE ,V_DEST_QA );

/* Non Redundant */
uns32 mds_just_send(MDS_HDL ,MDS_SVC_ID ,MDS_SVC_ID ,MDS_DEST,
                    MDS_SEND_PRIORITY_TYPE,TET_MDS_MSG * );
uns32 mds_send_get_ack(MDS_HDL ,MDS_SVC_ID ,MDS_SVC_ID ,MDS_DEST ,uns32 ,
                       MDS_SEND_PRIORITY_TYPE,TET_MDS_MSG *);
uns32 mds_send_get_response(MDS_HDL ,MDS_SVC_ID ,MDS_SVC_ID ,MDS_DEST ,
                            uns32 ,MDS_SEND_PRIORITY_TYPE,TET_MDS_MSG *);
uns32 mds_send_response(MDS_HDL ,MDS_SVC_ID ,TET_MDS_MSG *);
uns32 mds_sendrsp_getack(MDS_HDL,MDS_SVC_ID ,uns32,TET_MDS_MSG *);
uns32 mds_broadcast_to_svc(MDS_HDL ,MDS_SVC_ID ,MDS_SVC_ID ,
                           NCSMDS_SCOPE_TYPE ,MDS_SEND_PRIORITY_TYPE,
                           TET_MDS_MSG *);


/*Direct Send*/
uns32 mds_direct_send_message(MDS_HDL,MDS_SVC_ID,MDS_SVC_ID,
                              MDS_CLIENT_MSG_FORMAT_VER,MDS_SENDTYPES,
                              MDS_DEST,uns32,MDS_SEND_PRIORITY_TYPE,char *);
uns32 mds_direct_broadcast_message(MDS_HDL ,MDS_SVC_ID ,MDS_SVC_ID ,
                                   MDS_CLIENT_MSG_FORMAT_VER,
                                   MDS_SENDTYPES ,NCSMDS_SCOPE_TYPE,
                                   MDS_SEND_PRIORITY_TYPE);
uns32 mds_direct_response(MDS_HDL ,MDS_SVC_ID, MDS_CLIENT_MSG_FORMAT_VER, MDS_SENDTYPES ,uns32 );

/*General Purpose*/
uns32 mds_service_retrieve(MDS_HDL ,MDS_SVC_ID ,SaDispatchFlagsT );
uns32 mds_query_vdest_for_anchor(MDS_HDL ,MDS_SVC_ID ,MDS_DEST ,MDS_SVC_ID ,
                                 V_DEST_RL );
uns32 mds_query_vdest_for_role(MDS_HDL ,MDS_SVC_ID ,MDS_DEST ,MDS_SVC_ID ,
                               V_DEST_QA );
uns32 mds_service_query_for_pwe(MDS_HDL ,MDS_SVC_ID );
uns32 is_service_on_adest(MDS_HDL ,MDS_SVC_ID );
uns32 change_role(MDS_HDL ,MDS_SVC_ID ,V_DEST_RL );

/************** TEST CASE FUNCTION DECLARATIONS ************/
void tet_adest_rcvr_thread(void);
void tet_adest_all_rcvrack_thread(void);
void tet_change_role_thread(void);
void tet_vdest_rcvr_thread(void); 
void tet_adest_all_chgrole_rcvr_thread(void);
void tet_adest_all_rcvrack_chgrole_thread(void);
void tet_vdest_Srcvr_thread(void); 
void tet_vdest_Arcvr_thread(void);
void tet_destroy_PWE_ADEST_twice_tp(void);
void tet_create_PWE_ADEST_twice_tp(void);
void tet_create_default_PWE_ADEST_tp(void);
void tet_create_PWE_upto_MAX_tp(void);

void tet_create_Nway_VDEST(int);
void tet_create_MxN_VDEST(int);
void tet_create_OAC_VDEST_tp(int);
void tet_destroy_VDEST_twice_tp(int);
void tet_destroy_ACTIVE_MxN_VDEST_tp(int);
void tet_test_PWE_VDEST_tp(int);
void tet_create_default_PWE_VDEST_tp(void);
void tet_chg_standby_to_queisced_tp(int);
void tet_create_PWE_upto_MAX_VDEST(void);
void tet_create_named_VDEST(int); 
void tet_VDS(int);

void tet_svc_install_tp(int);
void tet_svc_unstall_tp(int);
void tet_svc_install_upto_MAX(void);
void tet_svc_subscr_ADEST(int);
void tet_svc_subscr_VDEST(int);

void tet_just_send_tp(int);
void tet_send_ack_tp(int);


void tet_broadcast_to_svc_tp(int);
void tet_send_response_tp(int);
void tet_send_response_ack_tp(int choice);
void tet_send_all_tp(int choice);


void tet_direct_just_send_tp(int);
void tet_direct_send_all_tp(int choice);
void TET_direct_just_send_tp(int choice);
void tet_direct_send_ack_tp(int);
void tet_direct_broadcast_to_svc_tp(int);

void tet_direct_send_response_tp(int);
void tet_direct_send_response_ack_tp(int choice);

void tet_query_pwe_tp(int choice);

/*Multi Process/Node Test Case Functions*/
void tet_just_send_both(int);
void tet_send_ack_both(int);
void tet_send_response_both(int);
void tet_send_resp_ack_both(int);

void tet_direct_just_send_both(int);
void tet_direct_send_ack_both(int);
void tet_direct_send_resp_both(int);
void tet_direct_send_resp_ack_both(int);

void tet_red_just_send_both(int);
void tet_red_send_ack_both(int);
void tet_red_send_resp_both(int);
void tet_red_sendresp_ack_both(int);



void tet_dispatch_blocking_both(int choice);
void msg_priority_conformance(int);
void sequential_delivery(int);
void MXN_redundancy(void);
void all_events(void);
void cb_fail_all_events(void);
void red_all_events(void);
void tet_stress(void);
void tet_flood_both(int choice);
void tet_just_send_MAXSIZE_both(int choice);
void tet_VDS_both(void);
void tet_dummy_both(void);
void tet_message_from_unknown_adest(void);
void tet_svc_subscribe_MAX(void);
void ncs_agents_shut_start(int choice); 
void tet_vdest_install_thread(void);
 void tet_svc_install_upto_MAX(void);
 void tet_vdest_uninstall_thread(void);
 void tet_adest_cancel_thread(void);
 void tet_adest_retrieve_thread(void);
 uns32 tet_initialise_setup(NCS_BOOL fail_no_active_sends); 
 void tet_adest_rcvr_thread(void);
 void tet_adest_rcvr_svc_thread(void);
 void tet_vdest_rcvr_resp_thread(void);
 void tet_vdest_rcvr_thread(void);
 void tet_Dadest_all_rcvr_thread(void);
 void tet_Dadest_all_chgrole_rcvr_thread(void);
 void tet_adest_all_chgrole_rcvr_thread(void);
 void tet_vdest_all_rcvr_thread(void);
 void tet_adest_all_rcvrack_thread(void);
 void tet_adest_all_rcvrack_chgrole_thread(void);
 void tet_Dadest_all_rcvrack_chgrole_thread(void);
 void tet_change_role_thread(void);
 void tet_adest_all_rcvr_thread(void);
 void tet_vdest_Srcvr_thread(void);
 void tet_vdest_Arcvr_thread(void);
 void tet_Dadest_rcvr_thread(void);
 void tet_Dvdest_rcvr_thread(void);
 void tet_Dvdest_rcvr_all_rack_thread(void);
 void tet_Dvdest_rcvr_all_thread(void);
 void tet_Dvdest_rcvr_all_chg_role_thread(void);
 void tet_Dvdest_Srcvr_thread(void);
 void tet_Dvdest_Srcvr_all_thread(void);
 void tet_Dvdest_Arcvr_thread(void);
 void tet_Dvdest_Arcvr_all_thread(void);
 void tet_destroy_PWE_ADEST_twice_tp(void);
 void tet_create_PWE_ADEST_twice_tp(void);
 void tet_create_default_PWE_ADEST_tp(void);
 void tet_create_PWE_upto_MAX_tp(void);
 void tet_create_PWE_upto_MAX_VDEST(void);
 void tet_create_default_PWE_VDEST_tp(void);
 void Print_return_status(uns32 rs); 
void tet_vdest_install_thread(void);
 void tet_svc_install_upto_MAX(void);
 void tet_vdest_uninstall_thread(void);
 void tet_adest_cancel_thread(void);
 void tet_adest_retrieve_thread(void);
 void tet_adest_rcvr_thread(void);
 void tet_adest_rcvr_svc_thread(void);
 void tet_vdest_rcvr_resp_thread(void);
 void tet_vdest_rcvr_thread(void);
 void tet_Dadest_all_rcvr_thread(void);
 void tet_Dadest_all_chgrole_rcvr_thread(void);
 void tet_adest_all_chgrole_rcvr_thread(void);
 void tet_vdest_all_rcvr_thread(void);
 void tet_adest_all_rcvrack_thread(void);
 void tet_adest_all_rcvrack_chgrole_thread(void);
 void tet_Dadest_all_rcvrack_chgrole_thread(void);
 void tet_change_role_thread(void);
 void tet_adest_all_rcvr_thread(void);
 void tet_vdest_Srcvr_thread(void);
 void tet_vdest_Arcvr_thread(void);
 void tet_Dadest_rcvr_thread(void);
 void tet_Dvdest_rcvr_thread(void);
 void tet_Dvdest_rcvr_all_rack_thread(void);
 void tet_Dvdest_rcvr_all_thread(void);
 void tet_Dvdest_rcvr_all_chg_role_thread(void);
 void tet_Dvdest_Srcvr_thread(void);
 void tet_Dvdest_Srcvr_all_thread(void);
 void tet_Dvdest_Arcvr_thread(void);
 void tet_Dvdest_Arcvr_all_thread(void);
 void tet_destroy_PWE_ADEST_twice_tp(void);
 void tet_create_PWE_ADEST_twice_tp(void);
 void tet_create_default_PWE_ADEST_tp(void);
 void tet_create_PWE_upto_MAX_tp(void);
 void tet_create_PWE_upto_MAX_VDEST(void);
 void tet_create_default_PWE_VDEST_tp(void);
uns32 mds_send_get_redack(MDS_HDL mds_hdl,
                          MDS_SVC_ID svc_id,
                          MDS_SVC_ID to_svc,
                          MDS_DEST to_vdest,
                          V_DEST_QA to_anc,
                          uns32 time_to_wait,
                          MDS_SEND_PRIORITY_TYPE priority,
                          TET_MDS_MSG *message);
uns32 mds_send_redrsp_getack(MDS_HDL mds_hdl,
                             MDS_SVC_ID svc_id,
                             uns32 time_to_wait,
                             TET_MDS_MSG *response);
uns32   tet_sync_point(void);

#endif
