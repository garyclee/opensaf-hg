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
            
  This module contains target system specific declarations related to
  Memory Pool Management.            
  ..............................................................................
*/

/* Get compile time options...*/

#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_stack.h"
#include "sysf_def.h"
#include "sysf_ipc.h"
#include "ncssysf_def.h"
#include "ncssysfpool.h"
#include "ncs_sysm.h"
#include "ncsusrbuf.h"
#include <assert.h>



/******************************************************************************** 
 * H J _ M P O O L _ S T A T
 * Statistics kept on each 'bucket' of memory; Totals derived from there
 */


typedef struct ncs_mpool_stat
{

#if ((NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1) || (NCS_MMGR_DEBUG == 1))
  uns32  size;           /* buffer payload sizes in this pool   */
  uns32  allocs;         /* ncs_mem_alloc() called and memout  (no of buffers) i.e allocated from free pool or from os */
  uns32  frees;          /* ncs_mem_free() called and mem back (no of buffers) i.e freed to os or freed to free pool */
  uns32  errors;         /* various troubles, most very bad     */
  uns32  highwater;      /* max number of hunks in this pool    */
  uns32  failed_alloc;   /* returning a NULL ptr on alloc       */
  uns32  bytes_inuse;    /* This stat only used for pool size 0 */
#endif
  uns32  num_inuse;      /* num of buffers currently out there  */
  uns32  min_size;       /* minimum no of buffers can  present in free pool when memory usage is high*/
  uns32  num_avail;      /* num of buffers currently in free pool    */
  uns32  num_freed_to_os; /* no of buffers  freed to os from the pool */
} NCS_MPOOL_STAT;

/* Statistics kept based on compile time flag constraints       */


#define MEM_STAT_FREED_TO_OS_PLUS(m) m->stat.num_freed_to_os++;
#define MEM_STAT_AVAIL_PLUS(m)       m->stat.num_avail++;
#define MEM_STAT_AVAIL_MINUS(m)       m->stat.num_avail--;
#define MEM_STAT_INUSE_PLUS(m)       m->stat.num_inuse++;
#define MEM_STAT_INUSE_MINUS(m)      m->stat.num_inuse--;

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))

#define MEM_STAT_SIZE(m,s)           m->stat.size = s
#define MEM_STAT_FAILED_ALLOC(m)     m->stat.failed_alloc++;
#define MEM_STAT_ALLOCS(m)           m->stat.allocs++;
#define MEM_STAT_FREES(m)            m->stat.frees++;
#define MEM_STAT_ERRORS(m)           m->stat.errors++;

#else

#define MEM_STAT_SIZE(m,s)
#define MEM_STAT_FAILED_ALLOC(m)
#define MEM_STAT_ALLOCS(m)
#define MEM_STAT_FREES(m)
#define MEM_STAT_ERRORS(m)

#endif


#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))

struct ncs_sysmon;

extern struct ncs_sysmon gl_sysmon;

#endif

/******************************************************************************** 
 * H J _ M M G R _ S T A T S
 *
 * Statistics kept on the whole memory manager (all buckets combined)
 */



typedef struct ncs_mmgr_stats
{
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
  uns32  max;     /* MAX available memory */
  uns32  alloced; /* bytes allocated (with out header size )i.e combination of bytes alloced from os and from free pool*/
  uns32  freed;   /* total bytes freed (with out header size) including bytes freed to os and bytes freed to free pool */
  uns32  hwm;     /* high water mark in bytes its in sync with "curr field" */
  uns32  freed_to_os;  /* total bytes of memory freed to OS rather than placing in free pool (with out header size)*/
  uns32  total_os_allocs;  /* total bytes of alloc's done from  OS (with including header size)*/
  uns32  total_os_frees;  /* total bytes of memory freed to OS rather than placing in free pool (with including header size)*/
#endif
  uns32  bytes_inuse;    /* current byte in use of alloc'ed memory (with out header size) */ 
  uns32  bytes_avail;  /* total bytes of  memory in free pool(with out including header */
} NCS_MMGR_STATS;

/******************************************************************************** 
 * H J _ M P O O L _ E N T R Y _ D B G
 *
 * Keeping track of facts associated with this Memory buffer for Debugging
 */

typedef struct ncs_mpool_entry_dbg
{
  NCS_SERVICE_ID prev_svc_id; /* the previous owner of this mem */
  unsigned int  prev_sub_id;

  NCS_SERVICE_ID service_id;  /* the current owner fo this mem */
  unsigned int  sub_id;

  unsigned int  line;        /* file & line of allocation */
  char*         file;

  unsigned int  loc_line;    /* file & line of marking client */
  char*         loc_file;

  char*         usr_data;    /* user provided marker information */

  char*         memptr;
  unsigned int  entrysize;

  unsigned int  count;       /* keeps track of life-time in ticks */
#if (NCS_MMGR_STACK_DEBUG == 1)
  char          *stack_str; /* Buffer to store stack when memory
                                              gets allocated */
#endif

#if (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)
  char asc_tod[32];
#endif

} NCS_MPOOL_ENTRY_DBG;

#define NCS_MPOOL_STAT_NULL ((NCS_MPOOL_STAT *)0)

/* our OS Services Memory Manager creates 9 pools of buffers in 1 memory
 * region.  The first pool has 16 byte buffers; each successive pool has
 * the next power-of-2 byte buffers; the last pool has 4096 byte buffers.
 */

#define MMGR_NUM_POOLS   9
#define STACK_STR_SIZE   300

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
#if (NCSSYSM_STKTRACE == 1)

/******************************************************************************** 
 * H J _ O S _ S T K T R A C E _ E N T R Y
 *
 * Used to keep track of call-stack for a memory allocation when proper flags
 * are enabled. Its only use is for debugging for memory leaks.
 */

#define NCSSYSM_STKTRACE_INFO_MAX    5

typedef struct ncs_stktrace_info
  {
  uns32                  flags;
  uns32                  line;
  char                   file[13];
  NCS_OS_STKTRACE_ENTRY  *ste;

  } NCSSYSM_STKTRACE_INFO;

#endif /*(NCSSYSM_STKTRACE == 1)*/
#endif /*((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))*/

/******************************************************************************** 
 * P O O L _ E N T R Y _ P A D
 * See writeup for #define NCS_MEM_ALLOC_PAD_BIAS in ncs_opt.h file
 */

/* See writeup for #define NCS_MEM_ALLOC_PAD_BIAS in ncs_opt.h file */

#if (NCS_MEM_ALLOC_PAD_BIAS == 0)

#define POOL_ENTRY_PAD

#else /* Must be set to some bias value to pad */

#define POOL_ENTRY_PAD  uns8  pad[NCS_MEM_ALLOC_PAD_BIAS];

#endif

/******************************************************************************** 
 * H J _ M P O O L _ E N T R Y 
 * Each allocated hunk of memory has one of these 'on top of' and contiguous
 * with the allocated hunk. Sysfpool keeps lots of data on memory when
 * DEBUG is enabled.
 */

typedef struct ncs_mpool_entry
{

#if ((NCS_MMGR_DEBUG  == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))

  /* Debug data galore for giving insight into memory leaks/crashes/etc. */

  uns32         start_marker; /* Start of structure marker 0x33333333 */
#endif
#if ((NCS_MMGR_DEBUG           == 1) || \
     (NCSSYSM_MEM_DBG_ENABLE   == 1) || \
     (NCSSYSM_BUF_DBG_ENABLE   == 1) || \
     (NCSSYSM_MEM_STATS_ENABLE == 1) || \
     (NCSSYSM_MEM_WATCH_ENABLE == 1))

  uns32         from_pool_sz; /* the pool from whence I came */

  unsigned int  free_line;    /* file & line of free-er */
  char*         free_file;

  unsigned int  line;         /* file & line of allocation */
  char*         file;

  unsigned int  loc_line;     /* file & line marked by 'real' owner */
  char*         loc_file;

  NCS_SERVICE_ID prev_svc_id;  /* the previous owner of this mem */
  unsigned int  prev_sub_id;

  unsigned int  prev_loc_line;/* file & line marked by 'previous real' owner */
  char*         prev_loc_file;

  NCS_SERVICE_ID service_id;   /* the current owner fo this mem */
  unsigned int  sub_id;

  char*         usr_data;     /* user provided marking information */

  NCS_BOOL       ignore;       /* ncsmem_whatsout() values */
  unsigned int  count;
  unsigned int  age;          /* pulse age */
#if (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)
  char asc_tod[33];
#endif
#endif
#if (NCS_MMGR_STACK_DEBUG == 1)
  char          stack_str[STACK_STR_SIZE]; /* Buffer to store stack when memory
                                              gets allocated */
#endif

#if (NCS_MMGR_STACK_TRACE == 1)
  char* memory_owners;
#endif

#if (NCSSYSM_STKTRACE == 1)
  NCS_OS_STKTRACE_ENTRY  *stkentry;
#endif

  /* The REAL stuff when DBG is OFF */

  struct ncs_mpool_entry  *next;
  struct ncs_mpool_entry  *prev;
  struct ncs_mpool        *pool;
  NCS_BOOL                 owned;
  uns32                   real_size;

  POOL_ENTRY_PAD /* Add pad bytes to sizeof NCS_MPOOL_ENTRY: No semicolen is right!! */

} NCS_MPOOL_ENTRY;

/******************************************************************************** 
 * H J _ M P O O L
 * Each mem bucket is called an NCS_MPOOL; free and inuse buffers of a particular
 * size live off of this pool anchor.
 */

