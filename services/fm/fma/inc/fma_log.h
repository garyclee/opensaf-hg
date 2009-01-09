/*****************************************************************************
 *                                                                           *
 * NOTICE TO PROGRAMMERS:  RESTRICTED USE.                                   *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED TO YOU AND YOUR COMPANY UNDER A LICENSE         *
 * AGREEMENT FROM EMERSON, INC.                                              *
 *                                                                           *
 * THE TERMS OF THE LICENSE AGREEMENT RESTRICT THE USE OF THIS SOFTWARE TO   *
 * SPECIFIC PROJECTS ("LICENSEE APPLICATIONS") IDENTIFIED IN THE AGREEMENT.  *
 *                                                                           *
 * IF YOU ARE UNSURE WHETHER YOU ARE AUTHORIZED TO USE THIS SOFTWARE ON YOUR *
 * PROJECT,  PLEASE CHECK WITH YOUR MANAGEMENT.                              *
 *                                                                           *
 *****************************************************************************

 
 ..............................................................................

DESCRIPTION:

******************************************************************************
*/





#define FMA_LOG_VERSION 1

/******************************************************************************
                      Logging offset indices for SE-API
 ******************************************************************************/
typedef enum fma_log_seapi {
   FMA_LOG_SEAPI_CREATE,
   FMA_LOG_SEAPI_DESTROY,
   FMA_LOG_SEAPI_INSTANTIATE,
   FMA_LOG_SEAPI_UNINSTANTIATE,
   FMA_LOG_SEAPI_SUCCESS,
   FMA_LOG_SEAPI_FAILURE,
   FMA_LOG_SEAPI_MAX
} FMA_LOG_SEAPI;


/******************************************************************************
                      Logging offset indices for MDS
 ******************************************************************************/
typedef enum fma_log_mds {
   FMA_LOG_MDS_HDLS_GET,
   FMA_LOG_MDS_REG,
   FMA_LOG_MDS_INSTALL,
   FMA_LOG_MDS_SUBSCRIBE,
   FMA_LOG_MDS_UNREG,
   FMA_LOG_MDS_SEND,
   FMA_LOG_MDS_SENDRSP,
   FMA_LOG_MDS_RCV_CBK,
   FMA_LOG_MDS_CPY_CBK,
   FMA_LOG_MDS_SVEVT_CBK,
   FMA_LOG_MDS_ENC_CBK,
   FMA_LOG_MDS_FLAT_ENC_CBK,
   FMA_LOG_MDS_DEC_CBK,
   FMA_LOG_MDS_FLAT_DEC_CBK,
   FMA_LOG_MDS_SUCCESS,
   FMA_LOG_MDS_FAILURE,
   FMA_LOG_MDS_MAX
} FMA_LOG_MDS;



/******************************************************************************
                        Logging offset indices for LOCK
 ******************************************************************************/
typedef enum fma_log_lock {
   FMA_LOG_LOCK_INIT,
   FMA_LOG_LOCK_FINALIZE,
   FMA_LOG_LOCK_TAKE,
   FMA_LOG_LOCK_GIVE,
   FMA_LOG_LOCK_SUCCESS,
   FMA_LOG_LOCK_FAILURE,
   FMA_LOG_LOCK_MAX
} FMA_LOG_LOCK;


/******************************************************************************
                     Logging offset indices for control block
 ******************************************************************************/
typedef enum fma_log_cb {
   FMA_LOG_CB_CREATE,
   FMA_LOG_CB_DESTROY,
   FMA_LOG_CB_HDL_ASS_CREATE,
   FMA_LOG_CB_HDL_ASS_REMOVE,
   FMA_LOG_CB_RETRIEVE,
   FMA_LOG_CB_RETURN,
   FMA_LOG_CB_SUCCESS,
   FMA_LOG_CB_FAILURE,
   FMA_LOG_CB_MAX
} FMA_LOG_CB;


/******************************************************************************
                    Logging offset indices for selection object
 ******************************************************************************/
typedef enum fma_log_sel_obj {
   FMA_LOG_SEL_OBJ_CREATE,
   FMA_LOG_SEL_OBJ_DESTROY,
   FMA_LOG_SEL_OBJ_IND_SEND,
   FMA_LOG_SEL_OBJ_IND_REM,
   FMA_LOG_SEL_OBJ_SUCCESS,
   FMA_LOG_SEL_OBJ_FAILURE,
   FMA_LOG_SEL_OBJ_MAX
} FMA_LOG_SEL_OBJ;

/******************************************************************************
                       Logging offset indices for  FMA APIs
 ******************************************************************************/
