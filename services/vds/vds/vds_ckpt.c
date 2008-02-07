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

  MODULE NAME: VDS_CKPT.C
  
..............................................................................

  DESCRIPTION: This file contains VDS CKPT integration specific routines
  
..............................................................................

  FUNCTIONS INCLUDED in this module:

******************************************************************************/
#include "vds.h"
extern uns32 gl_vds_hdl;



static NCS_BOOL  initial_vdest = TRUE;
/*  
 * Static Function Prototypes
 */
static uns32 vds_ckpt_cbinfo_write(VDS_CB  *cb);
static VDS_VDEST_DB_INFO *vds_ckpt_new_vdest(VDS_CB *vds_cb,
                                             VDS_CKPT_DBINFO *info);
static void vds_ckpt_fill_ckpt_dbinfo(VDS_CKPT_DBINFO *dbinfo_node,
                                      VDS_VDEST_DB_INFO *vdest_dbinfo);

static void vds_ckpt_buffer_decode(VDS_CKPT_DBINFO *vds_ckpt_dbinfo_buffer ,char *decode);

static void vds_ckpt_buffer_encode(VDS_CKPT_DBINFO *source ,char *dest);

/****************************************************************************
  Name          :  vds_ckpt_initialize
 
  Description   :  This routine initializes and Opens CKPT.
 
  Arguments     :  cb - pointer to the VDS_CB .
 
  Return Values :  uns32 (NCSCC_RC_FAILURE /NCSCC_RC_SUCCESS).
 
  Notes         :  None
******************************************************************************/
uns32 vds_ckpt_initialize(VDS_CB *cb)
{
   SaCkptCallbacksT *reg_callback_set = NULL;
   SaVersionT       ver;
   SaAisErrorT      rc = SA_AIS_OK;
   SaNameT          ckptName;
   SaCkptCheckpointCreationAttributesT  ckptCreateAttr;
   SaCkptCheckpointOpenFlagsT           ckptOpenFlags;
    
   VDS_TRACE1_ARG1("vds_ckpt_initialize\n");
   
   /* Fill the CKPT version */
   m_VDS_CKPT_VER_GET(ver);

   /* Initialize CKPT */
   rc = saCkptInitialize(&cb->ckpt.ckpt_hdl, reg_callback_set, &ver);
   if (rc != SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_INIT,
                          VDS_LOG_CKPT_FAILURE,
                                NCSFL_SEV_ERROR, rc);
      /* Added by vishal : for VDS error handling enhancement */
      if (rc == SA_AIS_ERR_TRY_AGAIN)
      {
          m_NCS_TASK_SLEEP(1000);
      }
      saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                    0, SA_AMF_COMPONENT_RESTART, 0);
      return NCSCC_RC_FAILURE;
   }

   m_NCS_OS_MEMSET(&ckptName, 0, sizeof(ckptName));

   /* Fill the Checkpoint Name */
   m_NCS_OS_MEMCPY(ckptName.value,
                   VDS_CKPT_DBINFO_CKPT_NAME, 
                   VDS_CKPT_DBINFO_CKPT_NAME_LEN);

   ckptName.length = VDS_CKPT_DBINFO_CKPT_NAME_LEN;

   /* Update the attributes to open the Checkpoint */
   m_VDS_CKPT_UPDATE_CREATE_ATTR(ckptCreateAttr); 

   /* Set flag options to open ckpt */
   m_VDS_CKPT_SET_OPEN_FLAGS(ckptOpenFlags);

   /* Open the Checkpoint */
   rc = saCkptCheckpointOpen(cb->ckpt.ckpt_hdl,
                             &ckptName, 
                             &ckptCreateAttr,
                             ckptOpenFlags, 
                             VDS_CKPT_TIMEOUT, 
                             &cb->ckpt.checkpointHandle); 
   if (rc != SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_OPEN,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
      if (rc == SA_AIS_ERR_TRY_AGAIN)
      {
          m_NCS_TASK_SLEEP(1000);
      }

      rc = saCkptFinalize(cb->ckpt.ckpt_hdl);
      if (rc != SA_AIS_OK)
      { 

         m_VDS_LOG_CKPT(VDS_LOG_CKPT_FIN,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
      }
      cb->ckpt.ckpt_hdl = 0;
      /* Added by vishal : for VDS error handling enhancement */
      saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                    0, SA_AMF_COMPONENT_RESTART, 0);
      
      return NCSCC_RC_FAILURE;
   }

   return NCSCC_RC_SUCCESS;
}


