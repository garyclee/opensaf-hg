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

  This file captures all the generic routines common to BAM module to generate 
  the mibsets for the SAF specified configurable objects in the AMF.
  
******************************************************************************
*/

#include <ncs_opt.h>
#include <gl_defs.h>
#include <t_suite.h>
#include <ncsgl_defs.h>
#include <ncs_mib_pub.h>
#include <mac_papi.h>
#include <ncsmiblib.h>

#include "bam.h"
#include "bamParser.h"
#include "bam_log.h"

EXTERN_C MABMAC_API uns32 gl_mac_handle;
EXTERN_C uns32 gl_bam_avd_cfg_msg_num;
EXTERN_C uns32 gl_bam_avm_cfg_msg_num;

/* This will go away in future release when the AVSv MIB is changed to 
** milliseconds.
*/
#define NCS_BAM_MILLI_TO_NANO 1000000
/*****************************************************************************
  PROCEDURE NAME: ncs_bam_cfg_mib_arg
  DESCRIPTION   : Populats/Initializes the MIB arg structure.
  ARGUMENTS     :
                  mib      : Pointer to NCSMIB_ARG structure
                  index    : Index of the table
                  index_len: Index length
                  tid      : Table ID
                  usr_key  : MIB Key pointer
                  ss_key   : Subsystem key pointer
                  rspfnc   : Response function
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/
static uns32 
ncs_bam_cfg_mib_arg(NCSMIB_ARG    *mib,
                       uns32        *index,
                       uns32         index_len,
                       NCSMIB_TBL_ID  tid)
{
   
   ncsmib_init(mib);
   
   mib->i_idx.i_inst_ids  = index;
   mib->i_idx.i_inst_len  = index_len;
   mib->i_tbl_id          = tid;
   /* The response function, user handle/key and 
    * service provider (mab?) key is not encoded at this point
    *
    * Add when necessary
    */
#if 0
   mib->i_usr_key         = usr_hdl;
   mib->i_mib_key         = ss_hdl;  
   mib->i_rsp_fnc         = rspfnc;
#endif
   
   return NCSCC_RC_SUCCESS;
}

uns32
ncs_bam_build_mib_idx(NCSMIB_IDX *mib_idx, char *idx_val, NCSMIB_FMAT_ID format)
{
   uns32 *ptr = NULL;
   uns32 inst_len=0;
   
   switch(format)
   {
   case NCSMIB_FMAT_INT:
      //mib_idx->i_inst_ids[0] = atoi(idx_val);
      /* the malloc size is two times the uns32, one for the length */

      mib_idx->i_inst_ids = (uns32 *)m_MMGR_ALLOC_BAM_DEFAULT_VAL(sizeof(uns32));
      
      if(mib_idx->i_inst_ids == NULL)
      {
         m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
         break;
      }
      ptr = (uns32 *)mib_idx->i_inst_ids;
      //ptr[0] = 1;
      ptr[0] = atoi(idx_val);
      mib_idx->i_inst_len = 1;
      break;
   case NCSMIB_FMAT_OCT:
      
      /*mib_idx->i_inst_ids = (uns32 *)malloc(sizeof(idx_val) * sizeof(uns32));*/
      /* +1 for the prefix length in the instance. */
	   mib_idx->i_inst_ids = (uns32 *)m_MMGR_ALLOC_BAM_DEFAULT_VAL((strlen(idx_val) + 1) * sizeof(uns32));
      if(mib_idx->i_inst_ids == NULL)
      {
         m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
         break;
      }
      ptr = (uns32 *)mib_idx->i_inst_ids;
      inst_len = strlen(idx_val);

      mib_idx->i_inst_len = inst_len+1;
      *ptr++ = inst_len;
      while(inst_len--)
         *ptr++ = (uns32)(*idx_val++);
      break;
   case NCSMIB_FMAT_BOOL:
   default:
      mib_idx->i_inst_len = 0;
      mib_idx->i_inst_ids = NULL;
      return 0;
   }
   return 0;
}

