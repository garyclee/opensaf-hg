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
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_SYSM_H
#define NCS_SYSM_H

#include "ncs_svd.h"

/* SMM Temporary till include strategy is nailed */

/***************************************************************************

    S y s t e m   M o n i t o r   C o m p i l e r   F l a g

      STATS flags: Statistics Gathering

    - Each flag enables the instrumentation for a particular dimension of
      RTOS system resource statistics, complete with associated overhead.
      This flag also implies all 'Reports' functionality is available.

      WATCH flags: Resource Use Monitoring

    - Each flag enables only that part of the instrumentation that allows
      the associated subscription service to behave correctly.

      Default behaviors per dimension

      If a STATS and WATCH flags are OFF, 
        * The statistics API will return 0 for all fields.
        * Subscriptions will be taken (SUCCESS), but never satisfied.

      If a STATS is ON and WATCH flag is OFF, 
        * The statistics API will return valid values
        * Subscriptions will be taken (SUCCESS), but never satisfied

      If a STATS is OFF and WATCH flag is ON,
        * The statistics API will return 0 for all fields
        * Subscriptions will be taken (SUCCESS), and honored

      If a STATS is ON and WATCH flag is ON,
        * The statistics API will return valid values
        * Subscriptions will be taken (SUCCESS), and honored
    
 Debug flags:

            NCSSYSM_IPC_DBG_ENABLE
            NCSSYSM_MEM_DBG_ENABLE
            NCSSYSM_BUF_DBG_ENABLE
            NCSSYSM_TMR_DBG_ENABLE
            NCSSYSM_LOCK_DBG_ENABLE 

 Statistics flags:

            NCSSYSM_IPC_STATS_ENABLE
            NCSSYSM_MEM_STATS_ENABLE
            NCSSYSM_BUF_STATS_ENABLE
            NCSSYSM_TMR_STATS_ENABLE 
            NCSSYSM_LOCK_STATS_ENABLE 
            NCSSYSM_CPU_STATS_ENABLE 

 Subsystem or Target System Watch callback flags:

            NCSSYSM_IPC_WATCH_ENABLE 
            NCSSYSM_MEM_WATCH_ENABLE 
            NCSSYSM_BUF_WATCH_ENABLE 
            NCSSYSM_TMR_WATCH_ENABLE 
            NCSSYSM_CPU_WATCH_ENABLE 
            NCSSYSM_IPRA_WATCH_ENABLE

 Exclusively Target System Watch callback flags:

            NCSSYSM_QLT_WATCH_ENABLE    (IPC Queue Latency Time)
*/

/***************************************************************************

    S y s t e m  M o n i t o r

    There are basically two clients of System Monitoring Services:

    1) The Target System (AKE) that knows all and sees all. This client will 
    configure the system monitor and perhaps track resource levels in a
    general sense.

    2) Individual Subsystems will track particular resource levels and make
    local decisions based on these levels.

    There are two System Monitoring API entry points:
    
    ncssysm_lm() This is Layer Management services that provides resource
                configuration, statistics and report services. The expected
                client is the Target System (aka AKE)

    ncssysm_ss() This is Subsystem subscription services that provide a 
                means to 'watch' specific levels of specific resources 
                (Memory, queue depth, etc.). The expected client of these 
                services is Subsystems as well as the Target System.

    The expected paradigm is that a function with the same prototype as
    ncssysm_ss() is given to a subsystem. The subsystem then subscribes to
    the WATCH events it is interested in.

    As per below:  
    
                                                               +--------------+   
           +--seed--ncssysm_ss()--+--in-each-subsys---+---------+-             |
           |                     |                   |         |    A K E     |      
    +------V-------+   +---------V----+    +---------V----+    |              |
    | subsys A     |   | subsys B     |    | subsys C     |    |              |
    |              |   |              |    |              |    |              |
    | ncssysm_ss(|) |   | ncssysm_ss(|) |    | ncssysm_ss(|) |    | ncssysm_lm(|) |
    +-----------|--+   +-----------|--+    +-----------|--+    +-----------|--+  
    +-----------V------------------V-------------------V-------------------|--+
    |                                                                      v  |
    |                     LEAP / OS  Svcs      S y s   M o n                  |
    |                                                                         |
    +-------------------------------------------------------------------------+
    +-------------------------------------------------------------------------+
    |                           LEAP / OS Primitives                          |
    +-------------------------------------------------------------------------+

 Implementation considerations (KYLE !!)
 =======================================

  LEAP 'lite' is the shortest code path between OS Svcs and OS Prims. It 
     involves the least amount of overhead.
  LEAP 'Inflator' is all of the instrumentation called out here or that
     already exists in our OS Services Layer.
     - This inflator code lives files called sysm_xxx. partitioned by LEAP
     OS Category, such as:
          sysm_mem.c/h, 
          sysm_buf.c/h,
          sysm_tmr.c/h
          sysm_ipc.c/h
          etc.
      + The *.h files contain/hide all instrumentation structures needed to 
        accomodate the required functionality.
      + The *.c files contain all the instrumentation functions needed to
        carry out the functionality.
      + the 'bridge' between sysm_ files and sysf_files are internal macros 
        that get populated with correct sysm_ functions if a compile flag is
        ON, or not populated if a compile flag is OFF.
     - These files live in sub-directories 'common/sysm' (TBD.. Don't do yet).
     - The noted compile flags will determine what capabilities are actually
         alive in any given compile, though stubbed versions of the APIs 
         are available and return SUCCESS even if functionally disabled. This
         allows initialization threads and scripts to work the same regardless.

 ***************************************************************************/

/*****************************************************************************
  
    S y s t e m  M o n i t o r  :  s u b s y s t e m    
                                   S u b s c r i p t i o n
                                   S e r v i c e s

    What follows are the definitions used in the ncssysm_ss call. The services
    provided are:

    NCSSYSM_SS_OP_MEM  - mem use % per virtual vouter 
    NCSSYSM_SS_OP_BUF  - buf use % per virtual router
    NCSSYSM_SS_OP_TMR  - tmr use % per virtual router  
    NCSSYSM_SS_OP_IPC  - queue depth % for a given mbx

*****************************************************************************/