typedef struct ncs_mpool
{
  unsigned int  size;
  NCS_MPOOL_STAT    stat;
  NCS_LOCK          lock; /* each bucket is its own critical region        */
  NCS_MPOOL_ENTRY  *free; /* front of free pool All allocs come from front */
  NCS_MPOOL_ENTRY  *end;  /* end of free pool.. All frees go to end        */

  NCS_MPOOL_ENTRY  *inuse;/* list of alloced memory blocks                 */
  uns32            outstanding_mem;/* total bytes of memory allocated from 
                                     this pool but not yet freed to Leap 
                                     memory manager                        */ 
                             

} NCS_MPOOL;


/******************************************************************************** 
 * H J _ M M G R 
 * The set of mpool 'buckets' are gathered under this abstraction.
 */

typedef struct ncs_mmgr
  {
  NCSLPG_OBJ       lpg;    /* local persistence guard.. cheaper than a lock */
  NCS_MPOOL*       mpools;

  NCS_MMGR_STATS   stats;
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
#if (NCSSYSM_STKTRACE == 1)
  NCSSYSM_STKTRACE_INFO stktrace_info[NCSSYSM_STKTRACE_INFO_MAX];
#endif
#endif
} NCS_MMGR;

EXTERN_C LEAPDLL_API NCS_MMGR mmgr;

LEAPDLL_API void  ncs_mem_dump_entry(
                      NCS_MPOOL_ENTRY  *me, 
                      uns32 i, 
                      unsigned int ignore_count, 
                      char *pBuf,
                      LEAP_PRINT_CALLBACK ptr_custom, void *ucontext);

LEAPDLL_API uns32  ncs_mem_whatsout(
                      NCS_MPOOL_ENTRY_DBG* entry, 
                      uns32 slots);

LEAPDLL_API uns32  ncs_mem_fetch_stat (
                      uns32         size,
                      NCS_MPOOL_STAT *storage);

#if (NCS_MMGR_DEBUG != 0)
  static void leap_mem_print(void * ucontext, char *payload);
#endif

/* Pad the size of the thing allocated to an even boundary for WBS machines */

#if (TRGT_IS_WBS == 0)

#define MMGR_PAD_SIZE(s) s
#else
#define MMGR_PAD_SIZE(s) (s%4?(s+(4-(s%4))):s)

#endif

/* Runtime constants......................................................... */

#define MMGR_NUM_POOLS            9  /* number of buckets in sysfpool manager */
#define STACK_STR_SIZE          300  /* DEBUG assist when doing stack trace   */


/* Mark top & bottom of alloc'ed memory; check when freed for corruption      */ 

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))

#define BOTTOM_MARKER  0x35353535      /* marker; printable characters "5555" */
#define START_MARKER   0x33333333      /* marker; printable characters "3333" */

/*******************************************************************************
 *  G L _ M P O O L _ E N T R Y
 *
 *  This global variable is for debugging. Since it is global, you can find
 *  this symbol anywhere in your image (using QuickWatch in Developer's Studio
 *  during a memory corruption crash for example). When found, you can assign
 *  it a ptr value so that you can easily 'cast' memory to the NCS_MPOOL_ENTRY
 *  type to read all the fields.
 *
 *  You know where a NCS_MPOOL_ENTRY begins in memory because the first 4 bytes
 *  of this structure have been 'stamped' with the value 0x33333333 (printable
 *  characters "3333"), as per the START_MARKER defined above.
 */

NCS_MPOOL_ENTRY*  gl_mpool_entry; 

/* default assigned file-name for unclaimed allocations in NONE                */

static char gl_none[5] = "NONE";

#if(NCS_MMGR_STACK_TRACE == 1)
void* gl_ptr = NULL;
#endif

#endif /* ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1)) */

#if ((NCS_MMGR_DEBUG  == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1)) 

/* create array of int flags to determine whether or not MEM DEBUG
 * info is printed out when doing a "what's out" dump
 *
 * DEFAULT = 0 ==> DO NOT IGNORE SUBSYSTEM
 */

#endif

#if ((NCS_MMGR_DEBUG  == 1 ) && (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1))

/* create int flag to determine whether or not MEM DEBUG
 * prints out an allocation timestamp when doing a "what's out" dump
 *
 * DEFAULT = 1 ==> IGNORE TIMESTAMP
 */

int ignore_timestamp = 1;

#endif


/*******************************************************************************
 *  H J _ M M G R
 *
 *  This is the master, global data structure of the sysfpool memory manager.
 *  All things are in it or attached to it.
 */

NCS_MMGR   mmgr = {
  {
          /* open   */ FALSE, 
          /* inhere */ 0
  }, 
          /* pools  */ NULL,
 {
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
  0xffffffff,0,0,0,0,0,0,
#endif
  0,0},
  };


  /* PROTOTYPES */

  char* ncs_fname(char* fpath);


 /****************************************************************************
  ****************************************************************************
  ****************************************************************************
  *
  * Function Utilities: Various ways to get an NCS_MPOOL and calc nominal size
  */

#if (NCS_MMGR_DEBUG != 0)
/****************************************************************************
 * mpool_fetch_last
 */

static NCS_MPOOL * mpool_fetch_last (void)
  {
  NCS_MPOOL *mp = mmgr.mpools;
  
  while(mp->size != 0)
    mp++;
  return mp;
  }

#endif /* (NCS_MMGR_DEBUG != 0) */

/****************************************************************************
 *  mpool_fetch
 */

static NCS_MPOOL * mpool_fetch (unsigned int size)
  {
  NCS_MPOOL *mp = mmgr.mpools;
  
  while (mp->size != 0)
    {
    if (size <= mp->size)
      return mp;
    mp++; 
    }
  return mp;
  }

/****************************************************************************
 * get_nominal_size
 */

static unsigned int get_nominal_size(unsigned int nbytes)
  {
  NCS_MPOOL *mp = mmgr.mpools;
  
  while (mp->size != 0)
    {
    if (nbytes <= mp->size)
      return mp->size;
    mp++; 
    }
  return mp->size;
  }

 /****************************************************************************
  *
  * Function Name: ncs_mem_create
  *
  * Purpose:       put H&J memory pool (reference implementation) into start
  *                state.
  *
  ****************************************************************************/

uns32
ncs_mem_create ()
  {
  uns32          mdex, len, size;
  NCS_MPOOL       *mp;
  
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1)) 
  mmgr.stats.max= 0xffffffff;    
  mmgr.stats.alloced = 0; 
  mmgr.stats.freed = 0; 
  mmgr.stats.hwm = 0;    
  mmgr.stats.freed_to_os = 0; 
  mmgr.stats.total_os_allocs = 0;  
  mmgr.stats.total_os_frees = 0;  
#endif
  mmgr.stats.bytes_inuse = 0;    
  mmgr.stats.bytes_avail =0 ;  

  /** Allocate and zero out the array of pools. **/
  len = (MMGR_NUM_POOLS + 1) * sizeof(NCS_MPOOL);
  
  if ((mmgr.mpools = (NCS_MPOOL *)m_NCS_OS_MEMALLOC (len, NCS_MEM_REGION_PERSISTENT)) == NULL)
    return NCSCC_RC_FAILURE;
  
  memset ((char *)mmgr.mpools, '\0', len);
  
  for (mdex = 0, mp = mmgr.mpools, size = 16;
  mdex <= MMGR_NUM_POOLS;
  mdex++, mp++, size*=2)
    {
    mp->size      = (mdex < MMGR_NUM_POOLS) ? size : 0;
    if(size==256) /* special case this to allow usrbufs to fit in */
      {
      mp->size+=4;
      }
    
    MEM_STAT_SIZE(mp,mp->size);
    
    m_NCS_LOCK_INIT(&mp->lock); /* initialize the local bucket lock */
    mp->free      = NULL;
    mp->inuse     = NULL;
    mp->end       = NULL;
    mp->stat.min_size  = 20;  /*Minimum number of memory pools*/
    }
  
   ncslpg_create(&mmgr.lpg);

  
  return NCSCC_RC_SUCCESS;
  }

 /****************************************************************************
  *
  * Function Name: ncs_mem_destroy
  *
  * Purpose:       Recover all memory resources in the H&J memory pool.
  */

uns32
ncs_mem_destroy ()
  {
  NCS_MPOOL       *mp;
  NCS_MPOOL_ENTRY *me;
  uns32          temp; 
  
  USE(temp);          /* used when particular DEBUG flags are enabled */
  
  if ( ncslpg_destroy(&mmgr.lpg) == FALSE)             /* already closed ?? */
    return NCSCC_RC_FAILURE;
  
  for (mp = mmgr.mpools; mp->size != 0; mp++)
    {
    m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* lock this particular pool */
    while ((me = mp->free) != NULL)
      {
      mp->free = me->next;
      if (me->next != NULL)
        me->next->prev = NULL;
      m_NCS_OS_MEMFREE (me, NCS_MEM_REGION_PERSISTENT); 
      }
    
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
    if (mp->inuse != NULL)
      { /* Just to put a breakpoint */
      temp = 1;
      }
#endif
    m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* unlock this particular pool */
    }
  
  m_NCS_OS_MEMFREE (mmgr.mpools, NCS_MEM_REGION_PERSISTENT);
  mmgr.mpools = NULL;


  return NCSCC_RC_SUCCESS;
  }

 /****************************************************************************
  *
  * Function Name: ncs_mem_alloc
  *
  * Purpose:       Allocate memory from the best-fit pool. If free memory
  *                is not in that pool, go to the HEAP and get some.
  *
  *                If debug is enabled, store lots of stuff about the
  *                invoker.
  *
  ****************************************************************************/

