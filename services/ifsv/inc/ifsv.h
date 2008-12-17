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



/* IFSV.H: Master include file for all Interface Service and user *.C files */

#ifndef IFSV_H
#define IFSV_H

#include <stdarg.h>
/* Get the compile time options */
#include "ncs_opt.h"

/* Get General Definations */
#include "gl_defs.h"

/* Get target's suite of header files...*/
#include "t_suite.h"

/* From /base/common/inc */
#include "ncs_svd.h"
#include "usrbuf.h"
#include "ncsft.h"
#include "ncsft_rms.h"
#include "ncs_ubaid.h"
#include "ncsencdec.h"
#include "ncs_stack.h"
#include "ncs_mib.h"
#include "ncsmiblib.h"
#include "ncs_mtbl.h"
#include "ncs_log.h"
#include "ncs_lib.h"
#include "ncsdlib.h"
#include "ncs_util.h"

/* From targsvcs/common/inc */

#include "mds_papi.h"  
#include "ncs_edu_pub.h"

#if 0
#if (NCS_MDS == 1)
#include "mds_inc.h"
#endif
#endif 

/* From /base/products/rms/inc */

#if (NCS_RMS == 1)
#include "rms_env.h"
#endif

/* From /base/products/mab/inc */
#if (NCS_MAB == 1)
#include "mab.h"
#endif
/* AMF header files */
#include "saAis.h"
#include "saAmf.h"

/* mtree header files */
#include "ncs_mtree.h"
#include "ncs_xmtre.h"

/* DTS header file */
#include "dta_papi.h"

/* IFSV common header files */
#include "ifsv_papi.h"
#include "ifa_papi.h"
#include "mbcsv_papi.h"

/* IFSV Common Header Files */
#include "ifsvinit.h"
#include "ifsv_red_common.h"
#include "ifsv_cb.h"
#include "ifsv_tmr.h"
#include "ifsv_db.h"
#if (NCS_VIP == 1)
#include "vip.h"  
#endif
#include "ifsv_evt.h"
#include "ifsv_log.h"
#include "ifsv_mem.h"
#include "ifsv_saf.h"
#include "ifsv_mapi.h"

#if(NCS_IFSV_IPXS == 1)
#include "ipxs_papi.h"
#include "ipxs_evt.h"
#include "ifsvipxs.h"
#include "ipxs_db.h"
#include "ipxs_mem.h"
#include "ifnd_ipxs.h"

#endif

/* Files related to redundancy.*/
#include "ifnd_red.h"
#include "ifd_red.h"

#if(NCS_VIP == 1)
#include "vip_db.h" 
#include "ifsv_vip_mem.h"
#endif

#include "ifsv_mib.h"
#include "ifsv_mds.h"


#if (NCS_DTS == 1)
#include "dts_papi.h"
#endif

/* Definations for Tunnel layer attribute Maps */
#define NCS_IFSV_TAM_SIF            (0x00000001) /* Tunnel Source IF index */
#define NCS_IFSV_TAM_SIP            (0x00000002) /* Tunnel Source IP address */
#define NCS_IFSV_TAM_CIP            (0x00000004) /* Configured IP address */
#define NCS_IFSV_TAM_V6ENBL         (0x00000008) /* IPv6 Enable flag TBD:RSR */

/************************** Common function declarations *******************/
extern void ifsv_tmr_stop (IFSV_TMR *tmr,IFSV_CB *cb);
extern uns32 ifsv_tmr_start (IFSV_TMR *tmr, IFSV_CB *cb);

#if(NCS_IFSV_IPXS == 1)
extern uns32 ipxs_ifip_record_del(IPXS_CB *cb, NCS_PATRICIA_TREE *ifip_tbl, IPXS_IFIP_NODE *ifip_node);
extern void ipxs_ip_record_list_del(IPXS_CB *cb, IPXS_IFIP_INFO *ifip_info);
extern uns32 ipxs_iprec_get(IPXS_CB *cb, IPXS_IP_KEY *ipkeyNet,
                NCS_IP_ADDR_TYPE type, IPXS_IP_NODE  **ip_node, NCS_BOOL test);
extern void ipxs_intf_rec_list_free(NCS_IPXS_INTF_REC *i_intf_rec);
extern uns32 ipxs_ifsv_ifip_info_attr_cpy(IPXS_IFIP_INFO *src, NCS_IPXS_IPINFO *dest);


#endif

#if(NCS_VIP == 1)
extern NCS_IFSV_IFINDEX    ifsv_vip_get_global_ifindex(IFSV_CB *cb,uns8 *str);

extern uns32 
ifnd_ipxs_proc_ifip_upd(IPXS_CB *cb, IPXS_EVT *ipxs_evt, IFSV_SEND_INFO *sinfo);

extern uns32 
ifnd_vip_evt_process(IFSV_CB *cb, IFSV_EVT *evt);

extern uns32 
ifnd_process_ifa_crash(IFSV_CB *cb, MDS_DEST *mdsDest);

extern uns32 
ifnd_vip_evt_process(IFSV_CB *cb, IFSV_EVT *evt);

#endif

#if(NCS_IFSV_BOND_INTF == 1)
extern uns32
ifsv_bonding_check_ifindex_is_master(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX ifindex,
                                                                        NCS_IFSV_IFINDEX *bonding_ifindex);
extern IFSV_INTF_DATA *
ifsv_binding_delete_master_ifindex(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bonding_ifindex);

extern void ifsv_delete_binding_interface(IFSV_CB *ifsv_cb,NCS_IFSV_IFINDEX bonding_ifindex);


#endif




#endif