/*****************************************************************************
  
          C o n t r i b u t i n g   D a t a  S t r u c t u r e s
    
*****************************************************************************/

/***************************************************************************
 * When a threshold is crossed, is it going UP or DOWN?
 ***************************************************************************/

typedef enum ncssysm_dir {	/* homeostatic direction for this EVENT          */
	    NCSSYSM_DIR_UP = 1,	/* threshold crossed while going UP              */
	NCSSYSM_DIR_DOWN	/* threshold crossed while going DOWN            */
} NCSSYSM_DIR;

/***************************************************************************
 * Callback MEM data to subsystem when a subscription threshold is crossed
 * And MEM Subscribe structure subsystem uses at subscription time.
 ***************************************************************************/

typedef struct ncssysm_mem_evt {
	NCS_KEY i_usr_key;	/* return user key provided at subscription time */
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber           */
	NCSSYSM_DIR i_dir;	/* direction of threshold crossing               */
	uns32 i_subscr_pcnt;	/* subscription triggered on this percentile   */
	uns32 i_cur_pcnt;	/* current percentile value                      */
	NCSCONTEXT i_opaque;	/* client data given at register time            */

} NCSSYSM_MEM_EVT;

typedef uns32 (*NCSMEM_EVT) (NCSSYSM_MEM_EVT * info);	/* callback prototype */

typedef struct ncssysm_mem_reg {
	uns32 i_percent;	/* at what percentile does SYSMON call back?     */
	NCSCONTEXT i_opaque;	/* client data that callback will give back      */
	NCSMEM_EVT i_cb_func;	/* function pointer for callback                 */
	NCSCONTEXT o_hdl;	/* SYSMON handle for unreg operation             */

} NCSSYSM_MEM_REG;

typedef struct ncssysm_mem_unreg {
	NCSCONTEXT i_hdl;

} NCSSYSM_MEM_UNREG;

/***************************************************************************
 * Callback CPU data to subsystem when a subscription threshold is crossed
 * And CPU Subscribe structure subsystem uses at subscription time.
 ***************************************************************************/

typedef NCSSYSM_MEM_EVT NCSSYSM_CPU_EVT;

typedef uns32 (*NCSCPU_EVT) (NCSSYSM_CPU_EVT * info);	/* callback prototype */

typedef NCSSYSM_MEM_REG NCSSYSM_CPU_REG;

typedef NCSSYSM_MEM_UNREG NCSSYSM_CPU_UNREG;

/***************************************************************************
 * Callback IPRA data to subsystem when a subscription threshold is crossed
 * And IPRA Subscribe structure subsystem uses at subscription time.
 ***************************************************************************/

typedef struct ncssysm_ipra_evt {
	NCS_KEY i_usr_key;	/* return user key provided at subscription time  */
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber            */
	NCSSYSM_DIR i_dir;	/* direction:up - IP Resource is not available
				   down - IP Resource is available again */
	NCSCONTEXT i_opaque;	/* client data given at register time             */

} NCSSYSM_IPRA_EVT;

typedef uns32 (*NCSIPRA_EVT) (NCSSYSM_IPRA_EVT * info);	/* callback prototype */

typedef struct ncssysm_ipra_reg {
	NCSCONTEXT i_opaque;	/* client data that callback will give back      */
	NCSIPRA_EVT i_cb_func;	/* function pointer for callback                 */
	NCSCONTEXT o_hdl;	/* SYSMON handle for unreg operation             */

} NCSSYSM_IPRA_REG;

typedef struct ncssysm_ipra_unreg {
	NCSCONTEXT i_hdl;

} NCSSYSM_IPRA_UNREG;

/***************************************************************************
 * Callback BUF data to subsystem when a subscription threshold is crossed
 * And BUF Subscribe structure subsystem uses at subscription time.
 ***************************************************************************/

typedef struct ncssysm_buf_evt {
	NCS_KEY i_usr_key;	/* return user key provided at subscription time */
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber           */
	NCSSYSM_DIR i_dir;	/* direction of threshold crossing               */
	uns32 i_subscr_pcnt;	/* subscription triggered on this percentile   */
	uns32 i_cur_pcnt;	/* current percentile value                      */
	NCSCONTEXT i_opaque;	/* client data given at register time            */
	uns32 i_pool_id;	/* pool id                                       */

} NCSSYSM_BUF_EVT;

typedef uns32 (*NCSBUF_EVT) (NCSSYSM_BUF_EVT * info);	/* callback prototype */

typedef struct ncssysm_buf_reg {
	uns32 i_percent;	/* at what percentile does SYSMON call back?     */
	uns32 i_pool_id;	/* pool id                                       */
	NCSCONTEXT i_opaque;	/* client data that callback will give back      */
	NCSBUF_EVT i_cb_func;	/* function pointer for callback                 */
	NCSCONTEXT o_hdl;	/* SYSMON handle for unreg operation             */

} NCSSYSM_BUF_REG;

typedef struct ncssysm_buf_unreg {
	NCSCONTEXT i_hdl;

} NCSSYSM_BUF_UNREG;

/***************************************************************************
 * Callback TMR data to subsystem when a subscription threshold is crossed
 * And BUF Subscribe structure subsystem uses at subscription time.
 ***************************************************************************/

typedef struct ncssysm_tmr_evt {
	NCS_KEY i_usr_key;	/* return user key provided at subscription time */
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber           */
	NCSSYSM_DIR i_dir;	/* direction of threshold crossing               */
	uns32 i_subscr_pcnt;	/* subscription triggered on this percentile   */
	uns32 i_cur_pcnt;	/* current percentile value                      */
	NCSCONTEXT i_opaque;	/* client data given at register time            */

} NCSSYSM_TMR_EVT;

typedef uns32 (*NCSTMR_EVT) (NCSSYSM_TMR_EVT * info);	/* callback prototype  */

