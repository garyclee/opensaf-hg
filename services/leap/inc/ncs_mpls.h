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

  MODULE NAME: MPLSDEFS.H



..............................................................................

  DESCRIPTION:

Some common definitions for MPLS go here now.

******************************************************************************
*/


#ifndef _NCS_MPLS_H_
#define _NCS_MPLS_H_

/** LSP Identifier
 **/

typedef struct APS_ldp_lspid_tag
{
   uns16       local_lsp_id;        /* Local CRLSP ID */
   NCS_IPV4_ADDR   ingress_router_id;/* Ingress LSR Router ID */
} APS_LDP_LSPID;


typedef enum
{
   APS_GMPLS_LABEL_CLASS_CLASSIC,
   APS_GMPLS_LABEL_CLASS_GENERALIZED,
   APS_GMPLS_LABEL_CLASS_WAVEBAND
} APS_GMPLS_LABEL_CLASS;


/* This structure is for holding G-MPLS labels, where the label is one or
 * more 32-bit quantities.  The number of components comprising the label
 * is not known ahead of time.  Thus, the following structure must be
 * allocated when the number of comonents is known.
 * See m_MMGR_ALLOC_GMPLS_LABEL for allocation of structure.
 */
typedef struct aps_gmpls_label
{
    APS_GMPLS_LABEL_CLASS label_type;
    uns16   num_components;    /* number of entries in the glabel array */
    uns32   glabel[1];
} APS_GMPLS_LABEL;

/**
 ** Defines for setting the 'type' field in the ER Hop.
 **/
#define APS_ER_HOP_TYPE_IPV4      0x0801
#define APS_ER_HOP_TYPE_IPV6      0x0802
#define APS_ER_HOP_TYPE_AS        0x0803
#define APS_ER_HOP_TYPE_LSPID     0x0804
#define APS_ER_HOP_TYPE_PATH_TERM 0x0064
#define APS_ER_HOP_TYPE_LABEL     0x0063  
#define APS_ER_HOP_UNNUMBERED_INTF_ID 0x062

typedef struct aps_er_hop
{
   uns16       type;    /* One of APS_ER_HOP_TYPE_xxx above */
   uns8        loose;   /* Loose or Strict hop */
   union
   {
      struct
      {
         NCS_IPV4_ADDR   ipv4_addr;  /* IPV4 prefix       */
         uns8     prefix_len;       /* Length of prefix  */
         uns8     flags;            /* RSVP Only         */
      }IPv4;
      struct
      {
         uns8     ipv6_addr[16]; /* IPV6 prefix (Not supported) */
         uns8     prefix_len;    /* Length of prefix*/
      }IPv6;
      struct
      {
         uns16    as_num;        /* Autonomous System number */
      } AutonomousSys;
      struct
      {
         APS_LDP_LSPID   lspid;   /* LSPID CR-LDP Only */
      } LspId;
      struct
      {
         uns8 u_bit;   /* 0 = downstream, 1 = upstream */
         APS_GMPLS_LABEL *label;
      }Label;
      struct
      {
         NCS_IPV4_ADDR router_id;
         uns32 interface_id;
      }UnnumberedIntfId;
   } value;
}APS_ER_HOP;

/*******************************************************************************
 * APS_ER_HOP_LIST:
 * This is a malloc-able form of the APS_ER_HOP structure, with the
 * capability to link them together.
 *******************************************************************************/
typedef struct aps_er_hop_list
{
   struct aps_er_hop_list *next_hop;  /* APS_ER_HOP_LIST_NULL means end of list */
   APS_ER_HOP              hop;

} APS_ER_HOP_LIST;

#define APS_ER_HOP_LIST_NULL ((APS_ER_HOP_LIST*)0)


/*******************************************************************************
 * APS_ERO_LIST:
 * Linked list of APS_ER_HOP_LIST pointer.
 *******************************************************************************/
typedef struct aps_ero_list
{
   struct aps_ero_list  * next_ero;  /* NCS_ERO_LIST_NULL means end of list */
   APS_ER_HOP_LIST      * er_hop_list;

} APS_ERO_LIST;

#define APS_ERO_LIST_NULL ((APS_ERO_LIST*)0)

/********************************************************************************
 * NCS_ER_STATUS_CODE:  an ENUM of error codes for dealing with 
 *                     processing explicit routes.
 ********************************************************************************/
typedef enum aps_er_status_code
{
   APS_ER_STATUS_CODE_NO_ERROR, /* Success. RDB looks up next-hop */

   APS_ER_STATUS_CODE_NO_HOP,   /* 1st hop is missing.  Illegal ER Object/TLV */
   APS_ER_STATUS_CODE_BAD_1ST_HOP,  /* 1st hop is bad.  Not us, or otherwise unprocessable. */
   APS_ER_STATUS_CODE_BAD_STRICT_NODE,  /* can't find path supporting strict hop */
   APS_ER_STATUS_CODE_GEN_ERR,  /* General error (memory alloc failure, etc.) */

   APS_ER_STATUS_CODE_BAD_HOP,  /* Hop encountered in invalid spot (e.g.,
                                * LABEL behind a loose hop) */

   APS_ER_STATUS_CODE_MAX       /* Must be last. */
} APS_ER_STATUS_CODE;

#endif /* #ifndef _NCS_MPLS_H_ */


