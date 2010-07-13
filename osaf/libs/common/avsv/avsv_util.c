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

uns32 avsv_cpy_SU_DN_from_DN(SaNameT *d_su_dn, SaNameT *s_dn_name)
{
	char *tmp = NULL;

	memset(d_su_dn, 0, sizeof(SaNameT));

	/* SU DN name is  SU name + NODE name */

	/* First get the SU name */
	tmp = strstr((char*)s_dn_name->value, "safSu");

	/* It might be external SU. */
	if (NULL == tmp)
		tmp = strstr((char*)s_dn_name->value, "safEsu");

	if (!tmp)
		return NCSCC_RC_FAILURE;

	if (strlen(tmp) < SA_MAX_NAME_LENGTH) {
		strcpy((char*)d_su_dn->value, tmp);

		/* Fill the length and return the pointer */
		d_su_dn->length = strlen((char*)d_su_dn->value);
	} else
		return NCSCC_RC_FAILURE;

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

uns32 avsv_cpy_node_DN_from_DN(SaNameT *d_node_dn, SaNameT *s_dn_name)
{
	char *tmp = NULL;

	memset(d_node_dn, 0, sizeof(SaNameT));

	/* get the node name */
	tmp = strstr((char*)s_dn_name->value, "safNode");

	if (!tmp)
		return NCSCC_RC_FAILURE;

	if (strlen(tmp) < SA_MAX_NAME_LENGTH) {
		strcpy((char*)d_node_dn->value, tmp);

		/* Fill the length and return the pointer */
		d_node_dn->length = strlen((char*)d_node_dn->value);
	} else
		return NCSCC_RC_FAILURE;

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

uns32 avsv_cpy_SI_DN_from_DN(SaNameT *d_si_dn, SaNameT *s_dn_name)
{
	char *tmp = NULL;

	memset(d_si_dn, 0, sizeof(SaNameT));

	/* get the si name */
	tmp = strstr((char*)s_dn_name->value, "safSi");

	if (!tmp)
		return NCSCC_RC_FAILURE;

	if (strlen(tmp) < SA_MAX_NAME_LENGTH) {
		strcpy((char*)d_si_dn->value, tmp);

		/* Fill the length and return the pointer */
		d_si_dn->length = strlen((char*)d_si_dn->value);
	} else
		return NCSCC_RC_FAILURE;

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

	return ((val1 == val2) ? 0 : ((val1 > val2) ? 1 : 2));
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

	return ((val1 == val2) ? 0 : ((val1 > val2) ? 1 : 2));
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
	int i = 0;
	SaNameT name1_net, name2_net;

	name1_net = *((SaNameT *)key1);
	name2_net = *((SaNameT *)key2);

	i = m_CMP_HORDER_SANAMET(name1_net, name2_net);

	return ((i == 0) ? 0 : ((i > 0) ? 1 : 2));
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

	return ((i == 0) ? 0 : ((i > 0) ? 1 : 2));
}

/****************************************************************************
  Name          : avsv_sa_name_is_null
 
  Description   : This routine determines if sa-name is NULL.
 
  Arguments     : name - ptr to the name
 
  Return Values : TRUE / FALSE
 
  Notes         : None.
******************************************************************************/
NCS_BOOL avsv_sa_name_is_null(SaNameT *name)
{
	SaNameT null_name;

	memset(&null_name, 0, sizeof(SaNameT));

	if (!m_CMP_HORDER_SANAMET(*name, null_name))
		return TRUE;
	else
		return FALSE;
}

/**
 * Create a DN for an association class. Escape commas in the child DN.
 * @param child_dn
 * @param parent_dn
 * @param rdn_tag
 * @param dn[out]
 */
void avsv_create_association_class_dn(const SaNameT *child_dn, const SaNameT *parent_dn,
	const char *rdn_tag, SaNameT *dn)
{
	char *p = (char*) dn->value;
	int i;

	memset(dn, 0, sizeof(SaNameT));

	p += sprintf((char*)dn->value, "%s=", rdn_tag);

	/* copy child DN and escape commas */
	for (i = 0; i < child_dn->length; i++) {
		if (child_dn->value[i] == ',')
			*p++ = 0x5c; /* backslash */

		*p++ = child_dn->value[i];
	}

	if (parent_dn != NULL) {
		*p++ = ',';
		strcpy(p, (char*)parent_dn->value);
	}

	dn->length = strlen((char*)dn->value);
}

/**
 * Initialize a DN by searching for needle in haystack
 * @param haystack
 * @param dn
 * @param needle
 */
void avsv_sanamet_init(const SaNameT *haystack, SaNameT *dn, const char *needle)
{
	char *p;

	memset(dn, 0, sizeof(SaNameT));
	p = strstr((char*)haystack->value, needle);
	assert(p);
	dn->length = strlen(p);
	memcpy(dn->value, p, dn->length);
}

/**
 * Convert a SAF AMF Component category bit field to a AVSV Comp type value.
 * @param saf_comp_category
 * 
 * @return AVSV_COMP_TYPE_VAL
 */
AVSV_COMP_TYPE_VAL avsv_amfcompcategory_to_avsvcomptype(SaAmfCompCategoryT saf_comp_category)
{
	AVSV_COMP_TYPE_VAL avsv_comp_type = AVSV_COMP_TYPE_INVALID;

	if (saf_comp_category & SA_AMF_COMP_SA_AWARE) {
		if ((saf_comp_category & ~(SA_AMF_COMP_SA_AWARE | SA_AMF_COMP_LOCAL)) == 0)
			return AVSV_COMP_TYPE_SA_AWARE;
		else
			return AVSV_COMP_TYPE_INVALID;
	} 

	if (saf_comp_category & SA_AMF_COMP_PROXY) {
		if ((saf_comp_category & ~(SA_AMF_COMP_PROXY | SA_AMF_COMP_LOCAL | SA_AMF_COMP_SA_AWARE)) == 0)
			return AVSV_COMP_TYPE_SA_AWARE;
		else
			return AVSV_COMP_TYPE_INVALID;
	} 

	if ((saf_comp_category & SA_AMF_COMP_PROXIED) && (saf_comp_category &  SA_AMF_COMP_LOCAL)) {
		if ((saf_comp_category & ~(SA_AMF_COMP_PROXIED | SA_AMF_COMP_LOCAL)) == 0)
			return AVSV_COMP_TYPE_PROXIED_LOCAL_PRE_INSTANTIABLE;
		else
			return AVSV_COMP_TYPE_INVALID;
	} 

	if ((saf_comp_category & SA_AMF_COMP_PROXIED_NPI) && (saf_comp_category & SA_AMF_COMP_LOCAL)) {
		if ((saf_comp_category & ~(SA_AMF_COMP_PROXIED_NPI | SA_AMF_COMP_LOCAL)) == 0)
			return AVSV_COMP_TYPE_PROXIED_LOCAL_NON_PRE_INSTANTIABLE;
		else
			return AVSV_COMP_TYPE_INVALID;
	} 

	if (saf_comp_category & SA_AMF_COMP_LOCAL) {
		if ((saf_comp_category & ~SA_AMF_COMP_LOCAL) == 0)
			return AVSV_COMP_TYPE_NON_SAF;
		else
			return AVSV_COMP_TYPE_INVALID;
	} 

	if ((saf_comp_category == 0) || (saf_comp_category == SA_AMF_COMP_PROXIED))
		return AVSV_COMP_TYPE_EXTERNAL_PRE_INSTANTIABLE;

	if (saf_comp_category & SA_AMF_COMP_PROXIED_NPI) {
		if ((saf_comp_category & ~SA_AMF_COMP_PROXIED_NPI) == 0)
			return AVSV_COMP_TYPE_NON_SAF;
		else
			return AVSV_COMP_TYPE_EXTERNAL_NON_PRE_INSTANTIABLE;
	} 

	/* Container type not yet supported, will return AVSV_COMP_TYPE_INVALID */

	return avsv_comp_type;
}