/* This function add secondary index to the existing primary index */
uns32
ncs_bam_add_sec_mib_idx(NCSMIB_IDX *mib_idx, char *idx_val, NCSMIB_FMAT_ID format)
{
   uns32 *ptr = NULL;
   uns32 temp_buff[128];
   uns32 oct_len = 0;
   uns32 inst_len=mib_idx->i_inst_len;

   /* copy the contents into a temp buffer */
   m_NCS_MEMCPY(temp_buff, mib_idx->i_inst_ids, mib_idx->i_inst_len * sizeof(uns32));

   ncs_bam_free(mib_idx->i_inst_ids);

   switch(format)
   {
   case NCSMIB_FMAT_INT:

      mib_idx->i_inst_ids = (uns32 *)m_MMGR_ALLOC_BAM_DEFAULT_VAL(
                                 (inst_len + 1) * sizeof(uns32));

      if(mib_idx->i_inst_ids == NULL)
      {
         m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
         break;
      }
      /* copy the contents back from temp buffer */
      m_NCS_MEMCPY((uns32 *)mib_idx->i_inst_ids, temp_buff, inst_len * sizeof(uns32));

      /* Move the ptr to the end of the existing index in mib_idx struct */
      ptr = (uns32 *)mib_idx->i_inst_ids + mib_idx->i_inst_len;
            
      ptr[0] = atoi(idx_val);
      mib_idx->i_inst_len = inst_len + 1;
      break;
   case NCSMIB_FMAT_OCT:
      
      /* +1 for the prefix length in the instance. */
      mib_idx->i_inst_ids = (uns32 *)m_MMGR_ALLOC_BAM_DEFAULT_VAL(
                              (inst_len + strlen(idx_val) + 1) * sizeof(uns32));
      
      if(mib_idx->i_inst_ids == NULL)
      {
         m_LOG_BAM_MEMFAIL(BAM_ALLOC_FAILED);
         break;
      }
      /* copy the contents back from temp buffer */
      m_NCS_MEMCPY((uns32 *)mib_idx->i_inst_ids, temp_buff, inst_len * sizeof(uns32));
      
      /* Move the ptr to the end of the existing index in mib_idx struct */
      ptr = (uns32 *)mib_idx->i_inst_ids + mib_idx->i_inst_len;
      
      oct_len = strlen(idx_val);
      
      mib_idx->i_inst_len = inst_len+oct_len+1;
      
      *ptr++ = oct_len;
      while(oct_len--)
         *ptr++ = (uns32)(*idx_val++);

      break;
   case NCSMIB_FMAT_BOOL:
   default:
      return 0;
   }
   return 0;
}

uns32
ncs_bam_generate_counter64_mibset(NCSMIB_TBL_ID table_id, uns32 param_id,
                                NCSMIB_IDX *mib_idx, char *val)
{
   NCSMIB_ARG  mib_arg;
   uns8        space[1024];
   NCSMEM_AID  mem_aid;
   uns32       status = NCSCC_RC_SUCCESS;
   uns8        buff_64bit[8];
   SaInt64T    value = 0;

   m_NCS_OS_MEMSET(&mib_arg, 0, sizeof(NCSMIB_ARG));
   m_NCS_OS_MEMSET(space, 0, sizeof(space));

   ncs_bam_cfg_mib_arg(&mib_arg, (uns32*)mib_idx->i_inst_ids, 
                                  mib_idx->i_inst_len, table_id); 
   ncsmem_aid_init(&mem_aid, space, 1024);

   NCSMIB_SET_REQ*       set = &mib_arg.req.info.set_req;
   mib_arg.i_op   = NCSMIB_OP_REQ_SET;
   set->i_param_val.i_fmat_id  = NCSMIB_FMAT_OCT;
   set->i_param_val.i_param_id = param_id;
   set->i_param_val.i_length = 8;

   for(uns8 x=0; x < strlen(val); x++)
   {
      char c = val[x];
      value = (value * 10) + atoi(&c);
   }

   /* This will go away in future release when the AVSv MIB is changed to 
   ** milliseconds.
   */
   value = value * NCS_BAM_MILLI_TO_NANO;

   set->i_param_val.info.i_oct = &buff_64bit[0];
   m_NCS_OS_HTONLL_P(&buff_64bit[0], value);

   /* send the request to MAB */
   /* Fill in the Key  */
   mib_arg.i_mib_key = (uns64)gl_mac_handle; 
   mib_arg.i_usr_key = (uns64)gl_mac_handle; 
   
   m_NCS_OS_MEMSET(space, 0, sizeof(space));
   
   
   status = ncsmib_sync_request(&mib_arg, ncsmac_mib_request, 
                                 NCS_BAM_MIB_REQ_TIMEOUT, &mem_aid);
   if ((status != NCSCC_RC_SUCCESS) || (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS))
   {
      /* call the MAB function prototype to log the failed mib*/
      ncsmib_pp(&mib_arg);
   }
   gl_bam_avd_cfg_msg_num++;
   gl_bam_avm_cfg_msg_num++;
   return status;
}

