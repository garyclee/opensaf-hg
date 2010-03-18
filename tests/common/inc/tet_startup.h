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
#include "ncs_mds_def.h"

typedef struct sys_cfg_info {

/* MDS Information  .............*/

   uns32                cluster_id;
   uns32                node_id;
   NCS_BOOL             hub_flag;
   uns32                pcon_id;
   uns32                mds_ip_addr;
   uns32                mds_ifindex;

/* FLAGS     .......................*/

   NCS_BOOL            dirflag;
   

}SYS_CFG_INFO;

#endif