void *
ncs_mem_alloc(unsigned int  nbytes,
             void*         mem_region,
             NCS_SERVICE_ID service_id,
             unsigned int  sub_id,
             unsigned int  line,
             char*         file)
  {
  NCS_MPOOL_ENTRY *me;
  NCS_MPOOL       *mp;
  unsigned int    len;
  NCS_BOOL         from_heap = FALSE;
  char           *payload;
  time_t          tod;
  
  USE(tod);
  USE(mem_region);

  if (nbytes <= 0)
    return NULL;
  
  if (ncslpg_take(&mmgr.lpg) == FALSE)
    return NULL; /* Either not created yet or already shut down */

#if ((NCSSYSM_MEM_STATS_ENABLE == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1))

  /* SMM below we  add nbytes for basic test; In fact, this will map to    */
  /* a particular pool which may commit more memory than what is asked for */
  if(mmgr.stats.bytes_inuse + nbytes>= mmgr.stats.max) /* SMM critical region!!   */
     {
     NCSSYSM_RES_LMT_EVT evt; 
     
     evt.i_vrtr_id = gl_sysmon.vrtr_id; 
     evt.i_obj_id  = NCSSYSM_OID_MEM_TTL; 
     evt.i_ttl     = mmgr.stats.max;
     if(gl_sysmon.res_lmt_cb)
       (*gl_sysmon.res_lmt_cb)(&evt); /* callback .. Out of Memory !!! */
     
     return (void*)ncslpg_give(&mmgr.lpg, 0);
     }
#endif
  
  if ((mp = mpool_fetch (nbytes)) == NULL)
    {
    m_LEAP_FAILURE(NCSFAIL_MEM_NULL_POOL,0);
    ncslpg_give(&mmgr.lpg,0);
    return NULL;
    }
  
  m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* Lock the specific pool being used */

  if ((me = mp->free) == NULL)/* ............. Gotta get memory from HEAP ?? */
    {
    len = mp->size ?  (mp->size + sizeof(NCS_MPOOL_ENTRY) + sizeof(uns32)) :
                      (nbytes   + sizeof(NCS_MPOOL_ENTRY) + sizeof(uns32)) ;
    

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
     mmgr.stats.total_os_allocs += len;  
#endif
    if ((me = (NCS_MPOOL_ENTRY *)m_NCS_OS_MEMALLOC(len, mem_region)) == NULL)
      {
      MEM_STAT_FAILED_ALLOC(mp);
      m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* gotta go */
      ncslpg_give(&mmgr.lpg, 0);
      return NULL; 
      }
  
    from_heap = TRUE; /* We set now/ test later to shorten critical regions */    
    }
  else if (me->owned == TRUE) /* ............ got one, but isit available?? */
    {
    MEM_STAT_ERRORS(mp);
    m_LEAP_FAILURE(NCSFAIL_MEM_SCREWED_UP,0);    /* how this could happen ?? */ 
    m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);     /* gotta go */
    ncslpg_give(&mmgr.lpg, 0);  
    return NULL;

    }
  else /* ........................we pull it off the front of the free queue */
    {
    mp->free  = me->next;
    if (me->next != NULL)
      me->next->prev = NULL;

    if (mp->free == NULL)                     /* SMM update free end pointer */
      mp->end = NULL;

    MEM_STAT_AVAIL_MINUS(mp);
    mmgr.stats.bytes_avail -=mp->size; 
    }
  
  /* tie it into the inuse list */
  me->next      = mp->inuse;
  mp->inuse     = me;
  me->prev      = NULL;
  
  if (me->next != NULL)
    me->next->prev = me;
  
  /* OK, its tied  in */  
 
 
  MEM_STAT_INUSE_PLUS(mp);

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1))
  me->ignore    = FALSE;
  me->age       = 0;
  me->count     = 0;


  MEM_STAT_ALLOCS(mp);
  
  if(mp->size == 0) /* keep track of byte-count in last free-for-all pool */
    mp->stat.bytes_inuse += nbytes;
    
   if (mp->stat.num_inuse > mp->stat.highwater)
    mp->stat.highwater = mp->stat.num_inuse;
#endif
   /* */
   {
    uns32 alloced_bytes;   /* setup to see if we passed system max bytes */
    if(mp->size == 0)
      alloced_bytes = nbytes;
    else
      alloced_bytes = mp->size;
    
    mmgr.stats.bytes_inuse += alloced_bytes;

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1))
    mmgr.stats.alloced += alloced_bytes;
    if(mmgr.stats.bytes_inuse > mmgr.stats.hwm)
      mmgr.stats.hwm = mmgr.stats.bytes_inuse;         /* SMM global critical region */
#endif
   }


    mp->outstanding_mem += MMGR_PAD_SIZE(nbytes);

    m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);         /* critical region is over */
    
    if (from_heap == TRUE)
      {
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
      
      me->prev_sub_id   = 0;  /* we clear it out once; from then on, we record */
      me->prev_svc_id   = 0;
      me->prev_loc_file = gl_none;
      me->prev_loc_line = 0;
      me->start_marker  = START_MARKER;                        /* mark it once */
      me->free_file     = gl_none;               /* starts off with no free-er */
      me->free_line     = 0;
      me->from_pool_sz  = mp->size;
#endif
      me->pool          = mp;
      }
    
    me->real_size    = MMGR_PAD_SIZE(nbytes);
    me->owned        = TRUE;
    

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
    
    me->loc_file     = gl_none;
    me->loc_line     = 0;
    me->service_id   = service_id;
    me->sub_id       = sub_id;
    me->line         = line;
    me->file         = file;
    me->usr_data     = gl_none;
    
#if (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)
    m_GET_ASCII_TIME_STAMP(tod, me->asc_tod);
#endif
#endif
    
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
#if (NCSSYSM_STKTRACE == 1) /* SMM below is critical region  */
      {
      int   index;
      char *pfile;
      
      pfile = strrchr(me->file, NCS_OS_PATHSEPARATOR_CHAR);
      if(NULL == pfile)
        {
        pfile = me->file;
        }
      else
        {
        pfile++;
        }
      
      for(index=0; index<NCSSYSM_STKTRACE_INFO_MAX; index++)
        {
        if (me->line == mmgr.stktrace_info[index].line)
          {
          if(0 == strcmp(pfile, mmgr.stktrace_info[index].file))
            {
            me->stkentry = m_NCS_OS_MEMALLOC(sizeof(NCS_OS_STKTRACE_ENTRY), NCS_MEM_REGION_PERSISTENT);
            m_NCS_OS_STACKTRACE_GET(me->stkentry);
            me->stkentry->flags = 0;
            me->stkentry->ticks = m_NCS_OS_GET_TIME_MS;
              {
              NCS_OS_STKTRACE_ENTRY *ste;
              int count = 0;
              
              if(NULL == mmgr.stktrace_info[index].ste)
                {
                mmgr.stktrace_info[index].ste = me->stkentry;
                mmgr.stktrace_info[index].ste->next = NULL;
                }
              else
                {
                ste = mmgr.stktrace_info[index].ste;
                while(NULL != ste->next)
                  {
                  ste = ste->next;
                  count++;
                  }
                ste->next = me->stkentry;
                ste->next->next = NULL;
                }
              }
            }
          }
        }
      }
#endif /* SMM this is a critical region. Still Need to study this!! */
      
#if (NCS_MMGR_STACK_TRACE == 1)
      me->memory_owners = NULL;
#endif
      
#endif
      
      payload = (char *)me + sizeof(NCS_MPOOL_ENTRY);
      
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1)) 
     {
        uns32* bottom; /* Requester can't write below this! */
        
        bottom  = (uns32*)((char*)(payload + me->real_size));
        *bottom = BOTTOM_MARKER;
        }
        
#endif
        
#if(NCS_MMGR_STACK_TRACE == 1)
  gl_ptr = (void*)payload;
#endif
  
  ncslpg_give(&mmgr.lpg,0);
  return payload;
  }
  
  
 /****************************************************************************
  *
  * Function Name: ncs_mem_free
  *
  * Purpose:       Put memory back into the correct pool.
  *
  ****************************************************************************/
  
void
ncs_mem_free(void          *free_me,
    void          *mem_region,
    NCS_SERVICE_ID service_id,
    unsigned int  sub_id,
    unsigned int  line,
    char*         file)
    
    {
    NCS_MPOOL_ENTRY *me;
    NCS_MPOOL       *mp;
    time_t         tod;
    NCS_BOOL       free_mem_list = FALSE; 

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
    uns32          control_size = sizeof(NCS_MPOOL_ENTRY) + sizeof(uns32);
#endif
    
    USE(tod); 
    USE(mem_region);
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))
    USE(service_id);
    USE(sub_id);
#endif
    
    if (free_me == NULL)
      {
      m_LEAP_FAILURE(NCSFAIL_FREEING_NULL_PTR,0);
      return;
      }
    
    if ((me = (NCS_MPOOL_ENTRY*)(((char*)free_me) - sizeof(NCS_MPOOL_ENTRY))) == NULL)
      {
      m_LEAP_FAILURE(NCSFAIL_MEM_SCREWED_UP,0);
      return;
      }
    
    if (ncslpg_take(&mmgr.lpg) == FALSE)
      {
      m_NCS_OS_MEMFREE(me, mem_region);/* must be shut down; free back to OS */
      return; 
      }
    
    if ((mp = me->pool) == NULL)
      {
      m_LEAP_FAILURE(NCSFAIL_MEM_NULL_POOL,0);
      ncslpg_give(&mmgr.lpg, 0);
      return;
      }
    
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
    
