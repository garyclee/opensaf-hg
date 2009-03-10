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
 
  DESCRIPTION:This file defines structure for avm_hpi_util.c.
  ..............................................................................

 
******************************************************************************
*/




#ifndef __AVM_HPI_UTIL_H__
#define __AVM_HPI_UTIL_H__ 

/* macro definitions */
#define EPATH_BEGIN_CHAR          '{'
#define EPATH_END_CHAR            '}'
#define EPATH_SEPARATOR_CHAR      ','


/* list of entity path types */
typedef struct
{
   uns8               *etype_str;
   SaHpiEntityTypeT   etype_val;

}HPI_ENTITY_TYPE_LIST;

extern HPI_ENTITY_TYPE_LIST gl_hpi_ent_type_list[];

extern uns32 
hpi_convert_entitypath_string(SaHpiEntityPathT *entity_path, uns8 *ent_path_str);

#endif /* __AVM_HPI_UTIL_H__ */
