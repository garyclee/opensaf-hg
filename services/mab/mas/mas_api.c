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



  DESCRIPTION:

  This file contains all Public APIs for the MIB Access Client (MAC) portion
  of the MIB Acess Broker (MAB) subsystemt.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "mab.h"

#if (NCS_MAB == 1)

/* global pointer to get the MAS_TBL in the AMF callback */
MAS_TBL*         gl_mas_tbl;
extern MAS_ATTRIBS gl_mas_amf_attribs;

/*****************************************************************************

  PROCEDURE NAME:    ncsmas_lm

  DESCRIPTION:       Core API for all MIB Access Server layer management 
                     operations used by a target system to instantiate and
                     control a MAS instance. Its operations include:

                     CREATE  a MAS instance
                     DESTROY a MAS instance
                     SET     a MAS configuration object
                     GET     a MAS configuration object

  RETURNS:           SUCCESS - All went well
                     FAILURE - internal processing didn't like something.
                               enable m_MAB_DBG_SINK() for details.

*****************************************************************************/
uns32 ncsmas_lm( NCSMAS_LM_ARG* arg)
{

    if (arg == NULL)
        return m_MAB_DBG_SINK(NCSCC_RC_FAILURE); 
        
    switch(arg->i_op)
    {
        /* create MAS */ 
        case  NCSMAS_LM_OP_CREATE:
            return mas_svc_create(&arg->info.create);

        /* destroy MAS */ 
        case  NCSMAS_LM_OP_DESTROY:
            return mas_svc_destroy(&arg->info.destroy);

        /* Can not help for any other type of request */ 
        default:
          break;
    }

    return m_MAB_DBG_SINK(NCSCC_RC_FAILURE);
}


/*****************************************************************************
 *****************************************************************************

     P R I V A T E  Support Functions for MAS APIs are below

*****************************************************************************
*****************************************************************************/


/*****************************************************************************

  PROCEDURE NAME:    mas_svc_create

  DESCRIPTION:       Create an instance of MAS, set configuration profile to
                     default, install this MAS with MDS and subscribe to MAS
                     events.

  RETURNS:           SUCCESS - All went well
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 mas_svc_create(NCSMAS_CREATE* create)
{
    MAS_CSI_NODE      *csi = NULL; 
    SaAmfHAStateT     init_ha_role; 
    uns32             status;
    
    m_MAB_DBG_TRACE("\nmas_svc_create():entered.");

    m_LOG_MAB_API(MAB_API_MAS_SVC_CREATE);

    if (create == NULL)
        return NCSCC_RC_FAILURE; 

    /* get the initial role from RDA */ 
    status = app_rda_init_role_get((NCS_APP_AMF_HA_STATE*)&init_ha_role); 
    if (status != NCSCC_RC_SUCCESS)
    {
        /* log the failure */ 
        m_LOG_MAB_ERROR_I(NCSFL_SEV_CRITICAL, 
                          MAB_MAS_ERR_RDA_INIT_ROLE_GET_FAILED, status);
        m_MAB_DBG_TRACE("\nmas_svc_create():left:RDA init role get failed");
        return status;
    }

    /* log the RDA returned state */ 
    m_LOG_MAB_HDLN_I(NCSFL_SEV_NOTICE, MAB_HDLN_RDA_GIVEN_INIT_ROLE, init_ha_role);

    /* install the default CSI */ 
    csi = mas_amf_csi_install(m_MAS_DEFAULT_ENV_ID, init_ha_role); 
    if (csi == NULL)
    {
        /* log the error */ 
        m_LOG_MAB_ERROR_II(NCSFL_LC_FUNC_RET_FAIL, NCSFL_SEV_CRITICAL, 
                           MAB_MAS_ERR_DEF_ENV_INIT_FAILED, 
                           m_MAS_DEFAULT_ENV_ID, init_ha_role); 
        m_MAB_DBG_TRACE("\nmas_svc_create():left:CSI install failed");
        return NCSCC_RC_FAILURE;  
    }

    /* update the env handle */ 
    csi->csi_info.env_hdl = create->i_mds_hdl; 

    /* add to the list of CSIs */ 
    mas_amf_csi_list_add_front(csi); 

    /* log that DEFAULT PWE is installed successfully */ 
    m_LOG_MAB_HDLN_II(NCSFL_SEV_INFO, MAB_HDLN_MAS_DEF_ENV_INIT_SUCCESS, 
                      m_MAS_DEFAULT_ENV_ID, init_ha_role); 

    m_MAB_DBG_TRACE("\nmas_svc_create():left.");
    return NCSCC_RC_SUCCESS; 
}

/*****************************************************************************

  PROCEDURE NAME:    mas_svc_destroy

  DESCRIPTION:       Destroy an instance of MAS. Withdraw from MDS and free
                     this MAS_TBL of MAS.

  RETURNS:           SUCCESS - all went well.
                     FAILURE - something went wrong. Turn on m_MAB_DBG_SINK()
                               for details.

*****************************************************************************/
uns32 mas_svc_destroy(NCSMAS_DESTROY * destroy)
{
  uns32 status = NCSCC_RC_FAILURE; 
  SaAisErrorT saf_status = SA_AIS_OK;
#if (NCS_MAS_RED == 1)
  NCS_MBCSV_ARG mbcsv_arg; 
#endif

  m_MAB_DBG_TRACE("\nmas_svc_destroy():entered.");

  m_LOG_MAB_API(MAB_API_MAS_SVC_DESTROY);
  
  status = mas_amf_prepare_will(); 
  if(status != NCSCC_RC_SUCCESS)
  {
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                             MAB_MAS_ERR_PREPARE_WILL_FAILED, status);
        m_MAB_DBG_TRACE("\nmas_svc_destroy():left.");
  }

  /* finalize the interface with AMF */
  saf_status = ncs_app_amf_finalize(&gl_mas_amf_attribs.amf_attribs);
  if (saf_status != SA_AIS_OK)
  {
        /* log that AMF interface finalize failed */
        m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                             MAB_MAS_ERR_APP_AMF_FINALIZE_FAILED, saf_status);
        m_MAB_DBG_TRACE("\nmas_svc_destroy():left.");
  }

#if (NCS_MAS_RED == 1)
  /* finalize the interface with MBCSv */ 
  m_NCS_OS_MEMSET(&mbcsv_arg, 0, sizeof(NCS_MBCSV_ARG)); 
  mbcsv_arg.i_op = NCS_MBCSV_OP_FINALIZE; 
  /* Not good to access the global variable at some many different places */ 
  mbcsv_arg.i_mbcsv_hdl = gl_mas_amf_attribs.mbcsv_attribs.mbcsv_hdl; 
  status = ncs_mbcsv_svc(&mbcsv_arg); 
  if (status != NCSCC_RC_SUCCESS)
  {
      /* log the failure */ 
      m_LOG_MAB_ERROR_DATA(NCSFL_SEV_CRITICAL, NCSFL_LC_SVC_PRVDR,
                           MAB_MAS_ERR_MBCSV_FINALIZE_FAILED, status);
      m_MAB_DBG_TRACE("\nmas_svc_destroy():left.");
  }
#endif

  m_MAB_DBG_TRACE("\nmas_svc_destroy():left.");
  return status;
}

#endif /* (NCS_MAB == 1) */