/*****************************************************************************
  PROCEDURE NAME: ncs_bam_build_and_generate_mibsets
  DESCRIPTION   : Send out a Set request not SETROW.
  ARGUMENTS     :
                  
  RETURNS       : SUCCESS or FAILURE
  NOTES:
*****************************************************************************/

uns32
ncs_bam_build_and_generate_mibsets(NCSMIB_TBL_ID table_id, uns32 param_id, 
                                NCSMIB_IDX *mib_idx, char *val, NCSMIB_FMAT_ID format)
{
   NCSMIB_ARG  mib_arg;
   uns8        space[1024];
   NCSMEM_AID   mem_aid;
   uns32       status = NCSCC_RC_SUCCESS;

   if(val == NULL)
      return status;

   m_NCS_OS_MEMSET(&mib_arg, 0, sizeof(NCSMIB_ARG));
   m_NCS_OS_MEMSET(space, 0, sizeof(space));

   ncs_bam_cfg_mib_arg(&mib_arg, (uns32*)mib_idx->i_inst_ids,mib_idx->i_inst_len, table_id); 
   ncsmem_aid_init(&mem_aid, space, 1024);

   NCSMIB_SET_REQ*       set = &mib_arg.req.info.set_req;
   mib_arg.i_op   = NCSMIB_OP_REQ_SET;
   set->i_param_val.i_fmat_id  = format;

   switch(format)
   {
   case NCSMIB_FMAT_INT:
      
      set->i_param_val.i_param_id = param_id;
      set->i_param_val.i_length = 4;
      set->i_param_val.info.i_int = atoi(val);
      break;
   case NCSMIB_FMAT_OCT:
      set->i_param_val.i_param_id = param_id;
      set->i_param_val.info.i_oct = (uns8*)val;
      set->i_param_val.i_length   = (uns8)strlen(val);
      if(strlen(val) == 0)
         return status;
      break;
#if 0
   case NCSMIB_FMAT_2LONG:
      actlen = (uns8)strlen(val);
      set->i_param_val.i_param_id = param_id;
      set->i_param_val.i_length = 8;

      if(strlen(val) > 8)
      {
         /* not a INT64 better send it as oct stream */
         set->i_param_val.info.i_oct = (uns8*)val;
         set->i_param_val.i_length   = actlen;
      }
      for(x=0; x < 8; x++)
      {
         buff_64bit[x] = 0;
      }
      for(x=0; x < actlen; x++)
      {
         char tmp = val[x];
         /*buff_64bit[8-actlen+x] = atoi(&tmp);*/
         buff_64bit[x] = atoi(&tmp);
      }
      my_long_long = (SaInt64T *)malloc(8 * sizeof(uns8));
      my_long_long = (SaInt64T *)&buff_64bit[0];

      m_NCS_OS_HTONLL_P(set->i_param_val.info.i_oct, *my_long_long);
      set->i_param_val.info.i_oct = (const unsigned char *)&buff_64bit;
      break;

#endif
   case NCSMIB_FMAT_BOOL:
      set->i_param_val.i_fmat_id  = NCSMIB_FMAT_INT;
      set->i_param_val.i_param_id = param_id;
      set->i_param_val.i_length = 4;

      if((strcmp(val,"0") == 0) || (strcmp(val,"false") == 0) )
      {
         set->i_param_val.info.i_int = NCS_SNMP_FALSE;
      }
      else
      {
         set->i_param_val.info.i_int = NCS_SNMP_TRUE;
      }
      break;

   default:
      return status;
   }
   
   /* send the request to MAB */
   /* Fill in the Key  */
   mib_arg.i_mib_key = (uns64)gl_mac_handle; 
   mib_arg.i_usr_key = (uns64)gl_mac_handle; 
   
   m_NCS_OS_MEMSET(space, 0, sizeof(space));
   
   
   status = ncsmib_sync_request(&mib_arg, ncsmac_mib_request, 
                                 NCS_BAM_MIB_REQ_TIMEOUT, &mem_aid);
   if ((status != NCSCC_RC_SUCCESS) || (mib_arg.rsp.i_status != NCSCC_RC_SUCCESS ))
   {
      /* call the MAB function prototype to print the failed mib */
      ncsmib_pp(&mib_arg);
   }
   else
   {
      /* Sent the request to MAB, and the response is in mib_arg
       *
       * Want to do anything ??
       */

   }
   gl_bam_avd_cfg_msg_num++;
   gl_bam_avm_cfg_msg_num++;

   return status;
}

