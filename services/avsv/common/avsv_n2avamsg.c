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

  This file contains utility routines for managing the messages between AvD & 
  AvA.
..............................................................................

  FUNCTIONS INCLUDED in this module:


  
******************************************************************************
*/

#include "avsv.h"
#include "avsv_amfparam.h"
#include "avsv_n2avamsg.h"
#include "avsv_n2avamem.h"


/****************************************************************************
  Name          : avsv_nda_ava_msg_free
 
  Description   : This routine frees the AvA message.
 
  Arguments     : msg - ptr to the AvA msg
 
  Return Values : None
 
  Notes         : None.
******************************************************************************/
void avsv_nda_ava_msg_free (AVSV_NDA_AVA_MSG *msg)
{
   if (!msg) return;

   /* free the message content */
   avsv_nda_ava_msg_content_free(msg);

   /* free the message */
   m_MMGR_FREE_AVSV_NDA_AVA_MSG(msg);

   return;
}


/****************************************************************************
  Name          : avsv_nda_ava_msg_content_free
 
  Description   : This routine frees the content of the AvA message.
 
  Arguments     : msg - ptr to the AvA msg
 
  Return Values : None.
 
  Notes         : This routine is used by AvA as it does not alloc nda 
                  message while decoding from mds.
******************************************************************************/
void avsv_nda_ava_msg_content_free (AVSV_NDA_AVA_MSG *msg)
{
   if (!msg) return;

   switch (msg->type)
   {
   case AVSV_AVA_API_MSG:
   case AVSV_AVND_AMF_API_RESP_MSG:
      break;

   case AVSV_AVND_AMF_CBK_MSG:
      if (msg->info.cbk_info)
      {
         avsv_amf_cbk_free(msg->info.cbk_info);
         msg->info.cbk_info = 0;
      }
      break;

   default:
      break;
   }

   return;
}


/****************************************************************************
  Name          : avsv_nda_ava_msg_copy
 
  Description   : This routine copies the AvA message.
 
  Arguments     : dmsg - ptr to the dest msg
                  smsg - ptr to the source msg
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None
******************************************************************************/
uns32 avsv_nda_ava_msg_copy (AVSV_NDA_AVA_MSG *dmsg, AVSV_NDA_AVA_MSG *smsg)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if (!dmsg || !smsg)
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   /* copy the common fields */
   m_NCS_OS_MEMCPY(dmsg, smsg, sizeof(AVSV_NDA_AVA_MSG));

   switch (smsg->type)
   {
   case AVSV_AVA_API_MSG:
   case AVSV_AVND_AMF_API_RESP_MSG:
      break;

   case AVSV_AVND_AMF_CBK_MSG:
      rc = avsv_amf_cbk_copy(&dmsg->info.cbk_info, smsg->info.cbk_info);
      break;

   default:
      m_AVSV_ASSERT(0);
   }

done:
   return rc;
}


/****************************************************************************
  Name          : avsv_amf_cbk_copy
 
  Description   : This routine copies the AMF callback info message.
 
  Arguments     : o_dmsg - double ptr to the dest cbk-info (o/p)
                  smsg   - ptr to the source cbk-info
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE.
 
  Notes         : None
******************************************************************************/
uns32 avsv_amf_cbk_copy (AVSV_AMF_CBK_INFO **o_dcbk, AVSV_AMF_CBK_INFO *scbk)
{
   uns32 rc = NCSCC_RC_SUCCESS;

   if (!o_dcbk || !scbk)
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   /* allocate the dest cbk-info */
   *o_dcbk = m_MMGR_ALLOC_AVSV_AMF_CBK_INFO;
   if ( !(*o_dcbk) )
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   /* copy the common fields */
   m_NCS_OS_MEMCPY(*o_dcbk, scbk, sizeof(AVSV_AMF_CBK_INFO));

   switch (scbk->type)
   {
   case AVSV_AMF_HC:
   case AVSV_AMF_COMP_TERM:
   case AVSV_AMF_CSI_REM:
   case AVSV_AMF_PXIED_COMP_INST:
   case AVSV_AMF_PXIED_COMP_CLEAN:
      break;

   case AVSV_AMF_PG_TRACK:
      /* memset notify buffer */
      m_NCS_OS_MEMSET(&(*o_dcbk)->param.pg_track.buf, 0, 
                      sizeof(SaAmfProtectionGroupNotificationBufferT));

      /* copy the notify buffer, if any */
      if (scbk->param.pg_track.buf.numberOfItems)
      {
         (*o_dcbk)->param.pg_track.buf.notification = 
            m_MMGR_ALLOC_AVSV_COMMON_DEFAULT_VAL(sizeof(SaAmfProtectionGroupNotificationT) * 
                                       scbk->param.pg_track.buf.numberOfItems);
         if (!(*o_dcbk)->param.pg_track.buf.notification)
         {
            rc = NCSCC_RC_FAILURE;
            goto done;
         }

         m_NCS_OS_MEMCPY((*o_dcbk)->param.pg_track.buf.notification, 
                         scbk->param.pg_track.buf.notification, 
                         sizeof(SaAmfProtectionGroupNotificationT) * 
                            scbk->param.pg_track.buf.numberOfItems);
         (*o_dcbk)->param.pg_track.buf.numberOfItems = 
            scbk->param.pg_track.buf.numberOfItems;
      }
      break;

   case AVSV_AMF_CSI_SET:
      /* memset avsv & amf csi attr lists */
      m_NCS_OS_MEMSET(&(*o_dcbk)->param.csi_set.attrs, 0, sizeof(NCS_AVSV_CSI_ATTRS));
      m_NCS_OS_MEMSET(&(*o_dcbk)->param.csi_set.csi_desc.csiAttr, 0, 
                      sizeof(SaAmfCSIAttributeListT));

      /* copy the avsv csi attr list */
      if (scbk->param.csi_set.attrs.number)
      {
         (*o_dcbk)->param.csi_set.attrs.list = 
                     m_MMGR_ALLOC_AVSV_COMMON_DEFAULT_VAL(sizeof(NCS_AVSV_ATTR_NAME_VAL) * 
                                                scbk->param.csi_set.attrs.number);
         if (!(*o_dcbk)->param.csi_set.attrs.list)
         {
            rc = NCSCC_RC_FAILURE;
            goto done;
         }

         m_NCS_OS_MEMCPY((*o_dcbk)->param.csi_set.attrs.list, scbk->param.csi_set.attrs.list, 
                         sizeof(NCS_AVSV_ATTR_NAME_VAL) * scbk->param.csi_set.attrs.number);
         (*o_dcbk)->param.csi_set.attrs.number = scbk->param.csi_set.attrs.number;
      }

      /* copy the amf csi attr list */
      if (scbk->param.csi_set.csi_desc.csiAttr.number)
      {
         rc = avsv_amf_csi_attr_list_copy(&scbk->param.csi_set.csi_desc.csiAttr,
                                          &(*o_dcbk)->param.csi_set.csi_desc.csiAttr);
         if ( NCSCC_RC_SUCCESS != rc ) goto done;
      }
      break;

   default:
      m_AVSV_ASSERT(0);
   }

done:
   if ( (NCSCC_RC_SUCCESS != rc) && *o_dcbk )
      avsv_amf_cbk_free(*o_dcbk);

   return rc;
}


