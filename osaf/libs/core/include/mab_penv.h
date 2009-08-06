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

  DESCRIPTION: Common abstractions shared by two or more of the MAB sub-parts,
               being OAC, MAS and MAC.

*******************************************************************************/

/* Module Inclusion Control....*/

#ifndef MAB_PENV_H
#define MAB_PENV_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "mds_papi.h"

/*****************************************************************************

  MAB_LM_EVT  data structure of info provided by MAB Subcomponent to explain
  noteworthy events (alarms, informational conditions) that are happening
  inside MAC/MAS/OAC.

*****************************************************************************/

	typedef enum mab_lm_evt_type {
/*|        EVENT  (i_event)     |               ARGS  (i_args)       |
  +------------------------------------------------------------------+*/
		OAC_TBL_REG_EXIST,	/*|        uns32 (tbl_id)              | */
		OAC_TBL_UNREG_NONEXIST,	/*|        uns32 (tbl_id)              | */
		OAC_FLTR_REG_NO_TBL,	/*|        uns32 (tbl_id)              | */
		OAC_FLTR_TYPE_MISMATCH,	/*|        &MAB_LM_FLTR_TYPE_MM        | */
		OAC_FLTR_UNREG_NO_TBL,	/*|        uns32 (tbl_id)              | */
		OAC_FLTR_UNREG_NO_FLTR,	/*|        &MAB_LM_FLTR_NULL           | */
		MAS_FLTR_TYPE_REG_UNSUPP,	/*|        NCSMIB_FLTR_TYPE             | */
		MAS_FLTR_TYPE_REG_MISMATCH,	/*|        &MAB_LM_FLTR_TYPE_MM        | */
		MAS_FLTR_SIZE_REG_MISMATCH,	/*|        &MAB_LM_FLTR_SIZE_MM        | */
		MAS_FLTR_REG_EXIST,	/*|        &MAB_LM_FLTR_XST            | */
		MAS_FLTR_REG_OVERLAP,	/*|        &MAB_LM_FLTR_OVRLP          | */
		MAS_FLTR_REG_INVALID_SA,	/*|        &MAB_LM_FLTR_INVAL_SA       | */
		MAS_FLTR_REG_SS_MISMATCH,	/*|        &MAB_LM_FLTR_SS_MM          | */
		MAS_FLTR_UNREG_NO_FLTR,	/*|        &MAB_LM_FLTR_NULL           | */
		MAS_FLTR_UNREG_SS_MISMATCH,	/*|        &MAB_LM_FLTR_SS_MM          | */
		MAS_FLTR_MRRSP_NO_FLTR,	/*|        &MAB_LM_FLTR_NULL           | */
		MAS_MAB_TRANS_AT_NONPRIM,	/*|        NULL                        | */
		MAS_FLTR_REG_AT_NONPRIM,	/*|        NULL                        | */
		MAS_FLTR_UNREG_AT_NONPRIM,	/*|        NULL                        | */
		MAS_MAB_MDS_FWD_FAILED	/*|        NULL                        | */
	} MAB_LM_EVT_TYPE;

	typedef struct mab_lm_evt {
		NCS_VRID i_vrid;	/* The  virtual router ID of event         */
		uns32 i_usr_key;	/* User Key value provided at CREATE time  */
		uns8 i_which_mab;	/* NCSMAB_MAC|NCSMAB_MAS|NCSMAB_OAC|NCSMAB_MAS */
		MAB_LM_EVT_TYPE i_event;	/* one of the Event types enumerated       */
		NCSCONTEXT i_args;	/* Value of i_arg based on value of i_event */

	} MAB_LM_EVT;

/*****************************************************************************
  Filter Type Miss match details 
*****************************************************************************/

	typedef struct mab_lm_fltr_type_mm {
		uns32 i_xst_type;
		uns32 i_new_type;

	} MAB_LM_FLTR_TYPE_MM;

/*****************************************************************************
  Filter Size match details 
*****************************************************************************/

	typedef struct mab_lm_fltr_size_mm {
		uns32 i_xst_bgn_idx;
		uns32 i_new_bgn_idx;
		uns32 i_xst_idx_len;
		uns32 i_new_idx_len;

	} MAB_LM_FLTR_SIZE_MM;

/*****************************************************************************
  Filter Already exists details 
*****************************************************************************/

	typedef struct mab_lm_fltr_xst {
		uns32 i_fltr_id;
		MDS_DEST i_vcard;

	} MAB_LM_FLTR_XST;

/*****************************************************************************
  Filter of type RANGE overlaps with already present filters
*****************************************************************************/

	typedef struct mab_lm_fltr_ovrlp {
		struct mas_fltr *i_xst_fltr;
		struct mas_fltr *i_new_fltr;

	} MAB_LM_FLTR_OVRLP;