/*****************************************************************************
  PROCEDURE NAME: ncs_bam_search_table_for_oid
  DESCRIPTION   : Searches the array and populates the table-id and param-id.
  ARGUMENTS     :                  
  RETURNS       : 
  NOTES:
*****************************************************************************/

SaAisErrorT 
ncs_bam_search_table_for_oid(TAG_OID_MAP_NODE *globalTable, uns32 table_size,
                             char * lkupNode, 
                             char *tagName, NCSMIB_TBL_ID *table_id, 
                             uns32 *param_id, NCSMIB_FMAT_ID  *format)
{
   TAG_OID_MAP_NODE 	tmp_node;
   for (int x=0; x< (int)table_size; x++)
   {
      tmp_node = globalTable[x];
      if(strcmp(tmp_node.pKey, lkupNode) == 0)
      {
         if(strcmp(tmp_node.sKey, tagName) == 0)
         {
            /* entry found return the param_id and table_id */
            *param_id = tmp_node.param_id;	
            *table_id = tmp_node.table_id;
            *format  =  tmp_node.format;
            return SA_AIS_OK;
         }
      }
   }
   return SA_AIS_ERR_NOT_EXIST;
} /* end search routine */


/*****************************************************************************
  PROCEDURE NAME: get_dom_node_by_name
  DESCRIPTION   : This routine accepts the type of prototype and the instance
                  name passed by the instance and returns the DOMNode in the
                  XML document. The second argument is type one of the 
                  a. nodePrototype
                  b. SUPrototype
                  c. CSIPrototype etc..
  ARGUMENTS     :                  
  RETURNS       : 
  NOTES:
*****************************************************************************/

DOMNode *
get_dom_node_by_name(char *prototypeName, char* type)
{
   DOMNode *tmpNode;
   
   XMLCh *str = XMLString::transcode(type);
   DOMNodeList *element = gl_dom_doc->getElementsByTagName( str);

   if(element->getLength())
   {
      for(unsigned int i=0;i < element->getLength();i++)
      {
           tmpNode = element->item(i);
           if(tmpNode->getNodeType() != DOMNode::ELEMENT_NODE)
           {
              continue;
           }
           DOMNamedNodeMap *attributesNodes = tmpNode->getAttributes();
           if(attributesNodes->getLength() )
           {
              for(unsigned int x=0; x < attributesNodes->getLength(); x++)
              {
                 char *tag = XMLString::transcode(attributesNodes->item(x)->getNodeName());
                 if(m_NCS_STRCMP(tag, "name") == 0 )
                 {
                    char *val = XMLString::transcode(attributesNodes->item(x)->getNodeValue());
                    if(m_NCS_STRCMP(val, prototypeName) == 0)
                    {
                       XMLString::release(&tag);
                       XMLString::release(&val);
                       return tmpNode;
                    }
                    XMLString::release(&val);
                 }
                 XMLString::release(&tag);
              }
           }
      }
   }
   XMLString::release(&str);
   return NULL;
}
