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

  
  .....................................................................  
  
  DESCRIPTION: This file describes AMF Interface definitions for PSS
*****************************************************************************/ 
#ifndef PSR_AMF_H
#define PSR_AMF_H

/*****************************************************************************
 * Data Structure Used to hold mapping between handle, vrid and mail box
 *****************************************************************************/
typedef struct pssmbx_info 
{
   SYSF_MBX    pss_mbx;       /* PSS Mail box */
   NCSCONTEXT  pss_task_hdl;  /* PSS Task handle */
}PSSMBX_INFO;

typedef struct pss_handles
{
   uns32                   pss_cb_hdl;
#if (NCS_PSS_RED == 1)
   NCS_MBCSV_HDL           mbcsv_hdl;
   SaSelectionObjectT      mbcsv_sel_obj;
#endif
   uns32                   pssts_hdl;
}PSS_HANDLES; 

typedef struct pss_csi_node
{
   /* workload details of this CSI */ 
   NCS_APP_AMF_CSI_ATTRIBS   csi_info; 

   /* pointer to the list of tables maintained in a particular CSI */ 
   PSS_PWE_CB   *pwe_cb; 

   /* to go to the next CSI */ 
   struct pss_csi_node *next; 
}PSS_CSI_NODE; 

typedef struct pss_attribs
{
   /* amf details */ 
   NCS_APP_AMF_ATTRIBS amf_attribs; 

   NCS_APP_AMF_HA_STATE    ha_state;  /* Use as global constant. This stores
                the current ha-role and also reflects the initial seeded role */
   
   /* Other PSS varaibles */ 
   PSS_CB          *pss_cb; 

   /* misc global info */ 
   PSSMBX_INFO     mbx_details; 
 
   /* miscellaneous informations */ 
   PSS_HANDLES     handles; 

   /* list of CSIs */ 
   PSS_CSI_NODE     *csi_list; 

}PSS_ATTRIBS; 

#define m_PSS_HB_INVOCATION_TYPE    SA_AMF_HEALTHCHECK_AMF_INVOKED

#define m_PSS_HELATH_CHECK_KEY "A9FD64E12F" /* KEY to be published to customers */

#define m_PSS_RECOVERY              SA_AMF_COMPONENT_FAILOVER

/* #define m_PSS_DISPATCH              SA_DISPATCH_ALL */ /* Fix */
#define m_PSS_DISPATCH              SA_DISPATCH_ONE 

/* default CSI Env ID */ 
#define m_PSS_DEFAULT_ENV_ID         1

typedef uns32 (*PSS_MDS_VDEST_ROLE_CHG_API)(NCSCONTEXT handle);

/* global varible to store the attributes of PSSv */ 
EXTERN_C PSS_ATTRIBS gl_pss_amf_attribs; 

EXTERN_C void
pss_mbx_amf_process(SYSF_MBX   *pss_mbx); 

EXTERN_C uns32 
pss_amf_componentize(struct ncs_app_amf_attribs *amf_attribs, SaAisErrorT *o_saf_status); 

/* add the env details to the front of the global CSI list */ 
EXTERN_C void 
pss_amf_csi_list_add_front(PSS_CSI_NODE*); 

EXTERN_C PSS_CSI_NODE* 
pss_amf_csi_install(PW_ENV_ID,  SaAmfHAStateT); 

/* to remove all the CSIs in PSS */
EXTERN_C SaAisErrorT
pss_amf_csi_remove_all(void);

EXTERN_C uns32 pss_confirm_ha_role(PW_ENV_ID pwe_id, SaAmfHAStateT  ha_role);
    
/* Cleanup function of PSS */
EXTERN_C uns32
pss_amf_prepare_will(void); 

/* USR1 signal handler of PSS */
EXTERN_C void 
pss_amf_sigusr1_handler(int i_sig_num); 

/* Decodes the CSI attribute list */
EXTERN_C PW_ENV_ID
pss_amf_csi_attrib_decode(SaAmfCSIAttributeListT); 

EXTERN_C uns32 pss_amf_vdest_role_chg_api_callback(MDS_CLIENT_HDL handle);
/* Function which initialises interface with AMF */
EXTERN_C uns32
pss_amf_initialize(NCS_APP_AMF_ATTRIBS* amf_attribs, SaAisErrorT *o_saf_status);


#endif


