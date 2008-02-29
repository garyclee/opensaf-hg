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

  $Header: /ncs/software/release/UltraLinq/MAB/MAB1.0/base/products/mab/inc/mac_api.h 8     3/27/01 5:41p Stevem $



..............................................................................

  DESCRIPTION:

  _Public_ MIB Access Client (MAC) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef MAC_API_H
#define MAC_API_H

/**************************************************************************

 Per Virtual Router API  

    Layer Management API as single entry point

    Create         - Create an instance of MAC (per virtual router)
    Destroy        - Destrou an instance of MAC
    Set            - Set a configuration object value
    Get            - Get a configuration object value

    
    Initiate a MIB Access Request via MIB Access Controller

    Request        - MIB Access Request of some sort
   
****************************************************************************/

/***************************************************************************
 * Create an instance of a MAC service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsmac_create
  {
  NCS_VRID        i_vrid;          /* Virtual Rtr; same in MAS, OAC     */
  SYSF_MBX*       i_mbx;           /* Mail box to receive MAB messages  */
  MAB_LM_CB       i_lm_cbfnc;      /* layer mgmt callback function ptr  */
  uns8            i_hm_poolid;     /* Handle Manager pool id            */
  uns32           o_mac_hdl;       /* m_NCSMAC_VALIDATE_HDL() answer     */
  SaNameT         i_inst_name;     /* Name of this instance of this MAA */ 

  } NCSMAC_CREATE;

/***************************************************************************
 * Destroy an instance of a MAC service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsmac_destroy
  {
  uns32           i_mac_hdl;       /* m_NCSMAC_VALIDATE_HDL() answer     */
  PW_ENV_ID       i_env_id; 
  SaNameT         i_inst_name; 
  } NCSMAC_DESTROY;


/***************************************************************************
 * Set or Get configuration data for this virtual router instance
 ***************************************************************************/

/* MAC Configuration Object Identifiers whose values to get or set..       */
/* ======================================================================= */
/* RW : Read or Write; can do get or set operations on this object         */
/* R- : Read only; can do get operations on this object                    */
/* -W : Write only; can do set operations on this object                   */

typedef enum {                 /*     V a l u e   E x p r e s s i o n      */
                               /*------------------------------------------*/
  NCSMAC_OBJID_LOG_VAL,         /* RW bitmap of log categories to install   */
  NCSMAC_OBJID_LOG_ON_OFF,      /* RW ENABLE|DISABLE logging                */
  NCSMAC_OBJID_SUBSYS_ON,       /* RW ENABLE|DISABLE MAC services           */

  } NCSMAC_OBJID;

/* MAC Configuration 'explanation' structure                              */


typedef struct ncsmac_set
  {
  uns32           i_mac_hdl;
  uns32           i_obj_id;
  uns32           i_obj_val;

  } NCSMAC_SET;


typedef struct ncsmac_get
  {
  uns32           i_mac_hdl;
  uns32           i_obj_id;
  uns32           o_obj_val;

  } NCSMAC_GET;


/***************************************************************************
 * The operations set that a MAC instance supports
 ***************************************************************************/

typedef enum ncsmac_lm_op
  {
  NCSMAC_LM_OP_CREATE,
  NCSMAC_LM_OP_DESTROY,
  NCSMAC_LM_OP_GET,
  NCSMAC_LM_OP_SET,

  } NCSMAC_OP;

/***************************************************************************
 * The MAC API single entry point for all services
 ***************************************************************************/

typedef struct ncsmac_lm_arg
  {
  NCSMAC_OP           i_op;            /* Operation; CREATE,DESTROY,GET,SET */

  union 
    {
    NCSMAC_CREATE     create;
    NCSMAC_DESTROY    destroy;
    NCSMAC_GET        get;
    NCSMAC_SET        set;

    } info;

  } NCSMAC_LM_ARG;


/***************************************************************************
 * Per Virtual Router Instance of Layer Management
 ***************************************************************************/

EXTERN_C MABMAC_API uns32 ncsmac_lm          ( NCSMAC_LM_ARG* arg );
EXTERN_C MABMAC_API uns32 maclib_request     ( NCS_LIB_REQ_INFO* req );
/* to destroy MAA */
/* Removing the static specifier to make it extern function */
EXTERN_C uns32
maclib_mac_destroy(NCS_LIB_REQ_INFO * req_info);

#endif /* MAC_API_H */
