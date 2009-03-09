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

/** Module Inclusion Control...
 **/
#ifndef NCSSYSFPOOL_H
#define NCSSYSFPOOL_H

#include "ncs_hdl_pub.h"
#include "ncssysf_lck.h"

#ifdef  __cplusplus
extern "C" {
#endif

/* Memory Manager Debugging defaults to OFF.  This compile time flag
 * controls debugging of the Memory Manager, including enabling of
 * statistics.
 * To override this value, you can define NCS_MMGR_DEBUG to 1 here or in
 * the compiler command line.
 */

#ifndef NCS_MMGR_STACK_DEBUG
#define NCS_MMGR_STACK_DEBUG              0
#endif

#ifndef NCS_MMGR_STACKTRACE
#define NCS_MMGR_STACKTRACE               0
#endif

#ifndef NCS_MMGR_ENABLE_PRINT_TIMESTAMP
#define NCS_MMGR_ENABLE_PRINT_TIMESTAMP   0
#endif

/******************************************************************************** 
 * E R R O R _ B E H A V I O I R
 *
 * The error behavior flag allows a user to set policy reguarding
 * what behavior LEAP should use when a Memory Manement error is
 * Detected: 
 *    - Ignore the error and keep going, 
 *    - Print a Banner of file/line of offender 
 *    - Crash the box after the Banner is printed
 * Errors are BUGs that ought to be fixed. The memory manager has
 * 4 bugs that it looks for. They are:
 *
 * NCSFAIL_DOUBLE_DELETE     Double delete of same pointer
 * NCSFAIL_FREEING_NULL_PTR  Freeing a NULL pointer
 * NCSFAIL_MEM_SCREWED_UP    Generally bad pointer coming back to MMGR
 * NCSFAIL_FAILED_BM_TEST    Bottom Marker test failed.
 *
 * NOTE: NCS_MMGR_DEBUG must be one for some errors to be detected.
 */

#define IGNORE_ERR  1
#define BANNER_ERR  2
#define CRASH_ERR   3

#ifndef NCS_MMGR_ERROR_BEHAVIOR
#define NCS_MMGR_ERROR_BEHAVIOR    CRASH_ERR
#endif


/* Our OS Services implementation of the Memory Manager ignores the
 * memory region parameter.  If the target system has different memory
 * regions, then the m_NCS_MEM_ALLOC and m_NCS_MEM_FREE macros should be
 * redefined to make use of the memory region information.  The default
 * definitions for the memory regions is found in ncs_osprm.h.  To override
 * the defaults, add #defines in the os_defs.h file under osprims.
 */

                      
/* debugging is ENABLED - include debugging args */

#define m_NCS_MEM_ALLOC(nbytes, mem_region, service_id, sub_id) \
          ncs_mem_alloc(nbytes, (void *)(mem_region), service_id, sub_id, __LINE__, __FILE__)

#define m_NCS_MEM_FREE(free_me, mem_region, service_id, sub_id) \
          ncs_mem_free((void *)(free_me), (void *)(mem_region), service_id, sub_id, __LINE__, __FILE__)

#define m_NCS_MEM_REALLOC(payload, nbytes, mem_region, service_id, sub_id) \
          ncs_mem_realloc((void *)(payload), nbytes, (void *)(mem_region), service_id, sub_id)

#define m_NCS_MEM_DBG_LOC(ptr) if (NULL != ptr)\
                                 ncs_mem_dbg_loc(ptr,__LINE__,__FILE__)

#define m_NCS_MEM_DBG_USR(ptr,usrdata) ncs_mem_dbg_usr(ptr,usrdata)

/***************************************************************************** 
*m_NCS_MEM_OUTSTANDING( )
*  
*SYNOPSIS:
*
*#define m_NCS_MEM_OUTSTANDING() ncs_mem_outstanding()
*EXTERN_C LEAPDLL_API uns64          ncs_mem_outstanding   (void);
*
*DESCRIPTION:
*   This macro returns the instantaneous number of  bytes of memory in the 
*   process that  have been allocated  from the  LEAP Memory Manager but 
*   have not yet been  returned to the LEAP memory manager (using  m_NCS_MEM_FREE) 
*   at the time of invocation. 
*ARGUMENTS:
*        void
*RETURN VALUES:
*       uns64
*       :bytes of memory allocated  but not yet freed.
* 
*****************************************************************************/ 
#define m_NCS_MEM_OUTSTANDING() ncs_mem_outstanding()

/* MEMORY TRACE SUPPORT */
#if(NCS_MMGR_STACK_TRACE == 1)

#define  OWNERS_TRACED     10
#define  OWNER_NAME        14
#define  MEM_SET_NULL      0
#define  MEM_FROM_OWNER    1
#define  MEM_TO_OWNER      2

typedef struct mem_trace_record_tag
{
   unsigned short transfer;
   unsigned short line;
   char           file[OWNER_NAME];
}MEM_TRACE_RECORD;

void ncs_mem_dbg_trace_owner(void* ptr, int transfer, unsigned int line, char* file);

/* Memory trace support function to trace the chain of files which owns the memory */
#define m_NCS_MEM_DBG_TRACE_OWNER(ptr, transfer) ncs_mem_dbg_trace_owner(ptr, transfer, __LINE__, __FILE__)

#endif /* NCS_MMGR_STACK_TRACE == 1 */



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 
               FUNCTION PROTOTYPES
 
 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

EXTERN_C LEAPDLL_API uns32          ncs_mem_create     (void);
EXTERN_C LEAPDLL_API uns32          ncs_mem_destroy    (void);

EXTERN_C LEAPDLL_API uns32          ncs_mem_stats_dump (char*         filename);

EXTERN_C LEAPDLL_API void          *ncs_mem_alloc      (unsigned int  nbytes,
                                           void          *mem_region,
                                           NCS_SERVICE_ID service_id,
                                           unsigned int  sub_id,
                                           unsigned int  line,
                                           char*         file);

EXTERN_C LEAPDLL_API void           ncs_mem_free       (void          *free_me,
                                           void          *mem_region, 
                                           NCS_SERVICE_ID service_id,
                                           unsigned int  sub_id,
                                           unsigned int  line,
                                           char*         file);

EXTERN_C LEAPDLL_API void          *ncs_mem_realloc    (void          *payload,
                                           unsigned int  nbytes,
                                           void          *mem_region,
                                           NCS_SERVICE_ID service_id,
                                           unsigned int  sub_id);



EXTERN_C LEAPDLL_API void           ncs_mem_dbg_usr    (void          *ptr,
                                           char*         usrdata);
EXTERN_C LEAPDLL_API char*          ncs_mem_hex_dump_entry(NCSCONTEXT address);


EXTERN_C LEAPDLL_API void           ncs_mem_dbg_loc    (void          *ptr,
                                           unsigned int  line,
                                           char*         file);

EXTERN_C LEAPDLL_API uns64          ncs_mem_outstanding   (void);
EXTERN_C LEAPDLL_API uns32          ncs_mem_whatsout_dump    (void               );
EXTERN_C LEAPDLL_API uns32          ncs_mem_whatsout_summary (char*    mem_file);
EXTERN_C LEAPDLL_API void           ncs_mem_ignore     (NCS_BOOL        whatsout);
EXTERN_C LEAPDLL_API void           ncs_mem_ignore_subsystem (unsigned int zero_or_one, 
                                                 unsigned int svc_id);

/* Dumps the mpool-entry that precedes the payload part used by 
   LEAP client. 
*/
EXTERN_C LEAPDLL_API void           ncs_mpool_entry_dump(NCSCONTEXT payload_address);
typedef void (*LEAP_PRINT_CALLBACK) (void *ucontext, char *next_full_line);
EXTERN_C LEAPDLL_API uns32          ncs_mem_whatsout_dump_custom(LEAP_PRINT_CALLBACK customptr,void *ucontext);

#if (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)

EXTERN_C LEAPDLL_API void           ncs_mem_ignore_timestamp (unsigned int zero_or_one);

#endif

EXTERN_C LEAPDLL_API void * ncs_next_mpool_entry(void *arg);


#if (NCS_MMGR_STACK_DEBUG == 1)
EXTERN_C LEAPDLL_API unsigned int ncs_dump_stack(char *stack_str, unsigned int str_size);
#endif

EXTERN_C LEAPDLL_API void ncs_mem_ignore_timestamp(unsigned int zero_or_one);

EXTERN_C LEAPDLL_API char* ncs_fname(char* fpath);

#ifdef  __cplusplus
}
#endif

#endif /* NCSSYSFPOOL_H */
