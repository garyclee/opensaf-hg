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

  DESCRIPTION:

  _Public_ Object Access Client (OAC) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef OAC_PAPI_H
#define OAC_PAPI_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "ncsgl_defs.h"
#include "ncs_mib_pub.h"
#include "mab_penv.h"

/******************************************************************************

 Per Virtual Router API 

    Subsystem Integration Entry Points for participating in MAB services

    tbl owned   - This subsystem owns this MIB table
    row owned   - This subsystem owns this range of rows in this MIB Table
    row gone    - This subsystem no longer services these rows
    tbl gone    - This subsystem no longer services this MIB table


    request     - Entry point for subsystems to ask a MIB question of the MAB

*******************************************************************************/

/***************************************************************************
 * Some subsystem declares it is the local owner of some MIB table
 ***************************************************************************/

typedef struct ncsoac_tbl_owned
  {

  uns64           i_mib_key;             /* NCSMIB_ARG::i_mib_key field val */
  NCSMIB_REQ_FNC  i_mib_req;             /* NetPlane Common MIB Access API */
  uns32           i_ss_id;               /* Subsystem ID                   */

  NCS_BOOL      is_persistent;   /* If write-able objects of normal-MIBs are 
                                    to be made persistent, this flag needs
                                    to be set to TRUE. */

  /* Name of the client(service-user) owning this table.
   * Each service-user of the Persistency service is required to register
   * a unique client-string(PCN, as it is referred to) at the time, the
   * MIBs are getting registered with the OAA. This PCN is used for
   * all operations related to persistency.
   * This is used by OAA for sending the Table-bind event to PSSv.
   * In case of distributed MIB rows, the client has the option of
   * registering the rows using a different PCN string with OAA.
   * If the distributed rows are logically falling under one client, then
   * the same PCN string can be used for registering the rows with OAA.
   */
  char          *i_pcn;

  } NCSOAC_TBL_OWNED;


/*****************************************************************************
  NCSOAC_EOP_USR_IND_CB : EndOfPlayback indication structure to the user from OAA
*****************************************************************************/
typedef struct ncsoac_eop_usr_ind_cb_tag
{
   uns32                 i_mib_key;   /* Service Provider's ID value      */
   uns32                 i_wbreq_hdl; /* Unique-handle corresponding to the
                                         warmboot request */
   uns32                 i_rsp_status; /*NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE */
}NCSOAC_EOP_USR_IND_CB; 


/*
 * EndOfPlayback Indication-handler that user registers with OAA for
 * each warmboot-request. OAA will invoke this function, once it receives
 * a status update from PSSv regarding the playback session.
 */
typedef void (*NCSOAC_EOP_USR_IND_FNC)(struct ncsoac_eop_usr_ind_cb_tag *eop_cb); 



/***************************************************************************
 * The operations set that a OAC callback API supports
 ***************************************************************************/

typedef enum ncsoac_ss_cb_op
  {
  NCSOAC_SS_CB_OP_IDX_FREE,
  NCSOAC_SS_CB_OP_SA_DATA

  } NCSOAC_SS_CB_OP;

/***************************************************************************
 * The OAC callback API entry point
 ***************************************************************************/

typedef struct ncsoac_ss_cb_arg
  {
  NCSOAC_SS_CB_OP      i_op;            
  uns64                i_ss_hdl;
  union 
    {
     NCSMIB_ARG*       mib_req;
     NCSMAB_IDX_FREE*  idx_free;
    } info;

  } NCSOAC_SS_CB_ARG;

/*****************************************************************************
  OAC_SS_CB
        function prototype for OAC Callback interface to a subsystem. 
        The customer provides a function pointer of this type to the OAC so
        that it can be invoked by MAB when previously allocated indeces are 
        no longer needed or when data for the "Staging Area" arrives 
        at the local OAC.
*****************************************************************************/

typedef uns32 (*NCSOAC_SS_CB)(NCSOAC_SS_CB_ARG* cbarg);

/***************************************************************************
 * The Table owning subsystem now declares it owns some range of rows
 ***************************************************************************/

typedef struct ncsoac_row_owned
  {
  NCSMAB_FLTR        i_fltr;                /* Description of what is owned  */
  NCSOAC_SS_CB       i_ss_cb;               /* Subsystem callback */
  uns64              i_ss_hdl;              /* Subsystem callback handle */
  uns32              o_row_hdl;             /* OAC returned handle of row    */

  } NCSOAC_ROW_OWNED;

/***************************************************************************
 * The same subsystem declares it no longer owns these MIB rows
 ***************************************************************************/

typedef struct ncsoac_row_gone
  {
  uns32          i_row_hdl;

  } NCSOAC_ROW_GONE;

/***************************************************************************
 * The subsystem wants warmboot to happen from PSSv
 ***************************************************************************/

/*****************************************************************************
  NCSOAC_PSS_TBL_LIST  List of MIB table IDs.
*****************************************************************************/
typedef struct ncsoac_pss_tbl_list_tag
{
  uns32           tbl_id;
  struct ncsoac_pss_tbl_list_tag *next;
}NCSOAC_PSS_TBL_LIST;


