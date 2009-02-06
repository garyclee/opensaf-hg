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

#include <configmake.h>

/*****************************************************************************
..............................................................................

..............................................................................

  DESCRIPTION:
  This file containts the structures and prototype declarations usesful for serv 
  ices to interface with nodeinitd. These definitions are used 
  both by nodeini  td and blladeinit API.

******************************************************************************
*/

#ifndef NID_API_H
#define NID_API_H

/*#include "ncs_opt.h"*/
#include "ncsgl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_scktprm.h"
#include "ncssysf_tsk.h"


/* API Error Codes */
#define NID_INV_PARAM    11
#define NID_OFIFO_ERR    22
#define NID_WFIFO_ERR    33

#define NID_MAGIC       0xAAB49DAA

#define NID_MAXSNAME                 15

#define NODE_HA_STATE  OSAF_LOCALSTATEDIR "node_ha_state"


/****************************************************************
 *       Message format used by nodeinitd and spawned services  *
 *       for communicating initialization status.               *
 ****************************************************************/
typedef struct nid_fifo_msg {
   uns32 nid_magic_no;   /* Magic number */
    uns8  nid_serv_name[NID_MAXSNAME];  /* Identifies the spawned service uniquely*/
   uns32 nid_stat_code;  /* Identifies the initialization status */
}NID_FIFO_MSG;

/**********************************************************************
 *    Exported finctions by NID_API                                    *
 **********************************************************************/
EXTERN_C LEAPDLL_API uns32 nid_notify(char *, uns32, uns32 *);  
EXTERN_C LEAPDLL_API uns32 nis_notify(uns8 *, uns32 *);  
EXTERN_C uns32 nid_create_ipc(uns8 * );
EXTERN_C uns32 nid_open_ipc(int32 *fd, uns8 * );
EXTERN_C void nid_close_ipc(void);
EXTERN_C uns32 nid_is_ipcopen(void);
#endif /*NID_API_H*/