typedef struct ncssysm_tmr_reg {
	uns32 i_percent;	/* at what percentile does SYSMON call back?     */
	NCSCONTEXT i_opaque;	/* client data that callback will give back      */
	NCSTMR_EVT i_cb_func;	/* function pointer for callback                 */
	NCSCONTEXT o_hdl;	/* SYSMON handle for unreg operation             */

} NCSSYSM_TMR_REG;

typedef struct ncssysm_tmr_unreg {
	NCSCONTEXT i_hdl;

} NCSSYSM_TMR_UNREG;

/***************************************************************************
 * Callback IPC data to subsystem when a subscription threshold is crossed
 * And IPC Subscribe structure subsystem uses at subscription time.
 ***************************************************************************/

typedef struct ncssysm_ipc_evt {
	NCS_KEY i_usr_key;	/* return user key provided at subscription time */
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber           */
	NCSSYSM_DIR i_dir;	/* direction of threshold crossing               */
	uns32 i_subscr_pcnt;	/* subscription triggered on this percentile   */
	uns32 i_cur_pcnt;	/* current percentile value                      */
	NCSCONTEXT i_opaque;	/* client data given at register time            */

} NCSSYSM_IPC_EVT;

typedef uns32 (*NCSIPC_EVT) (NCSSYSM_IPC_EVT * info);	/* callback prototype  */

typedef struct ncssysm_ipc_reg {
	uns8 *i_name;		/* [KEY VALUE] string version of task name       */
	uns32 i_percent;	/* at what percentile does SYSMON call back?     */
	NCSCONTEXT i_opaque;	/* client data that callback will give back      */
	NCSIPC_EVT i_cb_func;	/* function pointer for callback                 */
	NCSCONTEXT o_hdl;	/* SYSMON handle for unreg operation             */

} NCSSYSM_IPC_REG;

typedef struct ncssysm_ipc_unreg {
	NCSCONTEXT i_hdl;

} NCSSYSM_IPC_UNREG;

/***************************************************************************

    S u b s y s t e m  _S_ u b s c r i p t i o n  _S_ e r v i c e    A P I

 ***************************************************************************/

/***************************************************************************
 Subscriptions for the following Operations
 ***************************************************************************/

typedef enum ncssysm_ss_op {

	NCSSYSM_SS_OP_MEM_REG = 1,	/* reg for mem use % per virtual vouter     */
	NCSSYSM_SS_OP_BUF_REG,	/* reg for buf use % per virtual router     */
	NCSSYSM_SS_OP_TMR_REG,	/* reg for tmr use % per virtual router     */
	NCSSYSM_SS_OP_IPC_REG,	/* reg for ipc q % per task/work-q          */
	NCSSYSM_SS_OP_CPU_REG,	/* reg for cpu use % per virtual vouter     */
	NCSSYSM_SS_OP_IPRA_REG,	/* reg for IP Resource Availability         */

	NCSSYSM_SS_OP_MEM_UNREG,	/* unreg mem watch per virtual vouter       */
	NCSSYSM_SS_OP_BUF_UNREG,	/* unreg buf watch per virtual router       */
	NCSSYSM_SS_OP_TMR_UNREG,	/* unreg tmr watch per virtual router       */
	NCSSYSM_SS_OP_IPC_UNREG,	/* unreg ipc watch per task/work-q          */
	NCSSYSM_SS_OP_CPU_UNREG,	/* unreg cpu watch per virtual vouter       */
	NCSSYSM_SS_OP_IPRA_UNREG	/* unreg IP Resource Availability watch     */
} NCSSYSM_SS_OP;

/***************************************************************************
 Subscription Service argument structure
 ***************************************************************************/

typedef struct ncssysm_ss_arg {
	NCSSYSM_SS_OP i_op;	/* subsystem operation                 */
	NCS_VRID i_vrtr_id;	/* Virtual Rtr ID of this subsystem    */
	NCS_KEY *i_ss_key;	/* Key of subsys (copied for callback) */

	union {
		union {
			NCSSYSM_MEM_REG i_memory;	/* percent memory watch subscription   */
			NCSSYSM_BUF_REG i_buffer;	/* percent buffer watch subscription   */
			NCSSYSM_TMR_REG i_timer;	/* raw cnt timer watch subscription    */
			NCSSYSM_IPC_REG i_queue;	/* ttl q-depth ipc watch subscription  */
			NCSSYSM_CPU_REG i_cpu;	/* percent cpu watch subscription      */
			NCSSYSM_IPRA_REG i_ipra;	/* ip resource avalability watch subscr */
		} reg;
		union {
			NCSSYSM_MEM_UNREG i_memory;	/* percent memory watch deregistration  */
			NCSSYSM_BUF_UNREG i_buffer;	/* percent buffer watch deregistration  */
			NCSSYSM_TMR_UNREG i_timer;	/* raw cnt timer watch deregistration   */
			NCSSYSM_IPC_UNREG i_queue;	/* ttl q-depth ipc watch deregistration */
			NCSSYSM_CPU_UNREG i_cpu;	/* percent cpu watch deregistration     */
			NCSSYSM_IPRA_UNREG i_ipra;	/* ip resource avalability watch dereg. */
		} unreg;
	} op;

} NCSSYSM_SS_ARG;

/***************************************************************************
 Pointer of this function prototype passed to potential clients
 ***************************************************************************/

typedef uns32 (*NCSSYSM_SS) (NCSSYSM_SS_ARG * info);

/***************************************************************************
 System Monitoring function that matches prototype
 ***************************************************************************/

EXTERN_C LEAPDLL_API uns32 ncssysm_ss(NCSSYSM_SS_ARG * info);

