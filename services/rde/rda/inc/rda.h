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
 ******************************************************************************/

/*
  $Header:  $

  MODULE NAME: rda.h

..............................................................................

  DESCRIPTION:  This file declares the RDA functions to interact with RDE

******************************************************************************
*/

#ifndef RDA_H
#define RDA_H

/*
** includes
*/
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "ncs_opt.h"
#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#include "ncs_svd.h"
#include "ncs_hdl_pub.h"
#include "ncssysf_lck.h"
#include "ncsusrbuf.h"
#include "ncssysf_def.h"
#include "ncssysfpool.h"
#include "ncssysf_tmr.h"
#include "ncssysf_mem.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tsk.h"
#include "ncssysf_sem.h"
#include "ncsmib_mem.h"
#include "ncs_mib_pub.h"
#include "ncspatricia.h"
#include "ncs_mds_def.h"

#include "ncs_queue.h"
#include "ncs_scktprm.h"
#include "sysf_lck.h"
#include "ncs_main_papi.h"

/*
**
*/
#include "rda_papi.h"
#include "rde_rda_common.h"


/*
** Structure declarations
*/
typedef struct
{ 
   NCSCONTEXT      task_handle;
   NCS_BOOL        task_terminate;
   PCS_RDA_CB_PTR  callback_ptr;
   uns32           callback_handle;
   int             sockfd;
    
}RDA_CALLBACK_CB;

typedef struct
{ 
    struct sockaddr_un  sock_address;
    
}RDA_CONTROL_BLOCK;



/*
** Exportable Functions
*/
int pcs_rda_reg_callback   (uns32, PCS_RDA_CB_PTR, long *);
int pcs_rda_unreg_callback (long);
int pcs_rda_set_role       (PCS_RDA_ROLE);
int pcs_rda_get_role       (PCS_RDA_ROLE*);

#define RDE_RDA_SHELF_ID    2 /* As per BOM file. To send it to AVM*/

#endif /* RDA_H */

