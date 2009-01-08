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

  This module contains definitions for the H&J Signalling API User Profile
  Structure.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef USR_PROF_H
#define USR_PROF_H

#include "ncs_ubaid.h"
#include "ncssysf_lck.h"

#if (NCSSSS_ENABLE_REDUNDANCY  == 1)

#include "ft_api.h"
#include "ft_sss.h"

#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

               Declarations & Structures for User Profile Attributes

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#define NCSCC_USR_PROF_GET_ATTR 1
#define NCSCC_USR_PROF_SET_ATTR 2
#define NCSCC_USR_PROF_DEL_ATTR 3


/*
 * These are the attribute type contained within the user profile.
 * Note that if the total exceeds 32, the definitions below for UP_ATTR_BITS
 * must be changed to account for the additional bits.
 * Note also that this value is an index into the attribute dispatch table.
 */

#define NCSCC_USR_PROF_ATTR_CLID             0
#define NCSCC_USR_PROF_ATTR_RETRY_COUNT      1
#define NCSCC_USR_PROF_ATTR_ADDR             2
#define NCSCC_USR_PROF_ATTR_CALL_DATA        3
#define NCSCC_USR_PROF_ATTR_CALLPR_SIG       4
#define NCSCC_USR_PROF_ATTR_MAX              4


#define ATTR_IN_RANGE(n) ((n)<=NCSCC_USR_PROF_ATTR_MAX)
#define ATTR_OUT_OF_RANGE(n) ((n)>NCSCC_USR_PROF_ATTR_MAX)

/* partial structure definition to keep compilers happy */
struct ncscc_usr_prof;    
   
/*
 * Lookup table for attribute access
 */
typedef struct attr_descriptor
{
  uns32 (*set)(struct ncscc_usr_prof *up, void *opaque);
  uns32 (*get)(struct ncscc_usr_prof *up, void *opaque);
  uns32 (*del)(struct ncscc_usr_prof *up, void *opaque);
} ATTR_DESCRIPTOR;




/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                         Signal Handler Template

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*
 * The prescribed format for an inbound/outbound signal handler...
 */
typedef uns32 (*USR_SIGNAL_HANDLER)( NCSCONTEXT api_context,
         NCSCONTEXT usr_context,
         uns32 type,
         struct ncscc_call_data *,
         IE_DESC *);



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                     H&J Signalling API User Profile Structure

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* H J C C _ U P _ S E T
 * Data structure that contributes to critical region guarding and logging */

typedef struct ncscc_up_set
  {
  struct qscb *qscb;                     /* the fake, 'benign' qscb        */
  NCS_LOCK      lock;                     /* User Profile Pool lock         */
  } NCSCC_UP_SET;


/* H J C C _ U S R _ P R O F I L E 
 * Data struct that maps to application type and instance .................*/

typedef struct ncscc_usr_prof
{
  struct ncscc_usr_prof *next;
  char signature[4];   /* "USRP" Identity                  */
  char *container;   /* USRPROF Container(DO NOT MODIFY) */

  USR_SIGNAL_HANDLER  in_sigh;  /* Inbound Call Signal Handler      */
  USR_SIGNAL_HANDLER  out_sigh;  /* Outgoing Call Signal Handler     */
  NCSCONTEXT           usr_context;      /* The opaque user (client) arg  
        associated with this profile     */
  NCSCC_ATM_ADDRESS    up_addr;   /* The ATM addr for this usr prof   */
  unsigned int        clir_enforce:1; /* Enforce Caller ID                */
  unsigned int        retrys      :4; /* Call Retry count - default of 1  */
  unsigned int        callpr_sig  :1; /* Want Call Proceeding signal      */
  struct ncscc_call_data *cd;  /* Pointer to attached IE info      */

  /*-------------------------------------------------------------------------*/
  /*                            REDUNDANCY                                   */
  /*-------------------------------------------------------------------------*/

#if (NCSSSS_ENABLE_REDUNDANCY  == 1)

  NCSSS_NAME        re_name;             /* Redundant Entity Name             */
  uns8             re_role;             /* Fault Tolerance role ie. PRIMARY  */
  uns32            re_trig;             /* trigger configuration             */
  uns32            re_trig_saved;       /* shadow copy of trigger config     */
  uns32            re_async;            /* notify signal mask values         */
  NCSFTCC_NOTIFY    re_ascb;             /* notify signal callback func ptr   */
#if (NCSSSS_USE_RMS_V2 == 1)
  TRIGGER_CB_V2    re_updt;             /* TrigCB for Upated (config info)   */
  TRIGGER_CB_V2    re_addc;             /* TrigCB for ADD call               */
  TRIGGER_CB_V2    re_rmvc;             /* TrigCB for RMV call               */
  TRIGGER_CB_V2    re_addp;             /* TrigCB for ADD party              */
  TRIGGER_CB_V2    re_rmvp;             /* TrigCB for RMV party              */
  TRIGGER_CB_V2    re_trcb;             /* Trigger callback function ptr     */
  NCSRE_RED         rms_red;             /* RMS name; for navigation          */
#else
  TRIGGER_CALLBACK re_updt;             /* TrigCB for Upated (config info)   */
  TRIGGER_CALLBACK re_addc;             /* TrigCB for ADD call               */
  TRIGGER_CALLBACK re_rmvc;             /* TrigCB for RMV call               */
  TRIGGER_CALLBACK re_addp;             /* TrigCB for ADD party              */
  TRIGGER_CALLBACK re_rmvp;             /* TrigCB for RMV party              */
  TRIGGER_CALLBACK re_trcb;             /* Trigger Callback function         */
  NCSFT_RED         rms_red;             /* RMS name; for navigation          */
#endif
  void*            rms_handle;          /* gsft_handle; used to send msgs    */

#endif

} NCSCC_USR_PROF;