/****************************************************************************
  Name          : vds_ckpt_finalize

  Description   : This routine closes the open checkpoint and 
                  calls checkpoint Finalize API.

  Arguments     : cb - pointer to the VDS_CB

  Return Values : Success : NCSCC_RC_SUCCESS
                  Failure : NCSCC_RC_FAILURE

  Notes         : None
******************************************************************************/
uns32 vds_ckpt_finalize(VDS_CB *cb)
{
   SaAisErrorT  rc = SA_AIS_OK;

   VDS_TRACE1_ARG1("vds_ckpt_finalize\n");
   
   /* closing  checkpoint */
   rc =  saCkptCheckpointClose(cb->ckpt.checkpointHandle);
   if (rc != SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_CLOSE,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
   }

   cb->ckpt.checkpointHandle = 0;

   rc = saCkptFinalize(cb->ckpt.ckpt_hdl);
   if (rc != SA_AIS_OK)
   {  
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_FIN,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
      
      /* Added by vishal : for VDS error handling enhancement */
      /* saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                    0, SA_AMF_COMPONENT_RESTART, 0); */

      return NCSCC_RC_FAILURE;
   }

   cb->ckpt.ckpt_hdl = 0;

   return NCSCC_RC_SUCCESS;
}


/*****************************************************************************
  PROCEDURE NAME :  vds_ckpt_write()
 
  DESCRIPTION    :  This routine checkpoints VDS database info .

  ARGUMENTS      :  cb - ptr to the  VDS control block
                    vdest_dbinfo  - resource info to be checkpointed
                    vds_ckpt_write_flag - write or overwrite 

  RETURNS        :  Success : NCSCC_RC_SUCCESS.
                    Failure : NCSCC_RC_FAILURE.

  NOTES          :  None
*****************************************************************************/
uns32 vds_ckpt_write(VDS_CB *cb,
                     VDS_VDEST_DB_INFO *vdest_dbinfo,
                     VDS_CKPT_OPERATIONS vds_ckpt_write_flag)
{
   uns32 rc = NCSCC_RC_SUCCESS;
   VDS_TRACE1_ARG1("vds_ckpt_write\n");

   switch(vds_ckpt_write_flag)
   {
   case VDS_CKPT_OPERA_WRITE:     
       rc = vds_ckpt_dbinfo_write(cb, vdest_dbinfo);
       if (rc == NCSCC_RC_SUCCESS)
       {
          rc = vds_ckpt_cbinfo_write(cb); 
          if (rc != NCSCC_RC_SUCCESS)
          {
             vds_ckpt_dbinfo_delete(cb, &vdest_dbinfo->vdest_id);
          }
       }
       break;

   case VDS_CKPT_OPERA_OVERWRITE:
       rc = vds_ckpt_dbinfo_overwrite(cb, vdest_dbinfo);
       break;

   default: 
       /* log messages */
       break;
   }

   return rc; 
}


/*****************************************************************************
  PROCEDURE NAME :  vds_ckpt_cbinfo_write

  DESCRIPTION    :  This function Writes latest_allocated_vdest information 
                    to Checkpoint 

  ARGUMENTS      :  cb - ptr to the  VDS control block

  RETURNS        :  Success : NCSCC_RC_SUCCESS
                    Failure : NCSCC_RC_FAILURE

  NOTES          :  None
*****************************************************************************/
uns32 vds_ckpt_cbinfo_write(VDS_CB *cb)
{
   SaAisErrorT      rc = SA_AIS_OK;
   SaCkptSectionIdT section_id;
   MDS_DEST         allocated_vdest = 0;
   SaCkptSectionCreationAttributesT sec_create_attr;

   VDS_TRACE1_ARG1("vds_ckpt_cbinfo_write\n");
   
#if 0
   SaCkptIOVectorElementT  io_vector;
   SaUint32T  *erroneous_vector_index = NULL;
#endif 


   m_NCS_OS_MEMSET(&section_id, 0, sizeof(SaCkptSectionIdT));
    
   /* when ever initial vdest is created ,we need to create new section and
    * write the latest_allocated_vdest value to it, when ever the latest_ 
    * _allocated_vdest is updated we need to overwrite the section with  
    * updated value this is implemented by initial_vdest flag. 
    *
    * Initially it contains TRUE (section created and written, inital_vdest
    * changed to FALSE)
    * initial_vdest is FALSE means that section need to over write with 
    * updated latest_allocated_vdest value.
    */

   section_id.id = (SaUint8T *)VDS_CKPT_LATEST_VDEST_SECTIONID;
   section_id.idLen = VDS_CKPT_LATEST_VDEST_SECTIONID_LEN;
 
  
   /* Encode routine to encode from network order to host order */ 
   m_NCS_OS_HTONLL_P(&allocated_vdest,cb->latest_allocated_vdest);

   if (initial_vdest == FALSE)
   {
      /* overwrite existing section with updated latest_allocated_vdest 
         value identified by section id */

      m_VDS_LOG_GENERIC("CB OVERWRITE with",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(cb->latest_allocated_vdest),"latest allocated Vdest");

      rc = saCkptSectionOverwrite(cb->ckpt.checkpointHandle, 
                                  &section_id,
                                  (void *)&allocated_vdest,
                                  sizeof(MDS_DEST));

       if (rc == SA_AIS_OK)
       {
            m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_CB_OVERWRITE,
                                      VDS_LOG_CKPT_SUCCESS,
                                          NCSFL_SEV_INFO, rc);
            return NCSCC_RC_SUCCESS;
       }
       else
       {
           m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_CB_OVERWRITE,
                                     VDS_LOG_CKPT_FAILURE,
                                           NCSFL_SEV_ERROR, rc);
           if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_TRY_AGAIN)
           {
                return NCSCC_RC_FAILURE;
           }
           else
           {
                return NCSCC_RC_RESOURCE_UNAVAILABLE;
           }
       } 
   }
     
   m_NCS_OS_MEMSET(&sec_create_attr,
                   0,
                   sizeof(SaCkptSectionCreationAttributesT));
   sec_create_attr.sectionId = &section_id;
   sec_create_attr.expirationTime   = VDS_CKPT_DBINFO_RET_TIME;
   
   /* creating section with special section-id i.e VDS_CKPT_LATEST_VDEST_SECTIONID 
      which identifies this section uniquely */  
   rc = saCkptSectionCreate(cb->ckpt.checkpointHandle,
                            &sec_create_attr,
                            (uns8 *)&allocated_vdest,
                            sizeof(MDS_DEST));
   if (rc == SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_CB_CWRITE,
                                 VDS_LOG_CKPT_SUCCESS,
                                       NCSFL_SEV_INFO, rc);
      initial_vdest = FALSE;
      return NCSCC_RC_SUCCESS;
   }
   else if (rc == SA_AIS_ERR_EXIST)
   {
       /* Try overwriting it */
       initial_vdest = FALSE;
       rc = vds_ckpt_cbinfo_write(cb);
       return rc;
   }
   else
   {
        m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_CB_CWRITE,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);

        if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_TRY_AGAIN)
        {
            return NCSCC_RC_FAILURE;
        }
        else
        {
            return NCSCC_RC_RESOURCE_UNAVAILABLE;
        }
   }
}