#if (NCSSYSM_STKTRACE == 1)
      {
      int count = 0;
      int index;
      
      for(index=0; index<NCSSYSM_STKTRACE_INFO_MAX; index++)
        {
        if(NULL != mmgr.stktrace_info[index].ste)
          {
            {
            NCS_OS_STKTRACE_ENTRY *ste = mmgr.stktrace_info[index].ste;
            NCS_OS_STKTRACE_ENTRY * prev = NULL;
            NCS_OS_STKTRACE_ENTRY * delme = NULL;
            
            do
              {
              if(ste == me->stkentry)
                {
                delme = ste;
                if(NULL == prev)
                  {
                  mmgr.stktrace_info[index].ste = delme->next;
                  }
                else
                  {
                  prev->next = delme->next;
                  }
                ste = ste->next;
                m_NCS_OS_MEMFREE(delme, NCS_MEM_REGION_PERSISTENT);
                }
              else
                {
                prev = ste;
                ste = ste->next;
                }
              count++;
              }while(NULL != ste);
            }
            
          }
        }
      }
#endif
      
      
#if (NCS_MMGR_STACK_TRACE == 1)
      if(me->memory_owners)
        {
        m_NCS_OS_MEMFREE (me->memory_owners, mem_region);
        me->memory_owners = NULL;
        }
#endif
      
      if (me->from_pool_sz != mp->size)/* me & my referenced pool don't jive */
        m_LEAP_FAILURE(NCSFAIL_MEM_REC_CORRUPTED,0);
      
        {                      /* T E S T  lower boundary of returned memory */        
        uns32* bottom  = (uns32*)((char*)free_me + me->real_size);
        
        if (*bottom != BOTTOM_MARKER)
          {
          m_LEAP_FAILURE(NCSFAIL_FAILED_BM_TEST,0);
          ncslpg_give(&mmgr.lpg, 0);
          return;
          }
        }

      /* Does alloc and free ownership parties align?? */
        
      if ((me->service_id != service_id) || (me->sub_id != sub_id))
        {
        MEM_STAT_ERRORS(mp);
        m_LEAP_FAILURE(NCSFAIL_OWNER_CONFLICT,0);
        ncslpg_give(&mmgr.lpg, 0);
        return;
        }
        
        /* OK, adjust and cleanup DBG info before putting it back in pool */

        memset(free_me,0xff,mp->size);    /* set memory to generally bad value */
        me->free_line     = line;
        me->free_file     = file;
        
        me->prev_sub_id   = me->sub_id;
        me->prev_svc_id   = me->service_id;
        me->prev_loc_file = me->loc_file;
        me->prev_loc_line = me->loc_line;
        me->sub_id        = 0;
        me->service_id    = 0;
        me->loc_file      = gl_none;
        me->usr_data      = gl_none;
        me->loc_line      = 0;
        me->usr_data      = 0;
#if (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)
        m_GET_ASCII_TIME_STAMP(tod, me->asc_tod);
#endif
#endif
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))
        me->ignore      = TRUE;
#endif
        
        m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* Start critical region */
        
        if (me->owned == FALSE)
          {
          MEM_STAT_ERRORS(mp);
          m_LEAP_FAILURE(NCSFAIL_DOUBLE_DELETE,0); 
          m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
          ncslpg_give(&mmgr.lpg, 0);
          return;
          }
        
        me->owned  = FALSE;
        
        if (me->next != NULL)         /* splice it out of the inuse list */
          me->next->prev = me->prev;
        
        if (me->prev != NULL)
          me->prev->next = me->next;
        else
          mp->inuse = me->next;
        
        mp->outstanding_mem -= me->real_size;

        if (mp->size != 0)
        {
          /* splice it on to the end of the free list */
          if((mp->stat.num_avail <<2 >= mp->stat.num_inuse) && (mp->stat.num_avail > mp->stat.min_size))  
          {
              mmgr.stats.bytes_inuse  -= mp->size;
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))

              mmgr.stats.freed_to_os += mp->stat.size; 
              mmgr.stats.total_os_frees +=(mp->stat.size + control_size);
              mmgr.stats.freed += mp->stat.size;
#endif   
              free_mem_list = TRUE;          
              m_NCS_OS_MEMFREE (me, mem_region);
              MEM_STAT_FREED_TO_OS_PLUS(mp);
          }
          else
          {
              
              me->next = NULL; 
              if (mp->end != NULL)
              {
                  mp->end->next = me;
                  me->prev = mp->end;
              }
              else
              {
                  mp->free = me;
                  me->prev = NULL;
              }
              mp->end = me; /* In either case, I ('me') am the new end */
              
              MEM_STAT_AVAIL_PLUS(mp);
              mmgr.stats.bytes_avail += mp->size;
              mmgr.stats.bytes_inuse  -= mp->size;
              
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) )
              mmgr.stats.freed += mp->stat.size;
#endif
          }
        }
        else
        {
            mmgr.stats.bytes_inuse  -= me->real_size;
#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1))
            mmgr.stats.freed_to_os += me->real_size;
            mmgr.stats.total_os_frees +=(me->real_size + control_size);
            mmgr.stats.freed += me->real_size;
            mp->stat.bytes_inuse -= me->real_size; /* Bytes_inuse member applicable to this pool only */
            
#endif
            m_NCS_OS_MEMFREE (me, mem_region);
            MEM_STAT_FREED_TO_OS_PLUS(mp);
        }

        MEM_STAT_INUSE_MINUS(mp);
        MEM_STAT_FREES(mp);
        
        if(free_mem_list == TRUE)
        {
           while( ((mp->stat.num_avail <<2) >= mp->stat.num_inuse) && (mp->stat.num_avail > mp->stat.min_size))
             {
                   me= mp->free;
                   if(me != NULL)
                   {
                       mp->free  = me->next;
                       if (me->next != NULL)
                          me->next->prev = NULL;
                       if (mp->free == NULL)      /* SMM update free end pointer */
                          mp->end = NULL;
                   }
                    else
                   { 
                      break;
                   }

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_WATCH_ENABLE == 1) || (NCSSYSM_MEM_STATS_ENABLE == 1))

                   mmgr.stats.freed_to_os += mp->stat.size;
                   mmgr.stats.total_os_frees +=(mp->stat.size + control_size);
#endif
                   MEM_STAT_AVAIL_MINUS(mp);
                   MEM_STAT_FREED_TO_OS_PLUS(mp);
                   m_NCS_OS_MEMFREE (me, mem_region);
                   mmgr.stats.bytes_avail -=mp->size;
             }
        }

        m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* end critical region */
        ncslpg_give(&mmgr.lpg, 0);
  }
  
  /****************************************************************************
  *
  * Function Name: ncs_mem_realloc
  *
  * Purpose:       Realloc memory from the pool using ncs_mem_alloc/ncs_mem_free.
  *                Try to mimic the ansi realloc as much as possible:
  *                  if *payload == NULL then does a 'malloc'
  *                  if nbytes == 0 then does a 'free'
  *                  if *payload->size == nbytes then does nothing
  *                  if alloc fails, return NULL w/o toouching *payload
  *
  ****************************************************************************/
  void *
    ncs_mem_realloc(void *               payload,
                   unsigned int         nbytes,
                   void*                mem_region,
                   NCS_SERVICE_ID        service_id,
                   unsigned int         sub_id)
    {
    NCS_MPOOL_ENTRY *me;
    NCS_MPOOL       *mp;
    char           *temp_p = NULL;
    
    /* SMM  I dont see critical region in this function; it used to lock?! */
    
    /* payload is null - do a malloc */
    if (payload == NULL)
      {
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))
      temp_p = payload = m_NCS_MEM_ALLOC(nbytes, mem_region, service_id, sub_id);
#else
      temp_p = payload = m_NCS_MEM_ALLOC(nbytes, mem_region, 0, 0);
#endif
      }
    else
      {
      /* nbytes is 0 - do a free */
      if(nbytes == 0)
        {
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))
        m_NCS_MEM_FREE(payload, mem_region, service_id, sub_id);
#else
        m_NCS_MEM_FREE(payload, mem_region, 0, 0);
#endif
        temp_p = payload = NULL;/* assign the ptrs NULL for a little safety */
        }
      else
        {
        unsigned int nominal_size;
        me = (NCS_MPOOL_ENTRY*)(((char *)payload - sizeof(NCS_MPOOL_ENTRY)));
        assert(me->owned != FALSE);
        mp = me->pool;
        assert(mp != NULL);
        
        if (ncslpg_take(&mmgr.lpg) == FALSE)
          return NULL; 
        
       /* mp->size is the pool size (vs the requested size), so determine
          which pool nbytes is in and use that, nominal, size.  This
          behaviour probably differs slightly from an ansi realloc.
        */
        nominal_size = get_nominal_size(nbytes);
        
       /*  Note: mp->size (nominal_size) of zero has special meaning,
        *  which is, the payload resides outside the memory pool.
        *
        * (nominal_size != mp->size)  When mp->size is not 0 and this
        *      statement is true then we know that we are reallocing
        *  from one pool to another.
        *  (0 == mp->size ...) When this is try then we know know we
        *      are reallocing a memory block larger than the largest
        *      pool size.  So always execute this realloc block.
        */
        if((nominal_size != mp->size) || (0 == mp->size && nbytes != me->real_size))
          {
          
          /* finally do what you would expect this routine to do: alloc memory */
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))
          temp_p = m_NCS_MEM_ALLOC(nbytes, mem_region, service_id, sub_id);
#else
          temp_p = m_NCS_MEM_ALLOC(nbytes, mem_region, 0, 0);
