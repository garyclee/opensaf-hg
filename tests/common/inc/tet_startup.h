#ifndef _TET_STARTUP_H
#define _TET_STARTUP_H

/* system files ...*/



/* tet ware files ... */

#include "tet_api.h"

/* ncs_files .... */
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"
#include "ncssysf_ipc.h"
#include "ncssysf_tmr.h"

typedef struct sys_cfg_info {

/* MDS Information  .............*/

   uint32_t                cluster_id;
   uint32_t                node_id;
   bool             hub_flag;
   uint32_t                pcon_id;
   uint32_t                mds_ip_addr;
   uint32_t                mds_ifindex;

/* FLAGS     .......................*/

   bool            dirflag;
   

}SYS_CFG_INFO;

#endif