/*****************************************************************************
  PROCEDURE NAME :  vds_ckpt_buffer_encode

  DESCRIPTION    :  This function encodes the checkpoint data read from host 
                    byte order to network byte order.

  ARGUMENTS      :  source  -  ptr to the VDS_CKPT_DBINFO
                    decode  -  ptr to the buffer that needs to be encoded.

  RETURNS        :  void 

  NOTES          :  None
*****************************************************************************/
void vds_ckpt_buffer_encode(VDS_CKPT_DBINFO *source ,char *dest)
{
   int i=0;

   m_NCS_OS_MEMSET(dest, 0, VDS_CKPT_BUFFER_SIZE);

   m_NCS_OS_HTONS_P(dest, source->vdest_name.length);
   dest += 2;

   m_NCS_OS_MEMCPY(dest, source->vdest_name.value, SA_MAX_NAME_LENGTH);
   dest += SA_MAX_NAME_LENGTH;

   m_NCS_OS_HTONLL_P(dest,source->vdest_id);
   dest +=8;

   *dest =(char)source->persistent;
   dest++;

   m_NCS_OS_HTONS_P(dest,source->adest_count);
   dest += 2;
  
   for (i=0; i<VDS_CKPT_MAX_ADESTS; i++) 
   {
       m_NCS_OS_HTONLL_P(dest,source->adest[i]);
       dest += 8;
   }

   return;
}

/*****************************************************************************
  PROCEDURE NAME :  vds_ckpt_buffer_decode

  DESCRIPTION    :  This function decodes the checkpoint data read from network byte 
                    order to host byte order. 

  ARGUMENTS      :  vds_ckpt_dbinfo_buffer -  ptr to the VDS_CKPT_DBINFO
                    decode - ptr to the buffer that needs to be decoded.

  RETURNS        :  void 

  NOTES          :  None
*****************************************************************************/
void vds_ckpt_buffer_decode(VDS_CKPT_DBINFO *vds_ckpt_dbinfo_buffer ,char *decode)
{
  int i=0;
 
  m_NCS_OS_MEMSET(vds_ckpt_dbinfo_buffer,0,sizeof(VDS_CKPT_DBINFO));

  vds_ckpt_dbinfo_buffer->vdest_name.length =  m_NCS_OS_NTOHS_P(decode);
  decode += 2;

  m_NCS_OS_MEMCPY(vds_ckpt_dbinfo_buffer->vdest_name.value, decode, SA_MAX_NAME_LENGTH);
  decode += SA_MAX_NAME_LENGTH;

  vds_ckpt_dbinfo_buffer->vdest_id = m_NCS_OS_NTOHLL_P(decode);
  decode += 8;

  vds_ckpt_dbinfo_buffer->persistent =(uns32) *decode;
  decode++;

  vds_ckpt_dbinfo_buffer->adest_count = m_NCS_OS_NTOHS_P(decode);
  decode += 2;

  for(i=0;i<VDS_CKPT_MAX_ADESTS;i++)
  {        
     vds_ckpt_dbinfo_buffer->adest[i] = m_NCS_OS_NTOHLL_P(decode);
     decode += 8;
  }
  
  return;
}

