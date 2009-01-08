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

  This file contains healthcheck database related definitions.
  
******************************************************************************
*/

#ifndef AVND_HC_H
#define AVND_HC_H

typedef AVSV_HLT_INFO_MSG  AVND_HC_PARAM;

typedef struct avnd_hc_tag {
   NCS_PATRICIA_NODE    tree_node;   /* index is hc-key */
   AVSV_HLT_KEY         key;         /* healthcheck table index */
   SaTimeT              period;      /* periodicity value */
   SaTimeT              max_dur;     /* max duration value */
   NCS_BOOL             is_ext;      /* Whether it is for ext comp*/
   NCS_BOOL             rcvd_on_fover; /* Temporary flag to find out whether
                                        * update is received in the f-over message*/
} AVND_HC;


/* macro to get a healthcheck record from healthcheck-db */
#define m_AVND_HCDB_REC_GET(hcdb, hc_key) \
           (AVND_HC *)ncs_patricia_tree_get(&(hcdb), (uns8 *)&(hc_key))


/* Extern function declarations */
EXTERN_C uns32 avnd_hcdb_init(struct avnd_cb_tag *);
EXTERN_C uns32 avnd_hcdb_destroy(struct avnd_cb_tag *);
EXTERN_C AVND_HC *avnd_hcdb_rec_add(struct avnd_cb_tag *, AVND_HC_PARAM *, uns32 *);
EXTERN_C uns32 avnd_hcdb_rec_del(struct avnd_cb_tag *, AVSV_HLT_KEY *);
#endif /* !AVND_HC_H */