/*****************************************************************************
  NCSOAC_PSS_WARMBOOT_REQ : Warmboot request structure from user to OAA
*****************************************************************************/
typedef struct ncsoac_pss_warmboot_req
  {
  /* Name of the client(service-user) owning this table.
   * Each service-user of the Persistency service is required to register
   * a unique client-string(PCN, as it is referred to) at the time, the
   * MIBs are getting registered with the OAA. This PCN is used for
   * all operations related to persistency.
   *
   * This is used by OAA for sending the Table-bind event to PSSv.
   * In case of distributed MIB rows, the client is required to
   * provide the PCN string which was used for registering the particular
   * set of rows with OAA.
   */
  char          *i_pcn;

  /*
   * SPCN is a special category of the clients, which is recognized by PSSv.
   * In this category, the client has some configuration data issuable
   * from the SystemBOM-file. Any client, which doesn't have any configuration
   * data coming from the SystemBOM-file, won't fall under this category.
   * PSSv's behaviour during playback session is unique for this category.
   * NCS documentation talks about this behaviour in detail.
   */
  NCS_BOOL      is_system_client;  /* Specifies this is a SPCN client(like
                                      AvSv, etc) */
  NCSOAC_PSS_TBL_LIST *i_tbl_list;   /* Linked list pointer containing the list
                                        of MIB tables to be played-back */

  /* Extension for the end-of-playback notification to the client.
   * If the function pointer is NULL, then the end-of-playback notification
   * won't be delivered to the client. Rest of the persistency functionality
   * is allowed however.
   */
  NCSOAC_EOP_USR_IND_FNC  i_eop_usr_ind_fnc;

  /* User handle to be passed back to the client, at the time of delivering
     the status update */
  uns32           i_mib_key;

  /* Handle which OAA returns to the user, for the particular warmboot request.
     The user gets to store this handle and is required to correlate this with
     the EndOfPlayback indication provided by the OAA later. */
  uns32     o_wbreq_hdl;

  } NCSOAC_PSS_WARMBOOT_REQ;

/***************************************************************************
 * The subsystem wants to push dynamic data(in NCSMIB_ARG format) to PSSv
 ***************************************************************************/
typedef struct ncsoac_push_mibarg_data_to_pssv
  {
     NCSMIB_ARG       *arg;
  } NCSOAC_PUSH_MIBARG_DATA_TO_PSSV;


/***************************************************************************
 * The Table owning subsystem moves this row 
 ***************************************************************************/

typedef struct ncsoac_row_move
  {
  NCSMIB_ARG* i_move_req;

  } NCSOAC_ROW_MOVE;

/***************************************************************************
 * The same subsystem declares it no longer owns this MIB table
 ***************************************************************************/

typedef struct ncsoac_tbl_gone
  {
  void*           i_meaningless;       /* place holder struct; do nothing */
  
  } NCSOAC_TBL_GONE;


/***************************************************************************
 * The operations set that a OAC instance supports
 ***************************************************************************/

typedef enum ncsoac_ss_op
  {
  NCSOAC_SS_OP_TBL_OWNED,
  NCSOAC_SS_OP_ROW_OWNED,
  NCSOAC_SS_OP_ROW_GONE,
  NCSOAC_SS_OP_TBL_GONE,
  NCSOAC_SS_OP_ROW_MOVE,
  NCSOAC_SS_OP_WARMBOOT_REQ_TO_PSSV,
  NCSOAC_SS_OP_PUSH_MIBARG_DATA_TO_PSSV
  } NCSOAC_SS_OP;

/***************************************************************************
 * The OAC API single entry point for all services
 ***************************************************************************/

typedef struct ncsoac_ss_arg
  {
  NCSOAC_SS_OP  i_op;            /* Operation; TBL/ROW OWNED/GONE     */
  uns32         i_oac_hdl;       /* Same as Layer Mgmt usr_key value  */
  uns32         i_tbl_id;        /* Table ID in question              */

  union 
    {
    NCSOAC_TBL_OWNED  tbl_owned;
    NCSOAC_ROW_OWNED  row_owned;
    NCSOAC_ROW_GONE   row_gone;
    NCSOAC_TBL_GONE   tbl_gone;
    NCSOAC_ROW_MOVE   row_move;
    NCSOAC_PSS_WARMBOOT_REQ warmboot_req;
    NCSOAC_PUSH_MIBARG_DATA_TO_PSSV push_mibarg_data;
    } info;

  } NCSOAC_SS_ARG;

/* applications shall use this abstract name when there is an interface with
 * SPRR 
 */ 
#define m_OAA_SP_ABST_NAME "NCS_OAA"

/**************************************************************************
 * Single Entry API for the service users
 **************************************************************************/
EXTERN_C MABOAC_API uns32 ncsoac_ss    ( NCSOAC_SS_ARG* arg );

#ifdef  __cplusplus
}
#endif

#endif /* OAC_PAPI_H */


