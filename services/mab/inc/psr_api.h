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

 _Public_ Persistent Store & Restore service(PSSv) abstractions and 
 function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */


#ifndef PSR_API_H
#define PSR_API_H

/******************************************************************************

 Global API 

    Layer Management API as single entry point

    Create         - Create an instance of PSS (one global instance)
    Destroy        - Destroy the global instance of PSS
    Set            - Set a configuration object value
    Get            - Get a configuration object value


    Initiate a MIB Access Request via MIB Access Controller

    Request        - MIB Access Request of some sort

*******************************************************************************/

/***************************************************************************
 * Create an instance of a PSS service (one global)
 ***************************************************************************/

typedef enum
{
    PSS_SAVE_TYPE_IMMEDIATE = 1,
    PSS_SAVE_TYPE_ON_DEMAND,
    PSS_SAVE_TYPE_PERIODIC,
    PSS_SAVE_TYPE_MAX
} PSS_SAVE_TYPE;


typedef struct ncspss_create
  {
  MDS_HDL          i_mds_hdl;       /* MDS hdl for 'installing'                    */
  uns32            i_oac_key;       /* OAC handle for registering PSS's MIB Tables */
  SYSF_MBX*        i_mbx;           /* Mail box to receive MAB messages            */
  MAB_LM_CB        i_lm_cbfnc;      /* layer mgmt callback function ptr            */
  NCS_VRID         i_vrid;          /* Virtual Router - same as in MAC, MAS, OAC   */
  NCS_PSSTS        i_pssts_api;     /* Pointer to PSS Target Service function      */
  uns8             i_hmpool_id;     /* Handle Manager Pool Id                      */
  uns32            i_pssts_hdl;     /* Handle to the PSS Target Service            */
  PSS_SAVE_TYPE    i_save_type;     /* Save type                                   */
  NCSMIB_REQ_FNC   i_mib_req_func;  /* MIB request function pointer                */
  uns32            o_pss_hdl;

  } NCSPSS_CREATE;

/***************************************************************************
 * Destroy an instance of a PSS service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncspss_destroy
  {
  void*           i_meaningless;     /* place holder struct; do nothing */

  } NCSPSS_DESTROY;

/***************************************************************************
 * The operations set that a PSS instance supports
 ***************************************************************************/

typedef enum ncspss_lm_op
  {
  NCSPSS_LM_OP_CREATE,
  NCSPSS_LM_OP_DESTROY,
  } NCSPSS_OP;

/***************************************************************************
 * The PSS API single entry point for all services
 ***************************************************************************/

typedef struct ncspss_arg
  {
  NCSPSS_OP           i_op;            /* Operation; CREATE,DESTROY */
  uns32               i_usr_key;       /* Find PSS instance via VALIDATE    */

  union 
    {
    NCSPSS_CREATE     create;
    NCSPSS_DESTROY    destroy;
    } info;

  } NCSPSS_LM_ARG;

/***************************************************************************
 * Global Instance of Layer Management
 ***************************************************************************/

EXTERN_C MABPSR_API uns32 ncspss_lm    ( NCSPSS_LM_ARG* arg );
EXTERN_C uns32 pss_bind_with_dtsv(void);
EXTERN_C void pss_unbind_with_dtsv(void);

#endif /* PSR_API_H */



