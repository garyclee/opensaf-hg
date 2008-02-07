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

 MODULE NAME:  CLI_OPT.H

$Header: /ncs/software/leap/targsvcs/products/cli/cli_opt.h 6     4/17/01 9:19p Cmohana $
..............................................................................

  DESCRIPTION:

  
  (CLI).

******************************************************************************
*/

#ifndef CLI_OPT_H
#define CLI_OPT_H


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                            Compiler Options 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* This flag enables/disables logging in CLI */

#ifndef NCSCLI_LOG
#define NCSCLI_LOG 0
#endif


/* This flag is used to enable reading of script file */

#ifndef NCSCLI_FILE
#define NCSCLI_FILE  0
#endif

/*
 * NCSCLI_DEBUG
 *
 * This flag turns on some checks that should never need testing if all is
 * right in the world. If a check yield a failure, the macro m_CLI_DBG_SINK
 * is invoked. This user defined macro can do whatever you want. The default
 * is to print the file and line of the offense. You could put a breakpoint 
 * there to catch MDS in the act.
 */

#ifndef NCSCLI_DEBUG
#define NCSCLI_DEBUG        1
#endif

#endif