/*****************************************************************************
  PROCEDURE NAME : vds_ckpt_dbinfo_write()

  DESCRIPTION    : This function creates and writes  VDS database info 
                   to new sections.

  ARGUMENTS      : cb            - ptr to the VDS control block
                   vdest_dbinfo  - VDS_DB_INFO node 

  RETURNS        : Success : NCSCC_RC_SUCCESS
                   Failure : NCSCC_RC_FAILURE

  NOTES          : None
*****************************************************************************/
uns32 vds_ckpt_dbinfo_write(VDS_CB *cb, VDS_VDEST_DB_INFO *vdest_dbinfo)
{
   VDS_CKPT_DBINFO   vds_ckpt_dbinfo_node;
   VDS_CKPT_DBINFO   *temp_vds_ckpt_dbinfo_ptr = NULL;
#if 0
   SaUint32T               *erroneous_vector_index = NULL;
   SaCkptIOVectorElementT  io_vector;
#endif
   SaAisErrorT       rc = SA_AIS_OK;
   SaCkptSectionIdT  section_id;
   SaCkptSectionCreationAttributesT sec_create_attr;

   VDS_TRACE1_ARG1("vds_ckpt_dbinfo_write\n");

   m_NCS_OS_MEMSET(&vds_ckpt_dbinfo_node, 0, sizeof(VDS_CKPT_DBINFO));
   m_NCS_OS_MEMSET(&section_id, 0, sizeof(SaCkptSectionIdT));
   
   if ((temp_vds_ckpt_dbinfo_ptr  =(VDS_CKPT_DBINFO *)m_MMGR_ALLOC_CKPT_BUFFER(VDS_CKPT_BUFFER_SIZE)) == NULL)
   {
      m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_CKPT_BUFFER,
                            VDS_LOG_MEM_ALLOC_FAILURE,
                                        NCSFL_SEV_ERROR);

       return NCSCC_RC_FAILURE;
   }

    
   /* Update the VDS db_info data into the record thats going to checkpoint */ 
   vds_ckpt_fill_ckpt_dbinfo(&vds_ckpt_dbinfo_node, vdest_dbinfo);

   vds_ckpt_buffer_encode(&vds_ckpt_dbinfo_node,(char *)temp_vds_ckpt_dbinfo_ptr);

   section_id.id = (SaUint8T *)&vds_ckpt_dbinfo_node.vdest_id;
   section_id.idLen = sizeof(MDS_DEST);

   sec_create_attr.sectionId = &section_id;
   sec_create_attr.expirationTime = SA_TIME_END;

   /* creating section for every new vdest_db_info node identified by vdest_id.
     Hoping that this API will take care of checkpointing the data */
   rc = saCkptSectionCreate(cb->ckpt.checkpointHandle,
                            &sec_create_attr,
                            (SaUint8T *)temp_vds_ckpt_dbinfo_ptr,
                            VDS_CKPT_BUFFER_SIZE);
   /* Free temporary allocated buffer first */
   m_MMGR_FREE_CKPT_BUFFER(temp_vds_ckpt_dbinfo_ptr);

   if (rc == SA_AIS_OK)
   {
       m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_DB_CWRITE,
                                     VDS_LOG_CKPT_SUCCESS,
                                           NCSFL_SEV_INFO, rc);
       return NCSCC_RC_SUCCESS;
   }
   else
   {
       m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_DB_CWRITE,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
       if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_TRY_AGAIN)
       {
            return NCSCC_RC_FAILURE;
       }
       else
       {
            return NCSCC_RC_RESOURCE_UNAVAILABLE;
       }
   } 
}


/*****************************************************************************
  PROCEDURE NAME :  vds_ckpt_dbinfo_overwrite()

  DESCRIPTION    :  This function overwrites the sections with updated VDS 
                    database info in checkpoint.  

  ARGUMENTS      :  cb - ptr to the  VDS control block
                    vdest_dbinfo  - pointer to updated VDS_VDEST_DB_INFO node 

  RETURNS        :  Success : NCSCC_RC_SUCCESS
                    Failure : NCSCC_RC_FAILURE

  NOTES          :  None
*****************************************************************************/
uns32 vds_ckpt_dbinfo_overwrite(VDS_CB *cb, VDS_VDEST_DB_INFO *vdest_dbinfo)
{
   VDS_CKPT_DBINFO   vds_ckpt_dbinfo_node;
   SaAisErrorT       rc = SA_AIS_OK;
   SaCkptSectionIdT  section_id;
   VDS_CKPT_DBINFO   *temp_vds_ckpt_dbinfo_ptr = NULL;


   VDS_TRACE1_ARG1("vds_ckpt_dbinfo_overwrite\n");
   
   m_NCS_OS_MEMSET(&vds_ckpt_dbinfo_node, 0, sizeof(VDS_CKPT_DBINFO));
   m_NCS_OS_MEMSET(&section_id, 0, sizeof(SaCkptSectionIdT)); 

   /* Update the VDS db_info data into the record thats going to checkpoint */ 
   vds_ckpt_fill_ckpt_dbinfo(&vds_ckpt_dbinfo_node, vdest_dbinfo);

   if((temp_vds_ckpt_dbinfo_ptr  =(VDS_CKPT_DBINFO *)m_MMGR_ALLOC_CKPT_BUFFER(VDS_CKPT_BUFFER_SIZE))== NULL) 
   {
      m_VDS_LOG_MEM(VDS_LOG_MEM_VDS_CKPT_BUFFER,
                            VDS_LOG_MEM_ALLOC_FAILURE,
                                        NCSFL_SEV_ERROR);

      return NCSCC_RC_FAILURE;
   }
  
   vds_ckpt_buffer_encode(&vds_ckpt_dbinfo_node,(char *)temp_vds_ckpt_dbinfo_ptr);
  
   section_id.id = (SaUint8T *)&vds_ckpt_dbinfo_node.vdest_id;
   section_id.idLen = sizeof(MDS_DEST);
  
   /* overwrites existing section */
   rc = saCkptSectionOverwrite(cb->ckpt.checkpointHandle,
                               &section_id,
                               (void *)temp_vds_ckpt_dbinfo_ptr,
                               VDS_CKPT_BUFFER_SIZE);
    /* Free temporary checkpoint info first */
    m_MMGR_FREE_CKPT_BUFFER(temp_vds_ckpt_dbinfo_ptr);
   
   if (rc != SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_DB_OVERWRITE,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
       if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_TRY_AGAIN)
       {
            return NCSCC_RC_FAILURE;
       }
       else
       {
            return NCSCC_RC_RESOURCE_UNAVAILABLE;
       }
   }
   else
   {
        return NCSCC_RC_SUCCESS;
   }
}


