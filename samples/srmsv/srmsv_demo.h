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
 */

/*****************************************************************************

  DESCRIPTION:

  This file contains extern declarations for SRMSv toolkit application.
  
******************************************************************************
*/

#ifndef SRMSV_DEMO_H
#define SRMSV_DEMO_H

/* Top level routine to start SRMSv demo */
EXTERN_C uns32 ncs_srmsv_init(void);
EXTERN_C uns32 srmst_file_parser(char *in_file, char *out_string, uns16 line);
EXTERN_C uns32 srmst_str_parser(char *in_string,
                                char *in_split_pattern,
                                char *out_string,
                                uns16 num);
#endif /* !SRMSV_DEMO_H */

