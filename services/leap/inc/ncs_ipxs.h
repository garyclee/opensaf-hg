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

..............................................................................

******************************************************************************
*/


#ifndef NCS_IPXS_H
#define NCS_IPXS_H

#include "ncsgl_defs.h"
#include "ncs_osprm.h"

#if 0
#include "ncs_xlim.h"
#endif
#include "ncs_mpls.h"

/**************************************************************************

                          N e t P l a n e 
                          
                    I P   X L I M   S h i m  (IPXS)

 The IPXS service lays on top of XLIM and adds IP semantics 'on top of'
 the otherwise stupid XLIM interface attribute values. Recall, that XLIM
 is technology space neutral and does not want to get involved with IP, ATM
 Frame, or any other communications semantics. As such, we might project
 that some day there might be an ATMXS (ATM XLIM Shim) object for example.

 There is one IPXS service instance Per Virtual Router.

    Create         - Create a local IPXS DB per virtual router
    Destroy        - Destroy a local IPXS DB and recover resources
    IsLocal        - Given an ip-addr and mask, is this ip-addr local?
    IsAbstractNode - Given an explicit route hop, is this an abstract node?
    AddIPaddr      - Add this IP Address to the spec'ed IFINDEX iface.
   
****************************************************************************/

/***************************************************************************
 * Create an instance of IPXS; one per PWE
 ***************************************************************************/

typedef struct ipxs_create
  {
  NCS_VRID        i_vrtr_id;
  NCS_KEY*        i_xlim_usr_key;
  NCSCONTEXT      o_ncs_ipxs_hdl;
  } IPXS_CREATE;

/***************************************************************************
 * Destroy an instance of IPXS; Recover all IPXS resources
 ***************************************************************************/

typedef struct ipxs_destroy
  {
  uns32          i_meaningless;  /* top level i_ncs_ipxs_key says which one */
  } IPXS_DESTROY;

/***************************************************************************
 * Given an IP Address and subnet mask, is this address on this machine??
 ***************************************************************************/

typedef struct ipxs_is_local
  {
  NCS_IP_ADDR     i_ip_addr;      /* a candidate ip addr that may be local */
  uns8           i_maskbits;     /* subnet bitmask for the passed ip addr */
  NCS_BOOL        i_obs;          /* if this flag is set, then ti should be 
                                    check against full address */
  NCS_IFINDEX     o_iface;        /* interface index; use XLIM for details */
  NCS_BOOL        o_answer;       /* TRUE or FALSE                         */

  } IPXS_IS_LOCAL;

/***************************************************************************
 * Given an Explicit Route Hop, is this an abstract node??
 ***************************************************************************/

typedef struct ipxs_is_abstr_node
  {
  APS_ER_HOP*     i_ero;          /* an explicit route object              */
  NCS_BOOL        o_answer;       /* TRUE or FALSE                         */

  } IPXS_IS_ABSTR_NODE;

/***************************************************************************
 * Associate the passed IP Address with the passed IFINDEX. Validate first.
 ***************************************************************************/

typedef struct ipxs_add_ip_addr
  {
  NCS_IFINDEX     i_iface;        /* associated interface index            */
  uns8           i_maskbits;     /* maskbits                              */
  NCS_IP_ADDR     i_ip_addr;      /* IP Address                            */

  } IPXS_ADD_IP_ADDR;

#if (NCS_IPV6 == 1)
/* Error Value returned by IPXS layer, for IPV6-tunnel commands. */
typedef enum
  {
  NCS_IPXS_TUNNEL_INVLD_TUNN_ATTR_SPCFD = 1,
  NCS_IPXS_TUNNEL_INVLD_TUNN_SRC_SPCFD,
  NCS_IPXS_TUNNEL_INVLD_TUNN_MODE_OP,
  NCS_IPXS_TUNNEL_INVLD_TUNN_DEST_SPCFD,
  NCS_IPXS_TUNNEL_MODE_CHNG_NOT_ALLOWED,
  NCS_IPXS_TUNNEL_INVALID_ADDRESS_SPECIFIED,
  NCS_IPXS_TUNNEL_INVLD_TUNN_OP,
  NCS_IPXS_TUNNEL_UPDT_IFACE_OP_FAILED,
  NCS_IPXS_TUNNEL_VIEW_IFACE_OP_FAILED,
  NCS_IPXS_TUNNEL_RECORD_NOT_FOUND,
  NCS_IPXS_TUNNEL_ACT_NOT_POSSIBLE
  } NCS_IPXS_TUNNEL_ERROR;