/*****************************************************************************
  PROCEDURE NAME : vds_ckpt_fill_ckpt_dbinfo()

  DESCRIPTION    : This function populates VDS_CKPT_DB node from
                   VDS_VDEST_DB_INFO node.

  ARGUMENTS      : dbinfo_node  - ptr to the VDS_CKPT_DB which contains 
                   vdest_dbinfo - ptr to the VDS_VDEST_DB_INFO node that
                   needs to be filled  

  RETURNS        : None

  NOTES          : None
*****************************************************************************/
void vds_ckpt_fill_ckpt_dbinfo(VDS_CKPT_DBINFO *dbinfo_node,
                               VDS_VDEST_DB_INFO *vdest_dbinfo)
{
   VDS_VDEST_ADEST_INFO *adest_node;
   int adest_count;

   VDS_TRACE1_ARG1("vds_ckpt_fill_ckpt_dbinfo\n");
   
   dbinfo_node->vdest_name = vdest_dbinfo->vdest_name;
   dbinfo_node->vdest_id = vdest_dbinfo->vdest_id;
   dbinfo_node->persistent = vdest_dbinfo->persistent;
   
   adest_node  = vdest_dbinfo->adest_list;
   adest_count = 1;
   while(adest_node != NULL) 
   {
      if (adest_count <= VDS_CKPT_MAX_ADESTS)
         dbinfo_node->adest[adest_count - 1] = adest_node->adest;

      adest_node = adest_node->next;
      adest_count++;
   }

   if (adest_count > VDS_CKPT_MAX_ADESTS)
   {
      m_VDS_LOG_GENERIC("@@@@@@@@ Adest Count is Execceded:",adest_count,"in Fill CKPT DBINFO");

      dbinfo_node->adest_count = VDS_CKPT_MAX_ADESTS;

   }
   else
      dbinfo_node->adest_count = adest_count;

   return;
}


/*****************************************************************************
  PROCEDURE NAME :  vds_ckpt_dbinfo_delete

  DESCRIPTION    :  This function deletes the Section from the checkpoint 
                    identified by given vdest_id

  ARGUMENTS      :  cb       - ptr to the VDS control block.
                    vdest_id - key field which acts as section id to identify 
                               sections in checkpiont.

  RETURNS        :  Success : NCSCC_RC_SUCCESS
                    Failure : NCSCC_RC_FAILURE

  NOTES          :  None
*****************************************************************************/
uns32 vds_ckpt_dbinfo_delete(VDS_CB *cb, MDS_DEST *vdest_id) 
{
   SaAisErrorT      rc = SA_AIS_OK;
   SaCkptSectionIdT section_id;


   VDS_TRACE1_ARG1("vds_ckpt_dbinfo_delete\n");
   
   if (vdest_id == NULL)
   {  
      VDS_TRACE1_ARG1("vds_ckpt_dbinfo_delete : failed NULL check\n");
      return NCSCC_RC_FAILURE;
   }   

   m_NCS_OS_MEMSET(&section_id , 0, sizeof(SaCkptSectionIdT));

   section_id.id    = (SaUint8T *)vdest_id;
   section_id.idLen = sizeof(MDS_DEST);

   /* delete section from checkpoint identified by vdest_id 
      i.e vdest_id acts as section id  */

   m_VDS_LOG_GENERIC("SECTION DELETE with ID :",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(*vdest_id),"in CKPT");
     
   rc = saCkptSectionDelete(cb->ckpt.checkpointHandle, &section_id);
      if (rc != SA_AIS_OK)
   {
       m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_DELETE,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
       if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_TRY_AGAIN)
       {
            return NCSCC_RC_FAILURE;
       }
       else
       {
            return NCSCC_RC_RESOURCE_UNAVAILABLE;
       }
   }
   else
   {
        return NCSCC_RC_SUCCESS;
   }
}


