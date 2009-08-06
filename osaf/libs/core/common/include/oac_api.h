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

  _Public_ Object Access Client (OAC) abstractions and function prototypes

*******************************************************************************/

/*
 * Module Inclusion Control...
 */

#ifndef OAC_API_H
#define OAC_API_H

/***************************************************************************

 Per Virtual Router API 

    Layer Management API as single entry point

    Create         - Create an instance of OAC (per virtual router)
    Destroy        - Destrou an instance of OAC
    Set            - Set a configuration object value
    Get            - Get a configuration object value

    
    Initiate a MIB Access Request via MIB Access Controller

    Request        - MIB Access Request of some sort
   
****************************************************************************/

/***************************************************************************
 * Create an instance of a OAC service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsoac_create {
	NCS_VRID i_vrid;	/* Virtual Rtr; same in MAS, OAC     */
	SaNameT i_inst_name;	/* instane name, for which this OAC  */
	/*this getting created */
	SYSF_MBX *i_mbx;	/* Mail box to receive MAB messages  */
	MAB_LM_CB i_lm_cbfnc;	/* layer mgmt callback function ptr  */
	uns8 i_hm_poolid;	/* Pool Id required for Handle Mgr   */
	uns32 o_oac_hdl;	/* Handle Manager returned handle    */

} NCSOAC_CREATE;

/***************************************************************************
 * Destroy an instance of a OAC service instance (one per virtual router)
 ***************************************************************************/

typedef struct ncsoac_destroy {
	uns32 i_oac_hdl;	/* OAC Handle                        */
	PW_ENV_ID i_env_id;	/* environement id */
	SaNameT i_inst_name;	/* instance name */
} NCSOAC_DESTROY;

/***************************************************************************
 * Set or Get configuration data for this virtual router instance
 ***************************************************************************/

/* OAC Configuration Object Identifiers whose values to get or set..       */
/* ======================================================================= */
/* RW : Read or Write; can do get or set operations on this object         */
/* R- : Read only; can do get operations on this object                    */
/* -W : Write only; can do set operations on this object                   */

typedef enum {			/*     V a l u e   E x p r e s s i o n      */
			       /*------------------------------------------*/
	NCSOAC_OBJID_LOG_VAL,	/* RW bitmap of log categories to install   */
	NCSOAC_OBJID_LOG_ON_OFF,	/* RW ENABLE|DISABLE logging                */
	NCSOAC_OBJID_SUBSYS_ON,	/* RW ENABLE|DISABLE OAC services           */

} NCSOAC_OBJID;

/* OAC Configuration 'explanation' structure                              */

typedef struct ncsoac_set {
	uns32 i_oac_hdl;
	uns32 i_obj_id;
	uns32 i_obj_val;

} NCSOAC_SET;

typedef struct ncsoac_get {
	uns32 i_oac_hdl;
	uns32 i_obj_id;
	uns32 o_obj_val;

} NCSOAC_GET;

/***************************************************************************
 * The operations set that a OAC instance supports
 ***************************************************************************/

typedef enum ncsoac_lm_op {
	NCSOAC_LM_OP_CREATE,
	NCSOAC_LM_OP_DESTROY,
	NCSOAC_LM_OP_GET,
	NCSOAC_LM_OP_SET,

} NCSOAC_OP;

/***************************************************************************
 * The OAC API single entry point for all services
 ***************************************************************************/

typedef struct ncsoac_lm_arg {
	NCSOAC_OP i_op;		/* Operation; CREATE,DESTROY,GET,SET */

	union {
		NCSOAC_CREATE create;
		NCSOAC_DESTROY destroy;
		NCSOAC_GET get;
		NCSOAC_SET set;

	} info;

} NCSOAC_LM_ARG;

/***************************************************************************
 * Per Virtual Router Instance of Layer Management
 ***************************************************************************/

/***************************************************************************
 * Per Virtual Router Instance of Layer Management
 ***************************************************************************/

EXTERN_C MABOAC_API uns32 ncsoac_lm(NCSOAC_LM_ARG *arg);

#endif   /* OAC_API_H */
