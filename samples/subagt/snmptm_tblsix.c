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
..............................................................................

  MODULE NAME: SNMPTM_TBLSIX.C 

..............................................................................

  DESCRIPTION:  Defines the API to add/del SNMPTM TBLSIX entries and the 
                procedure that does the clean-up of SNMPTM tblsix tree.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

/*****************************************************************************
  Name          :  snmptm_create_tblsix_entry

  Description   :  This function creates an entry into SNMPTM TBLSIX table. 
                   Basically it adds a tblsix node to a tblsix patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblsix_key - pointer to the TBLSIX request info struct.
        
  Return Values :  tblsix - Upon successful addition of tblsix entry  
                   NULL   - Upon failure of adding tblsix entry  
    
  Notes         :  
*****************************************************************************/
SNMPTM_TBLSIX *snmptm_create_tblsix_entry(SNMPTM_CB *snmptm,
                                          SNMPTM_TBLSIX_KEY *tblsix_key)
{
   SNMPTM_TBLSIX *tblsix= NULL;

   /* Alloc the memory for TBLSIX entry */
   if ((tblsix = (SNMPTM_TBLSIX*)m_MMGR_ALLOC_SNMPTM_TBLSIX)
        == SNMPTM_TBLSIX_NULL)   
   {
      m_NCS_CONS_PRINTF("\nNot able to alloc the memory for TBLSIX \n");
      return NULL;
   }

   m_NCS_OS_MEMSET((char *)tblsix, '\0', sizeof(SNMPTM_TBLSIX));
   
   /* Copy the key contents to tblsix struct */
   tblsix->tblsix_key.count = tblsix_key->count;

   tblsix->tblsix_pat_node.key_info = (uns8 *)&((tblsix->tblsix_key));    

   /* Add to the tblsix entry/node to tblsix patricia tree */
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_add(&(snmptm->tblsix_tree),
                                                &(tblsix->tblsix_pat_node)))
   {
      m_NCS_CONS_PRINTF("\nNot able add TBLSIX node to TBLSIX tree.\n");

      /* Free the alloc memory of TBLSIX */
      m_MMGR_FREE_SNMPTM_TBLSIX(tblsix);

      return NULL;
   }

   m_NCS_CONS_PRINTF("\n\n ROW created in the TBLSIX, INDEX: %d, DATA: %d, NAME: %s\n", tblsix->tblsix_key.count, tblsix->tblsix_data, (char*)&tblsix->tblsix_name);

   return tblsix;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblsix_entry

  Description   :  This function deletes an entry from TBLSIX table. Basically
                   it deletes a tblsix node from a tblsix patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblsix - pointer to the TBLSIX struct.
        
  Return Values :  Nothing 
    
  Notes         :  
*****************************************************************************/
void snmptm_delete_tblsix_entry(SNMPTM_CB *snmptm,
                                 SNMPTM_TBLSIX *tblsix)
{

   m_NCS_CONS_PRINTF("\n\n ROW deleted in the TBLSIX, INDEX: %d, DATA: %d, NAME: %s\n", tblsix->tblsix_key.count, tblsix->tblsix_data, (char*)&tblsix->tblsix_name);

   /* Delete the tblsix entry from the tblsix patricia tree */
   ncs_patricia_tree_del(&(snmptm->tblsix_tree), &(tblsix->tblsix_pat_node));
  
   /* Free the memory of TBLSIX */
   m_MMGR_FREE_SNMPTM_TBLSIX(tblsix);

   return;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblsix

  Description   :  This function deletes all the tblsix entries (nodes) from
                   tblsix tree and destroys a tblsix tree.

  Arguments     :  snmptm  - Pointer to the SNMPTM CB structure

  Return Values :  

  Notes         :
*****************************************************************************/
void  snmptm_delete_tblsix(SNMPTM_CB *snmptm)
{
   SNMPTM_TBLSIX *tblsix;

   /* Delete all the TBLSIX entries */
   while(SNMPTM_TBLSIX_NULL != (tblsix = (SNMPTM_TBLSIX*)
                        ncs_patricia_tree_getnext(&snmptm->tblsix_tree, 0)))
   {
       /* Delete the node from the tblsix tree */
       ncs_patricia_tree_del(&snmptm->tblsix_tree, &tblsix->tblsix_pat_node);

       /* Free the memory */
       m_MMGR_FREE_SNMPTM_TBLSIX(tblsix);
   }

   /* Destroy TBLSIX tree */
   ncs_patricia_tree_destroy(&snmptm->tblsix_tree);

   return;
}


#endif