#endif
          assert(temp_p != NULL);
         /* malloc succeded so swap the new ptr for the payload ptr.
          if the alloc fails, act like malloc by returning NULL,
          but leave payload ptr as it is.
          */
          if (NULL!= temp_p)
            {
            size_t copy_size;
           /* we always want to limit the copy size to the smaller of
            the two blocks, but when we're outside the pools we can't
            rely on mp->size to tell us how big the block is.  So use
            the new real_size member of mem_pool (mp) when mp->size
            is zero.
            */
            if(0 == mp->size)
              copy_size = (nbytes<me->real_size)?nbytes:me->real_size;
            else
              copy_size = (nbytes<mp->size)?nbytes:mp->size;
            memcpy(temp_p, payload, copy_size);
            
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))
            m_NCS_MEM_FREE(payload, mem_region, service_id, sub_id);
#else
            m_NCS_MEM_FREE(payload, mem_region, 0, 0);
#endif
            payload = temp_p;
            }
          }
          /*
          ** either the new size == the old size (if outside the pools)
          ** or the realloc stays within a pool, update the allocated
          ** size and return the original payload ptr.
          **
          ** If memory debugging, move the BOTTOM_MARKER to the
          ** new bottom.
          */
        else
          {
          temp_p = payload;
          me->real_size = nbytes;
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_MEM_DBG_ENABLE == 1))
            {
            uns32* bottom;
            
            bottom = (uns32*)((char*)((char *)payload + me->real_size));
            *bottom = BOTTOM_MARKER;
            }
#endif
          }
        }
      }
    
    assert(temp_p == payload);
    ncslpg_give(&mmgr.lpg,0);
    return temp_p;
}

/****************************************************************************
*
* Function Name: ncs_mem_dump_stats
*
* Purpose:       print in table form the current memory manager statistics.
*
* Parameters:    filename.  Name of file to dump stats to.  If NULL or "",
*                           Stats are dumped to the console.
*
* Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
*
****************************************************************************/
uns32
ncs_mem_stats_dump(char * filename)
  {
#if (NCS_MMGR_DEBUG == 1)
  char       output_string[128];
  char       asc_tod[33];
  time_t     tod;
  FILE      *fh = NULL;
  NCS_MPOOL  *mp;
  uns32 niu = 0;
  uns32 na  = 0;
  uns32 al  = 0;
  uns32 fr  = 0;
  uns32 er  = 0;
  uns32 hw  = 0;
  uns32 fa  = 0;
  uns32 ttl = 0;
  uns32 control_size = sizeof(NCS_MPOOL_ENTRY) + sizeof(uns32);
  
  if(filename != NULL) {
    if(m_NCS_STRLEN(filename) != 0) {
      fh = sysf_fopen( filename, "at");
      if(fh == NULL) {
        m_NCS_CONS_PRINTF("Unable to open %s\n", filename);
        return NCSCC_RC_FAILURE;
        }
      }
    }
  
  if (ncslpg_take(&mmgr.lpg) == FALSE)
    return NCSCC_RC_FAILURE; /* must be shut down; free back to OS */
  
  asc_tod[0] = '\0';
  m_GET_ASCII_TIME_STAMP(tod, asc_tod);
  sysf_sprintf(output_string, "%s\n", asc_tod);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  
  
  sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|        |    M E M  P O O L   D E T A I L S  (all are raw counts)      |\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|   pool +--------+--------+--------+--------+--------+--------+--------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|   size |  in-use|   avail|  hwater|  allocs|   frees|  errors|  falloc| freed to os\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  
  /* We are walking one pool at a time, so others are busy .........................*/
  
  for (mp = mmgr.mpools; ; mp++)
    {
    m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
    sysf_sprintf(output_string, "%9d%9d%9d%9d%9d%9d%9d%9d%9d\n",
      mp->stat.size,
      mp->stat.num_inuse,
      mp->stat.num_avail,
      mp->stat.highwater,
      mp->stat.allocs,
      mp->stat.frees,
      mp->stat.errors,
      mp->stat.failed_alloc,mp->stat.num_freed_to_os);
    fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
    
    /* last pool needs to be handled differently since pool size does not apply */
    if(mp->size == 0) {
      niu += mp->stat.bytes_inuse;
      hw  += mp->stat.bytes_inuse;
      ttl += mp->stat.bytes_inuse + (control_size * mp->stat.num_inuse);
      }
    else {
      niu  += (mp->stat.num_inuse * mp->stat.size);
      na   += (mp->stat.num_avail * mp->stat.size) ;
      hw   += (mp->stat.highwater * mp->stat.size);
      ttl  += (mp->stat.num_inuse * (mp->stat.size + control_size));
      ttl  += (mp->stat.num_avail * (mp->stat.size + control_size));
      }
    al   += mp->stat.allocs;
    fr   += mp->stat.frees;
    er   += mp->stat.errors;
    fa   += mp->stat.failed_alloc;
    
    m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
    
    if(mp->size == 0) break; /* last pool? break out !! */
    }
  
  
  sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|               S T A T I S T I C     T O T A L S                       |\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|   size |  in-use|   avail|  hwater|  allocs|   frees|  errors|  falloc|freed to os\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|   no op|   bytes|   bytes|   bytes| raw cnt| raw cnt| raw cnt| raw cnt|  bytes \n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "|--------+--------+--------+--------+--------+--------+--------+--------+-------------|\n");
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "%9d%9d%9d%9d%9d%9d%9d%9d%9d\n",0,niu,na,hw,al,fr,er,fa,mmgr.stats.freed_to_os);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);

  sysf_sprintf(output_string, "\nTotal bytes allocated (includes NCS_MPOOL_ENTRY overhead) %9d\n",ttl);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  sysf_sprintf(output_string, "Overhead size: %d\n\n", control_size);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);
  
  sysf_sprintf(output_string, "Total OS allocs :%9d  bytes Total Os frees :%9d  bytes \n",mmgr.stats.total_os_allocs,mmgr.stats.total_os_frees);
  fh==NULL ? m_NCS_CONS_PRINTF("%s", output_string) : sysf_fprintf (fh, output_string);


  if (fh)
    sysf_fclose(fh);
    
  ncslpg_give(&mmgr.lpg, 0);
  
#endif
  
  return NCSCC_RC_SUCCESS;
  }
  
  
  /****************************************************************************
  *
  * Function Name: ncs_mem_fetch_stats
  *
  * Purpose:       Put a copy of the current pool stats into the stats
  *                storage space instance passed in by the invoker.
  *
  * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
  *
  ****************************************************************************/
  
  uns32
    ncs_mem_fetch_stat (uns32 size, NCS_MPOOL_STAT *storage)
    {
#if (NCS_MMGR_DEBUG == 1)
    NCS_MPOOL  *mp;
    
    if (ncslpg_take(&mmgr.lpg) == FALSE)
      return NCSCC_RC_FAILURE; /* must be shut down; lets get outta here */
    
    if(size == 0)
      mp=mpool_fetch_last();
    else if ((mp = mpool_fetch (size)) == NULL)
      return ncslpg_give(&mmgr.lpg, NCSCC_RC_FAILURE);
    
    m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* critical regions */
    
    memcpy((void *)storage, (const void *)&mp->stat, sizeof(NCS_MPOOL_STAT));
    
    m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* end critical region */
    
    return ncslpg_give(&mmgr.lpg, NCSCC_RC_FAILURE);
    
#else
    
    USE(size);
    USE(storage);
    return NCSCC_RC_FAILURE;
    
#endif
    }



uns32
ncs_mem_whatsout_dump_custom(LEAP_PRINT_CALLBACK customptr,void *ucontext)
{
#if (NCS_MMGR_DEBUG != 0)

      uns32            i = 1;
      NCS_MPOOL        *mp;
      NCS_MPOOL_ENTRY  *me;


      if(customptr == NULL)
        return NCSCC_RC_FAILURE;

      if (ncslpg_take(&mmgr.lpg) == FALSE)
        return NCSCC_RC_FAILURE; /* Either not created yet or already shut down */

      (*customptr)(ucontext, "|---|----+----+-----------+-----+-----------+--------+---+---+----------------+----|\n");
      (*customptr)(ucontext, "|         N O N - I G N O R E D   O U T S T A N D I N G   M E M O R Y              |\n");
      (*customptr)(ucontext, "|---|----+----+-----------+-----+-----------+--------+---+---+----------------+----|\n");
      (*customptr)(ucontext, "|  #|show|Real|   alloc'ed|Owner|Ownr claims| User ID|Svc|Sub|                |bloc|\n");
      (*customptr)(ucontext, "|  #| Cnt|line|    in file| line|    in file|| marker| ID| ID|    Ptr value   |size|\n");
      (*customptr)(ucontext, "|---|----+----+-----------+-----+-----------+--------+---+---+----------------+----|\n");

      for (mp = mmgr.mpools; ; mp++)
      {
        m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
        me = mp->inuse;
        while (me != NULL)
        {
          ncs_mem_dump_entry(me, i++, 0, NULL, customptr, ucontext);
          me = me->next;
        }
        m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
        /* This conditional is moved out of the for loop so that
        the last pool (size 0) is included
        */
        if(mp->size == 0) break;
      }

      ncslpg_give(&mmgr.lpg, 0);

#endif
      /* if DBG is off, return 0; else return last slot filled */
      return NCSCC_RC_SUCCESS;
}
 
    /****************************************************************************
    *
    * Function Name: ncs_mem_whatsout
    *
    * Purpose:       take a snapshot of all memory that is currently in use
    *                within the scope of the pool manager. Also, do not include
    *                any memory that has been marked 'ignore' due to the function
    *                call ncs_mem_ignore(TRUE) being invoked. Put the information
    *                in the array of NCS_MPOOL_ENTRY_DBG. The array is 'slots'
    *                big. Do not exceed this count.
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *
  ****************************************************************************/
  
  uns32
    ncs_mem_whatsout(NCS_MPOOL_ENTRY_DBG* entry, uns32 slots)
    {

    uns32            i = 0;
#if (NCS_MMGR_DEBUG != 0)
    
    NCS_MPOOL        *mp;
    NCS_MPOOL_ENTRY  *me;
    
    if (ncslpg_take(&mmgr.lpg) == FALSE)
      return NCSCC_RC_FAILURE; /* must be shut down; lets get outta here */
    
    for (mp = mmgr.mpools; ; mp++)
      {
      m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);
      me = mp->inuse;
      while (me != NULL)
        {
        if (me->ignore == FALSE)
          {
          if (i++ == slots)
            {
            ncslpg_give(&mmgr.lpg, 0);
            m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* leaving */
            return --i; /* filled in all the slots; bail out */
            }
          entry->line       = me->line;
          entry->file       = me->file;
          entry->loc_line   = me->loc_line;
          entry->loc_file   = me->loc_file;
          entry->usr_data   = me->usr_data;
          entry->service_id = me->service_id;
          entry->sub_id     = me->sub_id;
          entry->prev_svc_id= me->prev_svc_id;
          entry->prev_sub_id= me->prev_sub_id;
          entry->memptr     = (char *)me + sizeof(NCS_MPOOL_ENTRY);
          entry->entrysize  = mp->size; /* SMM do I care about this?? */
          entry->count      = ++(me->count);
#if (NCS_MMGR_STACK_DEBUG == 1)
          entry->stack_str  = me->stack_str;
#endif
          entry++;
          }
        me = me->next;
        }
      m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); 
                                              
      /* Conditional outside the for loop so last pool (size 0) is included */

      if(mp->size == 0) break;
      }
   
    ncslpg_give(&mmgr.lpg, 0);
    