#define m_IPXS_MAX_TUNNEL_ERROR_STR_LEN (64)

typedef struct ipxs_tunnel_error_str_map_tag
{
   NCS_IPXS_TUNNEL_ERROR errorCode;
   char          errorStr[m_IPXS_MAX_TUNNEL_ERROR_STR_LEN+1];
} IPXS_TUNNEL_ERROR_STR_MAP;

EXTERN_C void ipxs_print_tunnel_err_code (NCS_IPXS_TUNNEL_ERROR errCode);
#define m_IPXS_PRINTF_TUNNEL_ERROR_STRING(errCode) ipxs_print_tunnel_err_code(errCode)

/**************************************************************************
 * Validate a tunnel(IPV6-over-IPV4-only) and Activate, and return 
 * NCSCC_RC_SUCCESS for a permissible operation.
 **************************************************************************/
typedef struct ipxs_tunnel_modify_evt_tag
  {
  XLIM_UPDT_TUNN    updt_tunn_rec;
  NCS_IPXS_TUNNEL_ERROR o_error;
  } IPXS_TUNNEL_MODIFY_EVT;

/**************************************************************************
 * Update a tunnel(IPV6-over-IPV4-only) status, and return 
 * NCSCC_RC_SUCCESS if configuration information is sufficient enough for
 * IPV6 forwarding to be enabled.
 **************************************************************************/
typedef struct ipxs_update_tunnel_status_tag
  {
  NCSXLIM_IF_REC    *i_src_if_rec;
  NCSXLIM_TUNN_REC  *i_tunn_rec;       /* Ifrecord of the tunnel. */
  NCS_BOOL        o_is_forwdng_active;
  } IPXS_UPDATE_TUNNEL_STATUS;
#endif

/***************************************************************************
 * H J I P X S _ S V C _ A R G
 *
 * One of the preceeding data structures has been populated corresponding
 * to the OP value passed in by the invoking client.
 *
 ***************************************************************************/

typedef struct ncsipxs_svc_arg
  {
  NCS_KEY*  i_usr_key;   /* key for this instance of IPXS; always needed  */

  union {

/***************************************************************************
 * The AKE is the likely client using these services to start and seed LIL
 ***************************************************************************/

    IPXS_CREATE         create;
    IPXS_DESTROY        destroy;

/***************************************************************************
 * Subsystems are the likely clients to use these........
 ***************************************************************************/

    IPXS_IS_LOCAL       is_local;
    IPXS_IS_ABSTR_NODE  is_an;
    IPXS_ADD_IP_ADDR    ip_addr;
#if (NCS_IPV6 == 1)
    IPXS_TUNNEL_MODIFY_EVT       validate_tun_modify_op;
    IPXS_UPDATE_TUNNEL_STATUS    update_tun_status;
#endif

    } op;

  } NCSIPXS_SVC_ARG;

/* Operations mnemonics for a IPXS Services */

typedef enum
  {
  NCS_IPXS_CREATE,
  NCS_IPXS_DESTROY,

  NCS_IPXS_IS_LOCAL,
  NCS_IPXS_IS_ABSTR_NODE,
#if (NCS_IPV6 == 1)
  NCS_IPXS_VAL_N_APPLY_TUNNEL_MODIFY_OP,
  NCS_IPXS_UPDATE_TUNNEL_STATUS,
#endif
  NCS_IPXS_ADD_IP_ADDR

  } NCSIPXS_SVC_OP;

/***************************************************************/
/* Prototype for IPXS Service Entry point interface...         */
/*                                                             */
/*                                                             */
/***************************************************************/

/* The master prototype for IPXS Service entry point */

typedef uns32 (*IPXS_CTRL)( NCSIPXS_SVC_ARG* arg, NCSIPXS_SVC_OP   op);


EXTERN_C LEAPDLL_API uns32 ncs_ipxs_ctrl( NCSIPXS_SVC_ARG* arg, NCSIPXS_SVC_OP   op);

#define m_NCS_IPXS_CTRL ncs_ipxs_ctrl



#endif
