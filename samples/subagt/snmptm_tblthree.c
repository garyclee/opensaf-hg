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

  MODULE NAME: SNMPTM_TBLTHREE.C 

..............................................................................

  DESCRIPTION:  Defines the API to add/del SNMPTM TBLTHREE entries and the 
                procedure that does the clean-up of SNMPTM tblthree tree.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

/*****************************************************************************
  Name          :  snmptm_create_tblthree_entry

  Description   :  This function creates an entry into SNMPTM TBLTHREE table. 
                   Basically it adds a tblthree node to a tblthree patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblthree_key - pointer to the TBLTHREE request info struct.
        
  Return Values :  tblthree - Upon successful addition of tblthree entry  
                   NULL   - Upon failure of adding tblthree entry  
    
  Notes         :  
*****************************************************************************/
SNMPTM_TBLTHREE *snmptm_create_tblthree_entry(SNMPTM_CB *snmptm,
                                          SNMPTM_TBL_KEY *tblthree_key)
{

   SNMPTM_TBLTHREE *tblthree = NULL;

   /* Alloc the memory for TBLTHREE entry */
   if ((tblthree = (SNMPTM_TBLTHREE*)m_MMGR_ALLOC_SNMPTM_TBLTHREE)
        == SNMPTM_TBLTHREE_NULL)   
   {
      printf("\nNot able to alloc the memory for TBLTHREE \n");
      return NULL;
   }

   memset((char *)tblthree, '\0', sizeof(SNMPTM_TBLTHREE));
   
   /* Copy the key contents to tblthree struct */
   tblthree->tblthree_key.ip_addr = tblthree_key->ip_addr;

   tblthree->tblthree_pat_node.key_info = (uns8 *)&((tblthree->tblthree_key));    

   /* Add to the tblthree entry/node to tblthree patricia tree */
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_add(&(snmptm->tblthree_tree),
                                                &(tblthree->tblthree_pat_node)))
   {
      printf("\nNot able add TBLTHREE node to TBLTHREE tree.\n");

      /* Free the alloc memory of TBLTHREE */
      m_MMGR_FREE_SNMPTM_TBLTHREE(tblthree);

      return NULL;
   }

   tblthree->tblthree_row_status = NCSMIB_ROWSTATUS_ACTIVE; 

   return tblthree;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblthree_entry

  Description   :  This function deletes an entry from TBLTHREE table. Basically
                   it deletes a tblthree node from a tblthree patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblthree - pointer to the TBLTHREE struct.
        
  Return Values :  Nothing 
    
  Notes         :  
*****************************************************************************/
void snmptm_delete_tblthree_entry(SNMPTM_CB *snmptm,
                                SNMPTM_TBLTHREE *tblthree)
{

   /* Delete the tblthree entry from the tblthree patricia tree */
   ncs_patricia_tree_del(&(snmptm->tblthree_tree), &(tblthree->tblthree_pat_node));
  
   /* Free the memory of TBLTHREE */
   m_MMGR_FREE_SNMPTM_TBLTHREE(tblthree);

   return;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblthree

  Description   :  This function deletes all the tblthree entries (nodes) from
                   tblthree tree and destroys a tblthree tree.

  Arguments     :  snmptm  - Pointer to the SNMPTM CB structure

  Return Values :  

  Notes         :
*****************************************************************************/
void  snmptm_delete_tblthree(SNMPTM_CB *snmptm)
{
   SNMPTM_TBLTHREE *tblthree;

   /* Delete all the TBLTHREE entries */
   while(SNMPTM_TBLTHREE_NULL != (tblthree = (SNMPTM_TBLTHREE*)
                        ncs_patricia_tree_getnext(&snmptm->tblthree_tree, 0)))
   {
       /* Delete the node from the tblthree tree */
       ncs_patricia_tree_del(&snmptm->tblthree_tree, &tblthree->tblthree_pat_node);

       /* Free the memory */
       m_MMGR_FREE_SNMPTM_TBLTHREE(tblthree);
   }

   /* Destroy TBLTHREE tree */
   ncs_patricia_tree_destroy(&snmptm->tblthree_tree);

   return;
}


#endif