/* ensure that the this ordering matches that of API type definition */
typedef enum fma_log_api {
   FMA_LOG_API_FINALIZE,
   FMA_LOG_API_RESP,
   FMA_LOG_API_INITIALIZE,
   FMA_LOG_API_SEL_OBJ_GET,
   FMA_LOG_API_DISPATCH,
   FMA_LOG_API_NODE_RESET_IND,
   FMA_LOG_API_CAN_SWITCHOVER,
   FMA_LOG_API_NODE_HB_IND,
   FMA_LOG_API_ERR_SA_OK,
   FMA_LOG_API_ERR_SA_LIBRARY,
   FMA_LOG_API_ERR_SA_VERSION,
   FMA_LOG_API_ERR_SA_INVALID_PARAM,
   FMA_LOG_API_ERR_SA_NO_MEMORY,
   FMA_LOG_API_ERR_SA_BAD_HANDLE,
   FMA_LOG_API_ERR_SA_EMPTY_LIST,
   FMA_LOG_API_ERR_FAILED_OPERATION,
   FMA_LOG_API_MAX
} FMA_LOG_API;


/******************************************************************************
                Logging offset indices for FMA handle database
 ******************************************************************************/
typedef enum fma_log_hdl_db {
   FMA_LOG_HDL_DB_CREATE,
   FMA_LOG_HDL_DB_DESTROY,
   FMA_LOG_HDL_DB_REC_CREATE,
   FMA_LOG_HDL_DB_REC_DESTROY,
   FMA_LOG_HDL_DB_REC_ADD,
   FMA_LOG_HDL_DB_REC_CBK_ADD,
   FMA_LOG_HDL_DB_REC_DEL,
   FMA_LOG_HDL_DB_REC_GET,
   FMA_LOG_HDL_DB_SUCCESS,
   FMA_LOG_HDL_DB_FAILURE,
   FMA_LOG_HDL_DB_MAX
} FMA_LOG_HDL_DB;

/******************************************************************************
         Logging Offset Indices for Memfail
 ******************************************************************************/
typedef enum fma_log_mem
{
   FMA_LOG_MBX_EVT_ALLOC,
   FMA_LOG_FMA_FM_MSG_ALLOC,
   FMA_LOG_HDL_REC_ALLOC,
   FMA_LOG_CB_ALLOC,
   FMA_LOG_PEND_CBK_REC_ALLOC,
   FMA_LOG_CBK_INFO_ALLOC,
   FMA_LOG_MEM_ALLOC_SUCCESS,
   FMA_LOG_MEM_ALLOC_FAILURE,
   FMA_LOG_MEM_MAX
}FMA_LOG_MEM;

/******************************************************************************
           Logging Offset Indices for Mailbox
 ******************************************************************************/
typedef enum  fma_log_mbx
{
   FMA_LOG_MBX_CREATE ,
   FMA_LOG_MBX_ATTACH ,
   FMA_LOG_MBX_DESTROY,
   FMA_LOG_MBX_DETACH ,
   FMA_LOG_MBX_SEND   ,
   FMA_LOG_MBX_SUCCESS,
   FMA_LOG_MBX_FAILURE,
   FMA_LOG_MBX_MAX
}FMA_LOG_MBX;

/******************************************************************************
                     Logging offset indices for task
 ******************************************************************************/
typedef enum fma_log_task {
   FMA_LOG_TASK_CREATE,
   FMA_LOG_TASK_START,
   FMA_LOG_TASK_RELEASE,
   FMA_LOG_TASK_SUCCESS,
   FMA_LOG_TASK_FAILURE,
   FMA_LOG_TASK_MAX
} FMA_LOG_TASK;

/******************************************************************************
              Logging offset indices for miscellaneous  FMA events
 ******************************************************************************/
typedef enum fma_log_misc {
   /* keep adding misc logs here */
   FMA_FUNC_ENTRY,
   FMA_INVALID_PARAM,
   FMA_LOG_MISC_MAX
} FMA_LOG_MISC;

/******************************************************************************
              Logging offset indices for callbacks
 ******************************************************************************/
typedef enum fma_log_cbk {
   FMA_LOG_CBK_NODE_RESET_IND,
   FMA_LOG_CBK_SWOVER_REQ,
   FMA_LOG_CBK_SUCCESS,
   FMA_LOG_CBK_FAILURE,
   FMA_LOG_CBK_MAX
} FMA_LOG_CBK;

/******************************************************************************
 Logging offset indices for canned constant strings for the ASCII SPEC
 ******************************************************************************/
typedef enum fma_flex_sets {
   FMA_FC_SEAPI,
   FMA_FC_MDS,
   FMA_FC_LOCK,
   FMA_FC_CB,
   FMA_FC_SEL_OBJ,
   FMA_FC_API,
   FMA_FC_HDL_DB,
   FMA_FC_MEM,
   FMA_FC_MBX,
   FMA_FC_TASK,
   FMA_FC_MISC,
   FMA_FC_CBK
} FMA_FLEX_SETS;

