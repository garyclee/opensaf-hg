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
  This file containts the control block structure of HPL

******************************************************************************
*/
#ifndef HPL_CB_H
#define HPL_CB_H

/* structure for HPI private library information */

typedef struct his_hpl_info
{
   uns32         cb_hdl;     /* CB hdl returned by hdl mngr */
   uns8          pool_id;    /* pool-id used by hdl mngr */
   uns32         prc_id;     /* process identifier */
   MDS_HDL       mds_hdl;    /* mds handle */
   MDS_DEST      hpl_dest;   /* HPL absoulte address */
   HAM_INFO     *ham_inst;   /* list of MDS VDEST information of each HAM Instance */
   NCS_LOCK      cb_lock;    /* Lock for this control Block */
} HPL_CB;


/**************************************************************************
 * function declarations
 */

#endif /* HPL_CB_H */

