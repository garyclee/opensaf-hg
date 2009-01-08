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

*****************************************************************************/

#include "fma_fm_intf.h"

#define FMA_MBX_EVT_NULL ((FMA_MBX_EVT_T *)0)
#define FMA_FM_MSG_NULL ((FMA_FM_MSG *)0)

typedef struct fma_mbx_evt
{
    /* TODO: is thr need to keep fr_dest? can be needed later on */
    MDS_DEST    fr_dest;
    FMA_FM_MSG *msg;
} FMA_MBX_EVT_T;
