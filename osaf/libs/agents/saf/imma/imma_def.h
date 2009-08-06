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
 * Author(s): Ericsson AB
 *
 */

#ifndef IMMA_DEF_H
#define IMMA_DEF_H

/* Macros for Validating Version */
#define IMMA_RELEASE_CODE 'A'
#define IMMA_MAJOR_VERSION 0x02
#define IMMA_MINOR_VERSION 0x01

#define m_IMMA_VER_IS_VALID(ver) \
   ((ver->releaseCode == IMMA_RELEASE_CODE) && \
    (ver->majorVersion == IMMA_MAJOR_VERSION || ver->majorVersion == 0xff) && \
     (ver->minorVersion == IMMA_MINOR_VERSION || ver->minorVersion == 0xff) )

#define IMMSV_WAIT_TIME  1000   /* MDS wait time in case of syncronous call */ 

//#define m_IMMSV_CONVERT_SATIME_TEN_MILLI_SEC(t)      (t)/(10000000) /* 10^7 */

#endif
