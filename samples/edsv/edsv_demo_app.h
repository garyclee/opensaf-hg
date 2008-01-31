/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation 
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php 
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *   
 */

/*****************************************************************************
  DESCRIPTION:

  This file contains extern declarations for AvSv toolkit application.
  
******************************************************************************
*/

#ifndef EDSV_DEMO_APP_H
#define EDSV_DEMO_APP_H

/* Common header files */
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncs_svd.h"
#include "ncssysfpool.h"
#include "ncssysf_def.h"
#include "ncssysf_tsk.h"

/* SAF header files */
#include "saAis.h"
#include "saEvt.h"

#define NCS_SERVICE_ID_EDSVTM  (UD_SERVICE_ID_END + 2)

typedef enum
{
   NCS_SERVICE_EDSVTM_SUB_ID_SNMPTM_EVT_DATA = 1,
   NCS_SERVICE_EDSVTM_SUB_ID_EVT_PAT_ARRAY,
   NCS_SERVICE_EDSVTM_SUB_ID_EVT_PATTERNS,
   NCS_SERVICE_SNMPTM_SUB_ID_MAX
} NCS_SERVICE_EDSVTM_SUB_ID;


/* Top level routine to run EDSv demo */
EXTERN_C uns32 ncs_edsv_run(void);

#define m_MMGR_ALLOC_EDSVTM_EVENT_DATA(size)  m_NCS_MEM_ALLOC( \
                                            size, \
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_EDSVTM, \
                                            NCS_SERVICE_EDSVTM_SUB_ID_SNMPTM_EVT_DATA)

#define m_MMGR_FREE_EDSVTM_EVENT_DATA(p)  m_NCS_MEM_FREE(p, \
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_EDSVTM, \
                                            NCS_SERVICE_EDSVTM_SUB_ID_SNMPTM_EVT_DATA)

#define m_MMGR_ALLOC_EDSVTM_EVENT_PATTERN_ARRAY (SaEvtEventPatternArrayT *) \
                                             m_NCS_MEM_ALLOC(sizeof(SaEvtEventPatternArrayT), \
                                                       NCS_MEM_REGION_PERSISTENT, \
                                                       NCS_SERVICE_ID_EDSVTM, \
                                                       NCS_SERVICE_EDSVTM_SUB_ID_EVT_PAT_ARRAY)

#define m_MMGR_FREE_EDSVTM_EVENT_PATTERN_ARRAY(p)   m_NCS_MEM_FREE(p, \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_EDSVTM, \
                                                NCS_SERVICE_EDSVTM_SUB_ID_EVT_PAT_ARRAY)

#define m_MMGR_ALLOC_EDSVTM_EVENT_PATTERNS(n)  (SaEvtEventPatternT *) \
                                             m_NCS_MEM_ALLOC((n * sizeof(SaEvtEventPatternT)), \
                                                       NCS_MEM_REGION_PERSISTENT, \
                                                       NCS_SERVICE_ID_EDSVTM, \
                                                       NCS_SERVICE_EDSVTM_SUB_ID_EVT_PATTERNS)

#define m_MMGR_FREE_EDSVTM_EVENT_PATTERNS(p)        m_NCS_MEM_FREE(p, \
                                                       NCS_MEM_REGION_PERSISTENT, \
                                                       NCS_SERVICE_ID_EDSVTM, \
                                                       NCS_SERVICE_EDSVTM_SUB_ID_EVT_PATTERNS)

#endif /* !EDSV_DEMO_APP_H */