#endif
    
    /* if DBG is off, return 0; else return last slot filled */
    return i;
    }
  
  
#if (NCS_MMGR_DEBUG != 0)
    /****************************************************************************
    *
    * Function Name: ncs_next_mpool_entry
    *
    * Purpose:       This function returns the next in use memory pool entry
    *                following the one passed in as arg.  If arg is NULL, we
    *                start at the beginning of the first mpool.  If arg points
    *                to the last entry in a pool, we return the first entry in
    *                the next pool.  If arg is the last entry in the last pool,
    *                NULL is returned.
    *
    * Note:          The caller MUST have acquired the mpool lock!
    *
  ****************************************************************************/

  void *
    ncs_next_mpool_entry(void* arg)
    {
    NCS_MPOOL_ENTRY *me = (NCS_MPOOL_ENTRY *)arg;
    NCS_MPOOL       *mp;
    
    /*
    ** If arg is NULL, we start at the
    ** beginning of the first mpool.
    */
    if (me == (NCS_MPOOL_ENTRY *)NULL)
      {
      mp = mmgr.mpools;
      for (mp = mmgr.mpools; ; mp++)
        {
        if ((NCS_MPOOL_ENTRY *)NULL != (me = mp->inuse))
          return((void *)me);
          /* This conditional is moved out of the for loop so that
          the last pool (size 0) is included
        */
        if(mp->size == 0) break;
        }
        /* If there are no entries, return NULL
      */
      return((void *)NULL);
      
      }
    
      /*
      ** If there is a next in pool, return it.
    */
    if ((NCS_MPOOL_ENTRY *)NULL != me->next)
      return((void *)me->next);
    
      /*
      ** If we're at the last entry in the pool,
      ** return the first entry in the next pool.
    */
    if(me->from_pool_sz != 0)
      {
      mp = mpool_fetch(me->from_pool_sz);
      for (++mp; ;mp++)
        {
        if ((NCS_MPOOL_ENTRY *)NULL != (me = mp->inuse))
          return((void *)me);
          /* This conditional is moved out of the for loop so that
          the last pool (size 0) is included
        */
        if(mp->size == 0) break;
        }
      }
    
      /*
      ** Last entry of last pool.
      ** return NULL.
    */
    return((void *)NULL);
    }
#endif /* #if (NCS_MMGR_DEBUG != 0) */
  
  
    /****************************************************************************
    *
    * Function Name: ncs_mem_dump_entry
    *
    * Purpose:       This function does a pretty print of a single mempool
    *                entry, either into the provided buffer (you ensure that
    *                the buffer is large enough) or to the console if NULL.
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *                We assume pool has been locked by caller.
    *
  ****************************************************************************/

void
    ncs_mem_dump_entry(NCS_MPOOL_ENTRY  *me, uns32 i, unsigned int ignore_count, char *pBuf,LEAP_PRINT_CALLBACK ptr_custom ,void *ucontext)
    {
#if (NCS_MMGR_DEBUG == 1)
    char console_buf[1024];

    NCS_BOOL console = FALSE;
    NCS_MPOOL *mp;
#if(NCS_MMGR_STACK_TRACE == 1)
    char tmp[64];
    MEM_TRACE_RECORD* record;
    int      index;
#endif

    /* find pool */
    if(me->from_pool_sz == 0) {
      mp = mpool_fetch_last();
      }
    else {
      if ((mp = mpool_fetch (me->from_pool_sz)) == NULL)
        return;
      }

    if (pBuf == NULL)
      {
      pBuf = console_buf;
      console = TRUE;
      }

    if (me->ignore == TRUE)
      return;

    if (me->count < ignore_count) {
      ++me->count;
      return;
      }


#if (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)
    if (ignore_timestamp == 0)
      {
      sysf_sprintf (pBuf, "%4d%5d%5d%12s%6d%12s%9s%4d%4d%16lx%5d  %s\n",
        i++,                                /* Nmber */
        ++(me->count),                      /* count */
        me->line,                           /* Line  */

        ncs_fname(me->file),                 /* File  */
        me->loc_line,                       /* OwnrL */
        ncs_fname(me->loc_file),             /* OwnrF */
        me->usr_data,                       /* UsrD  */
        me->service_id,                     /* SvcID */
        me->sub_id,                         /* SubID */
        (long)((char *)me + sizeof(NCS_MPOOL_ENTRY)),/* Ptr   */
        mp->size == 0 ? me->real_size : mp->size,  /* Size  */
        me->asc_tod);
      }
    else
#endif
      {
      sysf_sprintf (pBuf, "%4d%5d%5d%12s%6d%12s%9s%4d%4d%16lx%5d\n",
        i++,                                /* Nmber */
        ++(me->count),                      /* count */
        me->line,                           /* Line  */
        ncs_fname(me->file),                 /* File  */
        me->loc_line,                       /* OwnrL */
        ncs_fname(me->loc_file),             /* OwnrF */
        me->usr_data,                       /* UsrD  */
        me->service_id,                     /* SvcID */
        me->sub_id,                         /* SubID */
        (long)((char *)me + sizeof(NCS_MPOOL_ENTRY)),/* Ptr   */
        mp->size == 0 ? me->real_size : mp->size);
      };

    /* report traced owners */
#if(NCS_MMGR_STACK_TRACE == 1)
    if(me->memory_owners)
      {
      for(index = 0; index < OWNERS_TRACED ; index ++)
        {
        record = (MEM_TRACE_RECORD*)(me->memory_owners + index * sizeof(MEM_TRACE_RECORD));
        if(record->transfer == 0)
          break;
        if(record->transfer == MEM_FROM_OWNER)
          sysf_sprintf(tmp, "\tFrom: %s at %d\n", record->file, record->line);
        else if(record->transfer == MEM_TO_OWNER)
          sysf_sprintf(tmp, "\tTo: %s at %d\n", record->file, record->line);
        m_NCS_STRCAT(pBuf, tmp);
        }
      }
#endif

    if (console == TRUE)
    {
      if(ptr_custom != NULL)
           (*ptr_custom)( ucontext, pBuf);
      else
           m_NCS_CONS_PRINTF (pBuf);
    }
#endif
    }



#if (NCS_MMGR_DEBUG != 0)

static void leap_mem_print(void *ucontext, char *payload)
{
    m_NCS_CONS_PRINTF(payload);
}
#endif 
    /****************************************************************************
    *
    * Function Name: ncs_mem_whatsout_dump
    *
    * Purpose:       This function is exactly like ncs_mem_whatsout(), except
    *                it sends the results to the console (printf() like).
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *
    ****************************************************************************/
    uns32
      ncs_mem_whatsout_dump()
      {
        uns32 rc = NCSCC_RC_SUCCESS; 

#if (NCS_MMGR_DEBUG != 0)
        rc = ncs_mem_whatsout_dump_custom(leap_mem_print, NULL);
#endif
        return rc;
      }

      /****************************************************************************
      *
      * Function Name: ncs_mem_whatsout_summary
      *
      * Purpose:       This function is exactly like ncs_mem_whatsout(), except
      *                it sends the results to the console (printf() like).
      *
      * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
      *
    ****************************************************************************/
    
    /* A helper struct to keep track of our hits */
    
    typedef struct ncs_mpool_hist
      {
      struct ncs_mpool_hist* next;
      NCS_MPOOL_ENTRY*       me;
      uns32                 hit_cnt;
      
      } NCS_MPOOL_HIST;
    
    /* We will use memory from the stack of this big */
    
