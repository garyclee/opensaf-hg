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

  This file contains utility routines common for AvSv.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

#include "avsv.h"


/*****************************************************************************
 * Function: avsv_cpy_SU_DN_from_DN
 *
 * Purpose:  This function copies the SU DN from the given DN and places
 *           it in the provided buffer.
 *
 * Input: d_su_dn - Pointer to the SaNameT where the SU DN should be copied.
 *        s_dn_name - Pointer to the SaNameT that contains the SU DN.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avsv_cpy_SU_DN_from_DN(SaNameT *d_su_dn,
                              SaNameT *s_dn_name)
{
   char          *tmp = NULL;

   m_NCS_MEMSET(d_su_dn, 0, sizeof(SaNameT));

   /* SU DN name is  SU name + NODE name */

   /* First get the SU name */
   tmp = m_NCS_OS_STRSTR(s_dn_name->value, "safSu");

   if(!tmp)
      return NCSCC_RC_FAILURE;
   
   m_NCS_OS_STRCPY(d_su_dn->value, tmp);

   /* Fill the length and return the pointer */
   d_su_dn->length = m_NCS_OS_STRLEN(d_su_dn->value);

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
 * Function: avsv_cpy_node_DN_from_DN
 *
 * Purpose:  This function copies the node DN from the given DN and places
 *           it in the provided buffer.
 *
 * Input: d_node_dn - Pointer to the SaNameT where the node DN should be copied.
 *        s_dn_name - Pointer to the SaNameT that contains the node DN.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avsv_cpy_node_DN_from_DN(SaNameT *d_node_dn,
                              SaNameT *s_dn_name)
{
   char          *tmp = NULL;

   m_NCS_MEMSET(d_node_dn, 0, sizeof(SaNameT));

   /* get the node name */
   tmp = m_NCS_OS_STRSTR(s_dn_name->value, "safNode");

   if(!tmp)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_STRCPY(d_node_dn->value, tmp);
   
   /* Fill the length and return the pointer */
   d_node_dn->length = m_NCS_OS_STRLEN(d_node_dn->value);

   return NCSCC_RC_SUCCESS;
}



/*****************************************************************************
 * Function: avsv_is_external_DN
 *
 * Purpose:  This function verifies if the DN has externalsuname token in it.
 *           If yes it returns true. This routine will be used for identifying
 *           the external SUs and components.
 *
 * Input: dn_name - Pointer to the SaNameT that contains the DN.
 *
 * Returns: FALSE/TRUE
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

NCS_BOOL avsv_is_external_DN(SaNameT *dn_name)
{
   return FALSE;
}


/*****************************************************************************
 * Function: avsv_cpy_SI_DN_from_DN
 *
 * Purpose:  This function copies the SI DN from the given DN and places
 *           it in the provided buffer.
 *
 * Input: d_si_dn - Pointer to the SaNameT where the SI DN should be copied.
 *        s_dn_name - Pointer to the SaNameT that contains the SI DN.
 *
 * Returns: NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 *
 * NOTES: none.
 *
 * 
 **************************************************************************/

uns32 avsv_cpy_SI_DN_from_DN(SaNameT *d_si_dn,
                              SaNameT *s_dn_name)
{
   char          *tmp = NULL;

   m_NCS_MEMSET(d_si_dn, 0, sizeof(SaNameT));

   /* get the si name */
   tmp = m_NCS_OS_STRSTR(s_dn_name->value, "safSi");

   if(!tmp)
      return NCSCC_RC_FAILURE;

   m_NCS_OS_STRCPY(d_si_dn->value, tmp);
   
   /* Fill the length and return the pointer */
   d_si_dn->length = m_NCS_OS_STRLEN(d_si_dn->value);

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : avsv_dblist_uns32_cmp
 
  Description   : This routine compares the uns32 keys. It is used by DLL 
                  library.
 
  Arguments     : key1 - ptr to the 1st key
                  key2 - ptr to the 2nd key
 
  Return Values : 0, if keys are equal
                  1, if key1 is greater than key2
                  2, if key1 is lesser than key2
 
  Notes         : None.
******************************************************************************/
uns32 avsv_dblist_uns32_cmp(uns8 *key1, uns8 *key2)
{
   uns32 val1, val2;

   val1 = *((uns32 *)key1);
   val2 = *((uns32 *)key2);

   return ( (val1 == val2) ? 0 : ( (val1 > val2) ? 1 : 2) );
}

/****************************************************************************
  Name          : avsv_dblist_uns64_cmp
 
  Description   : This routine compares the uns64 keys. It is used by DLL 
                  library.
 
  Arguments     : key1 - ptr to the 1st key
                  key2 - ptr to the 2nd key
 
  Return Values : 0, if keys are equal
                  1, if key1 is greater than key2
                  2, if key1 is lesser than key2
 
  Notes         : None.
******************************************************************************/
uns32 avsv_dblist_uns64_cmp(uns8 *key1, uns8 *key2)
{
   uns64 val1, val2;

   val1 = *((uns64 *)key1);
   val2 = *((uns64 *)key2);

   return ( (val1 == val2) ? 0 : ( (val1 > val2) ? 1 : 2) );
}

/****************************************************************************
  Name          : avsv_dblist_saname_cmp
 
  Description   : This routine compares the SaNameT keys. It is used by DLL 
                  library.
 
  Arguments     : key1 - ptr to the 1st key
                  key2 - ptr to the 2nd key
 
  Return Values : 0, if keys are equal
                  1, if key1 is greater than key2
                  2, if key1 is lesser than key2
 
  Notes         : None.
******************************************************************************/
uns32 avsv_dblist_saname_cmp(uns8 *key1, uns8 *key2)
{
   int i=0;
   SaNameT name1_net, name2_net;

   name1_net = *((SaNameT *)key1);
   name2_net = *((SaNameT *)key2);

   i = m_CMP_NORDER_SANAMET(name1_net, name2_net);

   return ( (i == 0) ? 0 : ( (i > 0) ? 1 : 2) );
}


/****************************************************************************
  Name          : avsv_dblist_sahckey_cmp
 
  Description   : This routine compares the SaAmfHealthcheckKeyT keys. It is 
                  used by DLL library.
 
  Arguments     : key1 - ptr to the 1st key
                  key2 - ptr to the 2nd key
 
  Return Values : 0, if keys are equal
                  1, if key1 is greater than key2
                  2, if key1 is lesser than key2
 
  Notes         : None.
******************************************************************************/
uns32 avsv_dblist_sahckey_cmp(uns8 *key1, uns8 *key2)
{
   int i = 0;
   SaAmfHealthcheckKeyT hc_key1, hc_key2;

   hc_key1 = *((SaAmfHealthcheckKeyT *)key1);
   hc_key2 = *((SaAmfHealthcheckKeyT *)key2);

   i = m_CMP_HORDER_SAHCKEY(hc_key1, hc_key2);

   return ( (i == 0) ? 0 : ( (i > 0) ? 1 : 2) );
}


/****************************************************************************
  Name          : avsv_sa_name_is_null
 
  Description   : This routine determines if sa-name is NULL.
 
  Arguments     : name - ptr to the name
 
  Return Values : TRUE / FALSE
 
  Notes         : None.
******************************************************************************/
NCS_BOOL avsv_sa_name_is_null (SaNameT *name)
{
   SaNameT null_name;

   m_NCS_OS_MEMSET(&null_name, 0, sizeof(SaNameT));

   if ( !m_CMP_HORDER_SANAMET(*name, null_name) )
      return TRUE;
   else
      return FALSE;
}


