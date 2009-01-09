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
  
  This module deals with the creation, accessing and deletion of the health 
  check database on the AVND. It also deals with all the MIB operations
  like get,getnext etc related to the Health Check Table.

..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avnd.h"


/****************************************************************************
  Name          : avnd_hcdb_init
 
  Description   : This routine initializes the healthcheck database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_hcdb_init(AVND_CB *cb)
{
   NCS_PATRICIA_PARAMS params;
   uns32               rc = NCSCC_RC_SUCCESS;

   m_NCS_OS_MEMSET(&params, 0, sizeof(NCS_PATRICIA_PARAMS));

   params.key_size = (sizeof(AVSV_HLT_KEY) -  sizeof(SaUint16T));
   rc = ncs_patricia_tree_init(&cb->hcdb, &params);
   if (NCSCC_RC_SUCCESS == rc)
      m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_CREATE, AVND_LOG_HC_DB_SUCCESS, 
                       0, NCSFL_SEV_INFO);
   else
      m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_CREATE, AVND_LOG_HC_DB_FAILURE, 
                       0, NCSFL_SEV_CRITICAL);

   return rc;
}


/****************************************************************************
  Name          : avnd_hcdb_destroy
 
  Description   : This routine destroys the healthcheck database. It deletes 
                  all the healthcheck records in the database.
 
  Arguments     : cb  - ptr to the AvND control block
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_hcdb_destroy(AVND_CB *cb)
{
   AVND_HC *hc = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* scan & delete each healthcheck record */
   while ( 0 != (hc = 
            (AVND_HC *)ncs_patricia_tree_getnext(&cb->hcdb, (uns8 *)0)) )
   {
      /*AvND is going down, but don't send any async update even for 
      external components, otherwise external components will be deleted
      from ACT.*/
      rc = avnd_hcdb_rec_del(cb, &hc->key);
      if ( NCSCC_RC_SUCCESS != rc ) goto err;
   }

   /* finally destroy patricia tree */
   rc = ncs_patricia_tree_destroy(&cb->hcdb);
   if ( NCSCC_RC_SUCCESS != rc ) goto err;

   m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_DESTROY, AVND_LOG_HC_DB_SUCCESS, 
                    0, NCSFL_SEV_INFO);
   return rc;

err:
   m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_DESTROY, AVND_LOG_HC_DB_FAILURE, 
                    0, NCSFL_SEV_CRITICAL);
   return rc;
}


/****************************************************************************
  Name          : avnd_hcdb_rec_add
 
  Description   : This routine adds a healthcheck record to the healthcheck 
                  database. If a healthcheck record is already present, 
                  nothing is done.
 
  Arguments     : cb   - ptr to the AvND control block
                  info - ptr to the healthcheck params
                  rc   - ptr to the operation result
 
  Return Values : ptr to the healthcheck record, if success
                  0, otherwise
 
  Notes         : None.
******************************************************************************/
AVND_HC *avnd_hcdb_rec_add(AVND_CB *cb, AVND_HC_PARAM *info, uns32 *rc)
{
   AVND_HC *hc = 0;

   *rc = NCSCC_RC_SUCCESS;

   /* verify if this healthcheck is already present in the db */
   if ( 0 != m_AVND_HCDB_REC_GET(cb->hcdb, info->name) )
   {
      *rc = AVND_ERR_DUP_HC;
      goto err;
   }

   /* a fresh healthcheck record... */
   hc = m_MMGR_ALLOC_AVND_HC;
   if (!hc) 
   {
      *rc = AVND_ERR_NO_MEMORY;
      goto err;
   }

   m_NCS_OS_MEMSET(hc, 0, sizeof(AVND_HC));
   
   /* Update the config parameters */
   m_NCS_OS_MEMCPY(&hc->key, &info->name, sizeof(AVSV_HLT_KEY));
   hc->period = info->period;
   hc->max_dur = info->max_duration;
   hc->is_ext = info->is_ext;

   /* Add to the patricia tree */
   hc->tree_node.bit = 0;
   hc->tree_node.key_info = (uns8*)&hc->key;
   *rc = ncs_patricia_tree_add(&cb->hcdb, &hc->tree_node);
   if ( NCSCC_RC_SUCCESS != *rc )
   {
      *rc = AVND_ERR_TREE;
      goto err;
   }
   
   m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_REC_ADD, AVND_LOG_HC_DB_SUCCESS, 
                    &info->name.name, NCSFL_SEV_INFO);
   return hc;

err:
   if (hc) m_MMGR_FREE_AVND_HC(hc);

   m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_REC_ADD, AVND_LOG_HC_DB_FAILURE, 
                    &info->name.name, NCSFL_SEV_CRITICAL);
   return 0;
}


/****************************************************************************
  Name          : avnd_hcdb_rec_del
 
  Description   : This routine deletes a healthcheck record from the 
                  healthcheck database. 
 
  Arguments     : cb     - ptr to the AvND control block
                  hc_key - ptr to the healthcheck key
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avnd_hcdb_rec_del(AVND_CB *cb, AVSV_HLT_KEY *hc_key)
{
   AVND_HC *hc = 0;
   uns32   rc = NCSCC_RC_SUCCESS;

   /* get the healthcheck record */
   hc = m_AVND_HCDB_REC_GET(cb->hcdb, *hc_key);
   if (!hc)
   {
      rc = AVND_ERR_NO_HC;
      goto err;
   }

   /* remove from the patricia tree */
   rc = ncs_patricia_tree_del(&cb->hcdb, &hc->tree_node);
   if ( NCSCC_RC_SUCCESS != rc )
   {
      rc = AVND_ERR_TREE;
      goto err;
   }

   m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_REC_DEL, AVND_LOG_HC_DB_SUCCESS, 
                    &hc_key->name, NCSFL_SEV_INFO);

   /* free the memory */
   m_MMGR_FREE_AVND_HC(hc);

   return rc;

err:
   m_AVND_LOG_HC_DB(AVND_LOG_HC_DB_REC_DEL, AVND_LOG_HC_DB_FAILURE, 
                    &hc_key->name, NCSFL_SEV_CRITICAL);
   return rc;
}