/****************************************************************************
  Name          : avsv_amf_cbk_free
 
  Description   : This routine frees callback information.
 
  Arguments     : cbk_info - ptr to the callback info
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avsv_amf_cbk_free (AVSV_AMF_CBK_INFO *cbk_info)
{
   if (!cbk_info) return;

   switch (cbk_info->type)
   {
   case AVSV_AMF_HC:
   case AVSV_AMF_COMP_TERM:
   case AVSV_AMF_CSI_REM:
   case AVSV_AMF_PXIED_COMP_INST:
   case AVSV_AMF_PXIED_COMP_CLEAN:
      break;

   case AVSV_AMF_PG_TRACK:
      /* free the notify buffer */
      if (cbk_info->param.pg_track.buf.numberOfItems)
         m_MMGR_FREE_AVSV_COMMON_DEFAULT_VAL(cbk_info->param.pg_track.buf.notification);
      break;

   case AVSV_AMF_CSI_SET:
      /* free the avsv csi attr list */
      if (cbk_info->param.csi_set.attrs.number)
         m_MMGR_FREE_AVSV_COMMON_DEFAULT_VAL(cbk_info->param.csi_set.attrs.list);

      /* free the amf csi attr list */
      avsv_amf_csi_attr_list_free(&cbk_info->param.csi_set.csi_desc.csiAttr);
      break;

   default:
      break;
   }

   /* free the cbk-info ptr */
   m_MMGR_FREE_AVSV_AMF_CBK_INFO(cbk_info);

   return;
}


/****************************************************************************
  Name          : avsv_amf_csi_attr_list_copy
 
  Description   : This routine copies the csi attribute list.
 
  Arguments     : dattr - ptr to the destination csi-attr list
                  sattr - ptr to the src csi-attr list
 
  Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avsv_amf_csi_attr_list_copy (SaAmfCSIAttributeListT *dattr, 
                                   SaAmfCSIAttributeListT *sattr)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   uns32 cnt;

   if ( !dattr || !sattr ) goto done;

   dattr->attr = m_MMGR_ALLOC_AVSV_NDA_INFO(sizeof(SaAmfCSIAttributeT) * sattr->number);
   if (!dattr->attr)
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   for ( cnt = 0; cnt < sattr->number; cnt++ )
   {
      /* alloc memory for attr name & value */
      dattr->attr[cnt].attrName = m_MMGR_ALLOC_AVSV_NDA_INFO(m_NCS_STRLEN(sattr->attr[cnt].attrName));
      if (!dattr->attr[cnt].attrName)
      {
         m_MMGR_FREE_AVSV_NDA_INFO(dattr->attr[cnt].attrName);
         goto done;
      }

      dattr->attr[cnt].attrValue = m_MMGR_ALLOC_AVSV_NDA_INFO(m_NCS_STRLEN(sattr->attr[cnt].attrValue));
      if (!dattr->attr[cnt].attrValue)
      {
         m_MMGR_FREE_AVSV_NDA_INFO(dattr->attr[cnt].attrName);
         m_MMGR_FREE_AVSV_NDA_INFO(dattr->attr[cnt].attrValue);
         goto done;
      }

      /* copy the attr name & value */
      m_NCS_STRCPY(dattr->attr[cnt].attrName, sattr->attr[cnt].attrName);
      m_NCS_STRCPY(dattr->attr[cnt].attrValue, sattr->attr[cnt].attrValue);

      /* increment the attr name-val pair cnt that is copied */
      dattr->number++;
   } /* for */


