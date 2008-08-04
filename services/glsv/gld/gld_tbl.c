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

#include "gld.h"

static uns32 glsv_maprec_getnext(GLSV_GLD_CB *cb,SaNameT *ckpt_name,GLSV_GLD_RSC_MAP_INFO *map_infoa);
static uns32 glsv_gld_map_from_instid(const uns32* i_inst_ids, uns32 i_inst_len, GLSV_GLD_RSC_MAP_INFO  *rsc_map_info,GLSV_GLD_CB *gld_cb);

/****************************************************************************
 * Name          : glsv_gld_map_from_instid
 *
 * Description   : This function maps instance_id to GLSV_GLD_RSC_MAP_INFO
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32 glsv_gld_map_from_instid(const uns32* i_inst_ids, uns32 i_inst_len, GLSV_GLD_RSC_MAP_INFO *rsc_map_info,GLSV_GLD_CB *gld_cb)
{
   SaNameT rsc_name;
   uns32   counter=0;
   GLSV_GLD_RSC_MAP_INFO *tmp_rsc_map_info = NULL;

   if(i_inst_ids == NULL)
      return NCSCC_RC_FAILURE; 

   m_NCS_MEMSET(&rsc_name,'\0',sizeof(SaNameT));

   rsc_name.length = *i_inst_ids;
   i_inst_ids++;

   for(;counter < rsc_name.length ;counter++)
   {
       rsc_name.value[counter] = (uns8)*i_inst_ids;
      i_inst_ids++;
   }
   rsc_name.value[counter]='\0';
   #if 0
   rsc_name.length = rsc_name.length + 1;
   #endif
   rsc_name.length = rsc_name.length;

   rsc_name.length = m_NCS_OS_HTONS(rsc_name.length);

   tmp_rsc_map_info = (GLSV_GLD_RSC_MAP_INFO *)ncs_patricia_tree_get(&gld_cb->rsc_map_info,
                                    (uns8*)&rsc_name);
   if(tmp_rsc_map_info)
     m_NCS_MEMCPY(rsc_map_info, tmp_rsc_map_info, sizeof(GLSV_GLD_RSC_MAP_INFO));

   return NCSCC_RC_SUCCESS;
}
/****************************************************************************
 * Name          :  salckresourceentry_set
 *
 * Description   :
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32  salckresourceentry_set(NCSCONTEXT cb, NCSMIB_ARG *arg,NCSMIB_VAR_INFO* var_info,NCS_BOOL test_flag)
{
      return NCSCC_RC_FAILURE; 
}
/****************************************************************************
 * Name          : salckresourceentry_get
 *
 * Description   :
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32 salckresourceentry_get(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data)
{
   GLSV_GLD_CB           *gld_cb = NULL;
   uns32                 rc = NCSCC_RC_SUCCESS;
   GLSV_GLD_RSC_INFO     *rsc_info = NULL;
   GLSV_GLD_RSC_MAP_INFO  rsc_map_info;

   m_NCS_MEMSET(&rsc_map_info,'\0',sizeof(GLSV_GLD_RSC_MAP_INFO));

   /* Get the CB pointer from the CB handle */
   gld_cb = (GLSV_GLD_CB *)ncshm_take_hdl(NCS_SERVICE_ID_GLD, arg->i_mib_key);

   if (gld_cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   if (gld_cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

    rc = glsv_gld_map_from_instid(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len, &rsc_map_info, gld_cb);
    if( rc != NCSCC_RC_SUCCESS)
    {
      /* Log the error */
       ncshm_give_hdl(gld_cb->my_hdl);
       return rc;
    }
   rsc_info = (GLSV_GLD_RSC_INFO*)ncs_patricia_tree_get(&gld_cb->rsc_info_id, 
                      (uns8*)&rsc_map_info.rsc_id);

   ncshm_give_hdl(gld_cb->my_hdl);

   if(rsc_info)
      *data = (NCSCONTEXT)rsc_info;
    else 
      return NCSCC_RC_FAILURE; 
    return NCSCC_RC_SUCCESS;
}
/****************************************************************************
 * Name          : salckresourceentry_extract
 *
 * Description   :
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32 salckresourceentry_extract(NCSMIB_PARAM_VAL *param, NCSMIB_VAR_INFO *var_info, 
                NCSCONTEXT data, NCSCONTEXT buffer)
{
    SaTimeT   creation_time; 
    uns32 rc = NCSCC_RC_FAILURE;
    GLSV_GLD_RSC_INFO *rsc_info = NULL;

    rsc_info = (GLSV_GLD_RSC_INFO*)data;

    if(var_info->param_id == saLckResourceCreationTimeStamp_ID )
    {
      creation_time = rsc_info->saf_rsc_creation_time;
      m_GLSV_UNS64_TO_PARAM(param,buffer,creation_time);
      return NCSCC_RC_SUCCESS;  
    }
    else
    if(var_info->param_id == saLckResourceIsOrphaned_ID)
    {
      param->i_fmat_id  = NCSMIB_FMAT_INT;
      param->info.i_int = (rsc_info->can_orphan == 1)?1 :2;
      param->i_length   = 4;
      return NCSCC_RC_SUCCESS;  
    }
    else
    {
        rc = ncsmiblib_get_obj_val(param, var_info, data, buffer);
        return rc;
    }

    return rc;
}
/****************************************************************************
 * Name          : glsv_maprec_getnext
 *
 * Description   : This function returns next 
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32 glsv_maprec_getnext(GLSV_GLD_CB *gld_cb,SaNameT *rsc_name,GLSV_GLD_RSC_MAP_INFO *map_info)
{
   GLSV_GLD_RSC_MAP_INFO *map;
   if(rsc_name)
   {
       map = (GLSV_GLD_RSC_MAP_INFO*)ncs_patricia_tree_getnext(&gld_cb->rsc_map_info,(uns8*)rsc_name);
       if(map != NULL)
       {
          *map_info = *map;
       }
       else
       {
          return NCSCC_RC_FAILURE;
       }
   }
   else
   {
       /* Get the first node */
      map = (GLSV_GLD_RSC_MAP_INFO*)ncs_patricia_tree_getnext(&gld_cb->rsc_map_info,(uns8*)NULL);
      if ( map == NULL)
        return NCSCC_RC_NO_INSTANCE;

      *map_info = *map;
   }

   if ( map_info == NULL)
      return NCSCC_RC_NO_INSTANCE;

   return NCSCC_RC_SUCCESS;
}
/****************************************************************************
 * Name          : salckresourceentry_next
 *
 * Description   :
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32 salckresourceentry_next(NCSCONTEXT cb, NCSMIB_ARG *arg, NCSCONTEXT *data,uns32* next_inst_id, uns32 *next_inst_id_len)
{
   GLSV_GLD_CB          *gld_cb = NULL;
   uns32                 rc = NCSCC_RC_SUCCESS,counter;
   GLSV_GLD_RSC_INFO     *rsc_info = NULL;
   GLSV_GLD_RSC_MAP_INFO rsc_map_info;

   m_NCS_OS_MEMSET(&rsc_map_info,'\0',sizeof(GLSV_GLD_RSC_MAP_INFO));


   gld_cb = (NCSCONTEXT) ncshm_take_hdl(NCS_SERVICE_ID_GLD, gl_gld_hdl);
   if(gld_cb ==  NULL)
   {
      m_LOG_GLD_HEADLINE(GLD_TAKE_HANDLE_FAILED, NCSFL_SEV_ERROR);
      return NCSCC_RC_FAILURE;
   }

   if (gld_cb == NULL)
      return NCSCC_RC_NO_INSTANCE;

   if ( arg->i_idx.i_inst_len == 0)
   {
      /* Get the first node */
      rc = glsv_maprec_getnext(gld_cb,(SaNameT*)NULL,&rsc_map_info);
      if( rc != NCSCC_RC_SUCCESS)
         return NCSCC_RC_NO_INSTANCE;
   }
   else
   {
     rc = glsv_gld_map_from_instid(arg->i_idx.i_inst_ids,arg->i_idx.i_inst_len,
              &rsc_map_info,gld_cb);
     if( rc != NCSCC_RC_SUCCESS)
     {
       /* Log the error */
        ncshm_give_hdl(gld_cb->my_hdl);
        return NCSCC_RC_NO_INSTANCE;
     }
     rc = glsv_maprec_getnext(gld_cb,&rsc_map_info.rsc_name,&rsc_map_info);
     if( rc != NCSCC_RC_SUCCESS)
     {
       /* Log the error */
        ncshm_give_hdl(gld_cb->my_hdl);
        return NCSCC_RC_NO_INSTANCE;
     }
   }
  
   rsc_info = (GLSV_GLD_RSC_INFO*)ncs_patricia_tree_get(&gld_cb->rsc_info_id, (uns8*)&rsc_map_info.rsc_id);
   if(rsc_info == NULL )
   {
     return NCSCC_RC_NO_INSTANCE;
   }

   *data = (NCSCONTEXT)rsc_info;

   rsc_map_info.rsc_name.length = rsc_info->lck_name.length;
   #if 0
   next_inst_id[0] = rsc_info->lck_name.length - 1;
   #endif
   next_inst_id[0] = rsc_info->lck_name.length;
   for(counter=0;counter< rsc_info->lck_name.length;counter++)
   {
      *(next_inst_id+counter+1) = (uns32)rsc_map_info.rsc_name.value[counter];
   }
   *next_inst_id_len = rsc_info->lck_name.length + 1;
   #if 0
   *next_inst_id_len = rsc_info->lck_name.length;
   #endif

   ncshm_give_hdl(gld_cb->my_hdl);
   return NCSCC_RC_SUCCESS;
}

/****************************************************************************
 * Name          : salckresourceentry_setrow
 *
 * Description   : 
 *
 * Arguments     :
 *
 * Return Values :
 *
 * Notes         : None.
 *****************************************************************************/
uns32 salckresourceentry_setrow(NCSCONTEXT cb, NCSMIB_ARG* args,
                                   NCSMIB_SETROW_PARAM_VAL* params,
                                   struct ncsmib_obj_info* obj_info,
                                   NCS_BOOL testrow_flag)
{
          return NCSCC_RC_FAILURE;
}
