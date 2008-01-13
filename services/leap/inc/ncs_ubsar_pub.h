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

  This module contains declarations that facilitate USRSR usage. These uses
  were needed and developed to provide functionality for segmentation and
  Reassembly of streams.
 
  NOTE: 
   This include and its corresponding ubuf_aid.c are put 'above' the sig
   directory since these services are not particular to any one sub-system.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_UBSAR_PUB_H
#define NCS_UBSAR_PUB_H

#include "ncssysf_mem.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

   STRUCTURES, UNIONS and Derived Types

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define NCS_UBSAR_MAX_SIZE        20 /* test-size,, Real user/Real size      */

#define NCS_UBSAR_MAX_TIME         2 /* The ubsar time interval (in seconds) */

#define NCS_UBSAR_TRLR_SIZE        2
#define NCS_UBSAR_APP_TRLR_SIZE    8

#define NCS_UBSAR_CPLT_TRLR_SIZE (NCS_UBSAR_TRLR_SIZE + NCS_UBSAR_APP_TRLR_SIZE)


#define UBSAR_TRLR_FLAG           0x8000
#define UBSAR_SEQ_MASK            0x7fff

/******************************************************************************
  Layer Management notification function takes as it parameter
  the event to be notified to the layer mgmt
******************************************************************************/

typedef enum ncs_ubsar_lm {

 NCS_UBSAR_LM_WRONG_SEQ_NO,
 NCS_UBSAR_LM_TIMER_EXPIRED

} NCS_UBSAR_LM;

/******************************************************************************
 Layer Management notification callback function
******************************************************************************/

typedef uns32 (*NCS_UBSAR_LM_CB) (NCSCONTEXT arg1, NCS_UBSAR_LM indication);

/******************************************************************************

  struct NCS_UBSAR : Private

  The NCS_UBSAR structure is a PRIVATE scratch area for marking state.
  A client should not look at its fields. Rather, the client uses
  the NCS_UBAR support function ncs_ubsar_request() to achieve desired
  results.
 
******************************************************************************/

typedef struct ncs_ubsar {

 USRBUF*         start;        /* keeps the USRBUF list for assembly    */
 NCS_UBSAR_LM_CB  lm_notify_func; /* LM notification function CB           */
  void*           lm_notify_arg;  /* arg1 of lm_notif_func arg list.....   */
 uns16       last_seq_no;   /* seq num of the last packet rcv'd      */
 uns32       max_size;       /* max packet size allowed on transport  */
 time_t     arrival_time;   /* time the last packet arrived          */
 uns32           time_interval;  /* Max pause allowed between two packets */
  SYSF_UBQ        ubq_seg;        /* store space for segmentation          */
  SYSF_UBQ        ubq_ubf;        /* store space for segmentation          */

} NCS_UBSAR;

/***************************************************************************
 ***************************************************************************

       H J _ U B S A R    p u b l i c    I n t e r f a c e
       
         L a y e r   M a n a g e m e n t    O p e r a t i o n s

    An NCS_UBSAR and its services perform segmentation and reassembly of 
    USRBUF 'packets' . 
    
    SEND When sending, UBSAR determines if and how many sub-packets a packet
         should be, given a configured 'max packet size'. Each sub-packet
         is expected to go over a reliable transport in a point to point 
         channel.
    RCV  When sub-packets arrive, UBSAR realizes that that this packet
         is in-sequence and re-assembles arriving USRBUFs into a single
         USRBUF.

 ***************************************************************************    
 ***************************************************************************/

/******************************************************************************
 Kinds of operations defined for the ubsar layer
******************************************************************************/

typedef enum {

  NCS_UBSAR_OP_INIT,
  NCS_UBSAR_OP_DESTROY,
  NCS_UBSAR_OP_SET_OBJ,
  NCS_UBSAR_OP_SGMNT,
  NCS_UBSAR_OP_ASMBL,
  NCS_UBSAR_OP_GETAPT
  
} NCS_UBSAR_OP;


/* Arguments associated with each of the above defined op types */
/******************************************************************************
 OP NCS_UBSAR_OP_INIT 
******************************************************************************/

typedef struct ncs_ubsar_init     
{
  NCS_UBSAR*       i_ubsar;
  uns32       i_size;
  uns32       i_time_interval;
  NCS_UBSAR_LM_CB i_lm_cb_func;
  void*           i_lm_cb_arg;

} NCS_UBSAR_INIT;

/******************************************************************************
 OP NCS_UBSAR_OP_SET_OBJ
******************************************************************************/

typedef enum {                      /*  V a l u e   E x p r e s s i o n      */
                                    /*---------------------------------------*/
  NCS_UBSAR_OBJID_SEG_SIZE,          /* MAX size USRBUF allowed on transport  */
  NCS_UBSAR_OBJID_TIMEOUT           /* MAX wait for next USRBUF in sequence  */

  } NCS_UBSAR_OBJID;

/* and here is the structure that uses these OBJECT IDs */

typedef struct ncs_ubsar_set     
{
  NCS_UBSAR*       i_ubsar;
  NCS_UBSAR_OBJID  i_obj;
  uns32           i_val;

} NCS_UBSAR_SET_OBJ;

/******************************************************************************
 OP NCS_UBSAR_OP_SGMNT
******************************************************************************/

typedef struct ncs_ubsar_segment
{
  NCS_UBSAR*    i_ubsar; 
  USRBUF*         i_bigbuf;
  uns32       i_app_trlr_1;
  uns32       i_app_trlr_2;
  SYSF_UBQ*       o_segments;
  uns32           o_cnt;

} NCS_UBSAR_SEGMENT;

/******************************************************************************
 OP NCS_UBSAR_OP_ASMBL
******************************************************************************/

typedef struct ncs_ubsar_assemble   
{
  NCS_UBSAR*    i_ubsar;
  USRBUF*         i_inbuf; 
  USRBUF*         o_assembled;

} NCS_UBSAR_ASSEMBLE;

/******************************************************************************
 OP NCS_UBSAR_OP_GETAPT 
******************************************************************************/

typedef struct ncs_ubsar_trlr   
{
  USRBUF*         i_inbuf; 
  uns32           o_app_trlr_1;
  uns32           o_app_trlr_2;

} NCS_UBSAR_TRLR;

/******************************************************************************
 OP NCS_UBSAR_OP_DESTROY 
******************************************************************************/

typedef struct ncs_ubsar_destroy   
{
  NCS_UBSAR*    i_ubsar;

} NCS_UBSAR_DESTROY;


/******************************************************************************

  struct NCS_UBSAR_ARGS : Public

  Big structure containing what the application wants the ubsar 
  layer to do. Contains the op type and a union of sub-structures
  for each op type 
******************************************************************************/

typedef struct ncs_ubsar_args {

 NCS_UBSAR_OP    op;    /* Operation to perform */

 union
 {
  NCS_UBSAR_INIT      init;
  NCS_UBSAR_DESTROY    destroy;
    NCS_UBSAR_SET_OBJ    set_obj;
  NCS_UBSAR_SEGMENT    segment;
  NCS_UBSAR_ASSEMBLE   assemble;
  NCS_UBSAR_TRLR       app_trlr;

 } info;

} NCS_UBSAR_ARGS;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                      Prototypes

               Public API's provided by UBSAR
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

EXTERN_C LEAPDLL_API uns32 ncs_ubsar_request   (NCS_UBSAR_ARGS*     args);

#endif