/****************************************************************************
  Name          :  vds_ckpt_new_vdest

  Description   :  This function creates VDS_VDEST_DB_INFO node and populates it 
                   with data retrived from checkpoint.

  Arguments     :  vds_cb - pointer to VDS control block. 
                   info   - pointer to checkpoint dbinfo data structure   
                            retrived from checkpoint to build vds database. 

  Return Values :  Success : NCSCC_RC_SUCCESS
                   Failure : NCSCC_RC_FAILURE

  Notes         :  None
******************************************************************************/
VDS_VDEST_DB_INFO  *vds_ckpt_new_vdest(VDS_CB *vds_cb,
                                       VDS_CKPT_DBINFO *info)
{
   VDS_VDEST_DB_INFO *db_info = NULL;
   uns32 rc = NCSCC_RC_SUCCESS;

   VDS_TRACE1_ARG1("vds_ckpt_new_vdest\n");

   db_info = m_MMGR_ALLOC_VDS_DB_INFO;
   if (db_info == NULL)
   {
      VDS_TRACE1_ARG1("vds_ckpt_new_vdest : failed in NULL check\n");
      return NULL;
   }

   m_NCS_OS_MEMSET(db_info, 0, sizeof(db_info));
   db_info->vdest_name = info->vdest_name;
   db_info->persistent = info->persistent;
   db_info->adest_list = NULL; 

   db_info->name_pat_node.key_info = (uns8*)&db_info->vdest_name;

   rc =  ncs_patricia_tree_add(&vds_cb->vdest_name_db, &db_info->name_pat_node);
   if(rc != NCSCC_RC_SUCCESS)
   {
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_NAME,
                               VDS_LOG_PAT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
   }
   else
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_NAME,
                             VDS_LOG_PAT_SUCCESS,
                                     NCSFL_SEV_INFO, rc);


   db_info->vdest_id = info->vdest_id;
   db_info->id_pat_node.key_info = (uns8*)&db_info->vdest_id;

   rc = ncs_patricia_tree_add(&vds_cb->vdest_id_db, &db_info->id_pat_node);
   if(rc != NCSCC_RC_SUCCESS)
   {
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_ID,
                               VDS_LOG_PAT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
   }
   else
     m_VDS_LOG_TREE(VDS_LOG_PAT_ADD_ID,
                             VDS_LOG_PAT_SUCCESS,
                                      NCSFL_SEV_INFO, rc);
 
  if(db_info->persistent == FALSE)
   {
    m_VDS_LOG_VDEST(db_info->vdest_name.value,m_MDS_GET_VDEST_ID_FROM_MDS_DEST(db_info->vdest_id),"F-NEW- CKPT-READ");
   }
   else
   {
    m_VDS_LOG_VDEST(db_info->vdest_name.value,m_MDS_GET_VDEST_ID_FROM_MDS_DEST(db_info->vdest_id),"T-NEW-CKPT-READ");
   }

   return db_info;
}