typedef enum fma_log_ids {
   FMA_LID_SEAPI,
   FMA_LID_MDS,
   FMA_LID_LOCK,
   FMA_LID_CB,
   FMA_LID_SEL_OBJ,
   FMA_LID_API,
   FMA_LID_HDL_DB,
   FMA_LID_MEM, 
   FMA_LID_MBX,
   FMA_LID_TASK,
   FMA_LID_MISC,
   FMA_LID_CBK
} FMA_LOG_IDS;



#if ( NCS_FMA_LOG == 1 )

#define m_FMA_LOG_SEAPI(op, st, sev)        fma_log_seapi(op, st, sev)
#define m_FMA_LOG_MDS(op, st, sev)          fma_log_mds(op, st, sev)
#define m_FMA_LOG_LOCK(op, st, sev)         fma_log_lock(op, st, sev)
#define m_FMA_LOG_CB(op, st, sev)           fma_log_cb(op, st, sev)
#define m_FMA_LOG_SEL_OBJ(op, st, sev)      fma_log_sel_obj(op, st, sev)
#define m_FMA_LOG_API(t, s, n, sev)         fma_log_api(t, s, n, sev)
#define m_FMA_LOG_HDL_DB(op, st, hdl, sev)  fma_log_hdl_db(op, st, hdl, sev)
#define m_FMA_LOG_MISC(op, sev,func_name)   fma_log_misc(op, sev,func_name)
#define m_FMA_LOG_MEM(op,st,sev)            fma_log_mem(op,st,sev)
#define m_FMA_LOG_MBX(op,st, sev)           fma_log_mbx(op,st,sev)
#define m_FMA_LOG_TASK(op,st,sev)           fma_log_task(op,st,sev)
#define m_FMA_LOG_CBK(op, st, sev)          fma_log_cbk(op, st, sev)
#define m_FMA_LOG_FUNC_ENTRY(func_name) ncs_logmsg(NCS_SERVICE_ID_FMA, FMA_LID_MISC, FMA_FC_MISC, NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_DEBUG, NCSFL_TYPE_TIC, FMA_FUNC_ENTRY, func_name)

#else /* NCS_FMA_LOG == 0 */

#define m_FMA_LOG_SEAPI(op, st, sev)
#define m_FMA_LOG_MDS(op, st, sev)
#define m_FMA_LOG_LOCK(op, st, sev)
#define m_FMA_LOG_CB(op, st, sev)
#define m_FMA_LOG_SEL_OBJ(op, st, sev)
#define m_FMA_LOG_API(t, s, n, sev)
#define m_FMA_LOG_HDL_DB(op, st, hdl, sev)
#define m_FMA_LOG_MISC(op, sev, func_name)
#define m_FMA_LOG_MEM(op,st,sev)
#define m_FMA_LOG_MBX(op,st, sev)
#define m_FMA_LOG_TASK(op,st,sev)
#define m_FMA_LOG_CBK(op, st, sev)
#define m_FMA_LOG_FUNC_ENTRY(func_name)

#endif


/*****************************************************************************
                       Extern Function Declarations
*****************************************************************************/
#if ( NCS_FMA_LOG == 1 )

EXTERN_C uns32 fma_log_reg (void);
EXTERN_C void  fma_log_dereg (void);

EXTERN_C void fma_log_seapi (FMA_LOG_SEAPI, FMA_LOG_SEAPI, uns8);
EXTERN_C void fma_log_mds (FMA_LOG_MDS, FMA_LOG_MDS, uns8);
EXTERN_C void fma_log_lock (FMA_LOG_LOCK, FMA_LOG_LOCK, uns8);
EXTERN_C void fma_log_cb (FMA_LOG_CB, FMA_LOG_CB, uns8);
EXTERN_C void fma_log_sel_obj (FMA_LOG_SEL_OBJ, FMA_LOG_SEL_OBJ, uns8);
EXTERN_C void fma_log_api (FMA_LOG_API, FMA_LOG_API, const SaNameT *, uns8);
EXTERN_C void fma_log_hdl_db (FMA_LOG_HDL_DB, FMA_LOG_HDL_DB, uns32, uns8);
EXTERN_C void fma_log_misc (FMA_LOG_MISC, uns8,void *);
EXTERN_C void fma_log_mem(FMA_LOG_MEM, FMA_LOG_MEM, uns8);
EXTERN_C void fma_log_mbx(FMA_LOG_MBX, FMA_LOG_MBX, uns8);
EXTERN_C void fma_log_task(FMA_LOG_TASK, FMA_LOG_TASK, uns8);
EXTERN_C void fma_log_cbk(FMA_LOG_CBK, FMA_LOG_CBK, uns8);

#endif

