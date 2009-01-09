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

struct fma_cb;

#define FMA_FM_SYNC_MSG_DEF_TIMEOUT 1000 /* 10 seconds*/

#define FMA_MDS_SUB_PART_VERSION 1

#define FMA_SUBPART_VER_MIN  1
#define FMA_SUBPART_VER_MAX  1

#define FMA_FM_MSG_FMT_VER_1 1

EXTERN_C uns32 fma_mds_reg (struct fma_cb *cb);
EXTERN_C uns32 fma_mds_dereg (struct fma_cb *cb);
EXTERN_C uns32 fma_fm_mds_msg_sync_send(struct fma_cb *cb, FMA_FM_MSG *i_msg, FMA_FM_MSG **o_msg, uns32 timeout);
EXTERN_C uns32 fma_fm_mds_msg_async_send(struct fma_cb *cb, FMA_FM_MSG *msg);
EXTERN_C uns32 fma_mds_callback (NCSMDS_CALLBACK_INFO *info);
EXTERN_C uns32 fma_fm_mds_msg_bcast(struct fma_cb *cb, FMA_FM_MSG *msg);