/*****************************************************************************
  
    S y s t e m  M o n i t o r  :  A K E
                                   L a y e r 
                                   M a n a g e m e n t

    What follows are the definitions used in the ncssysm_lm call. The services
    provided are:

    NCSSYSM_LM_OP_INIT          initialize LM Sys Mon
    NCSSYSM_LM_OP_TERMINATE     terminate LM Sys Mon; recover resources

    NCSSYSM_LM_OP_MEM_STATS     fetch raw mem statistics per virtual router
    NCSSYSM_LM_OP_CPU_STATS     fetch raw cpu statistics per virtual router
    NCSSYSM_LM_OP_BUF_STATS     fetch buffer statistics per virtual router
    NCSSYSM_LM_OP_TMR_STATS     fetch timer statistics per virtual router
    NCSSYSM_LM_OP_TASK_STATS    fetch task statistics per local system
    NCSSYSM_LM_OP_IPC_STATS     fetch queue info per task
    NCSSYSM_LM_OP_LOCK_STATS    fetch lock statistics per local system

*****************************************************************************/

/*****************************************************************************
  
          C o n t r i b u t i n g   D a t a  S t r u c t u r e s
    
*****************************************************************************/

/***************************************************************************
 * SysMon Stat or Report OutPut Policy (_OPP) ; Direct answer to......
 * NOTE: No matter which policies are enabled, the data is also provided
 * to the invoker as well.
 ***************************************************************************/

#define  OPP_TO_STDOUT  0x01	/* send result to stdout                */
#define  OPP_TO_MASTER  0x02	/* distributed; send to master via MDS  */
#define  OPP_TO_FILE    0x04	/* send result to file; file name given */

/***************************************************************************
 * Many ncssysm_lm operations allow you to specify output policies
 ***************************************************************************/

typedef struct ncssysm_opp {
	uns32 opp_bits;		/* bitmap of all output forms for operation  */
	char *fname;		/* required if OPP_TO_FILE opp_bits flag set */

} NCSSYSM_OPP;

/***************************************************************************
 * System Monitoring Configuration attributes 
 ***************************************************************************/

/* SYSM Configuration Object Identifiers whose values to get or set..      */
/* ======================================================================= */
/* RW : Read or Write; can do get or set operations on this object         */
/* R- : Read only; can do get operations on this object                    */
/* -W : Write only; can do set operations on this object                   */

typedef enum ncssysm_oid {	/*     V a l u e   E x p r e s s i o n      */
			       /*------------------------------------------*/

	/* Set MAX threshold for these Per virtual router Resources              */

	NCSSYSM_OID_MEM_TTL,	/* RW MAX heap space allowed alloc'ed       */
	NCSSYSM_OID_TMR_TTL,	/* RW MAX num tmrs allowed to run at once   */
	NCSSYSM_OID_IPC_TTL,	/* RW MAX depth any given IPC queue can go  */

	/* TRUE (means) Hide cur stats [a logical RESET], FALSE expose real stats */

	NCSSYSM_OID_MEM_CM_IGNORE,	/* RW hide/expose memory currently out      */
	NCSSYSM_OID_TMR_IGNORE,	/* RW MAX num tmrs outstanding at once      */
	NCSSYSM_OID_IPC_IGNORE,	/* RW hide/expose memory currently out      */

	/*                                                                       */

	NCSSYSM_OID_MEM_PULSE,	/* RW Increment MEM OUT every X seconds     */

	/* Set Watch Refresh Rates                                               */

	NCSSYSM_OID_MEM_WRR,	/* RW Watch Refresh Rate for Memory watches */
	NCSSYSM_OID_CPU_WRR,	/* RW Watch Refresh Rate for CPU watches */
	NCSSYSM_OID_BUF_WRR,	/* RW Watch Refresh Rate for USRBUF watches */
	NCSSYSM_OID_TMR_WRR,	/* RW Watch Refresh Rate for Timer watches  */
	NCSSYSM_OID_IPC_WRR,	/* RW Watch Refresh Rate for IPC watches    */

} NCSSYSM_OID;

/* Object Identifiers for Instance Get/Set Operations                      */

typedef enum ncssysm_inst_oid {	/*     V a l u e   E x p r e s s i o n      */
			       /*------------------------------------------*/

	/* Set MAX threshold for these Per virtual router Resources              */

	NCSSYSM_OID_BUF_TTL,	/* RW MAX USRBUF space allowed alloc'ed     */

	/* TRUE (means) Hide cur stats [a logical RESET], FALSE expose real stats */

	/* The i_inst field identifies the buffer pool id                        */
	NCSSYSM_OID_BUF_CB_IGNORE,	/* RW hide/expose buffers currently out     */

	/* The i_inst field identifies the subsystem id                          */
	NCSSYSM_OID_MEM_SS_IGNORE,	/* RW hide/expose current memory by subsys. */

	/*                                                                       */

	NCSSYSM_OID_BUF_PULSE,	/* RW Increment BUF OUT every X seconds     */

} NCSSYSM_INST_OID;

/***************************************************************************
 * Configuration operations
 ***************************************************************************/

typedef struct ncssysm_get {
	uns32 i_obj_id;		/* Object ID of thing to get                */
	uns32 o_obj_val;	/* The returned value; always fits in uns32 */

} NCSSYSM_GET;

typedef struct ncssysm_set {
	uns32 i_obj_id;		/* Object ID of thing to set                */
	uns32 i_obj_val;	/* the value to set for this Obect ID       */

} NCSSYSM_SET;

typedef struct ncssysm_iget {
	uns32 i_obj_id;		/* Object ID of thing to get                */
	uns32 i_inst_id;	/* Instance ID of thing to get              */
	uns32 o_obj_val;	/* The returned value; always fits in uns32 */

} NCSSYSM_IGET;

typedef struct ncssysm_iset {
	uns32 i_obj_id;		/* Object ID of thing to set                */
	uns32 i_inst_id;	/* Instance ID of thing to set              */
	uns32 i_obj_val;	/* the value to set for this Obect ID       */

} NCSSYSM_ISET;

/***************************************************************************
 * Callback Resource Limit data to subsystem when a total resource limit is reached
 ***************************************************************************/

typedef uns32 LEAM_ATTR_MAP;	/* Limit Event attribute bitmap */

/* Limit Event Attribute Map (NAM) values */
#define LEAM_INST_ID    0x01	/* Instance ID is valid */