done:
   if (NCSCC_RC_SUCCESS != rc)
   {
      avsv_amf_csi_attr_list_free(dattr);
      dattr->attr = 0;
      dattr->number = 0;
   }

   return rc;
}


/****************************************************************************
  Name          : avsv_amf_csi_attr_list_free
 
  Description   : This routine frees the csi attribute list.
 
  Arguments     : attr - ptr to the csi-attr list
 
  Return Values : None.
 
  Notes         : None.
******************************************************************************/
void avsv_amf_csi_attr_list_free (SaAmfCSIAttributeListT *attrs)
{
   uns32 cnt;

   if (!attrs) return;

   /* free the attr name-val pair */
   for ( cnt = 0; cnt < attrs->number; cnt++ )
   {
      m_MMGR_FREE_AVSV_NDA_INFO(attrs->attr[cnt].attrName);
      m_MMGR_FREE_AVSV_NDA_INFO(attrs->attr[cnt].attrValue);
   } /* for */

   /* finally free the attr list ptr */
   if (attrs->attr) m_MMGR_FREE_AVSV_NDA_INFO(attrs->attr);

   return;
}


/****************************************************************************
  Name          : avsv_amf_csi_attr_convert
 
  Description   : This routine converts the CSI attributes to a form that the 
                  application understands.
 
  Arguments     : cbk_info - ptr to the callback info

  Return Values : NCSCC_RC_SUCCESS / NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/
uns32 avsv_amf_csi_attr_convert (AVSV_AMF_CBK_INFO *cbk_info)
{
   SaAmfCSIAttributeListT *amf_attrs = 0;
   NCS_AVSV_CSI_ATTRS     *avsv_attrs = 0;
   uns32 cnt, rc = NCSCC_RC_SUCCESS;

   if ( (!cbk_info) || (AVSV_AMF_CSI_SET != cbk_info->type) ||
        (SA_AMF_CSI_ADD_ONE != cbk_info->param.csi_set.csi_desc.csiFlags) )
      goto done;

   /* get the amf & avsv attr ptrs */
   amf_attrs = &cbk_info->param.csi_set.csi_desc.csiAttr;
   avsv_attrs = &cbk_info->param.csi_set.attrs;

   if (!avsv_attrs->number) goto done;

   amf_attrs->attr = m_MMGR_ALLOC_AVSV_NDA_INFO(sizeof(SaAmfCSIAttributeT) * 
                                                avsv_attrs->number);
   if (!amf_attrs->attr)
   {
      rc = NCSCC_RC_FAILURE;
      goto done;
   }

   for ( cnt = 0; cnt < avsv_attrs->number; cnt++ )
   {
      /* alloc memory for attr name & value */
      amf_attrs->attr[cnt].attrName = m_MMGR_ALLOC_AVSV_NDA_INFO(avsv_attrs->list[cnt].name.length + 1);
      if (!amf_attrs->attr[cnt].attrName)
      {
         m_MMGR_FREE_AVSV_NDA_INFO(amf_attrs->attr[cnt].attrName);
         goto done;
      }

      amf_attrs->attr[cnt].attrValue = m_MMGR_ALLOC_AVSV_NDA_INFO(avsv_attrs->list[cnt].value.length + 1);
      if (!amf_attrs->attr[cnt].attrValue)
      {
         m_MMGR_FREE_AVSV_NDA_INFO(amf_attrs->attr[cnt].attrName);
         m_MMGR_FREE_AVSV_NDA_INFO(amf_attrs->attr[cnt].attrValue);
         goto done;
      }

      /* copy the attr name & value */
      m_NCS_OS_MEMCPY(amf_attrs->attr[cnt].attrName, avsv_attrs->list[cnt].name.value, 
                      avsv_attrs->list[cnt].name.length);
      m_NCS_OS_MEMCPY(amf_attrs->attr[cnt].attrValue, avsv_attrs->list[cnt].value.value, 
                      avsv_attrs->list[cnt].value.length);
      *(amf_attrs->attr[cnt].attrName + avsv_attrs->list[cnt].name.length) = '\0';
      *(amf_attrs->attr[cnt].attrValue + avsv_attrs->list[cnt].value.length) = '\0';

      /* increment the attr name-val pair cnt that is copied */
      amf_attrs->number++;
   } /* for */

done:
   if (NCSCC_RC_SUCCESS != rc)
   {
      avsv_amf_csi_attr_list_free(amf_attrs);
      amf_attrs->attr = 0;
      amf_attrs->number = 0;
   }

   return rc;
}


