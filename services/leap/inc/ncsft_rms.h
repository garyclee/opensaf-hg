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

  This module contains structure definitions that provide the interface hooks
  between the generic Fault Tolerant engine and a subscribing backend subhsystem.

******************************************************************************
*/


/*
 * Module Inclusion Control...
 */

#ifndef NCSFT_RMS_H
#define NCSFT_RMS_H

#include "ncsftrms.h"


#if (NCS_RMS == 1)

/* NOTE: The original file content of this file has now been pushed down to  */
/*       the RMS subsystems and partitioned in the following #include *.h    */
/*       files. Include order and dependencies are thus preserved.           */

#include "rms_evts.h"
#include "rmsv1api.h"

#endif


#endif