typedef struct ncssysm_res_lmt_evt {
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber           */
	uns32 i_obj_id;		/* Obj ID of the resource that reached its limit */
	uns32 i_ttl;		/* Total configured resource limit               */
	LEAM_ATTR_MAP i_attr;	/* Attribute mask for optional fields            */
	uns32 i_inst_id;	/* Inst ID of the resource that reached its limit */

} NCSSYSM_RES_LMT_EVT;

typedef uns32 (*RES_LMT_CB) (NCSSYSM_RES_LMT_EVT * info);	/* callback prototype */

typedef struct ncssysm_reg_lmt_cb {
	RES_LMT_CB i_lmt_cb_fnc;

} NCSSYSM_REG_LMT_CB;

/***************************************************************************
 * Raw Memory Statistics per Virtual Router
 ***************************************************************************/

typedef struct ncssysm_mem_stats {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results        */
	uns32 o_ttl;		/* configured value of how much mem in pool */
	uns32 o_hwm;		/* high water mark for mem consumption      */
	uns32 o_new;		/* ttl alloced so far divided by 1000       */
	uns32 o_free;		/* ttl freed so far divided by 1000         */
	uns32 o_curr;		/* ttl mem alloced to clients right now     */

} NCSSYSM_MEM_STATS;

/***************************************************************************
 * Raw CPU Statistics per Virtual Router
 ***************************************************************************/

typedef struct ncssysm_cpu_stats {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results        */
	uns32 o_curr;		/* current CPU utilization                  */

} NCSSYSM_CPU_STATS;

/***************************************************************************
 * Raw Usrbuf Statistics per Virtual Router
 ***************************************************************************/

typedef struct ncssysm_buf_stats {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results          */
	uns32 i_pool_id;	/* [KEY VALUE] Buffer pool in question        */
	uns32 o_ttl;		/* configured val of how much bufmem in pool  */
	uns32 o_hwm;		/* high water mark for buf consumption        */
	uns32 o_new;		/* ttl alloced so far divided by 1000         */
	uns32 o_free;		/* ttl freed so far divided by 1000           */
	uns32 o_curr;		/* ttl mem alloced to clients right now       */
	uns32 o_cnt;		/* ttl raw count of bufs out right now        */

	/* Count of Buffers chained to make a message at free time                 */

	uns32 o_fc_1;		/* 1 Buffer in chain at free time             */
	uns32 o_fc_2;		/* 2 Buffer in chain at free time             */
	uns32 o_fc_3;		/* 3 Buffer in chain at free time             */
	uns32 o_fc_4;		/* 4 Buffer in chain at free time             */
	uns32 o_fc_gt;		/* 5 or more Buffers in chain at free time    */

	/* Count of bytes in payload area at free time (actual message size)       */

	uns32 o_32b;		/* less than 32 byte payload at free time     */
	uns32 o_64b;		/* less than 64 byte payload at free time     */
	uns32 o_128b;		/* less than 128 byte payload at free time    */
	uns32 o_256b;		/* less than 256 byte payload at free time    */
	uns32 o_512b;		/* less than 512 byte payload at free time    */
	uns32 o_1024b;		/* less than 1024 byte payload at free time   */
	uns32 o_2048b;		/* less than 2048 byte payload at free time   */
	uns32 o_big_b;		/* greater than 2048 byte payload at free time */

} NCSSYSM_BUF_STATS;

/***************************************************************************
 * Timer Statistics per Virtual Router
 ***************************************************************************/

typedef struct ncssysm_tmr_stats {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results        */
	uns32 o_ttl;		/* configured value of max timers allowed   */
	uns32 o_hwm;		/* high water mark for mem consumption      */
	uns32 o_started;	/* ttl timers started so far                */
	uns32 o_stopped;	/* ttl timers stopped so far                */
	uns32 o_expired;	/* ttl timers expired (callbacks to apps)   */
	uns32 o_curr;		/* ttl timers running right now             */

} NCSSYSM_TMR_STATS;

/***************************************************************************
 * Task Statistics per Local Process Space
 ***************************************************************************/

typedef struct ncssysm_task_stats {	/* SMM */
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results        */
	uns32 o_ttl;		/* configured val of max msgs allowed on q  */
	uns32 o_hwm;		/* greatest depth of work queue so far      */
	uns32 o_on_q;		/* ttl number msgs enqueued so far          */
	uns32 o_off_q;		/* ttl number msgs dequeued so far          */
	uns32 o_curr;		/* ttl number msg currently on queue        */

} NCSSYSM_TASK_STATS;

/***************************************************************************
 * IPC (work queue) Statistics per task instance
 ***************************************************************************/

typedef struct ncssysm_ipc_stats {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results        */
	char *i_name;		/* [KEY VALUE] string version of task name  */
	uns32 o_ttl;		/* configured val of max msgs allowed on q  */
	uns32 o_hwm;		/* greatest depth of work queue so far      */
	uns32 o_on_q;		/* ttl number msgs enqueued so far          */
	uns32 o_off_q;		/* ttl number msgs dequeued so far          */
	uns32 o_curr;		/* ttl number msg currently on queue        */

} NCSSYSM_IPC_STATS;

/***************************************************************************
 * Lock Statistics per locak Process Space
 ***************************************************************************/

typedef struct ncssysm_lock_stats {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results        */
	uns32 o_ttl;		/* configured val of max msgs allowed on q  */
	uns32 o_hwm;		/* greatest num locks existing at once time */
	uns32 o_new;		/* ttl number of locks created              */
	uns32 o_free;		/* ttl number of locks destroyed            */
	uns32 o_curr;		/* ttl number of locks right now            */
	uns32 o_blocked;	/* ttl number of wait-threads on locks now  */
	uns32 o_inside;		/* ttl number of threads in crit-region now */

} NCSSYSM_LOCK_STATS;

/***************************************************************************
 * Callback QLT data to Targ Svc/AKE when the marked msg is actually 
 * serviced at the front of identified work queue.
 ***************************************************************************/