/*****************************************************************************
  Filter of type SAME_AS has issues
*****************************************************************************/

	typedef enum ncsmab_sa_fltr_err {	/* Used in structure MAB_LM_FLTR_INVAL_SA */
		    NCSMAB_SA_NO_TBL,
		NCSMAB_SA_TBL_NULL,
		NCSMAB_SA_LOOP
	} NCSMAB_SA_FLTR_ERR;

	typedef struct mab_lm_fltr_inval_sa {
		NCSMAB_SA_FLTR_ERR i_type;
		uns32 i_new_fltr_id;
		uns32 i_new_tbl_id;
		uns32 i_sa_tbl_id;

	} MAB_LM_FLTR_INVAL_SA;

/*****************************************************************************
  Did not find the filter spec'ed to deregister.
*****************************************************************************/

	typedef struct mab_lm_fltr_null {
		uns32 i_fltr_id;
		MDS_DEST i_vcard;

	} MAB_LM_FLTR_NULL;

/*****************************************************************************
  Subsystem mismatch; A MIB table can only belong to one subsystem
*****************************************************************************/

	typedef struct mab_lm_fltr_ss_mm {
		uns32 i_tbl_ss_id;
		uns32 i_fltr_ss_id;

	} MAB_LM_FLTR_SS_MM;

/*****************************************************************************
  MAB_LM_CB 
        function prototype for Layer Management Callback. The customer 
        provides a function pointer of this type to the MAC/MAS/OAC so
        that MAB can explain what issues it sees.
*****************************************************************************/

	typedef uns32 (*MAB_LM_CB) (MAB_LM_EVT *evt);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

      STRUCTURES, UNIONS and Derived Types

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/***************************************************************************
  
   M A B   R o w   O w n e r s h i p   F i l t e r s
 
  MAB Table 'access filter' added to describe operations destined for here
  NOTE: The following substructures express possible filter types.
        New ones can be added as new MIB Table organizations are discovered.
       
  NOTE: The MIB Object Access Table at the MAS and the actual MIB table
        apriori agree (by design) as to what access filter policy shall be
        used to describe reachability.

****************************************************************************/

/***************************************************************************
  Access Filter type 'range' :if the index value at <offset> is >= <min> or
                              <= <max>, forward request to OAC @ VCARD.
****************************************************************************/
	typedef struct ncsmab_range {
		uns32 i_idx_len;	/* number of indexing fields for tbl */
		uns32 i_bgn_idx;	/* begin index position for compare  */
		const uns32 *i_min_idx_fltr;	/* values marking 'minimum' compare  */
		const uns32 *i_max_idx_fltr;	/* values marking 'maximum' compare  */

	} NCSMAB_RANGE;

/***************************************************************************
  Access Filter type 'any' :For ANY request having to do with this table, 
                            forward request to OAC @ VCARD.
****************************************************************************/

	typedef struct ncsmab_any {

		uns32 meaningless;	/* NO INFORMATION (so far); place holder */

	} NCSMAB_ANY;

/***************************************************************************
 * Access Filter type 'same as' : use the VCARD location function associated
 *                                MIB table <table_id>.
 ***************************************************************************/
	typedef struct ncsmab_same_as {
		uns32 i_table_id;
	} NCSMAB_SAME_AS;

/***************************************************************************
 * Access Filter type 'default' : For any request not matching existing range
 *                                filters forward request to OAC @ VCARD 
 ***************************************************************************/
	typedef struct ncsmab_default {
		uns32 meaningless;
	} NCSMAB_DEFAULT;

/***************************************************************************
 * Access Filter type 'exact' : For an exact match of the index             
 ***************************************************************************/
	typedef struct ncsmab_exact {
		uns32 i_idx_len;	/* number of indexing fields for tbl */
		uns32 i_bgn_idx;	/* begin index position for compare  */
		const uns32 *i_exact_idx;	/* values marking exact compare  */
	} NCSMAB_EXACT;

/***************************************************************************
 * CMAN MIB Table Access Filter description
 ***************************************************************************/

	typedef enum ncsmib_fltr_type {
		NCSMAB_FLTR_RANGE,
		NCSMAB_FLTR_ANY,
		NCSMAB_FLTR_SAME_AS,
		NCSMAB_FLTR_SCALAR,
		NCSMAB_FLTR_DEFAULT,
		NCSMAB_FLTR_EXACT
	} NCSMAB_FLTR_TYPE;

	typedef struct ncsmab_fltr {
		NCSMAB_FLTR_TYPE type;

		union {
			NCSMAB_RANGE range;
			NCSMAB_ANY any;
			NCSMAB_SAME_AS same_as;
			NCSMAB_DEFAULT def;
			NCSMAB_EXACT exact;

		} fltr;

		NCS_BOOL is_move_row_fltr;
	} NCSMAB_FLTR;

/***************************************************************************
 * Index deallocation structure
 ***************************************************************************/
	typedef struct ncsmab_idx_free {
		NCSMAB_FLTR_TYPE fltr_type;
		uns32 idx_tbl_id;
		union {
			NCSMAB_RANGE range_idx_free;
			NCSMAB_EXACT exact_idx_free;
		} idx_free_data;
	} NCSMAB_IDX_FREE;

#ifdef  __cplusplus
}
#endif

#endif
