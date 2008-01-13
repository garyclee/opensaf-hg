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

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
 
#ifndef NCS_MIB_PUB_H
#define NCS_MIB_PUB_H


#include "ncsgl_defs.h"
#include "ncs_mtbl.h"
#include "ncs_stack_pub.h"

#ifdef  __cplusplus
extern "C" {
#endif

/***************************************************************************

      STRUCTURES, UNIONS and Derived Types

 ***************************************************************************/

/***************************************************************************
 *
 *   M I B  R e s p o n s e  S t a t u s   V a l u e s 
 *
 *   These Status values are actually declared in gl_defs.h
 * 
 *  NCSCC_RC_SUCCESS:          Success.
 *  NCSCC_RC_NO_SUCH_TBL:      Failure. Invalid Table ID
 *  NCSCC_RC_NO_OBJECT:        Failure. Invalid Parameter ID.
 *  NCSCC_RC_NO_INSTANCE:      Failure. Invalid instance info. Also used for
 *                            GET-NEXT end-of-table operations
 *  NCSCC_RC_INV_VAL:          Failure. Invalid value being set.
 *  NCSCC_RC_INV_SPECIFIC_VAL: Failure. Invalid value being set under
 *                            current conditions.
 *  NCSCC_RC_REQ_TIMOUT:       Failure. Timer expired waiting for response.
 *  NCSCC_RC_FAILURE:          Other Failure.
 *
 ***************************************************************************/


/***************************************************************************
 *
 *   P a r a m e t e r   A b s t r a c t i o n s
 *
 ***************************************************************************/

/*************************************************************************** 
 * General indexing information for Access operations
 ***************************************************************************/

typedef struct ncsmib_idx {

  const uns32*        i_inst_ids;
  uns32               i_inst_len;
  
  } NCSMIB_IDX;

/*************************************************************************** 
 * General parameter value expression
 ***************************************************************************/
typedef enum ncsmib_fmat_id
{
    NCSMIB_FMAT_INT = 1,
    NCSMIB_FMAT_OCT,
    NCSMIB_FMAT_BOOL,
    NCSMIB_FMAT_2LONG
}NCSMIB_FMAT_ID;


/***************************************************************************
 * General parameter ID expression
 ***************************************************************************/

typedef uns32 NCSMIB_PARAM_ID;

/***************************************************************************
 * General Parameter Abstraction
 ***************************************************************************/

typedef struct ncsmib_param_val
{
  NCSMIB_PARAM_ID              i_param_id;
  NCSMIB_FMAT_ID               i_fmat_id;  /* int or octets */
  uns16                       i_length;   /* Always octets? */

  union
  {
    uns32                     i_int;
    const uns8*               i_oct;
  } info;

} NCSMIB_PARAM_VAL;

/*typedef enum ncsmib_policy
{
   NCSMIB_POLICY_BELIEVE_ME = 1,
   NCSMIB_POLICY_OTHER
} NCSMIB_POLICY;*/
/* Changing this to a bitmask */
#define NCSMIB_POLICY_PSS_BELIEVE_ME           0x0001
#define NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER   0x0002
#define NCSMIB_POLICY_OTHER                    0x0004

/* Macro to verify whether this is the last event for the playback session */
#define m_NCSMIB_PSSV_PLBCK_IS_LAST_EVENT(policy) \
                  ((policy & NCSMIB_POLICY_PSS_LAST_PLBCK_TRIGGER) ? TRUE:FALSE)

/* Defining PSSv MIB client name length max */
#define NCSMIB_PCN_LENGTH_MAX            256

/*****************************************************************************

    H J M I B _ A R G  U S R B U F  E n c o d e   &  D e c o d e   R u l e s

These MAB operations have encoded USRBUFs associated with them:

� NCSMIB_OP_REQ_SETROW   encode some subset of all the objects in the table row 
                        specified by the request indexing information and the 
                        Table Id.
� NCSMIB_OP_REQ_TESTROW encode some subset of all the objects in the table row 
                        specified by the request indexing information and the 
                        Table Id.
� NCSMIB_OP_RSP_SETROW   echo back the same info found in the SETROW  request.
� NCSMIB_OP_RSP_TESTROW echo back the same info found in the TESTROW request. 
  NCSMIB_OP_RSP_NEXTROW  encode all readable objects in the table row specified 
                        by the response indexing information and the Table ID..
� NCSMIB_OP_RSP_GETROW   encode all readable objects in the table row specified 
                        by the response indexing information and the Table ID.

  NCSMIB_OP_RSP_MOVEROW  echo back the same infor found in the MOVEROW request.
  
The encode scheme in all of these cases are as follows

    +-----------------+
    | uns16   marker  |  : PARM_ENC_SEQ_MARKER sanity value
    +-----------------+
    | uns16   ttl_cnt |  : count of all NCSPARAM_VALs encoded
    +-----------------+
    | uns16   ttl_len |  : lenght of all NCSPARAM_VALs encoded
    +-----------------+
    | <param_val>..1  |  : As per ncsmib_paramval_encode()
    |                 |
    +-----------------+
    | <param_val>..n  |  : Where n == ttl_cnt
    |                 |
    +-----------------+

 All encoding is done in the context of the NetPlane USRBUF abstraction.

 =============================================================================

 NOTE: The same encode/decode loop and assist logic will be coded in 
 many places in many subsystems. 
 
 To curb this tendency, there is an assist structure called NCSPARM_AID 
 and a set of associated functions that manages USRBUF state associated 
 with the above encoding scheme/rules.
 
 The intent is to cut down on repetitive code, bugs and localize changes
 that might come about if the PARAM_VAL object should change.

******************************************************************************/

#define PARM_ENC_SEQ_MARKER 0x230F /* Arbitrary but unlikely by accident */
#define ROW_ENC_SEQ_MARKER    0x23F0 /* Arbitrary but unlikely by accident */
#define REMROW_ENC_SEQ_MARKER 0x23FF /* Arbitrary but unlikely by accident */
#define NO_NAME_OBJECT  0x45464553 /* Arbitrary but unlikely by accident */ 

/******************************************************************************

  H J P A R M _ A I D

  Assist object for encode/decode param_val operations in the context of
  NCSMIB_ARG. This object is imagined to live on the stack.

  There are five encode functions. They are:

  ncsparm_enc_init - put NCSPARM_AID in start state; set ptrs ttl_len/ttl_cnt
  ncsparm_enc_int  - encode a param_val of type integer
  ncsparm_enc_oct  - encode a param_val of type octetstring
  ncsparm_enc_parm - encode a properly populated param_val structure
  ncsparm_enc_done - fill in ttl_len and ttl_cnt and return encoded USRBUF.
  
  There are 3 decode functions. They are:

  ncsparm_dec_init - put NCSPARM_AID in start state; return #of params encoded
  ncsparm_dec_parm - populate the passed NCSMIB_PARM_VAL with decoded stuff
  ncsparm_dec_done - confirm all has been consumed and len & cnt checks align.

******************************************************************************/

typedef struct ncsparm_aid {

                   /* ENC functions           DEC functions                 */
                   /*=======================+===============================*/
  NCS_UBAID  uba;   /* usrbuf mgmt             usrbuf mgmt                   */
  uns16*    p_len; /* pts to where len goes   Not Used                      */
  uns16*    p_cnt; /* pts to where cnt goes   Not Used                      */
  uns16     cnt;   /* num parms encoded       num parms left to decode      */
  uns16     len;   /* Not Used                ttl len expected              */      

} NCSPARM_AID;

/******************************************************************************
  H J P A R M _ A I D  Encode functions
******************************************************************************/

EXTERN_C LEAPDLL_API void    ncsparm_enc_init     ( NCSPARM_AID*      pa);

EXTERN_C LEAPDLL_API uns32   ncsparm_enc_int      ( NCSPARM_AID*      pa, 
                                       NCSMIB_PARAM_ID   id, 
                                       uns32            val);

EXTERN_C LEAPDLL_API uns32   ncsparm_enc_oct      ( NCSPARM_AID*      pa, 
                                       NCSMIB_PARAM_ID   id, 
                                       uns16            len, 
                                       uns8*            octs);

EXTERN_C LEAPDLL_API uns32   ncsparm_enc_param    ( NCSPARM_AID*      pa, 
                                       NCSMIB_PARAM_VAL* val);

EXTERN_C LEAPDLL_API USRBUF* ncsparm_enc_done     ( NCSPARM_AID*      pa);

/******************************************************************************
  H J P A R M _ A I D  Decode functions
******************************************************************************/

EXTERN_C LEAPDLL_API uns32   ncsparm_dec_init     ( NCSPARM_AID*      pa,
                                       USRBUF*          ub);

EXTERN_C LEAPDLL_API uns32   ncsparm_dec_parm     ( NCSPARM_AID*      pa,
                                       NCSMIB_PARAM_VAL* val, 
                                       NCSMEM_AID*       ma);

EXTERN_C LEAPDLL_API uns32   ncsparm_dec_done     ( NCSPARM_AID*      pa);

/******************************************************************************
   
  H J S E T A L L R O W S _ A I D

  Assist object for encode/decode setallrows operations in the context of
  NCSMIB_ARG. This object is imagined to live on the stack.

  There are 8 encode functions. They are:

  ncssetallrows_enc_init - put NCSSETALLROWS_AID in start state;
                          set ptrs ttl_len/ttl_cnt
  ncsrow_enc_init        - put NCSPARM_AID in start state;
                          set ptrs ttl_len/ttl_cnt
  ncsrow_enc_inst_ids    - encode the NCSMIB_IDX parameter for a row
  ncsrow_enc_int         - encode a param_val of type integer
  ncsrow_enc_oct         - encode a param_val of type octetstring
  ncsrow_enc_parm        - encode a properly populated param_val structure
  ncsrow_enc_done        - fill in len and cnt for the encoded row
  ncssetallrows_enc_done - fill in ttl_len and ttl_cnt and return encoded USRBUF.
  
  There are 3 decode functions. They are:

  ncssetallrows_dec_init - put NCSSETALLROWS_AID in start state;
                          return # of rows encoded
  ncsrow_dec_init        - put NCSPARM_AID in start state;
                          return # of params encoded
  ncsrow_dec_inst_ids    - populate passed NCSMIB_IDX with decoded stuff
  ncsrow_dec_parm        - populate the passed NCSMIB_PARM_VAL with decoded stuff
  ncsrow_dec_done        - confirm all parameters have been consumed.
  ncssetallrows_dec_done - confirm that all rows have been consumed and cnt and len align
   
******************************************************************************/

typedef struct ncsrow_aid {
   /*=======================+===============================*/
   /* ENC functions           DEC functions                 */
   /*=======================+===============================*/
   NCS_UBAID   uba;   /* usrbuf mgmt             usrbuf mgmt                   */
   uns16*     p_len; /* pts to where len goes   Not Used                      */
   uns16*     p_cnt; /* pts to where cnt goes   Not Used                      */
   uns16      cnt;   /* num rows encoded        num rows left to decode       */
   uns16      len;   /* Not Used                ttl len expected              */
   NCSPARM_AID parm;  /* encode parameters       decode parameters             */
} NCSROW_AID;        /*=======================+===============================*/

/******************************************************************************
                     H J R O W _ A I D  Encode functions
******************************************************************************/

EXTERN_C LEAPDLL_API uns32   ncssetallrows_enc_init  ( NCSROW_AID*      ra);

EXTERN_C LEAPDLL_API uns32   ncsrow_enc_init         ( NCSROW_AID*      ra);

EXTERN_C LEAPDLL_API uns32   ncsrow_enc_inst_ids     ( NCSROW_AID*      ra,
                                          NCSMIB_IDX*      inst);

EXTERN_C LEAPDLL_API uns32   ncsrow_enc_int          ( NCSROW_AID*      ra,
                                          NCSMIB_PARAM_ID  id,
                                          uns32           val);

EXTERN_C LEAPDLL_API uns32   ncsrow_enc_oct          ( NCSROW_AID*       ra,
                                          NCSMIB_PARAM_ID   id,
                                          uns16            len,
                                          uns8*            octs);

EXTERN_C LEAPDLL_API uns32   ncsrow_enc_param        ( NCSROW_AID*       ra,
                                          NCSMIB_PARAM_VAL* val);

EXTERN_C LEAPDLL_API uns32   ncsrow_enc_done         ( NCSROW_AID*       ra);

EXTERN_C LEAPDLL_API USRBUF* ncssetallrows_enc_done  ( NCSROW_AID*       ra);

/******************************************************************************
H J R O W _ A I D  Decode functions
******************************************************************************/

EXTERN_C LEAPDLL_API uns32   ncssetallrows_dec_init ( NCSROW_AID*     ra,
                                         USRBUF*        ub);

EXTERN_C LEAPDLL_API uns32   ncsrow_dec_init        ( NCSROW_AID*      ra);

EXTERN_C LEAPDLL_API uns32   ncsrow_dec_inst_ids    ( NCSROW_AID*      ra,
                                         NCSMIB_IDX*      idx,
                                         NCSMEM_AID*      ma);

EXTERN_C LEAPDLL_API uns32   ncsrow_dec_param       ( NCSROW_AID*       ra,
                                         NCSMIB_PARAM_VAL* val,
                                         NCSMEM_AID*       ma);

EXTERN_C LEAPDLL_API uns32   ncsrow_dec_done        ( NCSROW_AID*       ra);

EXTERN_C LEAPDLL_API uns32   ncssetallrows_dec_done ( NCSROW_AID*       ra);

/******************************************************************************

   H J R E M R O W _ A I D

   Assist object for encode/decode param_val operations in the context of
   NCSMIB_ARG. This object is imagined to live on the stack.

   There are 3 encode functions. They are:

   ncsremrow_enc_init      - put NCSPARM_AID in start state; set ptrs ttl_len/ttl_cnt
   ncsremrow_enc_inst_ids  - encode a NCSMIB_IDX
   ncsremrow_enc_done      - fill in ttl_len and ttl_cnt and return encoded USRBUF.

   There are 3 decode functions. They are:

   ncsremrow_dec_init     - put NCSPARM_AID in start state; return #of NCSMIB_IDX encoded
   ncsremrow_dec_inst_ids - populate the passed NCSMIB_PARM_VAL with decoded stuff
   ncsremrow_dec_done     - confirm all has been consumed and len & cnt checks align.
            
******************************************************************************/

typedef struct ncsremrow_aid {
   
   /* ENC functions           DEC functions                 */
   /*=======================+===============================*/
   NCS_UBAID  uba;   /* usrbuf mgmt             usrbuf mgmt                   */
   uns16*    p_len; /* pts to where len goes   Not Used                      */
   uns16*    p_cnt; /* pts to where cnt goes   Not Used                      */
   uns16     cnt;   /* num parms encoded       num parms left to decode      */
   uns16     len;   /* Not Used                ttl len expected              */      
   
} NCSREMROW_AID;

/******************************************************************************
H J R E M R O W _ A I D  Encode functions
******************************************************************************/

EXTERN_C LEAPDLL_API uns32   ncsremrow_enc_init     ( NCSREMROW_AID*      rra);

EXTERN_C LEAPDLL_API uns32   ncsremrow_enc_inst_ids ( NCSREMROW_AID*      rra,
                                         NCSMIB_IDX*         idx);

EXTERN_C LEAPDLL_API USRBUF* ncsremrow_enc_done     ( NCSREMROW_AID*      rra);

/******************************************************************************
H J R E M R O W _ A I D  Decode functions
******************************************************************************/

EXTERN_C LEAPDLL_API uns32   ncsremrow_dec_init     ( NCSREMROW_AID*      rra,
                                         USRBUF*            ub);

EXTERN_C LEAPDLL_API uns32   ncsremrow_dec_inst_ids ( NCSREMROW_AID*      rra,
                                         NCSMIB_IDX*         idx,
                                         NCSMEM_AID*         ma);

EXTERN_C LEAPDLL_API uns32   ncsremrow_dec_done     ( NCSREMROW_AID*      rra);

/***************************************************************************
 *
 *   R e q u e s t - R e s p o n s e   A b s t r a c t i o n s
 *
 ***************************************************************************/

/***************************************************************************
 * MIB Object get request
 ***************************************************************************/

typedef struct ncsmib_get_req
{
  NCSMIB_PARAM_ID              i_param_id;

} NCSMIB_GET_REQ;

/***************************************************************************
 * MIB Object get-next request  (same form as 'get')
 ***************************************************************************/

typedef NCSMIB_GET_REQ          NCSMIB_NEXT_REQ;


/***************************************************************************
 * MIB Object get-info request  (same form as 'get')
 ***************************************************************************/

typedef NCSMIB_GET_REQ          NCSMIB_GET_INFO_REQ;


/***************************************************************************
 * MIB Object set request
 ***************************************************************************/

typedef struct ncsmib_set_req
{
  NCSMIB_PARAM_VAL             i_param_val;

} NCSMIB_SET_REQ;

/***************************************************************************
 * MIB Object test request (same form as 'set')
 ***************************************************************************/

typedef NCSMIB_SET_REQ         NCSMIB_TEST_REQ;

/***************************************************************************
 * MIB Object get BULK request
 ***************************************************************************/

typedef struct ncsmib_getrow_req
{
  uns32                       i_meaningless;

} NCSMIB_GETROW_REQ;

/***************************************************************************
 * MIB Object get-next BULK request (same form as 'get-BULK')
 ***************************************************************************/

typedef NCSMIB_GETROW_REQ      NCSMIB_NEXTROW_REQ;


/***************************************************************************
 * MIB Object set BULK request
 ***************************************************************************/

typedef struct ncsmib_setrow_req
{
  USRBUF*                     i_usrbuf;

} NCSMIB_SETROW_REQ;

/***************************************************************************
 * MIB Object set BULK ROWs request
 ***************************************************************************/
   
typedef struct ncsmib_setallrows_req
{
   USRBUF*                     i_usrbuf;
   
} NCSMIB_SETALLROWS_REQ;

/***************************************************************************
 * MIB Object REMOVE ROWs request
 ***************************************************************************/

typedef struct ncsmib_removerows_req
{
   USRBUF*                     i_usrbuf;
   
} NCSMIB_REMOVEROWS_REQ;

/***************************************************************************
 * MIB object test BULK request (same form as 'set-BULK')
 ***************************************************************************/

typedef NCSMIB_SETROW_REQ      NCSMIB_TESTROW_REQ;

/***************************************************************************
 * MIB object get response
 ***************************************************************************/

typedef struct ncsmib_get_rsp
{
  NCSMIB_PARAM_VAL             i_param_val;

} NCSMIB_GET_RSP;

/***************************************************************************
 * MIB object set response (same form as 'get')
 ***************************************************************************/

typedef NCSMIB_GET_RSP         NCSMIB_SET_RSP;

/***************************************************************************
 * MIB object test response (same form as 'get')
 ***************************************************************************/

typedef NCSMIB_GET_RSP         NCSMIB_TEST_RSP;

/***************************************************************************
 * MIB object get-next response
 ***************************************************************************/

typedef struct ncsmib_next_rsp
{
  NCSMIB_PARAM_VAL             i_param_val;
  NCSMIB_IDX                   i_next;

} NCSMIB_NEXT_RSP;

/***************************************************************************
 * MIB Table get BULK response
 ***************************************************************************/

typedef struct ncsmib_getrow_rsp
{
  USRBUF*                     i_usrbuf;

} NCSMIB_GETROW_RSP;

/***************************************************************************
 * MIB Table set BULK ROWs response
 ***************************************************************************/
   
typedef struct ncsmib_setallrows_rsp
{
   USRBUF*                     i_usrbuf;
   
} NCSMIB_SETALLROWS_RSP;
   
/***************************************************************************
 * MIB Table REMOVE ROWs response
 ***************************************************************************/

typedef struct ncsmib_removerows_rsp
{
   USRBUF*                     i_usrbuf;
   
} NCSMIB_REMOVEROWS_RSP;

/***************************************************************************
 * MIB object test BULK response (same form as 'getrow')
 ***************************************************************************/

typedef NCSMIB_GETROW_RSP      NCSMIB_TESTROW_RSP;

/***************************************************************************
 * MIB object set BULK response (same form as 'getrow')
 ***************************************************************************/

typedef NCSMIB_GETROW_RSP      NCSMIB_SETROW_RSP;

/***************************************************************************
 * MIB Table get-next BULK response
 ***************************************************************************/

typedef struct ncsmib_nextrow_rsp
{
  USRBUF*                     i_usrbuf;
  NCSMIB_IDX                   i_next;

} NCSMIB_NEXTROW_RSP;


/***************************************************************************
 * Move MIB row request
 ***************************************************************************/

typedef struct ncsmib_moverow_req
{
  MDS_DEST                    i_move_to;
  USRBUF*                     i_usrbuf;

} NCSMIB_MOVEROW_REQ;

typedef struct ncsmib_moverow_rsp
{
   MDS_DEST                   i_move_to;
   USRBUF*                    i_usrbuf;
   
} NCSMIB_MOVEROW_RSP;

/***************************************************************************
 * TRAP expression function: entirely inadequate at this point (not used)
 ***************************************************************************/

typedef struct ncsmib_trap
{
  uns32                        i_trap_id;

} NCSMIB_TRAP;




/***************************************************************************
 * MIB object get-info response
 ***************************************************************************/

typedef struct ncsmib_get_info_rsp
{
  uns32            settable;
  NCSMIB_FMAT_ID    type;
  uns32            max_size;
  uns32            min_value;
  uns32            max_value;
  char*            name;

} NCSMIB_GET_INFO_RSP;


/***************************************************************************
 * MIB object CLI request
 ***************************************************************************/
typedef struct ncsmib_cli_req
{
   uns16     i_cmnd_id;   /* Command ID, should be unique per CMD-TBL-ID */
   NCS_BOOL  i_wild_card; /* TRUE for a wild-card request */
   USRBUF    *i_usrbuf;   /* User data */
} NCSMIB_CLI_REQ;


/***************************************************************************
 * MIB object CLI response
 ***************************************************************************/
typedef struct ncsmib_cli_rsp
{
  uns16    i_cmnd_id; /* issued Command ID */
  NCS_BOOL o_partial;    /* Set TRUE for partial response of the REQ, else FALSE */
  USRBUF   *o_answer; /* Info content, if required */  
} NCSMIB_CLI_RSP;



/***************************************************************************
 * MIB Object Response structure
 ***************************************************************************/

typedef  struct ncsmib_rsp
  {

  uns32                 i_status;

  union
    {
    NCSMIB_GET_RSP       get_rsp;
    NCSMIB_SET_RSP       set_rsp;
    NCSMIB_TEST_RSP      test_rsp;
    NCSMIB_NEXT_RSP      next_rsp;
    NCSMIB_GETROW_RSP    getrow_rsp;
    NCSMIB_NEXTROW_RSP   nextrow_rsp;
    NCSMIB_SETROW_RSP    setrow_rsp;
    NCSMIB_TESTROW_RSP   testrow_rsp;
    NCSMIB_TRAP          trap;
    NCSMIB_GET_INFO_RSP  get_info_rsp;
    NCSMIB_SETALLROWS_RSP setallrows_rsp;
    NCSMIB_REMOVEROWS_RSP removerows_rsp;
    NCSMIB_MOVEROW_RSP    moverow_rsp;
    NCSMIB_CLI_RSP        cli_rsp;
    } info;

    /* Following fields provide addtional error information
     * in the response 
     */
     /* length of the binary stream */ 
     
     uns16                 add_info_len; 

     /* pointer to the binary stream, need not end with '\0' */
     uns8                  *add_info; 
  
  } NCSMIB_RSP;


/***************************************************************************
 * MIB Object Request structure
 ***************************************************************************/

typedef struct ncsmib_req
  {
  union
    {
    NCSMIB_GET_REQ       get_req;
    NCSMIB_SET_REQ       set_req;
    NCSMIB_SET_REQ       test_req;
    NCSMIB_NEXT_REQ      next_req;
    NCSMIB_GETROW_REQ    getrow_req;
    NCSMIB_NEXTROW_REQ   nextrow_req;
    NCSMIB_SETROW_REQ    setrow_req;
    NCSMIB_TESTROW_REQ   testrow_req;
    NCSMIB_MOVEROW_REQ   moverow_req;
    NCSMIB_GET_INFO_REQ  get_info_req;
    NCSMIB_SETALLROWS_REQ setallrows_req;
    NCSMIB_REMOVEROWS_REQ removerows_req;
    NCSMIB_CLI_REQ        cli_req;

    } info;
  
  } NCSMIB_REQ;


/***************************************************************************
 *
 *   C o m m o n   T r a n s a c t i o n   A b s t r a c t i o n s
 *
 ***************************************************************************/

 /***************************************************************************
 * MIB Object Response Operation Identifiers
 ***************************************************************************/

typedef enum ncsmib_op {

  NCSMIB_OP_RSP_GET         = 0x0001,   
  NCSMIB_OP_RSP_SET         = 0x0002,
  NCSMIB_OP_RSP_TEST        = 0x0003,
  NCSMIB_OP_RSP_NEXT        = 0x0004,
  NCSMIB_OP_RSP_GETROW      = 0x0005,
  NCSMIB_OP_RSP_NEXTROW     = 0x0006,
  NCSMIB_OP_RSP_SETROW      = 0x0007,
  NCSMIB_OP_RSP_TESTROW     = 0x0008,
  NCSMIB_OP_RSP_MOVEROW     = 0x0009,
  NCSMIB_OP_RSP_GET_INFO    = 0x000A,
  NCSMIB_OP_RSP_REMOVEROWS  = 0x000B,
  NCSMIB_OP_RSP_SETALLROWS  = 0x000C,

  NCSMIB_OP_RSP_CLI         = 0x000D,
  NCSMIB_OP_RSP_CLI_DONE    = 0x000E,

  NCSMIB_OP_REQ_GET         = 0x0010,
  NCSMIB_OP_REQ_SET         = 0x0020,
  NCSMIB_OP_REQ_TEST        = 0x0030,
  NCSMIB_OP_REQ_NEXT        = 0x0040,
  NCSMIB_OP_REQ_GETROW      = 0x0050,
  NCSMIB_OP_REQ_NEXTROW     = 0x0060,
  NCSMIB_OP_REQ_SETROW      = 0x0070,
  NCSMIB_OP_REQ_TESTROW     = 0x0080,
  NCSMIB_OP_REQ_MOVEROW     = 0x0090,
  NCSMIB_OP_REQ_GET_INFO    = 0x00A0,
  NCSMIB_OP_REQ_REMOVEROWS  = 0x00B0,
  NCSMIB_OP_REQ_SETALLROWS  = 0x00C0,
  NCSMIB_OP_REQ_CLI         = 0x00D0,

  NCSMIB_OP_STD_LEAP_END    = 0x0100 /* MIB Broker takes over from here */
  
  } NCSMIB_OP;

/***************************************************************************
 * Useful values and inline assist macros used to understand the Op ID.
 ***************************************************************************/

#define NCSMIB_OP_RSP         0x000F
#define NCSMIB_OP_REQ         0x00F0

#define m_NCSMIB_ISIT_A_RSP(op) (op & NCSMIB_OP_RSP)
#define m_NCSMIB_ISIT_A_REQ(op) (op & NCSMIB_OP_REQ)

#define m_NCSMIB_REQ_TO_RSP(op) (op >> 4) 
#define m_NCSMIB_RSP_TO_REQ(op) (op << 4)

/***************************************************************************
 * Compiler hint that there is such a structure in the world..
 ***************************************************************************/

struct ncsmib_arg;          /* this allows function prototypes below to fly */

/***************************************************************************
 * NCS_KEY i_usr_hdl   : See /base/common/inc/ncs_svd.h
 ***************************************************************************/


/***************************************************************************
 *
 *      N e t P l a n e   U n i v e r s a l  M I B   A c c e s s   A P I 
 *
 *      a n d    C a l l b a c k   F u n c t i o n  P r o t o t y p e s
 *
 ***************************************************************************/

/***************************************************************************
 * Function prototype used for MIB Object REQUESTs.. A Response is always
 * Asynchronous and abides by the NCSMIB_RSP_FNC function prototype.
 ***************************************************************************/

typedef uns32 (*NCSMIB_REQ_FNC)(struct ncsmib_arg* req);

/***************************************************************************
 * Function prototype used for MIB Object RESPONSEs.. A Requestor always 
 * provides a function pointer that matches this prototype to the invoked
 * subsystem.
 ***************************************************************************/

typedef uns32 (*NCSMIB_RSP_FNC)(struct ncsmib_arg* rsp);



/***************************************************************************
 *
 *      N e t P l a n e   U n i v e r s a l  M I B   A c c e s s   A P I 
 *
 *         A r g u m e n t  A b s t r a c t i o n   &   R u l e s
 *
 ***************************************************************************/

#define NCSMIB_STACK_SIZE 184

/***************************************************************************
 *
 * MIB Transaction structure explains all there is.........................
 *
 *  Note: 
 *    - Except for i_op, the Common Transaction Data shall not be changed
 *
 *    - The 'Stateless-Assist' private data must be preserved across 
 *      environmental boundaries. NetPlane provides encode/decode functions.
 *
 *    - The passed Request (NCSMIB_REQ) cannot be changed. The answer or its
 *      failure shall be recorded in the Response (NCSMIB_RSP) object.
 *    
 ***************************************************************************/

typedef struct ncsmib_arg
  {

  /*  C o m m o n   T r a n s a c t i o n   D a t a                      */

  NCSMIB_OP              i_op;        /* Flavors of Request/Response op   */
  NCSMIB_TBL_ID          i_tbl_id;    /* MIB table id operation is about  */
  uns64                 i_usr_key;   /* Service User's ID value          */
  uns64                 i_mib_key;   /* Service Provider's ID value      */
  uns32                 i_xch_id;    /* Unique transaction value         */
  NCSMIB_RSP_FNC         i_rsp_fnc;   /* callback function for RESPONSE   */
  NCSMIB_IDX             i_idx;       /* indexing info for operation      */
  uns16                  i_policy;    /* Bitmask, how to treat this request  */

  /*  S t a c k   S v c   f o r   N e s t e d  M I B   O p s             */

  NCS_STACK              stack;       /* private 'help' data;internal use */
  uns8                  space[NCSMIB_STACK_SIZE];          /* stack space */

  /*  M I B   P a r a m e t e r   P a r t i c u l a r  I n f o           */

  NCSMIB_REQ             req;         /* Request & parameter particulars  */
  NCSMIB_RSP             rsp;         /* Response & parameter particulars */

  } NCSMIB_ARG;

/***************************************************************************
 *
 *     P u b l i c  NCSMIB_ARG    A s s i s t   F u n c t i o n s 
 *
 ***************************************************************************/

EXTERN_C LEAPDLL_API uns32      ncsmib_sync_request ( NCSMIB_ARG*    arg, 
                                         NCSMIB_REQ_FNC req, 
                                         uns32         period_10ms,
                                         NCSMEM_AID*    ma         );

EXTERN_C LEAPDLL_API uns32      ncsmib_timed_request( NCSMIB_ARG*    arg, 
                                         NCSMIB_REQ_FNC req, 
                                         uns32         period_10ms);

EXTERN_C LEAPDLL_API uns32      ncsmib_tm_create    ( void                     );

EXTERN_C LEAPDLL_API uns32      ncsmib_tm_destroy   ( void                     );


/***************************************************************************
 *
 *     P u b l i c   NCSMIB_ARG   U t i l i t y   F u n c t i o n s
 *
 ***************************************************************************/

EXTERN_C LEAPDLL_API uns32      ncsmib_make_req_looklike_rsp( 
                                                 NCSMIB_ARG*    req, 
                                                 NCSMIB_ARG*    rsp,
                                                 NCSMEM_AID*    maid);
EXTERN_C LEAPDLL_API void       ncsmib_pp                   ( NCSMIB_ARG*    arg);

EXTERN_C LEAPDLL_API char*      mibpp_status               ( uns32         status);

EXTERN_C LEAPDLL_API void       ncsmib_req_pp                   ( NCSMIB_ARG*    arg);/* Fix for bug 60198 */

EXTERN_C LEAPDLL_API void       ncsmib_init                 ( NCSMIB_ARG*    arg);

EXTERN_C LEAPDLL_API NCSMIB_ARG* ncsmib_memcopy              ( NCSMIB_ARG*    arg);

EXTERN_C LEAPDLL_API void       ncsmib_memfree              ( NCSMIB_ARG*    arg);

EXTERN_C LEAPDLL_API uns32*     ncsmib_inst_memcopy         ( uns32         len, 
                                                 const uns32*  ref);

EXTERN_C LEAPDLL_API uns8*      ncsmib_oct_memcopy          ( uns32         len,
                                                 const uns8*   ref);

EXTERN_C LEAPDLL_API uns32      ncsmib_arg_free_resources   ( NCSMIB_ARG*    arg,
                                                 NCS_BOOL       is_req);


/***************************************************************************
 *
 *     P u b l i c   encode/decode   F u n c t i o n   P r o t o t y p e s
 *
 ***************************************************************************/

EXTERN_C LEAPDLL_API uns32      ncsmib_encode          ( NCSMIB_ARG*      arg, 
                                            NCS_UBAID*       uba, 
                                            uns16 msg_fmt_ver);

EXTERN_C LEAPDLL_API uns32      ncsmib_req_encode      ( NCSMIB_OP        op,
                                            NCSMIB_REQ*      req,
                                            NCS_UBAID*       uba);

EXTERN_C LEAPDLL_API uns32      ncsmib_rsp_encode      ( NCSMIB_OP        op,
                                            NCSMIB_RSP*      rsp,
                                            NCS_UBAID*       uba,
                                            uns16 msg_fmt_ver);


EXTERN_C LEAPDLL_API uns32      ncsmib_param_val_encode( NCSMIB_PARAM_VAL* mpv,
                                            NCS_UBAID*        uba);

EXTERN_C LEAPDLL_API uns32      ncsmib_inst_encode     ( const uns32*     inst_ids,
                                            uns32            inst_len,
                                            NCS_UBAID*        uba);

EXTERN_C LEAPDLL_API NCSMIB_ARG* ncsmib_decode          ( NCS_UBAID*        uba,
                                                          uns16             msg_fmt_ver);

EXTERN_C LEAPDLL_API uns32      ncsmib_req_decode      ( NCSMIB_OP         op,
                                            NCSMIB_REQ*       req,
                                            NCS_UBAID*        uba);

EXTERN_C LEAPDLL_API uns32      ncsmib_rsp_decode      ( NCSMIB_OP         op,
                                            NCSMIB_RSP*       rsp,
                                            NCS_UBAID*        uba,
                                            uns16             msg_fmt_ver);

EXTERN_C LEAPDLL_API uns32      ncsmib_param_val_decode( NCSMIB_PARAM_VAL* mpv,
                                            NCSMEM_AID*       ma,
                                            NCS_UBAID*        uba);

EXTERN_C LEAPDLL_API uns32      ncsmib_inst_decode     ( uns32**          inst_ids,
                                            uns32            inst_len,
                                            NCS_UBAID*        uba);

/* to extract the oid into uns32 array from octet stream */ 
EXTERN_C uns32
ncsmib_oid_extract(uns16 i_oid_length_in_ints, uns8 *i_oct, uns32 *o_obj_id);

/* to put the OID into param value from octet stream */ 
EXTERN_C uns32
ncsmib_oid_put(uns16    i_oid_len_in_ints,    
               uns32    *i_oid, 
               uns8     *io_oid_buff, 
               NCSMIB_PARAM_VAL *io_param_val); 
#ifdef  __cplusplus
}
#endif

#endif