typedef struct ncssysm_qlt_evt {
	NCS_KEY *i_usr_key;	/* return user key provided at subscription time */
	NCS_VRID i_vrtr_id;	/* virtual router ID of the subscriber           */
	char *i_name;		/* name of queue owner that we are to watch      */
	NCSCONTEXT i_opaque;	/* client data given at registration time        */
	uns32 i_depth;		/* Depth of queue when watch was set             */
	char *i_start;		/* Time stamp fo when watch started (was placed) */
	char *i_done;		/* Time stamp of when marked msg made it to front */
	char *i_latency;	/* Actual time on queue wainig                   */

} NCSSYSM_QLT_EVT;

typedef uns32 (*NCSQLT_EVT) (NCSSYSM_QLT_EVT * info);

typedef struct ncssysm_qlt_reg {
	NCS_KEY i_usr_key;	/* user key that SysMon will return at callback. */
	NCSCONTEXT i_opaque;	/* clientdata that callback will give back       */
	NCSQLT_EVT i_cb_func;	/* functio pointer for callback                  */

} NCSSYSM_QLT_REG;

/***************************************************************************
 * Reports per Virtual Router
 ***************************************************************************/

#define NCSSYSM_MEM_BKT_CNT 10

/* Total heap usage profile */

typedef struct ncssysm_mem_thup {
	uns32 o_bkt_size;	/* memory block size,this bucket                     */
	uns32 o_inuse;		/* memory blocks outstanding                         */
	uns32 o_avail;		/* memory blocks available                           */
	uns32 o_hwm;		/* high water mark, in 'blocks'                      */
	uns32 o_allocs;		/* total allocates so far                            */
	uns32 o_frees;		/* total frees so far                                */
	uns32 o_errors;		/* errors                                            */
	uns32 o_null;		/* returned NULL                                     */

} NCSSYSM_MEM_THUP;

/* Total heap usage report                                                 */

typedef struct ncssysm_mem_rpt_thup {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results */
	NCSSYSM_MEM_THUP o_bkt[NCSSYSM_MEM_BKT_CNT];	/* space for report          */

} NCSSYSM_MEM_RPT_THUP;

/* Subsystem usage profile                                                 */

typedef struct ncssysm_mem_ssup {
	uns32 o_bkt_size;	/* memory block size,this bucket                     */
	uns32 o_inuse_uni;	/* mem blocks in-use unignored                       */
	uns32 o_inuse_ign;	/* memory blocks that are ignored                    */
	uns32 o_inuse_ttl;	/* total of in-use space for this ss                 */

} NCSSYSM_MEM_SSUP;

/* Total Subsystem usage report                                            */

typedef struct ncssysm_mem_rpt_ssup {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results */
	uns32 i_ss_id;		/* Subsystem ID to check             */
	NCS_BOOL i_ignored;	/* report blocks marked 'ignored' ?  */
	NCSSYSM_MEM_SSUP o_ssu[NCSSYSM_MEM_BKT_CNT];	/* space for report          */

} NCSSYSM_MEM_RPT_SSUP;

#define NCSSYSM_MEM_MAX_WO 1024

/* Whats out memory profile                                                */

typedef struct ncssysm_mem_wo {
	uns32 o_aline;		/* line where mem alloced                            */
	uns8 *o_afile;		/* file where mem alloced                            */
	uns32 o_oline;		/* line where owner marked                           */
	uns8 *o_ofile;		/* file where owner marked                           */
	uns32 o_svcid;		/* svc_id of original alloc                          */
	uns32 o_subid;		/* struct_id of original alloc                       */
	uns32 o_age;		/* age                                               */
	NCSCONTEXT o_ptr;	/* pointer value of data area                        */
	uns32 o_bkt_sz;		/* block size at this bucket                         */
	uns32 o_act_sz;		/* actual mem size asked for                         */

} NCSSYSM_MEM_WO;

/* Whats out memory report                                                 */

typedef struct ncssysm_mem_rpt_wo {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results  */
	uns32 i_age;		/* at least these many ticks old      */
	uns32 i_ss_id;		/* 0 means (wild card) all subsystems */
	uns32 o_filled;		/* records filled                */
	NCSSYSM_MEM_WO o_wo[NCSSYSM_MEM_MAX_WO];	/* whats out records             */

} NCSSYSM_MEM_RPT_WO;

#define NCSSYSM_MEM_MAX_WOS 1024

/* Whats out memory summary profile                                        */

typedef struct ncssysm_mem_wos {
	uns32 o_inst;		/* number of allocs of age/file/line                    */
	uns32 o_age;		/* instances are this old                               */
	uns32 o_aline;		/* line where mem alloced                               */
	uns8 *o_afile;		/* file where mem alloced                               */
	uns32 o_svcid;		/* svc_id of original alloc                             */
	uns32 o_subid;		/* struct_id of original alloc                          */
	uns32 o_bkt_sz;		/* block size at this bucket                            */
	uns32 o_act_sz;		/* actual mem size asked for                            */

} NCSSYSM_MEM_WOS;

/* Whats out memory summary report                                         */

typedef struct ncssysm_mem_rpt_wos {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results  */
	uns32 i_age;		/* at least these many ticks old      */
	uns32 i_ss_id;		/* 0 means (wild card) all subsystems */
	uns32 o_filled;		/* records filled             */
	NCSSYSM_MEM_WOS o_wos[NCSSYSM_MEM_MAX_WOS];	/* whats out summary records  */

} NCSSYSM_MEM_RPT_WOS;

#define NCSSYSM_BUF_MAX_WO  1024
#define NCSSYSM_BUF_MAX_WOS 1024

/* Whats out buffer profile                                                */

typedef struct ncssysm_buf_wo {
	uns32 o_aline;		/* line where buf alloced                      */
	uns8 *o_afile;		/* file where buf alloced                      */
	uns32 o_oline;		/* line where owner marked                     */
	uns8 *o_ofile;		/* file where owner marked                     */
	uns32 o_svcid;		/* svc_id of original alloc                    */
	uns32 o_subid;		/* struct_id of original alloc                 */
	uns32 o_refcnt;		/* current reference count val                 */
	uns32 o_age;		/* current age                                 */
	NCSCONTEXT o_ptr;	/* pointer value of payload area               */

} NCSSYSM_BUF_WO;

