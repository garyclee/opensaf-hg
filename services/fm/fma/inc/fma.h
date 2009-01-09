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

#include "ncsgl_defs.h"

#include "t_suite.h"
#include "ncsdlib.h"
#include "ncs_saf.h"
#include "ncs_mib.h"
#include "ncs_lib.h"

#if 0
#include "ncs_mds.h"
#endif

#include "mds_papi.h"
#include "ncs_mda_pvt.h"
#include "ncs_edu_pub.h"
#include "ncs_mib_pub.h"
#include "ncsmiblib.h"
#include "ncs_main_pvt.h"

#include "ncs_log.h"
#include "dta_papi.h"

#include "oac_papi.h"
#include "oac_api.h"

#include "srmsv_papi.h"
#include "srma_papi.h"

#include "SaHpi.h"

#include "fm_papi.h"
#include "fma_hdl.h"
#include "fma_mem.h"
#include "fma_evt.h"
#include "fma_log.h"
#include "fma_logstr.h"
#include "fma_mds.h"
#include "fma_cb.h"

EXTERN_C uns32 gl_fma_hdl;
