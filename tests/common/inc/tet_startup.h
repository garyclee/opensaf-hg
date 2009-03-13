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


/* commented out all files that were included from the view based dirs
   and non public files */
#if 0
#include "gl_defs.h"
#include "ncs_opt.h"
#include "t_suite.h"
#include "ncs_main_papi.h"
#include "ncs_lib.h"
#include "mds_inc.h"
#include "rcp_inc.h"
#include "mda_dl_api.h"
#include "ncs_lim.h"
/*#include "lim.h" */
#include "md_cfg.h"
#include "ncs_mds.h"
#include "ifsvinit.h"

#if (NCS_VDS == 1)
#include "vds_dl_api.h"
#endif

#if (NCS_CLI == 1)
#include "cli.h"
#include "ncs_cli.h"
#endif

#if (NCS_AVA == 1)
#include "ava_dl_api.h"
#endif
#if (NCS_AVD == 1)
#include "avd_dl_api.h"
#endif
#if (NCS_AVND == 1)
#include "avnd_dl_api.h"
#endif

#if (NCS_SPA == 1)
#include "spa_dl_api.h"
#endif
#if (NCS_SPD == 1)
#include "spd_dl_api.h"
#endif
#if (NCS_SPND == 1)
#include "spnd_dl_api.h"
#endif

#if (NCS_GLA == 1)
#include "gla_dl_api.h"
#endif
#if (NCS_GLD == 1)
#include "gld_dl_api.h"
#endif
#if (NCS_GLND == 1)
#include "glnd_dl_api.h"
#endif

#if (NCS_IFA == 1)
#include "ifa_dl_api.h"
#endif
#if (NCS_IFD == 1)
#include "ifd_dl_api.h"
#endif
#if (NCS_IFND == 1)
#include "ifnd_dl_api.h"
#include "ifsvinit.h"
#endif

#if (NCS_FLA == 1)
#include "fla_dl_api.h"
#endif
#if (NCS_FLS == 1)
#include "fls_dl_api.h"
#include "ifsv_logstr.h"    
#endif


#endif /* 0 */


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
