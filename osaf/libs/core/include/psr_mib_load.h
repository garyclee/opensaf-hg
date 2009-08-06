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
..............................................................................


..............................................................................

  DESCRIPTION:

 _Public_ Persistent Store & Restore service (PSSv) abstractions and 
 function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef PSR_MIB_LOAD_H
#define PSR_MIB_LOAD_H

#ifdef  __cplusplus
extern "C" {
#endif
/* include required files */ 
#include "ncsgl_defs.h"
#include "ncsmiblib.h"

/* type of operations */ 
typedef enum ncspss_ss_op
{
   /* to load the information about a particular MIB table into PSS */ 
   NCSPSS_SS_OP_TBL_LOAD,

   /* to unload the information of a particular table from PSS */ 
   NCSPSS_SS_OP_TBL_UNLOAD, 

   NCSPSS_SS_OP_MAX
} NCSPSS_SS_OP;

typedef struct ncspss_tbl_arg_info_tag
{
   NCSMIB_OBJ_INFO *mib_tbl; /* information about this particular table */
   uns16           *objects_local_to_tbl; /* Set only for Augmented tables. First location would be
                                             the number of local objects. */
}NCSPSS_TBL_ARG_INFO;

/* 
 * Information to be given to PSS 
 */
typedef struct ncspss_ss_arg
{
   NCSPSS_SS_OP       i_op;   /* type of the operation */ 
   NCSCONTEXT          i_cntxt; /* context information */ 
   union 
   {
      uns32               tbl_id; /* table-id defined in ncs_mtbl.h or other */ 
      NCSPSS_TBL_ARG_INFO tbl_info;
   } info;
} NCSPSS_SS_ARG;

/*
 * Enrty point to talk to PSS for the following operations 
 * 1. Load the table information 
 * 2. Unload the table information 
 */
EXTERN_C MABPSR_API uns32 ncspss_ss(NCSPSS_SS_ARG*);

typedef uns32 (*NCSPSS_SS_FUNC)(NCSPSS_SS_ARG*);  /* Function prototype */


#ifdef  __cplusplus
}
#endif


#endif /* PSR_MIB_LOAD_H */