/* Whats out buffer report                                                 */

typedef struct ncssysm_buf_rpt_wo {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results           */
	uns32 i_pool_id;	/* pool id of whats out report                 */
	uns32 i_age;		/* at least these many ticks old               */
	uns32 i_ss_id;		/* 0 means (wild card) all subsystems          */
	uns32 o_filled;		/* records filled                              */
	NCSSYSM_BUF_WO o_wo[NCSSYSM_BUF_MAX_WO];	/* whats out records             */

} NCSSYSM_BUF_RPT_WO;

/* Whats out buffer summary profile                                        */

typedef struct ncssysm_buf_wos {
	uns32 o_inst;		/* number of allocs if age/file/line           */
	uns32 o_age;		/* instances are this old or more              */
	uns8 *o_afile;		/* file where alloced                          */
	uns32 o_aline;		/* line where alloced                          */
	uns32 o_svcid;		/* svc_id of original alloc                    */
	uns32 o_subid;		/* struct_id of original alloc                 */

} NCSSYSM_BUF_WOS;

/* Whats out buffer summary report                                         */

typedef struct ncssysm_buf_rpt_wos {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results           */
	uns32 i_pool_id;	/* pool id of whats out summary report         */
	uns32 i_age;		/* at least these many ticks old               */
	uns32 i_ss_id;		/* 0 means (wild card) all subsystems          */
	uns32 o_filled;		/* records filled                              */
	NCSSYSM_BUF_WOS o_wos[NCSSYSM_BUF_MAX_WOS];	/* whats out sum.records        */

} NCSSYSM_BUF_RPT_WOS;

/***************************************************************************
 * Memory leak tracking
 * - Turn on stack tracking for a id'ed file/line. This can be done N times.
 * - Turn off stack tracking for all file/line watches currently on.
 * - Report on all file/line stack traces that are >= N pulses old
 ***************************************************************************/

typedef struct ncssysm_mem_stk_add {
	char *i_file;		/* file name of where stack trace applies        */
	uns32 i_line;		/* line number of where stack trace applies      */

} NCSSYSM_MEM_STK_ADD;

typedef struct ncssysm_mem_stk_flush {
	uns32 i_meaningless;	/* no args to turn off all stacktraces */

} NCSSYSM_MEM_STK_FLUSH;

typedef struct ncssysm_mem_stk_rpt {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results             */
	char *i_file;		/* file name of stack watch                      */
	uns32 i_line;		/* line number of stack watch                    */
	uns32 i_ticks_min;	/* at least these # of ticks have passed (age)   */
	uns32 i_ticks_max;	/* no more than this # of ticks have passed (age) */
	uns32 io_rsltsize;	/* byte count of 'results' buffer                */
	uns8 *io_results;	/* put answer here; memory of i_rsltsize big     */
	uns32 io_marker;	/* placement holder between 'get next' calls     */
	NCS_BOOL o_more;	/* TRUE indicates additional results available   */
	uns32 o_records;	/* the count of the records returned             */

} NCSSYSM_MEM_STK_RPT;

/***************************************************************************
 * Buffer leak tracking
 * - Turn on stack tracking for a id'ed pool/file/line. Can be done N times.
 * - Turn off stack tracking for all pool/file/line watches currently on.
 * - Report on all pool/file/line stack traces that are >= N pulses old
 ***************************************************************************/

typedef struct ncssysm_buf_stk_add {
	uns32 i_pool;		/* pool identifier                                 */
	char *i_file;		/* file name of where stack trace applies          */
	uns32 i_line;		/* line number of where stack trace applies        */

} NCSSYSM_BUF_STK_ADD;

typedef struct ncssysm_buf_stk_flush {
	uns32 i_pool;		/* pool id to turn off all stacktraces             */

} NCSSYSM_BUF_STK_FLUSH;

typedef struct ncssysm_buf_stk_rpt {
	NCSSYSM_OPP i_opp;	/* Output policy to apply to results             */
	uns32 i_pool;		/* pool identifier                               */
	char *i_file;		/* file name of stack watch                      */
	uns32 i_line;		/* line number of stack watch                    */
	uns32 i_ticks_min;	/* at least these # of ticks have passed (age)   */
	uns32 i_ticks_max;	/* no more than this # of ticks have passed (age) */
	uns32 i_rsltsize;	/* byte count of 'results' buffer                */
	uns8 *io_results;	/* put answer here; memory of i_rsltsize big     */
	uns32 io_marker;	/* placement holder between 'get next' calls     */
	NCS_BOOL o_more;	/* TRUE indicates additional results available   */

} NCSSYSM_BUF_STK_RPT;

/***************************************************************************
 * IPC Queue Reports
 ***************************************************************************/

typedef void (*NCSIPC_LTCY_EVT) (void);	/* callback prototype                 */

typedef struct ncssysm_ipc_rpt_lbgn {
	char *i_name;		/* Queue name                                */
	NCSIPC_LTCY_EVT i_cbfunc;	/* report ready callback function            */
	uns32 o_cr_depth;	/* start queue depth                         */
	uns32 o_cr_pcnt;	/* start max queue depth percentile          */

} NCSSYSM_IPC_RPT_LBGN;

typedef struct ncsipc_rpt_ltcy {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results              */
	char *o_name;		/* Queue name                                     */
	uns32 o_st_time;	/* start time                                     */
	uns32 o_fn_time;	/* finish time                                    */
	uns32 o_st_depth;	/* start queue depth                              */
	uns32 o_fn_depth;	/* finish queue depth                             */

} NCSSYSM_IPC_RPT_LTCY;

typedef struct ncssysm_ipc_rpt_qabgn {
	uns32 i_howlong;	/* monitor queues this long in seconds             */

} NCSSYSM_IPC_RPT_QABGN;

