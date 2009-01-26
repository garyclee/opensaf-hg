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
FILE NAME: IFA_DB.H
DESCRIPTION: Enums, Structures and Function prototypes used by IFA.
******************************************************************************/

#ifndef IFA_DB_H
#define IFA_DB_H

/* The node is used to store the subscription information */

/* Macro to get the IFA CB handle */


typedef struct ifsv_subscr_info
{
   NCS_DB_LINK_LIST_NODE lnode;   /* Linked list node */
   NCS_IFSV_SUBSCR_SCOPE subscr_scope;/* To recognise the scope of the interface
                                       record: internal or external. This will 
                                       be useful for If Agent in deciding 
                                       whether the record coming from IfNd is 
                                       for Internal Interfaces or External 
                                       Interfaces. */

   NCS_IFSV_SUBSCR         info; /* Subscription information received from application */
} IFSV_SUBSCR_INFO;


/* Global Data base used by the IFA */
typedef struct ifa_cb
{
   /* MDS Parameters of Self */
   MDS_HDL           my_mds_hdl;    /* MDS Self PWE handle */
   MDS_DEST          my_dest;       /* MDS Self Destination */
   V_DEST_QA         my_anc;        /* MDS Self Anchor */

   /* MDS parameters of IFND */
   NCS_BOOL          is_ifnd_up;    /* IFND up flag */
   MDS_DEST          ifnd_dest;

   /* EDU handle, for performing encode/decode operations. */
   EDU_HDL              edu_hdl;

   uns8              hm_pid;     /* Handle Manager Pool ID */
   uns32             cb_hdl;     /* Handle manager Struct Handle */
   NCS_DB_LINK_LIST  subr_list;  /* Linked list of subscription info */

#if (NCS_IFSV_IPXS == 1)
   NCS_DB_LINK_LIST  ipxs_list;  /* IPXS Subscription list */
#endif
#if (NCS_VIP == 1)
   NCS_PATRICIA_TREE ifaDB;      /* IfA database */
#endif

}IFA_CB;

extern uns32 gl_ifa_cb_hdl;

uns32 ifsv_ifa_subscribe (IFA_CB *ifa_cb, NCS_IFSV_SUBSCR *i_subr);
uns32 ifsv_ifa_unsubscribe (IFA_CB *ifa_cb, NCS_IFSV_UNSUBSCR *i_unsubr);
uns32 ifsv_ifa_ifrec_get (IFA_CB *ifa_cb, NCS_IFSV_IFREC_GET *i_ifget, 
                            NCS_IFSV_IFGET_RSP *o_ifrec);
uns32 ifsv_ifa_ifrec_add (IFA_CB *ifa_cb,  NCS_IFSV_INTF_REC *i_ifrec);
uns32 ifsv_ifa_ifrec_del (IFA_CB *ifa_cb,  NCS_IFSV_SPT *i_ifdel);
uns32 ifsv_ifa_svcd_upd (IFA_CB *ifa_cb,  NCS_IFSV_SVC_DEST_UPD *i_svcd);
uns32 ifsv_ifa_svcd_get (IFA_CB *ifa_cb,  NCS_IFSV_SVC_DEST_GET *i_svcd);

uns32 ifa_app_send(IFA_CB *cb, NCS_IFSV_SUBSCR *subr, 
                   IFA_EVT *evt, IFSV_EVT_TYPE evt_type, uns32 rec_found);
#if (NCS_VIP == 1)

uns32 ncs_ifsv_vip_install(IFA_CB *ifa_cb , NCS_IFSV_VIP_INSTALL *instArg);
uns32 ncs_ifsv_vip_free(IFA_CB *pifa_cb , NCS_IFSV_VIP_FREE *pFreeArg);

uns32 ncs_vip_ip_lib_init (IFA_CB *cb);
uns32  ncs_ifsv_vip_verify_ipaddr(char *ipaddr);
uns32 m_ncs_validate_interface_name(char *str);


#endif

/* These below DLL symbol has to be removed after Unit test is completed */
IFADLL_API uns32 ifa_lib_init (NCS_LIB_CREATE *create);
IFADLL_API uns32 ifa_lib_destroy (NCS_LIB_DESTROY *destroy);

#if(NCS_IFSV_IPXS == 1)
/* IFA-IPXS function declerations */
uns32 ifa_ipxs_evt_process(IFA_CB *cb, IFSV_EVT *evt);
void ipxs_agent_lib_init(IFA_CB *cb);

#endif

#endif