#define MMGR_HIST_SPACE 3000
    
    
    /* The function itself.. put answer on console */
    
    uns32
      ncs_mem_whatsout_summary (char *mem_file)
      {
      
#if (NCS_MMGR_DEBUG != 0)
      
      FILE           *fh = (FILE *)NULL;
      
      NCS_BOOL         found;
      NCS_MPOOL*       mp;
      NCS_MPOOL_ENTRY* me;
      char            console_buf[1024];
      NCS_BOOL         console = TRUE;
      char           *pBuf = console_buf;
      NCSMEM_AID       ma;
      char            space[MMGR_HIST_SPACE];
      NCS_MPOOL_HIST*  hash[NCS_SERVICE_ID_MAX];
      NCS_MPOOL_HIST*  test;
      uns32           itr;
      
      
      if (mem_file != NULL)
        {
        if (NULL != (fh = sysf_fopen (mem_file, "at")))
          console = FALSE;

          /* If you can't open the file, dump to screen.*/
        }
      
      ncsmem_aid_init (&ma, (uns8*)space, MMGR_HIST_SPACE);
      
      memset (space, '\0', MMGR_HIST_SPACE);
      memset (hash,  '\0', sizeof(hash));
      
      if (ncslpg_take(&mmgr.lpg) == FALSE)
        return ((long) NULL); /* Either not created yet or already shut down */
      
      for (mp = mmgr.mpools; ; mp++)
        {
        m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE);    
        for (me = mp->inuse; me != NULL; me = me->next)
          {

          if (me->ignore == TRUE)
            continue;
          
          /* increment the number of times we've seen this memory */
          me->count++; 
          
          found = FALSE;
          
          
          /* OK, next line confuses some; ptr to first ptr so I know there is always test->next 
           */
          for (test = hash[me->service_id]; test != NULL; test = test->next)
            {
            if ( (test->me->line       == me->line)       && 
              (test->me->service_id == me->service_id) &&
              (test->me->sub_id     == me->sub_id)     && 
              (test->me->count      == me->count)      &&
              (strcmp(me->file,test->me->file) == 0) )
              {
              test->hit_cnt++;
              found = TRUE;
              break;
              }
            
            }
          
          if (found == FALSE)
            {
            if ((test = (NCS_MPOOL_HIST*)ncsmem_aid_alloc(&ma, sizeof(NCS_MPOOL_HIST))) == NULL)
              continue; /* means we have maxed out on 'reports';  You must have enough !!.. */
            test->next           = hash[me->service_id];
            test->me             = me;
            hash[me->service_id] = test;
            test->hit_cnt++;
            }
          }
        
        m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE);
        if(mp->size == 0) 
          break;
        }
      
      /** We are done with all pools, tell it your leaving **/
      
      ncslpg_give(&mmgr.lpg, 0);
      
      sysf_sprintf(pBuf, "%s%s%s%s%s%s",
        "|------+-----+-------------+------+----+----+-----|\n",
        "|         M E M  O U T   - S U M M A R Y          |\n",
        "|------+-----+-------------+------+----+----+-----|\n",
        "|hit   |show |   alloc'ed  |Owner |Svc |Sub |real |\n",
        "|Cnt   |Cnt  |    in file  | line | ID | ID |size |\n",
        "|------+-----+-------------+------+----+----+-----|\n"
        );
      
      if (fh)
        sysf_fprintf ( fh, pBuf);
      else
        m_NCS_CONS_PRINTF("%s", pBuf);
      
      
      
      for (itr = 0; itr < NCS_SERVICE_ID_MAX; itr++)
        {
        for (test = hash[itr]; test != NULL; test = test->next)
          {
          sysf_sprintf(pBuf, "|%6d|%5d|%13s|%6d|%4d|%4d|%5d|\n",
            test->hit_cnt,                   /* hits  */
            test->me->count,                 /* count */
            ncs_fname(test->me->file),        /* File  */
            test->me->line,                  /* Line  */
            test->me->service_id,            /* SvcID */
            test->me->sub_id,
            test->me->real_size);
          
          if (fh)
            sysf_fprintf ( fh, pBuf);
          else
            m_NCS_CONS_PRINTF("%s", pBuf);
          }
        }
      
      sysf_sprintf(pBuf,"|------+-----+-------------+------+----+----+-----|\n");
      
      if (fh)
        sysf_fprintf ( fh, pBuf);
      else
        m_NCS_CONS_PRINTF("%s", pBuf);
      
      if (fh)
        sysf_fclose(fh);
      
#endif
      
      return NCSCC_RC_SUCCESS;
      
  }
  
  
  
  /****************************************************************************
  *
  * Function Name: ncs_mem_ignore
  *
  * Purpose:       If the argument passed is TRUE, all currently in-use
  *                memory blocks are 'cloaked' to appear absent if the
  *                functions ncs_mem_whatsout() or ncs_mem_whatsout_pr() are
  *                invoked.
  *
  *                If FALSE is passed, any 'cloaked' memory is 'uncloaked'
  *                and again visible to the two functions mentioned above.
  *
  * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
  *
  ****************************************************************************/
  
  void
    ncs_mem_ignore(NCS_BOOL yes_r_no)
    {
#if (NCS_MMGR_DEBUG != 0)
    
    NCS_MPOOL        *mp;
    NCS_MPOOL_ENTRY  *me;
    
    if (ncslpg_take(&mmgr.lpg) == FALSE)
      return;
    
    for (mp = mmgr.mpools; ; mp++)
      {
      m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* critical region */
      me = mp->inuse;
      while (me != NULL)
        {
        me->ignore = yes_r_no;
        me = me->next;
        }
      m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* end, next bucket */
     
      /* This conditional is out of for loop so last pool (size 0) is included */

      if(mp->size == 0) break;
      }
    
    ncslpg_give(&mmgr.lpg, 0);
    
#endif
    
    }
  
  
    /****************************************************************************
    *
    * Function Name: ncs_mem_dbg_loc
    *
    * Purpose:       This function is invoked through the macro m_NCS_MEM_DBG_LOC.
    *                It marks the file and line that this macro is called from.
    *                It is intended to assist in solving memory leak bugs.
    *
    *                The idea is that a memory client may not be the allocator
    *                directly such that the allocation FILE/LINE values do not
    *                provide the insight to know who is responsible for this
    *                memory.
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *                The macro is empty if this flag is off, in which case there
    *                is no execution penalty associated with the macro.
  ****************************************************************************/
  
  void ncs_mem_dbg_loc(void  *ptr, unsigned int line, char* file)
    {

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
    NCS_MPOOL_ENTRY* me;
    
    if(ptr == NULL)
      return;
    
    if ((me = (NCS_MPOOL_ENTRY*)(((char*)ptr) - sizeof(NCS_MPOOL_ENTRY))) == NULL)
      return;
    me->loc_line = line;
    me->loc_file = file;
#endif
    }
  
    /****************************************************************************
    *
    * Function Name: ncs_mem_dbg_usr
    *
    * Purpose:       This function is invoked through the macro m_NCS_MEM_DBG_USR.
    *                It marks this memory record with user data of any kind.
    *                It is intended to assist in solving memory leak bugs.
    *
    *                The idea is that a memory client may want to mark the memory
    *                block with some telling piece of info marking some moment that
    *                will be helpful in determining where this memory is supposed
    *                to be deleted.
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *                The macro is empty if this flag is off, in which case there
    *                is no execution penalty associated with the macro.
  ****************************************************************************/
  void ncs_mem_dbg_usr(void *ptr, char* usrdata)
    {
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1))
    NCS_MPOOL_ENTRY* me;
    
    if ((me = (NCS_MPOOL_ENTRY*)(((char*)ptr) - sizeof(NCS_MPOOL_ENTRY))) == NULL)
      return;
    me->usr_data = usrdata;

#endif
    }
  

  
    /****************************************************************************
    *
    * Function Name: ncs_fname
    *
    * Purpose:       isolate just the file name out of a file path string.
    *
  ****************************************************************************/
  char* ncs_fname(char* fpath)
    {
    char* str;
    int   len;
    
    if (fpath == NULL)
      return "<NONE>";
    
    /* What we do to get pretty output SM */
    
    len = strlen(fpath);
    str = fpath + (len - 3); /* '.c' assumed */
    
    while ((*str  >= 'A' && *str <= 'Z') ||
      (*str  >= 'a' && *str <= 'z') ||
      (*str  >= '0' && *str <= '9') ||
      (*str == '_'))
      {
      str--;
      if (str < fpath) /* in case preceeding memory has (coincidental) chars */
        break;
      }
    return ++str;
    }
  
  
    /****************************************************************************
    *
    * Function Name: ncs_mem_ignore_subsystem
    *
    * Purpose:       If the argument passed is 1, memory blocks in-use
    *                by the specified subsystem are not displayed if the
    *                functions ncs_mem_whatsout() or ncs_mem_whatsout_pr() are
    *                invoked.
    *
    *                If 0 is passed, memory blocks in-use
    *                by the specified subsystem will be displayed if the
    *                two functions mentioned above are called.
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *
  ****************************************************************************/
  
  void
    ncs_mem_ignore_subsystem(unsigned int zero_or_one, unsigned int service_id)
    {
#if (NCS_MMGR_DEBUG != 0)
    
    if (service_id < (unsigned int)NCS_SERVICE_ID_MAX)
      {
      if (zero_or_one > 0)
        zero_or_one = 1;
      };
    
#endif
    }
  
  
    /****************************************************************************
    *
    * Function Name: ncs_mem_ignore_timestamp
    *
    * Purpose:       If the argument passed is 1, the timestamp
    *                associated with memory blocks in-use
    *                by the specified subsystem is not displayed if the
    *                functions ncs_mem_whatsout() or ncs_mem_whatsout_pr() are
    *                invoked.
    *
    *                If 0 is passed, the timestamp associated
    *                with memory blocks in-use
    *                by the specified subsystem will be displayed if the
    *                two functions mentioned above are called.
    *
    * Note:          This is only meaningful if NCS_MMGR_DEBUG is enabled.
    *
  ****************************************************************************/
  
  void
    ncs_mem_ignore_timestamp(unsigned int zero_or_one)
    {
#if (NCS_MMGR_DEBUG != 0) && (NCS_MMGR_ENABLE_PRINT_TIMESTAMP == 1)
    
    if (zero_or_one > 0)
      ignore_timestamp = 1;
    else
      ignore_timestamp = 0;
#else
    USE(zero_or_one);
#endif
    }
  
  
    /****************************************************************************
    *
    * ncs_mem_hex_dump_entry() dump the contents of memory in HEX value
    *
  ****************************************************************************/
