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

#ifndef LGSV_DEFS_H
#define LGSV_DEFS_H

#include "ncs_util.h"

#define LGA_POOL_ID           1 

#define LGSV_RELEASE_CODE 'A'
#define LGSV_MAJOR_VERSION 2
#define LGSV_MINOR_VERSION 1

/**********************************************************/

/* Macro to validate the version */
#define m_LGA_VER_IS_VALID(ver)   \
   ( (ver->releaseCode == LGSV_RELEASE_CODE) && \
     (ver->majorVersion == LGSV_MAJOR_VERSION || ver->minorVersion == LGSV_MINOR_VERSION))

#define m_LGA_FILL_VERSION(ver) \
         io_version->releaseCode = LGSV_RELEASE_CODE;\
         io_version->majorVersion = LGSV_MAJOR_VERSION;\
         io_version->minorVersion = LGSV_MINOR_VERSION; 

/* Macro to validate the dispatch flags */
#define m_LGA_DISPATCH_FLAG_IS_VALID(flag) \
   ( (SA_DISPATCH_ONE == flag) || \
     (SA_DISPATCH_ALL == flag) || \
     (SA_DISPATCH_BLOCKING == flag) )

#define m_GET_MY_VERSION(ver) \
     ver.releaseCode = 'A';  \
     ver.majorVersion = 0x02;  \
     ver.minorVersion = 0x01;

#define m_GET_MY_VENDOR(vendor) \
     vendor.length=13; \
     m_NCS_OS_MEMSET(&vendor.value,'\0',SA_MAX_NAME_LENGTH); \
     m_NCS_MEMCPY(&vendor.value,(uns8 *)"Motorola Inc.",13);

/** Macro to validate the channel open flags **/
#define m_LGA_LSTR_OPEN_FLAG_IS_VALID(flag) \
     (SA_LOG_STREAM_CREATE     & flag)

#define LGSV_NANOSEC_TO_LEAPTM 10000000

#define m_LGSV_UNS64_TO_VB(param,buffer,value)\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = 8; \
   param->info.i_oct = (uns8 *)buffer; \
   m_NCS_OS_HTONLL_P(param->info.i_oct,value); \

#endif /* LGSV_DEFS_H */
