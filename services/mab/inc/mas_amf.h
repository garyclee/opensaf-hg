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
  
  DESCRIPTION: This file describes AMF Interface definitions for MAS
*****************************************************************************/ 
#ifndef MAS_AMF_H
#define MAS_AMF_H

typedef struct mas_csi_node
{
      /* workload details of this CSI */ 
      NCS_APP_AMF_CSI_ATTRIBS   csi_info; 

      /* pointer to the list of tables maintained in a particular CSI */ 
      MAS_TBL   *inst; 

      /* to go to the next CSI */ 
      struct mas_csi_node *next; 
}MAS_CSI_NODE; 

typedef struct mas_attribs
{ 
    /* amf attributes */ 
    NCS_APP_AMF_ATTRIBS  amf_attribs; 

#if (NCS_MAS_RED == 1)
    /* MBCSv Attributes */
    MAS_MBCSV_ATTRIBS   mbcsv_attribs; 
#endif

    /* misc global info */ 
    MAS_MBX         mbx_details; 

   /* MAS MDS Handle */ 
   MDS_HDL          mas_mds_hdl; 

   /* default PWE Handle */ 
   MDS_HDL       mas_mds_def_pwe_hdl; 
   
   /* list of CSIs */ 
   MAS_CSI_NODE     *csi_list; 

}MAS_ATTRIBS; 

#define m_MAS_HB_INVOCATION_TYPE    SA_AMF_HEALTHCHECK_AMF_INVOKED

#define m_MAS_HELATH_CHECK_KEY "A9FD64E12D" /* KEY to be published to customers */

#define m_MAS_RECOVERY              SA_AMF_COMPONENT_FAILOVER

/* #define m_MAS_DISPATCH              SA_DISPATCH_ALL  */ /* Fix */
#define m_MAS_DISPATCH              SA_DISPATCH_ONE  

/* default CSI Env ID */ 
#define m_MAS_DEFAULT_ENV_ID         1

/* Maximum CSI Env ID */ 
#define m_MAS_MAX_ENV_ID            (NCSMDS_MAX_PWES)

/* global varible to store the attributes of MASv */ 
MAS_ATTRIBS gl_mas_amf_attribs; 

/* Start Function of the MAS thread */
EXTERN_C void
mas_mbx_amf_process(SYSF_MBX   *mas_mbx); 

/* add the env details to the front of the global CSI list */ 
EXTERN_C void 
mas_amf_csi_list_add_front(MAS_CSI_NODE*); 

/* Function which creates CSI either in ACTIVE or STANDBY state */
EXTERN_C MAS_CSI_NODE* 
mas_amf_csi_install(PW_ENV_ID,  SaAmfHAStateT); 

/* Cleanup function of MAS */
EXTERN_C uns32
mas_amf_prepare_will(void); 

/* USR1 signal handler function */
EXTERN_C void 
mas_amf_sigusr1_handler(int i_sig_num); 

/* Function which initialises interface with AMF */
EXTERN_C uns32
mas_amf_initialize(NCS_APP_AMF_ATTRIBS* amf_attribs);

/* frees the associated filters */
EXTERN_C  uns32
mas_table_rec_list_free(MAS_ROW_REC *tbl_list); 

/* sends the AMF response for QUIESCED state transition */ 
EXTERN_C  uns32
mas_amf_mds_quiesced_process(NCSCONTEXT mas_hdl); 

#endif