typedef struct ncssysm_ipc_qact {
	char *o_name;		/* Queue name                                       */
	uns32 o_st_depth;	/* start queue depth                                */
	uns32 o_st_pcnt;	/* start max queue depth percentile                 */
	uns32 o_dequeued;	/* messages consumed off the queue                  */
	uns32 o_enqueued;	/* messages placed on the queue                     */
	uns32 o_emty_cnt;	/* times no msgs in queue (task pends)              */
	uns32 o_fn_depth;	/* finish queue depth                               */
	uns32 o_fn_pcnt;	/* finish max queue depth percentile                */

} NCSSYSM_IPC_QACT;

#ifndef NCSIPC_MAX
#define NCSIPC_MAX    100
#endif

typedef struct ncssysm_ipc_rpt_qact {
	NCSSYSM_OPP i_opp;	/* output policy to apply to results */
	uns32 o_filled;		/* how many array elements filled    */
	NCSSYSM_IPC_QACT o_ipc[NCSIPC_MAX];	/* space for report                  */

} NCSSYSM_IPC_RPT_QACT;

/***************************************************************************
 * The operations set that a SYSM instance supports
 ***************************************************************************/

typedef enum ncssysm_lm_op {

	NCSSYSM_LM_OP_INIT = 1,
	NCSSYSM_LM_OP_TERMINATE,

	NCSSYSM_LM_OP_GET,
	NCSSYSM_LM_OP_SET,
	NCSSYSM_LM_OP_IGET,
	NCSSYSM_LM_OP_ISET,
	NCSSYSM_LM_OP_REG_LMT_CB,

	NCSSYSM_LM_OP_MEM_STATS,
	NCSSYSM_LM_OP_CPU_STATS,
	NCSSYSM_LM_OP_BUF_STATS,
	NCSSYSM_LM_OP_TMR_STATS,
	NCSSYSM_LM_OP_TASK_STATS,
	NCSSYSM_LM_OP_IPC_STATS,
	NCSSYSM_LM_OP_LOCK_STATS,

	NCSSYSM_LM_OP_MEM_STK_ADD,
	NCSSYSM_LM_OP_MEM_STK_FLUSH,
	NCSSYSM_LM_OP_MEM_STK_RPT,

	NCSSYSM_LM_OP_BUF_STK_ADD,
	NCSSYSM_LM_OP_BUF_STK_FLUSH,
	NCSSYSM_LM_OP_BUF_STK_RPT,

	NCSSYSM_LM_OP_MEM_RPT_THUP,
	NCSSYSM_LM_OP_MEM_RPT_SSUP,
	NCSSYSM_LM_OP_MEM_RPT_WO,
	NCSSYSM_LM_OP_MEM_RPT_WOS,

	NCSSYSM_LM_OP_BUF_RPT_WO,
	NCSSYSM_LM_OP_BUF_RPT_WOS,

	NCSSYSM_LM_OP_IPC_RPT_LBGN,
	NCSSYSM_LM_OP_IPC_RPT_LTCY,

	NCSSYSM_LM_OP_SENTINAL	/* no comma here */
} NCSSYSM_LM_OP;

/***************************************************************************
 * The SYSTEM MONITORING API single entry point for a system Master
 ***************************************************************************/

typedef struct ncssysm_lm_arg {
	NCSSYSM_LM_OP i_op;	/* One of the many Operations possible      */
	NCS_VRID i_vrtr_id;	/* virtual router ID for which-resource set */

	union {

		/* System  Monitoring  Housekeeping */

		/* System Monitoring Configurations */

		NCSSYSM_GET get;
		NCSSYSM_SET set;
		NCSSYSM_IGET iget;
		NCSSYSM_ISET iset;
		NCSSYSM_REG_LMT_CB reg_lmt_cb;	/* Register Resource Limit Callback */

		/* System  Monitoring  Statistics */

		NCSSYSM_MEM_STATS mem_stats;
		NCSSYSM_CPU_STATS cpu_stats;
		NCSSYSM_BUF_STATS buf_stats;
		NCSSYSM_TMR_STATS tmr_stats;
		NCSSYSM_TASK_STATS task_stats;
		NCSSYSM_IPC_STATS ipc_stats;
		NCSSYSM_LOCK_STATS lock_stats;

		/* System  Monitoring  Reports */

		NCSSYSM_MEM_RPT_THUP mem_rpt_thup;
		NCSSYSM_MEM_RPT_SSUP mem_rpt_ssup;
		NCSSYSM_MEM_RPT_WO mem_rpt_wo;
		NCSSYSM_MEM_RPT_WOS mem_rpt_wos;

		NCSSYSM_BUF_RPT_WO buf_rpt_wo;
		NCSSYSM_BUF_RPT_WOS buf_rpt_wos;

		/* Memory Leak watch functionality  */

		NCSSYSM_MEM_STK_ADD mem_stk_add;
		NCSSYSM_MEM_STK_FLUSH mem_stk_flush;
		NCSSYSM_MEM_STK_RPT mem_stk_rpt;

		/* Buffer Leak watch functionality  */

		NCSSYSM_BUF_STK_ADD buf_stk_add;
		NCSSYSM_BUF_STK_FLUSH buf_stk_flush;
		NCSSYSM_BUF_STK_RPT buf_stk_rpt;

		NCSSYSM_IPC_RPT_LBGN ipc_rpt_lat_bgn;
		NCSSYSM_IPC_RPT_LTCY ipc_rpt_lat;
		NCSSYSM_IPC_RPT_QABGN ipc_rpt_qabgn;
		NCSSYSM_IPC_QACT ipc_qact;
		NCSSYSM_IPC_RPT_QACT ipc_rpt_qact;

	} op;

} NCSSYSM_LM_ARG;

/***************************************************************************
 Function prototype of system monitoring LM svcs (no known need at this time)
 ***************************************************************************/

typedef uns32 (*NCSSYSM_LM) (NCSSYSM_LM_ARG * info);

/***************************************************************************
 System Monitoring function that matches prototype
 ***************************************************************************/

EXTERN_C LEAPDLL_API uns32 ncssysm_lm(NCSSYSM_LM_ARG * info);

#endif
