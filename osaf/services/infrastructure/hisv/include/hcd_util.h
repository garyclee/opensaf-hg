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

  This is the include file for HCD utilities (entity path string to entity
  path structure conversion function)

******************************************************************************
*/

#ifndef HCD_UTIL_H
#define HCD_UTIL_H

#include "hpl_msg.h"

/* macro definitions */
#define EPATH_BEGIN_CHAR          '{'
#define EPATH_END_CHAR            '}'
#define EPATH_SEPARATOR_CHAR      ','

#define MAX_HPI_ENTITY_TYPE        61

/**************************************************************************
 * his_hsm
 *
 * Name     : structure of domain arguments.
 */

typedef struct hpi_session_args
{
   SaHpiDomainIdT   domain_id;
   SaHpiSessionIdT  session_id;
   SaHpiRptEntryT   entry;
   uns32            chassis_id;
   uns32            discover_domain_err;
   uns32            rediscover;              /* re-discover the resources after failover  */
   uns32            session_valid;           /* track validity of a session */
} HPI_SESSION_ARGS;


/* list of entity path types */
typedef struct
{
   char *etype_str;
   SaHpiEntityTypeT   etype_val;

}ENTITY_TYPE_LIST;


/**************************************************************************
 * Extern function declarations
 */
EXTERN_C uns32  string_to_epath (uns8 *epathstr, uns32 epath_len,
                                   SaHpiEntityPathT *epathptr);
EXTERN_C uns32 get_chassis_id(SaHpiEntityPathT *epath, int32 *chassis_id);
EXTERN_C uns32 print_hotswap(SaHpiHsStateT cur_state, SaHpiHsStateT prev_state, 
                             uns32 board_num, SaHpiEntityTypeT type);
EXTERN_C uns32 print_invdata(HISV_INV_DATA *inv_data);
EXTERN_C int hpi_decode_6bitpack(unsigned char *inbuf, unsigned char inlen, unsigned char *outbuf);
EXTERN_C int hpi_decode_bcd_plus(unsigned char *inbuff, unsigned char inlen, unsigned char *outbuff);
EXTERN_C int hpi_decode_to_ascii(SaHpiTextTypeT data_type, unsigned char *inbuf,
                    unsigned char inlen, unsigned char *outbuf);

#endif /* !HCD_UTIL_H */