#define BYTES_PER_LINE     16
  char*  ncs_mem_hex_dump_entry(NCSCONTEXT address)
    {

#if (NCS_MMGR_DEBUG == 1)
    NCS_MPOOL_ENTRY*   me = NULL;
    NCS_MPOOL*         mp;
    char*             buffer = NULL;
    char              tmp[32];
    uns32             size;
    uns32             index;
    int               padding;
    char*             dump_ptr;
    char              c;
    
    /* search for the entry */
    for (mp = mmgr.mpools; mp->size != 0; mp++)
      {
      m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* lock this particular pool */
      me = mp->inuse;
      while (me != NULL)
        {
        dump_ptr = (char *)me + sizeof(NCS_MPOOL_ENTRY);
        
        /* Got the requested entry */
        if(dump_ptr == address)
          {
          size = mp->size == 0 ? me->real_size : mp->size;
          buffer = m_NCS_MEM_ALLOC(6 * size , 0,0,0);
          if(!buffer)
            return NULL;
          
          buffer[0] = 0;
          
          /* report entry contents */
          for(index = 0; index < size; index ++)
            {
            /* print the address of this line */
            if(index % BYTES_PER_LINE == 0)
              {
              sprintf(tmp, "0x%16lx  ",(long)(dump_ptr + index));
              m_NCS_STRCAT(buffer, tmp);
              }
            
            /* print the hex format of the character */
            sprintf(tmp, "%02x ", (*(dump_ptr + index) & 0XFF) );
            m_NCS_STRCAT(buffer, tmp);
            
            /* One line of hex is done, print characters */
            if((index + 1) % BYTES_PER_LINE == 0 || index + 1 == size)
              {
              padding = BYTES_PER_LINE;
              if(index + 1 == size && size % BYTES_PER_LINE != 0)
                {
                /* padding */
                padding = BYTES_PER_LINE - (index + 1) % BYTES_PER_LINE;
                
                while(padding-- > 0)
                  {
                  /* 3 spaces */
                  m_NCS_STRCAT(buffer, "   ");
                  }
                
                padding = (index + 1) % BYTES_PER_LINE;
                }
              
              /* print characters */
              padding --;
              while(padding >= 0)
                {
                c = *(dump_ptr + index - padding);
                if(isalnum(c))
                  sprintf(tmp, "%c", c);
                else
                  sprintf(tmp, "%c", '.');
                
                m_NCS_STRCAT(buffer, tmp);
                padding --;
                }
              m_NCS_STRCAT(buffer, "\n");
              }
            m_NCS_STRCAT(buffer, "\n");
            }
          m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* unlock this particular pool */
          return buffer;
          }
        me = me->next;
        }
      m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* unlock this particular pool */
      }
#endif     
    return NULL;
    }

  
#if (NCS_MMGR_DEBUG == 1)

#if(NCS_MMGR_STACK_TRACE == 1)
    /****************************************************************************
    *
    * Function to trace chain of files which have memory ownership.       
    *
    ****************************************************************************/
    void ncs_mem_dbg_trace_owner(void* ptrin, int transfer, unsigned int line, char* file)
      {
      NCS_MPOOL_ENTRY    *me;
      MEM_TRACE_RECORD  *record;
      int               index;
      void*             ptr = ptrin;
      
      /* Set the global memory pointer to NULL */
      if(transfer == MEM_SET_NULL)
        {
        gl_ptr = NULL;
        return;
        }
      
      /* Need a memory pointer */
      if(ptr == NULL)
        ptr = gl_ptr;
      
      if(ptr == NULL)
        return;
      
      if((me = (NCS_MPOOL_ENTRY*)(((char*)ptr) - sizeof(NCS_MPOOL_ENTRY))) == NULL)
        return;
      
      /* alloc buffer */
      if(me->memory_owners == NULL)
        {
        me->memory_owners = (char*)m_NCS_OS_MEMALLOC(OWNERS_TRACED * sizeof(MEM_TRACE_RECORD), 0);
        if(me->memory_owners)
          memset (me->memory_owners, 0, OWNERS_TRACED * sizeof(MEM_TRACE_RECORD));
        }
      
      /* record information in an empty slot */
      if(me->memory_owners)
        {
        for(index = 0; index < OWNERS_TRACED; index ++)
          {
          record = (MEM_TRACE_RECORD*)(me->memory_owners + index * sizeof(MEM_TRACE_RECORD));
          if(record->transfer == 0)
            {
            record->transfer = (unsigned short)transfer;
            record->line = (unsigned short)line;
            if(m_NCS_STRLEN(file) >= OWNER_NAME)
               m_NCS_STRNCPY(record->file, file + m_NCS_STRLEN(file) - OWNER_NAME + 1, OWNER_NAME-1);
            else
               m_NCS_STRNCPY(record->file, file , OWNER_NAME-1);
            record->file[OWNER_NAME-1] = 0;
            break;
            }
          }
        }
      }
#endif /* NCS_MMGR_STACK_TRACE == 1 */

#endif /* NCS_MMGR_DEBUG == 1*/    
    
uns64 ncs_mem_outstanding(void)
{
    NCS_MPOOL        *mp;
    uns64 temp_mem_outstanding = 0;

    if (ncslpg_take(&mmgr.lpg) == FALSE)
      return 0;

    for (mp = mmgr.mpools; ; mp++)
    {
      m_NCS_LOCK (&mp->lock, NCS_LOCK_WRITE); /* critical region */

      temp_mem_outstanding += mp->outstanding_mem;

      m_NCS_UNLOCK (&mp->lock, NCS_LOCK_WRITE); /* end, next bucket */
     
      /* This conditional is out of for loop so last pool (size 0) is included */
    
      if(mp->size == 0) break;
    }
 
    ncslpg_give(&mmgr.lpg, 0);
    return temp_mem_outstanding;
}
    
void  ncs_mpool_entry_dump(NCSCONTEXT payload_address)
{
#if (NCS_MMGR_DEBUG == 1)
    NCS_MPOOL_ENTRY    *me;
    uns8* mem_addr = (uns8 *)payload_address;

    if (mem_addr == NULL)
    {
       m_LEAP_FAILURE(NCSFAIL_FREEING_NULL_PTR,0);
       return;
    }

    if ((me = (NCS_MPOOL_ENTRY*)(((char*)mem_addr) - sizeof(NCS_MPOOL_ENTRY))) == NULL)
    {
       m_LEAP_FAILURE(NCSFAIL_MEM_SCREWED_UP,0);
       return;
    }

    m_NCS_CONS_PRINTF(" start_marker   0x%x \n",me->start_marker);
    m_NCS_CONS_PRINTF(" from_pool_size %5d \n",me->from_pool_sz);

    m_NCS_CONS_PRINTF(" free_line      %5d \n",me->free_line);

    m_NCS_CONS_PRINTF(" free_file      %s \n",ncs_fname(me->free_file));
    m_NCS_CONS_PRINTF(" line           %5d \n",me->line);
    m_NCS_CONS_PRINTF(" file           %s \n",ncs_fname(me->file));
    m_NCS_CONS_PRINTF(" loc_line       %5d \n",me->loc_line);
    m_NCS_CONS_PRINTF(" loc_file       %s \n",ncs_fname(me->loc_file));
    m_NCS_CONS_PRINTF(" prev_svc_id    %5d \n",me->prev_svc_id);
    m_NCS_CONS_PRINTF(" prev_sub_id    %5d \n",me->prev_sub_id);
    m_NCS_CONS_PRINTF(" prev_loc_line  %5d \n",me->prev_loc_line);
    m_NCS_CONS_PRINTF(" prev_loc_file  %s \n",ncs_fname(me->prev_loc_file));
    m_NCS_CONS_PRINTF(" service id     %4d \n",me->service_id);
    m_NCS_CONS_PRINTF(" sub id         %4d \n",me->sub_id);
    m_NCS_CONS_PRINTF(" usr data       0x%lx \n",(long)me->usr_data);
    m_NCS_CONS_PRINTF(" ignore         %4d \n", me->ignore);
    m_NCS_CONS_PRINTF(" count          %4d \n",++(me->count));
    m_NCS_CONS_PRINTF(" age            %4d \n",me->age);
    m_NCS_CONS_PRINTF(" next           0x%lx \n",(long)me->next);
    m_NCS_CONS_PRINTF(" prev           0x%lx \n",(long)me->prev);

    m_NCS_CONS_PRINTF(" pool           0x%lx \n",(long)me->pool);
    m_NCS_CONS_PRINTF(" owned          %4d \n",me->owned);
    m_NCS_CONS_PRINTF(" real_size      %5d \n",me->real_size);
    m_NCS_CONS_PRINTF(" mpool entry    0x%lx \n",(long)me);

    /* T E S T  lower boundary of returned memory */
    uns32* bottom  = (uns32*)((char*)mem_addr + me->real_size); 
    if(bottom)
       m_NCS_CONS_PRINTF(" bottom marker   0x%x \n",*bottom);

#endif
    return;
}    
