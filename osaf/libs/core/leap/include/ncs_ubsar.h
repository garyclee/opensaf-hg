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

  This module contains declarations that facilitate USRSR usage. These uses
  were needed and developed to provide functionality for segmentation and
  Reassembly of streams.
 
  NOTE: 
   This include and its corresponding ubuf_aid.c are put 'above' the sig
   directory since these services are not particular to any one sub-system.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_UBSAR_H
#define NCS_UBSAR_H

#include "ncs_ubsar_pub.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
               Private functions used by UBSAR
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

EXTERN_C uns32 ubsar_init(NCS_UBSAR_INIT *init);
EXTERN_C uns32 ubsar_segment(NCS_UBSAR_SEGMENT *segment);
EXTERN_C uns32 ubsar_assemble(NCS_UBSAR_ASSEMBLE *assemble);
EXTERN_C uns32 ubsar_get_app_trlr(NCS_UBSAR_TRLR *trlr);
EXTERN_C uns32 ubsar_destroy(NCS_UBSAR_DESTROY *destroy);

#endif