/****************************************************************************
  PROCEDURE NAME : vds_ckpt_read()

  DESCRIPTION    : Reads checkpoint and populates the VDS_DB and VDS_CB.

  ARGUMENTS      : cb - ptr to the VDS control block

  RETURNS        : Success : NCSCC_RC_SUCCESS
                   Failure : NCSCC_RC_FAILURE

  NOTES          : None
*****************************************************************************/
uns32 vds_ckpt_read(VDS_CB *cb)
{
   VDS_CKPT_DBINFO          vds_ckpt_dbinfo_node;
   VDS_VDEST_DB_INFO        *vdest_db_info_node = NULL;
   VDS_VDEST_ADEST_INFO     *tmp_adest_node = NULL;
   SaCkptIOVectorElementT   io_vector;
   SaUint32T                *erroneous_vector_index = NULL;
   SaAisErrorT              rc = SA_AIS_OK;
   SaAisErrorT              rc_while = SA_AIS_OK;
   SaCkptSectionDescriptorT sec_desc;
   SaCkptSectionIterationHandleT sec_iter_hdl = 0;
   int sec_iter_count = 0;
   int i = 0;
   SaCkptSectionIdT section_id;
   SaCkptSectionCreationAttributesT sec_create_attr;
   MDS_DEST temp_latest_allocated_vdest = 0;
   uns32 vds_version_current = 1; /* Hard coding VDS version to 1 */
   uns32 vds_version_obtained = 0; /* Hard coding VDS version to 1 */
   uns8 vds_data_buffer[VDS_CKPT_BUFFER_SIZE];


   VDS_TRACE1_ARG1("vds_ckpt_read\n");

   /* Create a section for VDS version if not present */

   m_NCS_OS_MEMSET(&io_vector, 0, sizeof(SaCkptIOVectorElementT));
   m_NCS_OS_MEMSET(&section_id, 0, sizeof(SaCkptSectionIdT));

   section_id.id = (SaUint8T *)VDS_CKPT_VERSION_SECTIONID;
   section_id.idLen = VDS_CKPT_VERSION_SECTIONID_LEN;

   io_vector.sectionId  = section_id;
   io_vector.dataBuffer = &vds_version_obtained;
   io_vector.dataOffset = 0;
   io_vector.readSize   = 0;
   io_vector.dataSize = sizeof(vds_version_obtained);

    /* Read VDS version section if it exists */
    rc = saCkptCheckpointRead(cb->ckpt.checkpointHandle,
                            &io_vector,
                            VDS_CKPT_NO_OF_SECTIONS_TOREAD,
                            erroneous_vector_index);
    /* If VDS version section doesn't exist, create one */
    if (rc == SA_AIS_ERR_NOT_EXIST)
    {
        m_NCS_OS_MEMSET(&sec_create_attr, 0,
                       sizeof(SaCkptSectionCreationAttributesT));
        sec_create_attr.sectionId = &section_id;
        sec_create_attr.expirationTime   = VDS_CKPT_DBINFO_RET_TIME;

        rc = saCkptSectionCreate(cb->ckpt.checkpointHandle,
                                &sec_create_attr,
                                (uns8 *)&vds_version_current,
                                sizeof(vds_version_current));
        if (rc != SA_AIS_OK)
        {
            m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_VDS_VERSION_CREATE,
                                     VDS_LOG_CKPT_FAILURE,
                                           NCSFL_SEV_ERROR ,rc);
            if (rc == SA_AIS_ERR_TRY_AGAIN)
             {
                 m_NCS_TASK_SLEEP(1000);
             }
            saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                            0, SA_AMF_COMPONENT_RESTART, 0);
            return NCSCC_RC_FAILURE;
        }
    }
    else if (rc != SA_AIS_OK)
    {
        m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_VDS_VERSION_READ,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR ,rc);
        if (rc == SA_AIS_ERR_TRY_AGAIN)
         {
                 m_NCS_TASK_SLEEP(1000);
         }
        saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                            0, SA_AMF_COMPONENT_RESTART, 0);
        return NCSCC_RC_FAILURE;
    }


   /* Initializes the section iterator */
   rc = saCkptSectionIterationInitialize(cb->ckpt.checkpointHandle,
                                         SA_CKPT_SECTIONS_ANY,
                                         0,
                                         &sec_iter_hdl);             
   if (rc != SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_ITER_INIT,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR ,rc);
      
      /* Added by vishal : for VDS error handling enhancement */
      if (rc == SA_AIS_ERR_TRY_AGAIN)
      {
          m_NCS_TASK_SLEEP(1000);
      }
      saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                    0, SA_AMF_COMPONENT_RESTART, 0);

      return NCSCC_RC_FAILURE;
   }  

   m_NCS_OS_MEMSET(&sec_desc, 0, sizeof(SaCkptSectionDescriptorT));
   
   /* Moves the iterator to next section and fills sec_desc with 
      section attributes i.e section_id */  
   while((rc_while = saCkptSectionIterationNext(sec_iter_hdl, &sec_desc))
         == SA_AIS_OK)
   { 
                
       VDS_TRACE1_ARG1("vds_ckpt_read:saCkptSectionIterationNext\n");
    
       m_VDS_LOG_GENERIC("SECTIONID in ",m_MDS_GET_VDEST_ID_FROM_MDS_DEST(*((uns64 *)sec_desc.sectionId.id)),"CKPTREAD -ITnext");
       sec_iter_count++;
       m_NCS_OS_MEMSET(&io_vector, 0, sizeof(SaCkptIOVectorElementT));
       
       /* Fill io_vector */
       io_vector.sectionId  = sec_desc.sectionId;
       io_vector.dataOffset = 0;
       io_vector.readSize   = 0;

       /* checks for section with VDS_CKPT_LATEST_VDEST_SECTIONID */    
       if (((sec_desc.sectionId.idLen == VDS_CKPT_LATEST_VDEST_SECTIONID_LEN) &&
            (!(m_NCS_OS_MEMCMP((void *)sec_desc.sectionId.id, 
                               (void *)VDS_CKPT_LATEST_VDEST_SECTIONID,
                               VDS_CKPT_LATEST_VDEST_SECTIONID_LEN)))))
       {
          
          io_vector.dataBuffer = &temp_latest_allocated_vdest;
          /* Fill io_vector datasize */
          io_vector.dataSize = sizeof(MDS_DEST);
          
          /* reads the section with VDS_CKPT_LATEST_VDEST_SECTIONID 
             as sectionId */   
          rc = saCkptCheckpointRead(cb->ckpt.checkpointHandle,
                                    &io_vector,
                                    VDS_CKPT_NO_OF_SECTIONS_TOREAD,
                                    erroneous_vector_index);
          if (rc == SA_AIS_OK)
          { 
             /* copies the section content into latest_allocated_vdest */
             cb->latest_allocated_vdest = m_NCS_OS_NTOHLL_P(io_vector.dataBuffer);
             initial_vdest = FALSE;
          }
          else
          {
             m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_CB_READ,
                                    VDS_LOG_CKPT_FAILURE,
                                            NCSFL_SEV_ERROR, rc);

             /* Added by vishal : for VDS error handling enhancement */
             if (rc == SA_AIS_ERR_TRY_AGAIN)
             {
                 m_NCS_TASK_SLEEP(1000);
             }
             saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                            0, SA_AMF_COMPONENT_RESTART, 0);
          }
       }
       /* Check for section with VDS_CKPT_VERSION_SECTIONID */ 
       else if ((sec_desc.sectionId.idLen == VDS_CKPT_VERSION_SECTIONID_LEN) &&
                   ( !(m_NCS_OS_MEMCMP((void *)sec_desc.sectionId.id, 
                                       (void *)VDS_CKPT_VERSION_SECTIONID,
                                       VDS_CKPT_VERSION_SECTIONID_LEN))))
       {
             /* Do Nothing */
       }   
       else
       {
          m_NCS_OS_MEMSET(&vds_ckpt_dbinfo_node, 0, sizeof(VDS_CKPT_DBINFO));

          /* Fill io_vector datasize */
          io_vector.dataSize = VDS_CKPT_BUFFER_SIZE;

           m_NCS_OS_MEMSET(vds_data_buffer, 0 ,VDS_CKPT_BUFFER_SIZE);

          io_vector.dataBuffer = vds_data_buffer;

          /* reads the section with vdest_id as section id */ 
          rc = saCkptCheckpointRead(cb->ckpt.checkpointHandle,
                                    &io_vector,
                                    VDS_CKPT_NO_OF_SECTIONS_TOREAD,
                                    erroneous_vector_index);
          if (rc != SA_AIS_OK)
          {
             m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_DB_READ,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
             
             /* Added by vishal : for VDS error handling enhancement */
             if (rc == SA_AIS_ERR_TRY_AGAIN)
             {
                 m_NCS_TASK_SLEEP(1000);
             }
             saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                            0, SA_AMF_COMPONENT_RESTART, 0);
          }
          else
          {
             /* creates and populates VDS_VDEST_DB_INFO node from VDS_CKPT_DB_INFO 
                node */
              
             vds_ckpt_buffer_decode(&vds_ckpt_dbinfo_node, vds_data_buffer);

             vdest_db_info_node = vds_ckpt_new_vdest(cb, &vds_ckpt_dbinfo_node);
             if (vdest_db_info_node != NULL)
             {
                i = 0;

                VDS_TRACE1_ARG3("VDEST name  --  %s VDEST id-- %lld ",vdest_db_info_node->vdest_name.value, vdest_db_info_node->vdest_id);  
                
                while (i < vds_ckpt_dbinfo_node.adest_count-1)
                { 
                   /* creates and populates VDS_VDEST_ADEST_INFO node by 
                      given adest */
                   tmp_adest_node = vds_new_vdest_instance(vdest_db_info_node,
                                             &vds_ckpt_dbinfo_node.adest[i]);
                   if (tmp_adest_node == NULL)
                   {
                      VDS_TRACE1_ARG1("vds_ckpt_read : Failed in adest NULL check\n");
                   }
                   
                   VDS_TRACE1_ARG2("Adest -- %lld \n", tmp_adest_node->adest);  
                   i++;
                
                }
                VDS_TRACE1_ARG1("\n");
             }
             else
             {
                /* LOG the message */
                VDS_TRACE1_ARG1("vds_ckpt_read : Failed in db info NULL check\n");

             }
          }
       }

       m_NCS_OS_MEMSET(&sec_desc, 0, sizeof(SaCkptSectionDescriptorT));
   } /* End of while loop of section iteration */
   if (rc_while != SA_AIS_ERR_NO_SECTIONS)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_ITER_NEXT,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR ,rc_while);
      
      /* Added by vishal : for VDS error handling enhancement */
      if (rc_while == SA_AIS_ERR_TRY_AGAIN)
      {
          m_NCS_TASK_SLEEP(1000);
      }
      saAmfComponentErrorReport(cb->amf.amf_handle, &cb->amf.comp_name,
                                    0, SA_AMF_COMPONENT_RESTART, 0);

      return NCSCC_RC_FAILURE;
   }
   
   if(sec_iter_count == 0)
     VDS_TRACE1_ARG2("vds_ckpt_read  : No sections found %d \n",rc);
         
   /* Finalize the section Iterator */
   rc = saCkptSectionIterationFinalize(sec_iter_hdl);
   if (rc != SA_AIS_OK)
   {
      m_VDS_LOG_CKPT(VDS_LOG_CKPT_SEC_ITER_FIN,
                                 VDS_LOG_CKPT_FAILURE,
                                       NCSFL_SEV_ERROR, rc);
   }
   return NCSCC_RC_SUCCESS;
}