#define NCSCC_UP_NULL (NCSCC_USR_PROF*)0

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                               ANSI Prototypes

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

uns32 dummy_sigh (NCSCONTEXT api_call_context,
    NCSCONTEXT usr_call_context,
    uns32  type,
    struct ncscc_call_data *call_data,
    IE_DESC *explicit_rsp);

EXTERN_C void           bind_to_up   (NCSCC_USR_PROF *up);
EXTERN_C void           unbind_to_up (NCSCC_USR_PROF *up);
EXTERN_C void           init_usr_profile_pool(void);
EXTERN_C void           dismantle_usr_profile_pool(void);

#if (NCSSSS_ENABLE_REDUNDANCY  == 1)

EXTERN_C NCSCC_USR_PROF *ftcc_up_find_by_name(NCSSS_NAME *up_name);
EXTERN_C void           ftcc_up_find_n_rmv_calls (NCSCC_USR_PROF *up);

#endif

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                               USRBUF pool ready 
                              Configuration APIs

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* bit map of Signalling Message types, so that priority can be mapped to it */
/* using API ncscc_set_usrbuf_priority().                                     */

typedef enum ncscc_mfv { /* Message Flag Values */

  NCSCC_CALL_PROC_SEND       = 0x00000001,
  NCSCC_CALL_SETUP_SEND      = 0x00000002,
  NCSCC_CONNECT_SEND         = 0x00000004,
  NCSCC_CONNECT_ACK_SEND     = 0x00000008,
  NCSCC_RESTART_SEND         = 0x00000010,
  NCSCC_RELEASE_SEND         = 0x00000020,
  NCSCC_RESTART_ACK_SEND     = 0x00000040,
  NCSCC_RELEASE_CPLT_SEND    = 0x00000080,
  NCSCC_CALL_STATUS_ENQ_SEND = 0x00000100,
  NCSCC_CALL_STATUS_SEND     = 0x00000200,
  NCSCC_ADD_PARTY_SEND       = 0x00000400,
  NCSCC_ADD_PARTY_ACK_SEND   = 0x00000800,
  NCSCC_ADD_PARTY_REJ_SEND   = 0x00001000,
  NCSCC_DROP_PARTY_SEND      = 0x00002000,
  NCSCC_DROP_PARTY_ACK_SEND  = 0x00004000,
  NCSCC_PARTY_ALERTING_SEND  = 0x00008000,
  NCSCC_ALERTING_SEND        = 0x00010000,
  NCSCC_NOTIFY_SEND          = 0x00020000,
  NCSCC_PASS_ALONG_SEND      = 0x00040000,
  NCSCC_LEAF_SETUP_FAIL_SEND = 0x00080000,
  NCSCC_LEAF_SETUP_SEND      = 0x00100000,
  NCSCC_PROGRESS_SEND        = 0x00200000,
  NCSCC_SETUP_ACK_SEND       = 0x00400000,
  NCSCC_INFORMATION_SEND     = 0x00800000

  } NCSCC_MFV;

#define NCSCC_MFV_MAX  21     /* as per number of enums above */
#define NCSIG_MSG_MAX 0x100  /* msg code point table for priority */

extern uns8 msg_priority[ ]; /* configure msg type to USRBUF priority */

EXTERN_C uns32 ncscc_set_usrbuf_pool_id (NCSCONTEXT native_ifcb, uns8 pool_id);
EXTERN_C void  ncscc_set_usrbuf_priority(uns32     msg_map,     uns8 priority);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                               Externs

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

extern NCSCC_USR_PROF *gl_usr_profile_anchor;

extern NCSCC_USR_PROF  gl_dummy_usr_profile;

extern NCSCC_UP_SET    gl_up_set;

#endif  /* USR_PROF_H */




