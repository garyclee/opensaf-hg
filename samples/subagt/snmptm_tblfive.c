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

  MODULE NAME: SNMPTM_TBLFIVE.C 

..............................................................................

  DESCRIPTION:  Defines the API to add/del SNMPTM TBLFIVE entries and the 
                procedure that does the clean-up of SNMPTM tblfive tree.
..............................................................................


******************************************************************************/

#include "snmptm.h"

#if (NCS_SNMPTM == 1)

/*****************************************************************************
  Name          :  snmptm_create_tblfive_entry

  Description   :  This function creates an entry into SNMPTM TBLFIVE table. 
                   Basically it adds a tblfive node to a tblfive patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblfive_key - pointer to the TBLFIVE request info struct.
        
  Return Values :  tblfive - Upon successful addition of tblfive entry  
                   NULL   - Upon failure of adding tblfive entry  
    
  Notes         :  
*****************************************************************************/
SNMPTM_TBLFIVE *snmptm_create_tblfive_entry(SNMPTM_CB *snmptm,
                                            SNMPTM_TBL_KEY *tblfive_key)
{
   SNMPTM_TBLFIVE *tblfive = NULL;
   uns32 ip_addr = 0;

   /* Alloc the memory for TBLFIVE entry */
   if ((tblfive = (SNMPTM_TBLFIVE*)m_MMGR_ALLOC_SNMPTM_TBLFIVE)
        == SNMPTM_TBLFIVE_NULL)   
   {
      m_NCS_CONS_PRINTF("\nNot able to alloc the memory for TBLFIVE \n");
      return NULL;
   }

   m_NCS_OS_MEMSET((char *)tblfive, '\0', sizeof(SNMPTM_TBLFIVE));
   
   /* Copy the key contents to tblfive struct */
   tblfive->tblfive_key.ip_addr = tblfive_key->ip_addr;

   tblfive->tblfive_pat_node.key_info = (uns8 *)&((tblfive->tblfive_key));    

   /* Add to the tblfive entry/node to tblfive patricia tree */
   if(NCSCC_RC_SUCCESS != ncs_patricia_tree_add(&(snmptm->tblfive_tree),
                                                &(tblfive->tblfive_pat_node)))
   {
      m_NCS_CONS_PRINTF("\nNot able add TBLFIVE node to TBLFIVE tree.\n");

      /* Free the alloc memory of TBLFIVE */
      m_MMGR_FREE_SNMPTM_TBLFIVE(tblfive);

      return NULL;
   }

   tblfive->tblfive_row_status = NCSMIB_ROWSTATUS_ACTIVE; 

   ip_addr = ntohl(tblfive_key->ip_addr.info.v4);
   m_NCS_CONS_PRINTF("\n\n ROW created in the TBLFIVE, INDEX: %d.%d.%d.%d", (uns8)(ip_addr >> 24),
                                            (uns8)(ip_addr >> 16),
                                            (uns8)(ip_addr >> 8),
                                            (uns8)(ip_addr)); 

   return tblfive;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblfive_entry

  Description   :  This function deletes an entry from TBLFIVE table. Basically
                   it deletes a tblfive node from a tblfive patricia tree.

  Arguments     :  snmptm - pointer to the SNMPTM control block
                   tblfive - pointer to the TBLFIVE struct.
        
  Return Values :  Nothing 
    
  Notes         :  
*****************************************************************************/
void snmptm_delete_tblfive_entry(SNMPTM_CB *snmptm,
                                 SNMPTM_TBLFIVE *tblfive)
{

   /* Delete the tblfive entry from the tblfive patricia tree */
   ncs_patricia_tree_del(&(snmptm->tblfive_tree), &(tblfive->tblfive_pat_node));
  
   /* Free the memory of TBLFIVE */
   m_MMGR_FREE_SNMPTM_TBLFIVE(tblfive);

   return;
}


/*****************************************************************************
  Name          :  snmptm_delete_tblfive

  Description   :  This function deletes all the tblfive entries (nodes) from
                   tblfive tree and destroys a tblfive tree.

  Arguments     :  snmptm  - Pointer to the SNMPTM CB structure

  Return Values :  

  Notes         :
*****************************************************************************/
void  snmptm_delete_tblfive(SNMPTM_CB *snmptm)
{
   SNMPTM_TBLFIVE *tblfive;

   /* Delete all the TBLFIVE entries */
   while(SNMPTM_TBLFIVE_NULL != (tblfive = (SNMPTM_TBLFIVE*)
                        ncs_patricia_tree_getnext(&snmptm->tblfive_tree, 0)))
   {
       /* Delete the node from the tblfive tree */
       ncs_patricia_tree_del(&snmptm->tblfive_tree, &tblfive->tblfive_pat_node);

       /* Free the memory */
       m_MMGR_FREE_SNMPTM_TBLFIVE(tblfive);
   }

   /* Destroy TBLFIVE tree */
   ncs_patricia_tree_destroy(&snmptm->tblfive_tree);

   return;
}


#endif

