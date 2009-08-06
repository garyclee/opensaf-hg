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

  This module contains data type defs usable throughout the system.
  Inclusion of this module into all C-source modules is strongly
  recommended.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef GL_DEFS_H
#define GL_DEFS_H

#include "ncsgl_defs.h"

#ifndef NCS_CLI
#if (NCS_PC_CLI == 1)
#define NCS_CLI              1	/* PowerCode and Command Line Interpreter */
#else
#define NCS_CLI              0	/* Command Line Interpreter */
#endif
#endif   /* NCS_CLI */

#endif   /* ifndef GL_DEFS_H */
