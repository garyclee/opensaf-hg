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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.1/base/products/mab/inc/mas_api.h 6     10/16/01 5:54p Questk $



..............................................................................

  DESCRIPTION:

 _Public_ MIB Access Server (MAS) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef MAS_API_H
#define MAS_API_H

/******************************************************************************

 Per Virtual Router API 

    Layer Management API as single entry point

    Create         - Create an instance of MAS (per virtual router)
    Destroy        - Destrou an instance of MAS
    
    Initiate a MIB Access Request via MIB Access Controller

    Request        - MIB Access Request of some sort
   
*******************************************************************************/

/***************************************************************************
 * Create an instance of a MAS service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsmas_create
  {
  MDS_HDL         i_mds_hdl;       /* MDS hdl for 'installing'          */
  NCS_VRID        i_vrid;          /* Virtual Rtr; same in MAS, OAC     */
  SYSF_MBX*       i_mbx;           /* Mail box to receive MAB messages  */
  MAB_LM_CB       i_lm_cbfnc;      /* layer mgmt callback function ptr  */
  uns8            i_hm_poolid;     /* Pool id for Handle Manager        */
  uns32           o_mas_hdl;       /* Handle registered with HM         */

  } NCSMAS_CREATE;

/***************************************************************************
 * Destroy an instance of a MAS service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsmas_destroy
  {
  uns32           i_mas_hdl;       /* Handle to MAS instance            */

  } NCSMAS_DESTROY;

/***************************************************************************
 * The operations set that a MAS instance supports
 ***************************************************************************/

typedef enum ncsmas_lm_op
  {
  NCSMAS_LM_OP_CREATE = 1,
  NCSMAS_LM_OP_DESTROY
  } NCSMAS_OP;

/***************************************************************************
 * The MAS API single entry point for all services
 ***************************************************************************/

typedef struct ncsmas_arg
  {
  NCSMAS_OP           i_op;            /* Operation; CREATE,DESTROY,GET,SET */

  union 
    {
    NCSMAS_CREATE     create;
    NCSMAS_DESTROY    destroy;
    } info;

  } NCSMAS_LM_ARG;

/***************************************************************************
 * Per Virtual Router Instance of Layer Management
 ***************************************************************************/
EXTERN_C uns32 mas_evt_cb(MAB_LM_EVT* evt);

EXTERN_C MABMAS_API uns32 ncsmas_lm    ( NCSMAS_LM_ARG* arg );

EXTERN_C MABMAS_API uns32 maslib_request     ( NCS_LIB_REQ_INFO* req );

#endif /* MAS_API_H */
