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




@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains code to allocate and free memory objects that are
  used by the Soft-ATM product suite.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  sysf_alloc_pkt....................Allocate a packet buffer
  sysf_free_pkt.....................Free a packet buffer
  sysf_ditto_pkt....................Duplicate a USRBUF chain
  sysf_get_chain_len................Calculate the data length of a bufr-chain
  sysf_reserve_at_end...............Append data space to a bufr-chain.
  sysf_remove_from_end..............Remove bytes (freeing buffers if nec.) from end of bufr-chain
  sysf_reserve_at_start.............Prepend data space to a bufr-chain.
  sysf_remove_from_start............Remove bytes (freeing buffers if nec.) from start of bufr-chain
  sysf_data_at_end..................Calculate pointer to contiguous data at end of buffer chain
  sysf_data_at_start................Calculate pointer to contiguous data at start of buffer chain
  sysf_calc_usrbuf_cksum_1s_comp....Calculates the checksum value of a USRBUF and stores it in a given variable
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/


/** H&J Includes...
 **/
#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_sysm.h"
#include "ncs_svd.h"
#include "ncssysfpool.h"
#include "ncssysf_def.h"
#include "ncssysf_mem.h"
#include "usrbuf.h"
#include "ncsusrbuf.h"
#include "sysf_def.h"
#include "sysf_ipc.h"

/**********************************************************************************/
/*                       L E A P   B U F F E R    P O O L                         */
/**********************************************************************************/        

#include "ncs_stack.h"

#if (NCS_USE_SYSMON == 1)

#define LBP_NO_LOCKS   0   /* No Lock     == 0 */
#define LBP_OBJ_LOCKS  1   /* Object Lock == 1 */
#define LBP_TASK_LOCKS 2   /* Task Lock   == 2 */

#ifndef NCSLBP_USE_LOCK_TYPE
#define NCSLBP_USE_LOCK_TYPE LBP_OBJ_LOCKS
#endif

#if (NCSLBP_USE_LOCK_TYPE == LBP_NO_LOCKS)                  /* NO Locks */

#define m_LBP_LK_CREATE(lk)
#define m_LBP_LK_INIT
#define m_LBP_LK(lk) 
#define m_LBP_UNLK(lk) 
#define m_LBP_LK_DLT(lk) 

#elif (NCSLBP_USE_LOCK_TYPE == LBP_TASK_LOCKS)            /* Task Locks */

#define m_LBP_LK_CREATE(lk)
#define m_LBP_LK_INIT            m_INIT_CRITICAL
#define m_LBP_LK(lk)             m_START_CRITICAL
#define m_LBP_UNLK(lk)           m_END_CRITICAL
#define m_LBP_LK_DLT(lk) 

#elif (NCSLBP_USE_LOCK_TYPE == LBP_OBJ_LOCKS)           /* Object Locks */


#define m_LBP_LK_CREATE(lk)      m_NCS_LOCK_INIT_V2(lk,0,0)
#define m_LBP_LK_INIT
#define m_LBP_LK(lk)             m_NCS_LOCK_V2(lk,   NCS_LOCK_WRITE,0, 0)
#define m_LBP_UNLK(lk)           m_NCS_UNLOCK_V2(lk, NCS_LOCK_WRITE, 0, 0)
#define m_LBP_LK_DLT(lk)         m_NCS_LOCK_DESTROY_V2(lk, 0, 0)

#endif 

extern UB_POOL_MGR gl_ub_pool_mgr;

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))                      
/* debugging is ENABLED - include debugging args */

#define m_NCS_LBP_ALLOC(nbytes, pri) \
          ncs_lbp_alloc(nbytes, pri, __LINE__, __FILE__)

#define m_NCS_LBP_FREE(free_me) \
          ncs_lbp_free((void *)(free_me), __LINE__, __FILE__)

#else
/* debugging is DISABLED - don't include debugging args */

#define m_NCS_LBP_ALLOC(nbytes, pri)  \
          ncs_lbp_alloc(nbytes, pri)

#define m_NCS_LBP_FREE(free_me)  \
          ncs_lbp_free((void *)(free_me))

#endif

#if (NCS_USE_SYSMON == 1)
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
#define m_NCS_LBP_RPT(arg) \
          ncs_lbp_rpt(arg)
#define m_NCS_LBP_IGNORE(val) \
          ncs_lbp_ignore(val) 
#define m_NCS_LBP_AGE ncs_lbp_age()
#else
#define m_NCS_LBP_RPT(arg) NCSCC_RC_SUCCESS
#define m_NCS_LBP_IGNORE(val)
#define m_NCS_LBP_AGE 
#endif
#endif

#if ((NCS_MMGR_DEBUG == 1) || (NCSSYSM_MEM_DBG_ENABLE == 1) || (NCSSYSM_BUF_DBG_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
struct ub_pool_mgr;
EXTERN_C LEAPDLL_API struct ub_pool_mgr gl_ub_pool_mgr;
#endif


#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))
#define m_NCS_LBP_LOC(mem,svc_id,sub_id,line,file) \
          ncs_lbp_loc(mem,svc_id,sub_id,line,file)
#else
#define m_NCS_LBP_LOC(mem,svc_id,sub_id,line,file)
#endif

/* Pad the size of the thing allocated to an even boundary for WBS machines */

#if (TRGT_IS_WBS == 0)

#define MMGR_PAD_SIZE(s) s
#else
#define MMGR_PAD_SIZE(s) (s%4?(s+(4-(s%4))):s)

#endif

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))

#define BOTTOM_MARKER  0x35353535 /* marker; printable characters "5555" */
#define START_MARKER   0x33333333 /* marker; printable characters "3333" */

#endif

typedef struct ncs_lbp_entry
{

#if ((NCS_MMGR_DEBUG  == 1) || (NCSSYSM_BUF_DBG_ENABLE == 1))

  /* Debug data galore for giving insight into memory leaks/crashes/etc. */

  uns32         start_marker; /* Start of structure marker 0x33333333 */
#endif
#if ((NCS_MMGR_DEBUG  == 1) || (NCSSYSM_BUF_DBG_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
  uns32         free_line;    /* file & line of free-er */
  char*         free_file;

  uns32         line;         /* file & line of allocation */
  char*         file;

  uns32         loc_line;     /* file & line marked by 'real' owner */
  char*         loc_file;

  NCS_SERVICE_ID prev_svc_id;  /* the previous owner of this mem */
  unsigned int  prev_sub_id;

  NCS_SERVICE_ID service_id;   /* the current owner fo this mem */
  unsigned int  sub_id;

  NCS_BOOL       ignore;       
  uns32         age;          /* pulse age */
#endif

  /* The REAL stuff when DBG is OFF */

  struct ncs_lbp_entry*    next;
  struct ncs_lbp_entry*    prev;
  struct ncs_lbpool*       pool;
  NCS_BOOL                 owned;
  size_t                  real_size;
  USRDATA*                data;

} NCS_LBP_ENTRY;

typedef struct ncs_lbpool
{
  NCS_BOOL       exists;
  uns32         size;
  NCS_LBP_ENTRY* free;
  NCS_LBP_ENTRY* inuse;

} NCS_LBPOOL;

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))
EXTERN_C void*          ncs_lbp_alloc      (unsigned int  nbytes,
                                           unsigned int  pri,
                                           unsigned int  line,
                                           char*         file);

EXTERN_C void           ncs_lbp_free       (void          *free_me,
                                           unsigned int  line,
                                           char*         file);

EXTERN_C void ncs_lbp_loc(void* mem,uns32 svc_id,uns32 sub_id,uns32 line,char* file);

static char gl_none[5] = "NONE"; /* loc_file name before assignment */

#else

EXTERN_C void*          ncs_lbp_alloc      (unsigned int nbytes, unsigned int pri);
EXTERN_C void           ncs_lbp_free       (void *free_me);

#endif


#if (NCS_USE_SYSMON == 1)
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
EXTERN_C uns32 ncs_lbp_rpt  (NCSUB_POOL_RPT_ARG* arg);

EXTERN_C void ncs_lbp_ignore(NCS_BOOL val);

EXTERN_C void ncs_lbp_age   (void);

uns32 lbp_rpt_wos   (NCSSYSM_BUF_RPT_WOS*   info); 
uns32 lbp_rpt_wo    (NCSSYSM_BUF_RPT_WO*    info);

#endif
#endif



NCS_LBPOOL leap_buf_pool = {TRUE,MMGR_PAD_SIZE(sizeof(USRDATA)),
NULL,
NULL
};


NCS_LOCK   lp_pool_lock;

static NCS_BOOL lb_pool_lock_inited = FALSE;


/*****************************************************************************/

#if (NCSSYSM_BUF_STATS_ENABLE == 1)

uns32 ncs_lbp_rpt(NCSUB_POOL_RPT_ARG* arg)
{
  switch(arg->op)
  {
  case   NCSUB_POOL_RPT_OP_WO:
      return lbp_rpt_wo  (arg->info.wo);
    break;

  case NCSUB_POOL_RPT_OP_WOS:
      return lbp_rpt_wos (arg->info.wos); 
    break;

  default:
    break;
  }
  return NCSCC_RC_SUCCESS;
}

void ncs_lbp_ignore(NCS_BOOL val)
{
  NCS_LBP_ENTRY  *pe;
  
  pe = leap_buf_pool.inuse;
  while (pe != NULL)
  {
    pe->ignore = val;
    pe = pe->next;
  }
}

void ncs_lbp_age(void)
{
  NCS_LBP_ENTRY  *pe;
  
    pe = leap_buf_pool.inuse;
    while (pe != NULL)
    {
      pe->age++;
      pe = pe->next;
    }
}

#endif

/********************/

#if (NCSSYSM_BUF_STATS_ENABLE == 1)

extern char* ncs_fname(char* fpath);

uns32 lbp_rpt_wo  (NCSSYSM_BUF_RPT_WO*    info)
{
  NCS_LBP_ENTRY*   pe;
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[256];
  char            asc_tod[33];
  time_t          tod;
  uns32           j;
  uns32           ss_id = 0;
  NCS_BOOL         all_ss = FALSE;
  uns32           min_age = 0;

  memset(info->o_wo,0,sizeof(info->o_wo));

  min_age = info->i_age;
  if(info->i_ss_id == 0)
    all_ss = TRUE;
  else
    ss_id = info->i_ss_id;

  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;

  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(strlen(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            printf("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }

    
    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sprintf(output_string, "%s\n", asc_tod);
    fh==NULL ? printf("%s", output_string) : fprintf (fh, output_string);
    
 
    sprintf(output_string, "|---|----+----+-----------+-----+-----------+--------+---+---+----------|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
    sprintf(output_string, "|  N O N - I G N O R E D   O U T S T A N D I N G   B U F   R E P O R T  |\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
    sprintf(output_string, "|---|----+----+-----------+-----+-----------+--------+---+---+----------+------|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
    sprintf(output_string, "|  #| Age|Real|   alloc'ed|Owner|Ownr claims|  Real  |Svc|Sub|          |RefCnt|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
    sprintf(output_string, "|  #| Cnt|line|    in file| line|    in file|  size  | ID| ID| Ptr value|      |\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
    sprintf(output_string, "|---|----+----+-----------+-----+-----------+--------+---+---+----------+------|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);

      for(pe = leap_buf_pool.inuse,j = 0;pe != NULL;pe = pe->next)
      {
        if(pe->ignore == TRUE)
          continue;
        if((all_ss == FALSE) && ((uns32)pe->service_id != ss_id))
          continue;
        if(pe->age < min_age)
          continue;

      sprintf (output_string, "%4d%5d%5d%12s%6d%12s%9d%4d%4d%10x%6d\n",
                        j + 1,                                /* Nmber */
                        pe->age,                            /* age */
                        pe->line,                           /* Line  */
                        ncs_fname(pe->file),                 /* File  */
                        pe->loc_line,                       /* OwnrL */
                        ncs_fname(pe->loc_file),             /* OwnrF */
                        pe->real_size,                      /* Real Size  */
                        pe->service_id,                     /* SvcID */
                        pe->sub_id,                         /* SubID */
                        (int)((char *)pe + sizeof(NCS_LBP_ENTRY)),/* Ptr   */
                        pe->data->RefCnt /* refCnt */);


      if(j < NCSSYSM_BUF_MAX_WO)
      {
        info->o_wo[j].o_aline = pe->line;
        info->o_wo[j].o_afile = (uns8*)ncs_fname(pe->file);
        info->o_wo[j].o_oline = pe->loc_line;
        info->o_wo[j].o_ofile = (uns8*)ncs_fname(pe->loc_file);
        info->o_wo[j].o_svcid = pe->service_id;
        info->o_wo[j].o_subid = pe->sub_id;
        info->o_wo[j].o_age   = pe->age;
        info->o_wo[j].o_ptr = (NCSCONTEXT)((char *)pe + sizeof(NCS_LBP_ENTRY));
        info->o_wo[j].o_refcnt = pe->data->RefCnt;
        info->o_filled = j + 1;
      }

      j++;

          if(to_std == TRUE) printf("%s", output_string);
          if(to_file == TRUE) fprintf (fh, output_string);

      }

    
      if (((to_file == TRUE)) && fh) 
      {
        sysf_fclose(fh);
      }
      
  }

  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }

  return NCSCC_RC_SUCCESS;
}
#endif 


#if (NCSSYSM_BUF_STATS_ENABLE == 1)
/* A helper struct to keep track of our hits */

typedef struct ncs_lbp_hist
{
  struct ncs_lbp_hist* next;
  NCS_LBP_ENTRY*       pe;
  uns32               hit_cnt;

} NCS_LBP_HIST;

/* We will use memory from the stack of this big */

#define LBP_HIST_SPACE 3000

uns32 lbp_rpt_wos (NCSSYSM_BUF_RPT_WOS*   info) 
{
  NCS_BOOL         found;
  NCSMEM_AID       ma;
  char            space[LBP_HIST_SPACE];
  NCS_LBP_HIST*    hash[NCS_SERVICE_ID_MAX];
  NCS_LBP_HIST*    test;
  uns32           itr;

  NCS_LBP_ENTRY*   pe;
  NCS_BOOL         to_std = FALSE;
  NCS_BOOL         to_file = FALSE;
  char            output_string[128];
  char            asc_tod[33];
  time_t          tod;
  uns32           j;
  uns32           ss_id = 0;
  NCS_BOOL         all_ss = FALSE;
  uns32           min_age = 0;

  memset(info->o_wos,0,sizeof(info->o_wos));

  ncsmem_aid_init (&ma, (uns8*)space, LBP_HIST_SPACE * sizeof(char));
  
  memset (space, '\0', LBP_HIST_SPACE * sizeof(char));
  memset (hash,  '\0', sizeof(hash));

  min_age = info->i_age;
  if(info->i_ss_id == 0)
    all_ss = TRUE;
  else
    ss_id = info->i_ss_id;

  to_std = info->i_opp.opp_bits & OPP_TO_STDOUT ? TRUE : FALSE;
  to_file = info->i_opp.opp_bits & OPP_TO_FILE ? TRUE : FALSE;

  if((to_std == TRUE) || (to_file == TRUE))
  {
    FILE* fh = NULL;
    
    if(to_file == TRUE)
    {  
      if(info->i_opp.fname != NULL)
      {
        if(strlen(info->i_opp.fname) != 0) 
        {
          fh = sysf_fopen( info->i_opp.fname, "at");
          if(fh == NULL) 
          {
            printf("Unable to open %s\n", info->i_opp.fname);
            return NCSCC_RC_FAILURE;
          }
        }
        else
        {
          return NCSCC_RC_FAILURE;
        }
      }
    }
   
      j = 0;
      for (pe = leap_buf_pool.inuse;pe != NULL;pe = pe->next)
      {
        if(pe->ignore == TRUE)
          continue;
        if((all_ss == FALSE) && ((uns32)pe->service_id != ss_id))
          continue;
        if(pe->age < min_age)
          continue;

      found = FALSE;
      

      /** OK, next line confuses some; ptr to first ptr so I know there is always test->next 
       **/
      for (test = hash[pe->service_id]; test != NULL; test = test->next)
      {
        if ( (test->pe->line       == pe->line)       && 
             (test->pe->service_id == pe->service_id) &&
             (test->pe->sub_id     == pe->sub_id)     && 
             (test->pe->age        == pe->age)      &&
             (strcmp(pe->file,test->pe->file) == 0) )
        {
          test->hit_cnt++;
          found = TRUE;
          break;
        }
        
      }

      if (found == FALSE)
      {
        if ((test = (NCS_LBP_HIST*)ncsmem_aid_alloc(&ma, sizeof(NCS_LBP_HIST))) == NULL)
          continue; /* means we have maxed out on 'reports';  You must have enough !!.. */
        test->next           = hash[pe->service_id];
        test->pe             = pe;
        hash[pe->service_id] = test;
        test->hit_cnt++;
      }

      }

    
    asc_tod[0] = '\0';
    m_GET_ASCII_TIME_STAMP(tod, asc_tod);
    sprintf(output_string, "%s\n", asc_tod);
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
    

  sprintf(output_string, "%s","|------+-----+-------------+------+----+----+-----+-----|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
  sprintf(output_string, "%s","|         B U F  O U T   - S U M M A R Y          |\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
  sprintf(output_string, "%s","|------+-----+-------------+------+----+----+-----+-----|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
  sprintf(output_string, "%s","|hit   |show |   alloc'ed  |Owner |Svc |Sub |real |bkt  |\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
  sprintf(output_string, "%s","|Cnt   |Cnt  |    in file  | line | ID | ID |size |size |\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
  sprintf(output_string, "%s","|------+-----+-------------+------+----+----+-----+-----|\n");
    if(to_std == TRUE) printf("%s", output_string);
    if(to_file == TRUE) fprintf (fh, output_string);
 
    
  if(to_std == TRUE) printf("%s", output_string);
  if(to_file == TRUE) fprintf (fh, output_string);

  j = 0;
  for (itr = 0; itr < NCS_SERVICE_ID_MAX; itr++)
  {
    for (test = hash[itr]; test != NULL; test = test->next)
    {
   
     sprintf(output_string, "|%6d|%5d|%13s|%6d|%4d|%4d|%5d|%5d|\n",
       test->hit_cnt,                   /* hits  */
       test->pe->age,                   /* age */
       ncs_fname(test->pe->file),        /* File  */
       test->pe->line,                  /* Line  */
       test->pe->service_id,            /* SvcID */
       test->pe->sub_id,
       test->pe->real_size, /* real size */
       test->pe->pool->size /* bucket size */);

     if(j < NCSSYSM_BUF_MAX_WOS)
     {
       info->o_wos[j].o_inst = test->hit_cnt;
       info->o_wos[j].o_age = test->pe->age;
       info->o_wos[j].o_aline = test->pe->line;
       info->o_wos[j].o_afile = (uns8*)ncs_fname(test->pe->file);
       info->o_wos[j].o_svcid = test->pe->service_id;
       info->o_wos[j].o_subid = test->pe->sub_id;
       info->o_filled = j + 1;
       j++;
     }

     if(to_std == TRUE) printf("%s", output_string);
     if(to_file == TRUE) fprintf (fh, output_string);
    }
  }
   
  sprintf(output_string,"|------+-----+-------------+------+----+----+-----+-----|\n");
  if(to_std == TRUE) printf("%s", output_string);
  if(to_file == TRUE) fprintf (fh, output_string);

      if (((to_file == TRUE)) && fh) 
      {
        sysf_fclose(fh);
      }
      
  }

  if(info->i_opp.opp_bits & OPP_TO_MASTER)
  {
    /* TBD */
  }  

  return NCSCC_RC_SUCCESS;
}
#endif  
/********************/
#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))

void ncs_lbp_loc(void* mem,uns32 svc_id,uns32 sub_id,uns32 line,char* file)
{
  NCS_LBP_ENTRY*    pe;

  if(mem == NULL)
    return;

  if ((pe = (NCS_LBP_ENTRY*)(((char*)mem) - sizeof(NCS_LBP_ENTRY))) == NULL)
    {
    m_LEAP_FAILURE(NCSFAIL_MEM_SCREWED_UP,0);
    return;
    }

  pe->prev_svc_id = pe->prev_svc_id;
  pe->service_id = svc_id;
  pe->prev_sub_id = pe->sub_id;
  pe->sub_id = sub_id;
  pe->loc_line = line;
  pe->loc_file = file;

}

#endif

/****************************************************************************
 *
 * Function Name: ncs_lbp_create
 *
 * Purpose:       put leap buffer pool (reference implementation) into start
 *                state.
 *
 ****************************************************************************/

uns32
ncs_lbp_create (void)
  {
     if(lb_pool_lock_inited == FALSE)
     {
      m_LBP_LK_CREATE(&lp_pool_lock);
      lb_pool_lock_inited = TRUE;
      }
      

    if(leap_buf_pool.exists == FALSE)
    {
      leap_buf_pool.size = MMGR_PAD_SIZE(sizeof(USRDATA));
      leap_buf_pool.free = NULL;
      leap_buf_pool.inuse = NULL;
      leap_buf_pool.exists = TRUE;
    }

  return NCSCC_RC_SUCCESS;
  }

/****************************************************************************
 *
 * Function Name: ncs_lbp_destroy
 *
 * Purpose:       Recover all memory resources in the H&J memory pool.
 *
 ****************************************************************************/

uns32
ncs_lbp_destroy (void)
  {
  NCS_LBP_ENTRY *pe;

  if (leap_buf_pool.exists != TRUE)
    return NCSCC_RC_FAILURE;

  if(lb_pool_lock_inited == TRUE)
  {
    m_LBP_LK_DLT(&lp_pool_lock);
    lb_pool_lock_inited = FALSE;
  }

    while ((pe = leap_buf_pool.free) != NULL)
      {
      leap_buf_pool.free  = pe->next;
      if (pe->next != NULL)
        pe->next->prev = NULL;
      m_NCS_OS_MEMFREE (pe, NULL); 
      }

  leap_buf_pool.exists = FALSE;

  return NCSCC_RC_SUCCESS;
  }

/****************************************************************************
 *
 * Function Name: ncs_lbp_alloc
 *
 * Purpose:       Allocate memory. If free memory
 *                is not in that pool, go to the HEAP and get some.
 *
 *                If debug is enabled, store lots of stuff about the
 *                invoker.
 *
 ****************************************************************************/

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))
void *
ncs_lbp_alloc(unsigned int  nbytes,
             unsigned int  pri,
             unsigned int  line,
             char*         file)
#else

void *
ncs_lbp_alloc(unsigned int nbytes, unsigned int pri)

#endif
  {
  NCS_LBP_ENTRY*   pe;
  unsigned int    len;
  char*           payload;
  m_LBP_LK_INIT;

  USE(pri);

  if(nbytes > sizeof(USRDATA))
    return NULL;

  m_LBP_LK(&lp_pool_lock);

  if ((pe = leap_buf_pool.free) == NULL)
    {
    m_LBP_UNLK(&lp_pool_lock);
    len = leap_buf_pool.size + sizeof(NCS_LBP_ENTRY) + sizeof(uns32);

    if ((pe = (NCS_LBP_ENTRY *)m_NCS_OS_MEMALLOC(len, NULL)) == NULL)
      {
      return NULL;
      }

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))

    pe->prev_sub_id = 0; /* we clear it out once; from then on, we record */
    pe->prev_svc_id = 0;
    pe->start_marker = START_MARKER;                      /* mark it once */
    pe->free_file    = gl_none;             /* starts off with no free-er */
    pe->free_line    = 0;

#endif

    pe->owned = FALSE;
    m_LBP_LK(&lp_pool_lock);
    if(leap_buf_pool.exists == FALSE)
    {
      /* LBP has been destroyed */
      m_NCS_OS_MEMFREE(pe,NULL);
      m_LBP_UNLK(&lp_pool_lock);
      return NULL;
    }
    }
  else if (pe->owned == TRUE)
    {
    m_NCSSYSM_BUF_ASSERT( 0 );
    m_LBP_UNLK(&lp_pool_lock);
    return NULL;
    }
  else
    {
    leap_buf_pool.free  = pe->next;
    if (pe->next != NULL)
      pe->next->prev = NULL;
    }

  pe->owned     = TRUE;
  pe->next      = leap_buf_pool.inuse;
  leap_buf_pool.inuse = pe;
  pe->prev      = NULL;
  pe->real_size = MMGR_PAD_SIZE(nbytes);

  if (pe->next != NULL)
    pe->next->prev = pe;

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))

  pe->loc_file     = gl_none;
  pe->loc_line     = 0;
  pe->service_id   = 0;
  pe->sub_id       = 0;
  pe->line         = line;
  pe->file         = file;

#endif

#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  pe->ignore       = FALSE;
  pe->age        = 0;
#endif

  m_LBP_UNLK(&lp_pool_lock);

  payload = (char *)pe + sizeof(NCS_LBP_ENTRY);
  pe->data = (USRDATA*)payload;

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1)) /* Requester can't write below this! */
    {
    uns32* bottom;

    bottom  = (uns32*)((char*)(payload + pe->real_size));
    *bottom = BOTTOM_MARKER;
    }

#endif

  return (void *)payload;
  }


/****************************************************************************
 *
 * Function Name: ncs_lbp_free
 *
 * Purpose:       Put memory back into the leap buffer pool.
 *
 ****************************************************************************/

#if ((NCS_MMGR_DEBUG != 0) || (NCSSYSM_BUF_DBG_ENABLE == 1))
void
ncs_lbp_free(void*          free_me,
            unsigned int   line,
            char*          file)

#else

void
ncs_lbp_free(void *free_me)

#endif
  {
  NCS_LBP_ENTRY *pe;
  m_LBP_LK_INIT;

  if (free_me == NULL)
    {
    m_LEAP_FAILURE(NCSFAIL_FREEING_NULL_PTR,0);
    return;
    }

  if ((pe = (NCS_LBP_ENTRY*)(((char*)free_me) - sizeof(NCS_LBP_ENTRY))) == NULL)
    {
    m_LEAP_FAILURE(NCSFAIL_MEM_SCREWED_UP,0);
    return;
    }

  m_LBP_LK(&lp_pool_lock);

  if (leap_buf_pool.exists != TRUE)
    {
    /** if this occurs, then the Leap Buffer Pool has been destroyed.
    ** Free the memory back to the OS...
    **/
    m_NCS_OS_MEMFREE(pe,NULL);
    m_LBP_UNLK(&lp_pool_lock);
    return;
    }

  if (pe->owned == FALSE)
    {
    m_LEAP_FAILURE(NCSFAIL_DOUBLE_DELETE,0);
    m_LBP_UNLK(&lp_pool_lock);
    return;
    }

  pe->owned  = FALSE;

#if (NCSSYSM_BUF_DBG_ENABLE == 1)

    {                      /* T E S T  lower boundary of returned memory */

    uns32* bottom  = (uns32*)((char*)free_me + pe->real_size);

    if (*bottom != BOTTOM_MARKER)
      {
      m_LEAP_FAILURE(NCSFAIL_FAILED_BM_TEST,0);
      m_LBP_UNLK(&lp_pool_lock);
      return;
      }
    }

  memset(free_me,0xff,leap_buf_pool.size);    /* set memory to generally bad value */
  pe->free_line   = line;
  pe->free_file   = file;

  pe->prev_sub_id = pe->sub_id;
  pe->prev_svc_id = pe->service_id;
  pe->sub_id      = 0;
  pe->service_id  = 0;
  pe->loc_file    = gl_none;
  pe->loc_line    = 0;
#endif

#if (NCSSYSM_BUF_STATS_ENABLE == 1)
  pe->ignore      = TRUE;
#endif

  if (pe->next != NULL)
    pe->next->prev = pe->prev;

  if (pe->prev != NULL)
    pe->prev->next = pe->next;
  else
    leap_buf_pool.inuse = pe->next;


  pe->next = leap_buf_pool.free;
  leap_buf_pool.free = pe;
  pe->prev = NULL;
  
  if (pe->next != NULL)
    pe->next->prev = pe;

  m_LBP_UNLK(&lp_pool_lock);
  
  }


#if (NCSSYSM_BUF_DBG_ENABLE == 1)

/****************************************************************************
 *
 * Function Name: ncs_ub_dbg_loc
 *
 * Purpose:       This function is invoked through the macro m_BUFR_STUFF_OWNER.
 *                Each UserBuf in its UserBuf chain will be stuffed with the
 *                file and line that this macro is called from
 *                It is intended to assist in solving memory leak bugs.
 *
 ****************************************************************************/
  void ncs_ub_dbg_loc(USRBUF* ub, uns32 l, char* f)
    {
    while (ub != NULL)
      {
      ncs_mem_dbg_loc(ub,l,f);
      ub = ub->next;
      }
    }

/****************************************************************************
 *
 * Function Name: ncs_buf_dbg_loc
 *
 * Purpose:       This function is invoked through the macro m_NCS_BUF_DBG_LOC.
 *                It marks the file and line that this macro is called from.
 *                It is intended to assist in solving memory leak bugs.
 *
 ****************************************************************************/

void* ncs_buf_dbg_loc(void  *ptr,int svc_id,int sub_id, unsigned int line, char* file)
  {
  NCS_MPOOL_ENTRY* me;

  if ((me = (NCS_MPOOL_ENTRY*)(((char*)ptr) - sizeof(NCS_MPOOL_ENTRY))) == NULL)
    return ptr;
  me->loc_line    = line;
  me->loc_file    = file;
  me->prev_svc_id = me->service_id;
  me->service_id  = svc_id;
  me->prev_sub_id = me->sub_id;
  me->sub_id      = sub_id;

  if(gl_ub_pool_mgr.pools[(((USRBUF*)ptr)->pool_ops->pool_id)].mem_loc != NULL)
    gl_ub_pool_mgr.pools[(((USRBUF*)ptr)->pool_ops->pool_id)].mem_loc((((USRBUF*)ptr)->payload),svc_id,sub_id,line,file);

  if (((USRBUF*)ptr)->next != NULL)
    ncs_buf_dbg_loc(((USRBUF*)ptr)->next, svc_id, sub_id, line, file);
  return ptr;
  }
#endif

#endif /* #if (NCS_USE_SYSMON == 1) */


/*************************************************************************************/


/*
 * CUSTOMER (interested in arguments for pool_id and priority)
 *
 * PRIORITY: urgency of need for requested memory;
 * POOL_ID : where the memory is to come from. Pools will probably
 *           reflect different needs by different Messaging services.
 * 
 * IMPLEMENTATION
 *
 * This NetPlane implementation does not do anything meaningful with 
 * the priority. The pool_id is used to dispatch to different memory
 * sources. Note however that most NetPlane subsystems do not care and
 * default to pool_id == 0, which maps to heap (a la sysfpool.c).
 *
 * The customer is invited to replace, adjust or embelish this reference
 * implementation,
 *
 * The following crude policy/implementation is reflected here:
 * - The pool_id is used to govern from which memory pool the USRDATA
 *   shall be allocated from. The USRBUF always comes from heap ( a
 *   la sysfpool.c).
 * - Once a USRBUF is created, all subsequent chained USRBUFs (that is,
 *   USRDATAs) will come from the same pool_id.
 * - Once a USRBUF is created of any priority, all subsequent 
 *   chained USRBUFs will be allocated at HIGH priority. That is,
 *   once a USRBUF transaction is started, the system is committed
 *   to following through by providing all subsequent needed system
 *   resources.
 * 
 * LEAP documentation has an Application note that reviews issues
 * associated with such a target system strategy. This implementation
 * is slightly different than that depicted in that application note.
 */



/***************************************************************************

 Various places USRDATA memory can come from; the pools

 ***************************************************************************/

/***************************************************************************
 * NCSUB_LEAP_POOL
 *
 * Default USRBUF Pool allocate/free function for default pool_id = 0
 ***************************************************************************/

void* sysf_leap_alloc(uns32 b,uns8 pool_id, uns8 pri)
{ 
  USE(pool_id); 
#if (NCS_USE_SYSMON == 1)
  return m_NCS_LBP_ALLOC(b,pri);
#else
  return m_NCS_MEM_ALLOC(b,NULL,NCS_SERVICE_ID_OS_SVCS,0);
#endif
}

void sysf_leap_free(void *data,uns8 pool_id)
{ 
#if (NCS_USE_SYSMON == 1)  
  m_NCS_LBP_FREE(data);
#else
  m_NCS_MEM_FREE(data,NULL,NCS_SERVICE_ID_OS_SVCS,0);
#endif
}

#if (NCS_USE_SYSMON == 1)
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

EXTERN_C uns32 sysf_leap_rpt   (NCSUB_POOL_RPT_ARG* arg);
EXTERN_C void  sysf_leap_loc   (void* mem,uns32 svc_id,uns32 sub_id,uns32 line,char* file);
EXTERN_C void  sysf_leap_ignore(NCS_BOOL val);
EXTERN_C void  sysf_leap_age   (void);

uns32 sysf_leap_rpt(NCSUB_POOL_RPT_ARG* arg)
{ return m_NCS_LBP_RPT(arg);}

void sysf_leap_loc(void* mem,uns32 svc_id,uns32 sub_id,uns32 line,char* file)
{ m_NCS_LBP_LOC(mem,svc_id,sub_id,line,file);}

void sysf_leap_ignore(NCS_BOOL val)
{ m_NCS_LBP_IGNORE(val); }

void sysf_leap_age(void)
{ m_NCS_LBP_AGE; }
#endif
#endif

/***************************************************************************
 * NCSUB_HEAP_POOL
 *
 * USRBUF Pool allocate/free function for heap; assumed mapping of OS_PRIMS
 ***************************************************************************/

void* sysf_heap_alloc( uns32 b,uns8 pool_id, uns8 pri)
  { USE(pool_id); USE(pri); return m_NCS_OS_MEMALLOC(b,NULL); }

void sysf_heap_free(void *data,uns8 pool_id)
  { m_NCS_OS_MEMFREE(data,NULL);}

/***************************************************************************
 * ERROR_POOL Functions
 *
 * Dummy USRBUF Pool alloc/free function for uninitialized pool references
 ***************************************************************************/

void* sysf_stub_alloc(uns32 b,uns8 pool_id, uns8 pri)
  {
    m_LEAP_DBG_SINK(0);
    return NULL;
  }

void sysf_stub_free(void *data,uns8 pool_id)
  { m_LEAP_DBG_SINK_VOID(NCSCC_RC_FAILURE);}


/***************************************************************************
 * GLOBAL gl_ub_pool_mgr set to default for pool 0 for backward 
 *        compatability, since NetPlane init sequences do not invoke
 *        NCS_MMGR_OSS_LM_OP_INIT.
 ***************************************************************************/

UB_POOL_MGR gl_ub_pool_mgr = {
/*--------+---------------+----------------+----------------*/
/*   busy |  pool_id      |   alloc func   |   free func    */
/*--------+---------------+----------------+----------------*/
  {
    /* Pool-id 0 */
#if (NCS_USE_SYSMON == 1)
    {TRUE, NCSUB_LEAP_POOL, sysf_leap_alloc, sysf_leap_free, 0, 0,
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

      ,sysf_leap_age,sysf_leap_ignore,sysf_leap_loc,sysf_leap_rpt,{0xffffffff,NCSUB_LEAP_POOL,0,0,0,0,0}
#endif
#else
    {TRUE, NCSUB_LEAP_POOL, sysf_leap_alloc, sysf_leap_free, 0, 0,
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

      ,NULL,NULL,NULL,NULL,{0xffffffff,NCSUB_LEAP_POOL,0,0,0,0,0}
#endif
#endif
    },
    /* Pool-id 1 */
    {TRUE, NCSUB_HEAP_POOL, sysf_heap_alloc, sysf_heap_free, 0, 0,
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

      ,NULL,NULL,NULL,NULL,{0xffffffff,NCSUB_HEAP_POOL,0,0,0,0,0}
#endif    
    },
    /* Pool-id 2 */
    {TRUE, NCSUB_UDEF_POOL, m_OS_UDEF_ALLOC, m_OS_UDEF_FREE, 0, 0,
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

      ,NULL,NULL,NULL,NULL,{0xffffffff,NCSUB_UDEF_POOL,0,0,0,0,0}
#endif     
    },
    /* Pool-id 3 : FOR MDS : PM-23/Jan/2005
       Header break up for fragmented MDS messages - 

       +--------------------+
       | ETH-HEADER = 14    |
       +--------------------+
       | FRAME-LEN  = 2     |   => Since ETHERNET FRAME has min 64 byte len
       +--------------------+
       | LOOP-BACK  = 4     |   => To find and discard looped back messages
       +--------------------+
       | RCP-SIGN   = 4     |   => RCP packet signature to eliminate alien packets
       +--------------------+
       | MDS-PROT   = 1     |
       +--------------------+
       | MDS-TYPE   = 1     |
       +--------------------+
       | FINAL-ADEST= 8     |
       +--------------------+
       | FROM-ADEST = 8     |
       +--------------------+
       | FRAG-NUM   = 4     |
       +--------------------+
       | MORE-FLAG  = 1     |
       +--------------------+

       So using a value of 50

       Trailer break up - RCP Header = 8 bytes (I think)

       So using a value of 25
       */
#define MDS_UB_HDR_MAX 50 
#define MDS_UB_TRLR_MAX 25
#if (NCS_USE_SYSMON == 1)
    {TRUE, NCSUB_MDS_POOL, sysf_leap_alloc, sysf_leap_free, MDS_UB_HDR_MAX, MDS_UB_TRLR_MAX, 

#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
      ,sysf_leap_age,sysf_leap_ignore,sysf_leap_loc,sysf_leap_rpt,{0xffffffff,NCSUB_MDS_POOL,0,0,0,0,0}
#endif /* #if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1)) */

    },
#else
    {TRUE, NCSUB_MDS_POOL, sysf_leap_alloc, sysf_leap_free, MDS_UB_HDR_MAX, MDS_UB_TRLR_MAX, 

#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
      ,NULL,NULL,NULL,NULL,{0xffffffff,NCSUB_MDS_POOL,0,0,0,0,0}
#endif /* #if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1)) */

    },
#endif /* else of #if (NCS_USE_SYSMON == 1) */
    /* Pool-id 4 */
    {FALSE,0,              sysf_stub_alloc, sysf_stub_free, 0, 0},
    }
  };

/***************************************************************************
 *
 * uns32 mmgr_ub_svc_init
 *
 * Description:
 *   put the USRBUF Pool Manager in start state by putting in default
 * allocation/free function pointers.
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS
 *
 ****************************************************************************/

static uns32 
mmgr_ub_svc_init(NCSMMGR_UB_INIT* init)
{
  uns16 i;

  m_PMGR_LK_INIT;

  m_PMGR_LK_CREATE(&gl_ub_pool_mgr.lock);

  m_PMGR_LK(&gl_ub_pool_mgr.lock);

  for(i = 1; i < UB_MAX_POOLS;i++)
    {
    /* If default set at compile time, pass it buy */

    if (gl_ub_pool_mgr.pools[i].busy != TRUE) 
      {
      gl_ub_pool_mgr.pools[i].busy      = FALSE;
      gl_ub_pool_mgr.pools[i].pool_id   = 0;
      gl_ub_pool_mgr.pools[i].mem_alloc = sysf_stub_alloc;
      gl_ub_pool_mgr.pools[i].mem_free  = sysf_stub_free;
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
      memset(&gl_ub_pool_mgr.pools[i].stats,0,sizeof(gl_ub_pool_mgr.pools[i].stats));
#endif
      }
    }

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

  return NCSCC_RC_SUCCESS;
}

/***************************************************************************
 *
 * uns32 mmgr_ub_svc_delete
 *
 * Description:
 *   Free up all USRBUF Pool Manager resources, being the lock in this 
 *   implementation.
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS
 *
 ****************************************************************************/

static uns32 
mmgr_ub_svc_delete(NCSMMGR_UB_DELETE* del)
{
  m_PMGR_LK_DLT(&gl_ub_pool_mgr.lock);

  return NCSCC_RC_SUCCESS;
}


/***************************************************************************
 *
 * uns32 mmgr_ub_svc_register
 *
 * Description:
 *   Find an empty slot in the pool properties array and install pool memory
 *   function pointers and mark it occupied.
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS
 *
 ****************************************************************************/

static uns32 
mmgr_ub_svc_register(NCSMMGR_UB_REGISTER* reg)
{
  NCSUB_POOL*   pool;

  m_PMGR_LK_INIT;

  if (reg->i_pool_id > UB_MAX_POOLS)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  pool = &gl_ub_pool_mgr.pools[reg->i_pool_id];

  if (pool->busy == TRUE)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  m_PMGR_LK(&gl_ub_pool_mgr.lock);

  pool->busy      = TRUE;
  pool->pool_id   = reg->i_pool_id;
  pool->mem_alloc = reg->i_mem_alloc;
  pool->mem_free  = reg->i_mem_free;
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
  pool->mem_age    = reg->i_mem_age;
  pool->mem_ignore = reg->i_mem_ignore;
  pool->mem_loc    = reg->i_mem_loc;
  pool->mem_rpt    = reg->i_mem_rpt;
#endif

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

  return NCSCC_RC_SUCCESS;
}

/***************************************************************************
 *
 * uns32 mmgr_ub_svc_deregister
 *
 * Description:
 *   Find the identified pool and replace the malloc and free with dummy
 *   malloc and free function pointers, so if used, errors will be generated.
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS
 *
 ****************************************************************************/

static uns32 
mmgr_ub_svc_deregister(NCSMMGR_UB_DEREGISTER* dereg)
{
  NCSUB_POOL*   pool;

  m_PMGR_LK_INIT;

  if (dereg->i_pool_id > UB_MAX_POOLS)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  pool = &gl_ub_pool_mgr.pools[dereg->i_pool_id];

  if (pool->busy == FALSE)
    return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);

  m_PMGR_LK(&gl_ub_pool_mgr.lock);

  pool->busy      = FALSE;
  pool->pool_id   = 0;
  pool->mem_alloc = sysf_stub_alloc;
  pool->mem_free  = sysf_stub_free;

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

  return NCSCC_RC_SUCCESS;
}

/***************************************************************************
 *
 * uns32 ncsmmgr_ub_lm
 *
 * Description:
 *   Single entry point for USRBUF pool managment services.
 *
 * Returns:
 *   Returns NCSCC_RC_SUCCESS
 *
 ****************************************************************************/

uns32 
ncsmmgr_ub_lm( NCSMMGR_UB_LM_ARG* arg ) 
{
  switch(arg->i_op)
    {  
    case NCSMMGR_LM_OP_INIT:
      return mmgr_ub_svc_init(&arg->info.init);

    case NCSMMGR_LM_OP_DELETE:
      return mmgr_ub_svc_delete(&arg->info.del);

    case NCSMMGR_LM_OP_REGISTER:
      return mmgr_ub_svc_register(&arg->info.reg);

    case NCSMMGR_LM_OP_DEREGISTER:
      return mmgr_ub_svc_deregister(&arg->info.dereg);

    default:
      break;
    }

  return m_LEAP_DBG_SINK(NCSCC_RC_FAILURE);
}


/***************************************************************************
 *
 * NCSUB_POOL* ncsmmgr_ub_getpool
 *
 * Description:
 *   Given a pool_id, fetch the associated NCSUB_POOL object
 *
 * Returns:
 *   Returns NULL, if does not align or not-present
 *           ptr   to NCSUB_POOL, if pool is available
 *
 ****************************************************************************/

NCSUB_POOL* ncsmmgr_ub_getpool(uns8 pool_id)
  {
  NCSUB_POOL* answer = NULL; /* init to default 'bad' answer */
  m_PMGR_LK_INIT;

  m_PMGR_LK(&gl_ub_pool_mgr.lock);
  if ((pool_id < UB_MAX_POOLS) && (gl_ub_pool_mgr.pools[pool_id].busy == TRUE))
    answer = &gl_ub_pool_mgr.pools[pool_id];
  else
    m_LEAP_DBG_SINK_VOID((long)NULL);

  m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
  return answer;
  }

/***********************************************************************/

/***************************************************************************
 *  sysf_alloc_pkt  
 ****************************************************************************/
USRBUF *
sysf_alloc_pkt( unsigned char pool_id, unsigned char priority, int num, unsigned int line, char* file)
{

  USRBUF *ub;
  USRDATA *ud;
  USE(priority);
  USE(num);

  m_PMGR_LK_INIT;


  ub = (USRBUF*)m_NCS_MEM_ALLOC(sizeof(USRBUF),
                               NCS_MEM_REGION_IO_DATA_HDR,
                               NCS_SERVICE_ID_OS_SVCS, 2);

  if (ub != (USRBUF*)0)
  {
      m_PMGR_LK(&gl_ub_pool_mgr.lock);

      if(pool_id >= UB_MAX_POOLS)
        {
        m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
        m_LEAP_DBG_SINK(0);
        return NULL;
        }        
     
#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

      if((gl_ub_pool_mgr.pools[pool_id].stats.curr + sizeof(USRDATA)) >= 
          gl_ub_pool_mgr.pools[pool_id].stats.max)
      {
        NCSSYSM_RES_LMT_EVT evt; 
        evt.i_vrtr_id = gl_sysmon.vrtr_id; 
        evt.i_obj_id = NCSSYSM_OID_BUF_TTL;
        evt.i_ttl = gl_ub_pool_mgr.pools[pool_id].stats.max;
        evt.i_attr = LEAM_INST_ID;
        evt.i_inst_id = pool_id;
        if(gl_sysmon.res_lmt_cb == NULL)
        (*gl_sysmon.res_lmt_cb)(&evt);

          m_NCS_MEM_FREE(ub, NCS_MEM_REGION_IO_DATA_HDR,
                        NCS_SERVICE_ID_OS_SVCS, 2);
          ub = (USRBUF*)0;
          m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
          return ub;
      }
     
#endif

      ud = (USRDATA*)gl_ub_pool_mgr.pools[pool_id].mem_alloc(sizeof(USRDATA),pool_id,priority);
      
      if (ud == (USRDATA*)NULL)
      {
          m_NCS_MEM_FREE(ub, NCS_MEM_REGION_IO_DATA_HDR,
                        NCS_SERVICE_ID_OS_SVCS, 2);
          ub = (USRBUF*)0;
          m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
      }
      else
      {

#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))

      if(gl_ub_pool_mgr.pools[pool_id].mem_loc != NULL)
        gl_ub_pool_mgr.pools[pool_id].mem_loc(ud,NCS_SERVICE_ID_OS_SVCS,2,__LINE__,__FILE__);

      gl_ub_pool_mgr.pools[pool_id].stats.curr += sizeof(USRDATA);
#endif
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
      gl_ub_pool_mgr.pools[pool_id].stats.alloced += sizeof(USRDATA);

      gl_ub_pool_mgr.pools[pool_id].stats.cnt++;
      if(gl_ub_pool_mgr.pools[pool_id].stats.curr > gl_ub_pool_mgr.pools[pool_id].stats.hwm)
      {
        gl_ub_pool_mgr.pools[pool_id].stats.hwm = gl_ub_pool_mgr.pools[pool_id].stats.curr;
      }

#endif

          /* STUFFing owner corrupts OSE alloc'ed mem; must comment out next line */
          /* m_BUFR_STUFF_OWNER(ud, line, file); */

          /* Set up data fields... */
          ud->RefCnt = 1;

          /* Set up USRBUF fields... */
          ub->payload  = ud;
          ub->pool_ops = &gl_ub_pool_mgr.pools[pool_id];
          m_PMGR_UNLK(&gl_ub_pool_mgr.lock);

          m_MMGR_NEXT(ub) = (USRBUF*)0;
          ub->link        = (USRBUF*)0;
          ub->count = 0;
#if (NCS_SIGL == 1)
          m_MMGR_SET_NPS(ub, 0);
          m_MMGR_SET_RTS(ub, 0);
#endif

          ub->start = gl_ub_pool_mgr.pools[pool_id].hdr_reserve;

#if (NCSSYSM_BUF_DBG_ENABLE == 1)
          m_BUFR_STUFF_OWNER(ub);
#endif
      }
  }
  return ub;

}

/***************************************************************************
 *  sysf_free_pkt
 ****************************************************************************/

void
sysf_free_pkt( USRBUF *ub)
{
  m_PMGR_LK_INIT;

  if (ub != 0)
  {
      uns8 pool_id = ub->pool_ops->pool_id;
      USRDATA*  ud = ub->payload;
      m_NCS_MEM_FREE(ub, NCS_MEM_REGION_IO_DATA_HDR,
                    NCS_SERVICE_ID_OS_SVCS, 2);
      if (--(ud->RefCnt) == 0)
        {
        m_PMGR_LK(&gl_ub_pool_mgr.lock);

        gl_ub_pool_mgr.pools[pool_id].mem_free(ud,pool_id);

#if ((NCSSYSM_BUF_WATCH_ENABLE == 1) || (NCSSYSM_BUF_STATS_ENABLE == 1))
        gl_ub_pool_mgr.pools[pool_id].stats.curr -= sizeof(USRDATA);
#endif
#if (NCSSYSM_BUF_STATS_ENABLE == 1)
        gl_ub_pool_mgr.pools[pool_id].stats.cnt--;
        gl_ub_pool_mgr.pools[pool_id].stats.freed += sizeof(USRDATA);
#endif

        m_PMGR_UNLK(&gl_ub_pool_mgr.lock);
 
        }
  }
}


/***************************************************************************
 *  sysf_ditto_pkt
 *
 * Duplicate a USRBUF (chain) by using reference counts.  New USRBUF headers
 * are allocated, but the same payload area is pointed to by the new USRBUF.
 ****************************************************************************/

USRBUF *
sysf_ditto_pkt( USRBUF *dup_me)
{

  USRBUF *ub, *ub_head;
  USRBUF **ubp;

  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (dup_me != NULL) );

  ub_head = (USRBUF*)0;
  ubp = &ub_head;

  /* March thru the USRBUF chain, duplicating the USRBUFs and
   * pointing to the same data area. 
   */
  
  while (dup_me != BNULL)
  {
      *ubp = (ub = (USRBUF*)m_NCS_MEM_ALLOC(sizeof(USRBUF),
                                           NCS_MEM_REGION_IO_DATA_HDR,
                                           NCS_SERVICE_ID_OS_SVCS, 2));
    
      if (ub == BNULL)
      {
          m_MMGR_FREE_BUFR_LIST(ub_head);
          ub_head = BNULL;
          break;
      }

      *ub = *dup_me;  /* copy buffer header... */
      ub->next = BNULL;  /* ...except link pointers. */
      ub->link = BNULL;

      /* When using newer usrbuf macros, we can share data. */
      /* KCQ: if there was ub->pool_ops->can_share field
              we'd know if we need to copy...
      */
      
      ub->payload->RefCnt++;  /* one more usrbuf referencing the data */

      ubp = &ub->link;
      dup_me = dup_me->link;   /* on to next USRBUF... */
  }
      
  return (ub_head);

}

/***************************************************************************
 *  sysf_copy_pkt
 *
 * Duplicate a USRBUF (chain) by making physical copies (i.e., allocating
 * a whole new USRBUF (chain) and payload areas).
 ****************************************************************************/

USRBUF *
sysf_copy_pkt(USRBUF *dup_me)
{

  USRBUF  *ub, *ub_head;
  USRBUF  **ubp;
  USRDATA *payload;

  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (dup_me != NULL) );

  ub_head = (USRBUF*)0;
  ubp = &ub_head;

  /* March thru the USRBUF chain, duplicating the USRBUFs and
   * the data area. 
   */
  
  while (dup_me != BNULL)
  {
      /* NOTE: We know the pool_id; The priority is fixed to 0. A real
       * implementation that cares would alter the priority to suit its
       * system policies.
       */

      *ubp = (ub = (USRBUF*)m_MMGR_ALLOC_POOLBUFR(dup_me->pool_ops->pool_id,NCSMEM_HI_PRI));

      if (ub == BNULL)
      {
          m_MMGR_FREE_BUFR_LIST(ub_head);
          ub_head = BNULL;
          break;
      }
      payload = ub->payload;   /* preserve payload through next statement */
      *ub = *dup_me;  /* copy buffer header... */
      ub->next = BNULL;  /* ...except link pointers. */
      ub->link = BNULL;
      /* restore preserved payload ptr. and copy data into it */
      ub->payload = payload;


      memcpy(ub->payload->Data,dup_me->payload->Data,PAYLOAD_BUF_SIZE);


      /* setup link pointers */
      ubp = &ub->link;
      dup_me = dup_me->link;   /* on to next USRBUF... */
  }
      
  return (ub_head);
}

/***************************************************************************
 *  sysf_get_chain_len
 ****************************************************************************/

uns32
sysf_get_chain_len( const USRBUF *my_len)
  {
  const USRBUF *ub;
  uns32         len;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (my_len != NULL) );
  
  for (len = 0, ub = my_len; (ub); ub = ub->link)
    len += ub->count;
  
  return len;
  }

/***************************************************************************
 * Procedure: sysf_calc_usrbuf_cksum_1s_comp()
 *
 * Does in-place checksumming of packet. 
 * Inputs:  USRBUF*:  ptr to the entire packet
 *          Len    :  packet length
 *          Cksum* :  where to put the checksum, if successful.
 *
 * Returns: void
 ***************************************************************************/
void
sysf_calc_usrbuf_cksum_1s_comp(USRBUF *const u, unsigned int PktLen, uns16 *const pCksum)
{
   uns32        Cksum32      = 0;
   int          bufLen       = 0;
   uns16       *p_operand    = NULL,
                work_var     = 0;
   NCS_BOOL      byte_swapped = FALSE;
   USRBUF      *pUBuf   = u;
   NCS_BOOL      is_odd_boundary = FALSE;
   
   union{
      uns8    c[2];
      uns16   s;
   } s_util;
   union{
      uns16   s[2];
      uns32   l;
   } l_util;

#define ADDCARRY(x) ((x)>65535?(x)-=65535:(x))
#define REDUCE   {l_util.l = Cksum32; Cksum32 = l_util.s[0] + l_util.s[1]; ADDCARRY(Cksum32);}


#if (USE_LITTLE_ENDIAN==1)
   #define m_ODD_BYTE_BOUNDARY_CONVERTION(flag, val, pDest16) (*pDest16 = (uns16)((!flag)?m_NCS_OS_NTOHS_P((uns8*)(val)):(*val)))
   #define m_ROTATE_BYTES(num) ((num & 0x0000ff00)>>8)|((num & 0x000000ff)<<8)
#else
#define m_ODD_BYTE_BOUNDARY_CONVERTION(flag, val, pDest16) ({if(flag){m_NCS_OS_HTONS_P((uns8*)pDest16, (uns16)(*val));}else{((*pDest16) = (*val));}})
   #define m_ROTATE_BYTES(num)  (num)
#endif

   s_util.s = (uns16)0;

   /* Do computation of check sum on cumulative of each link in the usrbuf chain */
   for(; pUBuf && PktLen; pUBuf = pUBuf->link)
   {
      /* Is there no data in this link, just continue to the next one */
      if(pUBuf->count == 0)
         continue;

      /* Get a pointer ot the first 2 bytes in this usrbuf */
      p_operand = (uns16*)(pUBuf->payload->Data + pUBuf->start);

      is_odd_boundary = (1 & (long)(p_operand));

      /* Last loop did we have data stradling more than one usrbuf? */
      if(bufLen == -1)
      {
         /* The first byte of this buffer is a continuation of
          * word spanning between this buffer and the last 
          * NOTE: s_util.c[0] holds the first portion from the last buffer
          */
         s_util.c[1] = *(uns8*)p_operand;

         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&s_util.s,&work_var);
         Cksum32 += (uns32)work_var;
         p_operand = (uns16*) ((uns8*)p_operand + 1);
         bufLen = pUBuf->count - 1;
         PktLen--;
      }
      else
         bufLen = pUBuf->count;
      
      /* Did the caller misconceive the length of the data? */
      if((int)PktLen < bufLen)
         /* Only count what the user says too */
         bufLen = PktLen;

      /* Update the packet processed length */
      PktLen -= bufLen;

      /* FORCE AN EVEN BYTE BOUNDARY : Done for machines 
       * that can only access memory on an even addresses 
       * boundary 
       */
      if((1 & (long)(p_operand)) && (bufLen > 0))
      {
         REDUCE;
         Cksum32 <<=8;
         s_util.c[0] = *(uns8*) p_operand;
         p_operand = (uns16*) ((uns8*)p_operand + 1);
         bufLen--;
         byte_swapped = TRUE;
      }

      /* Unroll the loop to make the overhead less */
      while((bufLen -= 32) >= 0)
      {
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[0],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[1],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[2],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[3],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[4],&work_var); 
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[5],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[6],&work_var); 
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[7],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[8],&work_var); 
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[9],&work_var); 
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[10],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[11],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[12],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[13],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[14],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[15],&work_var);
         Cksum32 += (uns32)work_var;

         p_operand += 16;
      }
      bufLen += 32;

      while((bufLen -= 8) >= 0)
      {
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[0],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[1],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[2],&work_var);
         Cksum32 += (uns32)work_var;
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[3],&work_var);
         Cksum32 += (uns32)work_var;
         p_operand += 4;
      }
      bufLen += 8;
      
      if(!byte_swapped && !bufLen)
         continue;
      
      REDUCE;
      while((bufLen -=2) >= 0)
      {
         m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&p_operand[0],&work_var);
         Cksum32 += (uns32)work_var;
         p_operand += 1;
      }
      
      if(byte_swapped)
      {
         REDUCE;
         Cksum32 <<=8;
         byte_swapped = FALSE;
         if(bufLen == -1)
         {
            s_util.c[1] = *(uns8*)p_operand;
            m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&s_util.s,&work_var);
            Cksum32 += (uns32)work_var;
            bufLen = 0;
         }
         else
            bufLen = -1;
      }
      else if(bufLen == -1)
         s_util.c[0] = *(uns8*)p_operand;
   }

   if(PktLen)
      printf("\nCKSUM ERROR : Bad PktLen\n");

   if(bufLen == -1)
   {
      s_util.c[1] = 0;
      m_ODD_BYTE_BOUNDARY_CONVERTION(is_odd_boundary,&s_util.s,&work_var);
      Cksum32 += (uns32)work_var;
   }

   REDUCE;

   /* Fix byte ordering */
   if(is_odd_boundary)
   {
     Cksum32 = m_ROTATE_BYTES(Cksum32);
   }

   /* Return the ones compliment */
   *pCksum = (uns16)(~Cksum32);  
}

/***************************************************************************
 *  sysf_reserve_at_end
 ****************************************************************************/
 char *sysf_reserve_at_end(USRBUF **ppb, unsigned int i_size)
 {
     uns32 io_size = i_size;
     char  *ret_buf;
     ret_buf = sysf_reserve_at_end_amap(ppb, &io_size, TRUE);
     /* Warn people if reserved size is not equal to requested size */
     if (ret_buf != NULL) 
         assert( io_size == i_size );
     return ret_buf;
 }
/***************************************************************************
 *  sysf_reserve_at_end_amap
 ****************************************************************************/

 char *sysf_reserve_at_end_amap( USRBUF **ppb, unsigned int *io_size, 
        NCS_BOOL total /* Whether total allocation is strictly required */)
   {
   USRBUF *ub;
   char *pContiguousData;
   uns32  ub_hdr_rsrv;
   uns32  ub_trlr_rsrv;
   int32  min_rsrv;
   int32  space_left;
   
   /* varify that the passed arguments are reasonably healthy */
   m_NCSSYSM_BUF_ASSERT( (!((ppb == NULL) || (*ppb == NULL) || (*io_size == 0))) );
   
   ub = *ppb;
   
   while (ub->link != (USRBUF*)0)
     {
     ub = ub->link;  /* advance to the last one, if nec. */
     *ppb = ub;    /* tell the caller... */
     }
   ub_hdr_rsrv = gl_ub_pool_mgr.pools[ub->pool_ops->pool_id].hdr_reserve;
   ub_trlr_rsrv = gl_ub_pool_mgr.pools[ub->pool_ops->pool_id].trlr_reserve;

   /* Determine the minimum bytes that need to be reserved in the least */
   min_rsrv = (int32)(total? *io_size : 1);
   space_left = sizeof(ub->payload->Data) - (ub_trlr_rsrv + ub->start + ub->count);

   /* Partial reservation is ok */
   if ( (ub->payload->RefCnt > 1) ||
        (space_left < min_rsrv))
     {
     /* Need to get one more! */
     
     ub = (*ppb = (ub->link = m_MMGR_ALLOC_POOLBUFR(ub->pool_ops->pool_id,NCSMEM_HI_PRI)));
     
     if (ub == (USRBUF*)0)
       {
       return NULL;
       }
     space_left = sizeof(ub->payload->Data) - (ub_trlr_rsrv + ub->start + ub->count);
     }
   
   if (space_left < (int32)*io_size)
   {
       /* Cannot reserve so many bytes at a time */
       if (total)
       {
           assert(0); 
           return NULL;
       }
       else
           *io_size = space_left;
   }

   
   /* We now know that there's some room in 'ub'. */
   pContiguousData = ub->payload->Data + ub->start + ub->count;
   ub->count += *io_size;  /* Reserve the room in the data buffer. */

   return pContiguousData;
   }

/***************************************************************************
 *  sysf_remove_from_end
 ****************************************************************************/

void sysf_remove_from_end(USRBUF *pb, unsigned int size)
  {
  USRBUF **pub;
  USRBUF *ub;
  unsigned int buflen;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (pb != NULL) );
  
  if (pb != BNULL)
    {
    while ((size > 0) && (sysf_get_chain_len(pb)))
      {
      
      /* Find the last one in the chain. */
      pub = (USRBUF **)NULL;
      ub = pb;
      
      while (ub->link != (USRBUF*)0)
        {
        pub = &ub->link, 
          ub = *pub;
        }
      
      buflen = ub->count;
      if (buflen > size)
        {
        /* We can do the unappand without freeing any buffers. */
        ub->count -= size;
        break;
        }
      size -= buflen;
      ub->count = 0;
      
      if (pub != (USRBUF**)0)  /* Was there a previous? */
        {
        *pub = (USRBUF*)0;  /* de-link this one... */
        sysf_free_pkt(ub);  /* And free it. */
        }
      }
    }
  }

/***************************************************************************
 *  sysf_reserve_at_start
 ****************************************************************************/

char *sysf_reserve_at_start(USRBUF **ppb, unsigned int size)
  {
  USRBUF *ub;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((ppb == NULL) || (*ppb == NULL))) );
  
  
  ub = *ppb;
  /* Can we prepend directly within this USRBUF? It would be nice. */
  
  
  if ((ub->payload->RefCnt > 1) || (ub->start < size))
    {
    /* We must prepend a USRBUF to the one passed. */
    ub = m_MMGR_ALLOC_POOLBUFR(ub->pool_ops->pool_id,NCSMEM_HI_PRI);
    
    if (ub == (USRBUF*)0)
      {
      /* FAIL!!! */
      return (char*)NULL;
      }
    ub->link = *ppb;  /* link dis to dat. */
    
    /* Adjust the next pointer just in case */
    ub->next = (*ppb)->next;
    (*ppb)->next = BNULL;
    
    *ppb = ub;  /* inform caller of new head */

    /* -----------
       In buffers allocated from MDS-POOL, ub->start may not be zero. Hence, 
       the following line would be incorrect
              ub->start += PAYLOAD_BUF_SIZE. 
       -----------
       Hence, it is now changed to
              ub->start = PAYLOAD_BUF_SIZE. 
       -----------
       However, it CANNOT be changed to because, for backward compatibility
       purposes, we should be able to support sysf_reserve_at_start(...,
       PAYLOAD_BUF_SIZE) 
              ub->start = 
                  (PAYLOAD_BUF_SIZE - 
                   gl_ub_pool_mgr.pools[ub->pool_ops->pool_id].trlr_reserve)
       -----------
    */
    ub->start = PAYLOAD_BUF_SIZE; 
    }
  
  ub->count += size;  /* do the actual prepend. */
  ub->start -= size;
  return (m_MMGR_DATA(ub,char*));
  }

/***************************************************************************
 *  sysf_remove_from_start
 ****************************************************************************/

void sysf_remove_from_start(USRBUF **ppb, unsigned int size)
  {
  USRBUF *ub;
  unsigned int buflen;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((ppb == NULL) || (*ppb == NULL))) );
  
  while (  ((ub = *ppb) != (USRBUF*)0) &&(size > 0) )
    {
    buflen = ub->count;
    
    if (buflen > size)
      {
      /* This is, like, simple. */
      ub->count = buflen - size;
      ub->start += size;
      break;  /* out of 'while' */
      }
    
    
    size -= buflen;   /* any more to unprepend? */
    *ppb = ub->link;  /* on to the next. */
    
    /* Adjust the next pointer just in case */
    if (*ppb != BNULL)
      (*ppb)->next=ub->next;
    
    ub->link = (USRBUF*)0;  /* not really necessary */
    sysf_free_pkt(ub);
    }
  }

/***************************************************************************
 *  sysf_data_at_end
 ****************************************************************************/

char *sysf_data_at_end(const USRBUF *pb, unsigned int size, char *spare)
  {
  const USRBUF *pb_work, *pbs;
  register unsigned int num_in_buff;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((pb == NULL) || (spare == NULL))) );
  
  /* First, find the last buffer and see if the data is already contiguous. */
  pb_work = pb;
  while (pb_work->link != (USRBUF*)0)
    pb_work = pb_work->link;
  
  /* pb_work now points to the LAST usrbuf in the chain. */
  num_in_buff = pb_work->count;
  if (num_in_buff >= size)
    {
    /* request is already contiguous. */
    return (m_MMGR_DATA(pb_work, char*) + num_in_buff - size);
    }
  
    /* We will need to make the data contiguous, because it spans USRBUFs. 
    * We copy it into the contiguous area provided by the nice caller. 
  */
  
  while (TRUE)
    {
    /* copy what data is in this working usrbuf ... then step backwards in chain */
    memcpy(spare + size - num_in_buff, m_MMGR_DATA(pb_work, char *), num_in_buff);  
    
    size -= num_in_buff; /* adjust amt of data still to find & copy */
    
    /* Now find the USRBUF pointing to 'pb_work' */
    if (pb_work == pb) /* if we are already at the head usrbuf ... */
      return NULL;  /* ... then, not enuf data in entire chain to satisfy... ??? */
    
    pbs = pb;                        /* pt to head usrbuf */
    while (pbs->link != pb_work)     /* find the usrbuf prior to working usrbuf */
      pbs = pbs->link;
    pb_work = pbs;                   /* make this prior usrbuf the working usrbuf */
    
    num_in_buff = pb_work->count;
    
    if (num_in_buff >= size)   /* is there enuf data in this usrbuf ? */
      {                          /* yes, copy it and leave; otherwise go loop again */
      memcpy(spare, m_MMGR_DATA(pb_work, char*) + num_in_buff - size, size);
      break;  /* from 'while' */
      }
    }
  
  return spare;  /* pointer to user-supplied contiguous area. */
  }

/***************************************************************************
 *  sysf_data_at_start
 ****************************************************************************/

char *sysf_data_at_start(const USRBUF *pb, unsigned int size, char *spare)
  {
  register unsigned int num_in_buff;
  char *spare_work;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((pb == NULL) || (spare == NULL))) );

  while ((num_in_buff = pb->count) == 0)  /* skip over null-length buffers */
    pb = pb->link;
  
  if (num_in_buff >= size)
    {
    /* request is already contiguous.  Just return pointer to it. */
    return m_MMGR_DATA(pb, char*);
    }
  
   /* The data requested is not contiguous.
    * We must copy it to the contiguous area provided by the caller.
    */
  spare_work = spare;
  
  while (TRUE)
    {
    memcpy(spare_work, m_MMGR_DATA(pb, char*), num_in_buff);  
    size -= num_in_buff;
    spare_work += num_in_buff;
    
    pb = pb->link;
    if (pb == (USRBUF*)0)
      return NULL;  /* not enuf in whole chain so return error */
    
    num_in_buff = pb->count;
    
    if (num_in_buff >= size)
      {
      memcpy(spare_work, m_MMGR_DATA(pb, char*), size);
      break;  /* from 'while' */
      }
    }
  
  return spare;  /* pointer to user-supplied contiguous area. */
  }


/*************************************************************************
**
**  sysf_data_in_mid
**
**  Returns a pointer to the data at "offset" in the current data packet
**  within the USRBUF chain headed by "pb".  If copy_flag is TRUE, data
**  is always copied to the "copy_buf" area.  If the copy_flag is FALSE,
**  data is only copied if the data spans more than a single USRBUF
**  ( that is, data is non-contiguous ).  The "size" comes in as the 
**  number of octets of interest.  If the length of the data packet is
**  not at least "offset" plus "size" then a NULL pointer is returned; 
**  although a partial copy may have been done.
**  
************************************************************************/

char *
sysf_data_in_mid(USRBUF *pb, unsigned int offset, unsigned int size, 
                 char *copy_buf, unsigned int copy_flag)
  {
  register unsigned int num_in_buf;
  register unsigned int cpcnt;
  char                  *cb_work;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (pb != NULL) );

  if ( pb->count >= ( offset + size ) )
    {
    if ( copy_flag == TRUE )
      {
      memcpy(copy_buf, m_MMGR_DATA_AT_OFFSET(pb, offset, char *), size);  
      return copy_buf;
      }
    else
      return m_MMGR_DATA_AT_OFFSET(pb, offset, char *);
    }
  
  /* Move to the USRBUF that contains the first byte of the data */
  num_in_buf = pb->count;
  while (num_in_buf <= offset)  
    {
    offset -= num_in_buf;
    pb = pb->link;
    if (pb == (USRBUF*)0)
      {
      return (char *)0;
      }
    num_in_buf = pb->count;
    }
  
  
  /* Check if we have all the data in one USRBUF */
  if ( num_in_buf >= ( offset + size ) )
    {
    if ( copy_flag == FALSE )
      {
      return m_MMGR_DATA_AT_OFFSET(pb, offset, char *);
      }
    else
      {
      /* copy data to caller supplied buffer */
      memcpy(copy_buf, m_MMGR_DATA_AT_OFFSET(pb, offset, char *), size);
      return copy_buf;
      }
    }
  
    /* 
    ** If we get here we must be spanning multiple USRBUFs.
    ** Copy data from first USRBUF to caller supplied buffer. 
    */
  cpcnt = num_in_buf - offset;
  memcpy(copy_buf, m_MMGR_DATA_AT_OFFSET(pb, offset, char *), cpcnt);
  
  size -= cpcnt;
  cb_work = copy_buf + cpcnt;
  
  while ( size > 0 )
    {
    pb = pb->link;
    if (pb == (USRBUF*)0)
      {
      /* USRBUF has run out of data; forced to fail */
      return (char *)0;
      }
    num_in_buf = pb->count;
    
    /* if this is last portion of data, copy and return */
    if ( num_in_buf >= size )
      {
      memcpy(cb_work, m_MMGR_DATA(pb, char *), size);
      return copy_buf;
      }
    
    /* do copy of data from this USRBUF and continue */
    memcpy(cb_work, m_MMGR_DATA(pb, char *), num_in_buf);  
    size -= num_in_buf;
    cb_work += num_in_buf;
    }
  
  /* we should never get here */
  return (char *)0;
  }

/*************************************************************************
**
**  sysf_reserve_in_mid  (INTERNAL USE ONLY)
**
** Returns a pointer to the beginning of a reserved contiguous
** area in the middle of a USRBUF chain.  Area will begin at 
** 'offset' bytes from the start of 'pb's payload.  Area will be
** 'size' bytes in length.  Returns zero if unable to allocate
** necessary space.
** This code has explicit knowledge of the USRBUF and payload
** data structures.  Furthermore, this code requires that the 
** 'start' field of newly allocated USRBUFs be set to zero.
**
************************************************************************/
static char *
sysf_reserve_in_mid (USRBUF *pb, unsigned int offset, unsigned int size )
  {
  unsigned int i;
  unsigned int pload_size;
  unsigned int post_data_len;
  unsigned int num_in_buf;
  char         *pload;
  USRBUF       *new_ub;
  USRBUF       *new_ub2;
  USRDATA      *ud;
  char         *src;
  char         *dest;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((pb == NULL) || (size == 0))) );

  /* Move to the USRBUF that contains the first byte of the data */
  num_in_buf = pb->count;
  while (num_in_buf <= offset)  
    {
    offset -= num_in_buf;
    pb = pb->link;
    if (pb == (USRBUF*)0)
      {
      return (char *)0;
      }
    num_in_buf = pb->count;
    }
  
  /* Code currently does not support spanning USRBUFs */
  if ( size > PAYLOAD_BUF_SIZE )
    return (char *)0;
  
  /* If payload has other users, need to allocate own copy*/
  if ( pb->payload->RefCnt > 1 )
    {
    ud = (USRDATA*)pb->pool_ops->mem_alloc(sizeof(USRDATA),pb->pool_ops->pool_id,NCSMEM_HI_PRI);
    
    if (ud == (USRDATA*)NULL)
      return (char *)0;
    
    ud->RefCnt = 1;
    memcpy(ud->Data, pb->payload->Data, sizeof(pb->payload->Data));
    pb->payload->RefCnt--;
    pb->payload = ud;
    }
  
  pload_size = sizeof(pb->payload->Data);
  pload = m_MMGR_DATA(pb, char *);
  post_data_len = pb->count - offset;
  
  /* 
  ** Check if it is possible to create space in the current
  ** USRBUF by moving part of the payload down in the buffer.
  ** We can't do that if all of the data won't fit.
  */
  if ((pb->start + pb->count + size) > pload_size)
    {
    /*
    ** if one USRBUF can hold reserved space plus the latter
    ** payload data, copy the latter payload data to new
    ** USRBUF 'size' bytes down.
    */
    if ( (post_data_len + size) <= pload_size )
      {
      /* 
      ** Allocate a new USRBUF and put the latter part
      ** of the current payload into it.
      */
      new_ub = m_MMGR_ALLOC_POOLBUFR(pb->pool_ops->pool_id,NCSMEM_HI_PRI);
      if (( new_ub == (USRBUF *)0 ) || ( new_ub->start != 0 ))
        return (char *)0;
      memcpy ( m_MMGR_DATA(new_ub, char *) + size, 
        m_MMGR_DATA_AT_OFFSET(pb, offset, char *), post_data_len);
      new_ub->link  = pb->link;
      new_ub->count = size + post_data_len;
      pb->link      = new_ub;
      pb->count     = offset;
      }
    else
      {
      /* 
      ** Need two new USRBUFs because one is not large 
      ** enough for all of our data. The new data will go in
      ** the one and the latter part of the current payload
      ** payload in the second.
      */
      new_ub = m_MMGR_ALLOC_POOLBUFR(pb->pool_ops->pool_id,NCSMEM_HI_PRI);
      if (( new_ub == (USRBUF *)0 ) || ( new_ub->start != 0 ))
        return (char *)0;
      new_ub2 = m_MMGR_ALLOC_POOLBUFR(pb->pool_ops->pool_id,NCSMEM_HI_PRI);
      if (( new_ub2 == (USRBUF *)0 ) || ( new_ub2->start != 0 ))
        return (char *)0;
      
      memcpy ( m_MMGR_DATA(new_ub2, char *), 
        m_MMGR_DATA_AT_OFFSET(pb, offset, char *), post_data_len);
      
      /* update links and sizes */
      new_ub2->link  = pb->link;
      new_ub2->count = post_data_len;
      new_ub->link   = new_ub2;
      new_ub->count  = size;
      pb->link       = new_ub;
      pb->count      = offset;
      }
    /* reserved space is at front of new USRBUF */
    return m_MMGR_DATA(new_ub, char *);
    }
  else
    {
    /* 
    ** There is enough space to copy down data in 
    ** the current USRBUF.
    */
    dest = m_MMGR_DATA_AT_OFFSET(pb, offset + size, char *);
    src = m_MMGR_DATA_AT_OFFSET(pb, offset, char *); 
    for ( i = 1; i <= (post_data_len); i++ )
      dest[post_data_len - i] = src[post_data_len - i];
    
    pb->count += size;
    
    return m_MMGR_DATA_AT_OFFSET(pb, offset, char *);
    }
}

/*************************************************************************
**
**  sysf_insert_in_mid
**
** Returns a pointer to the start of the insertion area.  Insertion
** area begins at 'offset' bytes from the start of the 'pb' payload
** and continues for 'size' bytes.  Inserts 'size' bytes of data from
** the 'ins_data' buffer.
**
************************************************************************/
char *
sysf_insert_in_mid (USRBUF *pb, unsigned int offset, 
                    unsigned int size, char *ins_data )
  {
  char *insert_spot;

  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((pb == NULL) || (ins_data == NULL))) );
  
  insert_spot = sysf_reserve_in_mid ( pb, offset, size );
  
  if ( insert_spot != (char *)0 )
    memcpy ( insert_spot, ins_data, size );
  
  return insert_spot;
  }

/*************************************************************************
**
**  sysf_write_in_mid
**
** Returns a pointer to the data to be copied.  Copy area 
** begins at 'offset' bytes from the start of the 'pb' payload
** and continues for 'size' bytes.  Copies 'size' bytes of data
** from the 'cdata' buffer to the copy area.  Returns (char *)0
** if data can not be copied into the payload area.
**
************************************************************************/
char *
sysf_write_in_mid (USRBUF *pb, unsigned int offset, 
                   unsigned int size, char * cdata )
  {
  USRDATA      *ud;
  unsigned int num_in_buf;

  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((pb == NULL) || (cdata == NULL))) );
  
  /* Move to the USRBUF that contains the first byte of the data */
  num_in_buf = pb->count;
  while (num_in_buf <= offset)  
    {
    offset -= num_in_buf;
    pb = pb->link;
    if (pb == (USRBUF*)0)
      {
      return (char *)0;
      }
    num_in_buf = pb->count;
    }
  
  /* Code currently does not support spanning USRBUFs */
  if ( num_in_buf < ( offset + size ) )
    return (char *)0;
  
  /* If payload has other users, need to allocate own copy*/
  if ( pb->payload->RefCnt > 1 )
    {
    ud = (USRDATA*)pb->pool_ops->mem_alloc(sizeof(USRDATA),pb->pool_ops->pool_id,NCSMEM_HI_PRI);
    
    if (ud == (USRDATA*)NULL)
      return (char *)0;
    
    ud->RefCnt = 1;
    memcpy(ud->Data, pb->payload->Data, sizeof(pb->payload->Data));
    pb->payload->RefCnt--;
    pb->payload = ud;
    }
  
  /* all set up; just do the copy */
  memcpy ( m_MMGR_DATA_AT_OFFSET(pb, offset, char *), cdata, size );
  return cdata;
  }



/****************************************************************************
 *
 * USRBUF *
 * sysf_ubq_dq_head(SYSF_UBQ *ubq)
 *
 * Description:
 *  Returns head of USRBUF Q
 *
 * Returns:
 *  Pointer to the USRBUF de-queued
 *
 * Notes:
 *
 ***************************************************************************/
USRBUF *
sysf_ubq_dq_head(SYSF_UBQ *ubq)
  {
  USRBUF *pbuf;
   
  if ((pbuf = ubq->head) != BNULL)
    {
    ubq->head = pbuf->next;
    if (--ubq->count == 0)
      ubq->tail = BNULL;
    }
  return pbuf;
  }

/****************************************************************************
 *
 * void
 * sysf_ubq_dq_specific(SYSF_UBQ *ubq, USRBUF *pbuf)
 *
 * Description:
 *  Returns and de-queues specific USRBUF from USRBUF Q
 *
 * Returns:
 *  Nothing
 *
 * Notes:
 *
 ****************************************************************************/
void
sysf_ubq_dq_specific(SYSF_UBQ *ubq, USRBUF *pbuf)
  {
  USRBUF *searcher;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((ubq == NULL) || (pbuf == NULL))) );
  
  m_NCS_LOCK_V2(&ubq->lock, NCS_LOCK_WRITE, NCS_SERVICE_ID_COMMON, 1);
  
  if (ubq->head == pbuf)
    sysf_ubq_dq_head(ubq);
  else
    {
    searcher = ubq->head;
    while (m_MMGR_NEXT(searcher) != BNULL)
      {
      if (m_MMGR_NEXT(searcher) == pbuf)
        {
        m_MMGR_NEXT(searcher) = m_MMGR_NEXT(pbuf);
        m_MMGR_NEXT(pbuf) = BNULL;
        ubq->count -= 1;
        if (ubq->tail == pbuf)
          ubq->tail = searcher;
        }
      else
        searcher = m_MMGR_NEXT(searcher);
      }
    }
  
  m_NCS_UNLOCK_V2(&ubq->lock, NCS_LOCK_WRITE, NCS_SERVICE_ID_COMMON, 1);
  }

/****************************************************************************
 *
 * USRBUF *
 * sysf_ubq_scan_specific(SYSF_UBQ *ubq, USRBUF *pbuf)
 *
 * Description:
 *  Returns specific USRBUF from USRBUF Q
 *
 * Returns:
 *  Pointer to USRBUF found or NULL
 *
 * Notes:
 *
 ****************************************************************************/
USRBUF *
sysf_ubq_scan_specific(SYSF_UBQ *ubq, USRBUF *pbuf)
  {
  USRBUF *searcher;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((ubq == NULL) || (pbuf == NULL))) );
  
  for (searcher = ubq->head;
  searcher != BNULL;
  searcher = m_MMGR_NEXT(searcher))
    {
    if (searcher == pbuf)
      return pbuf;
    }
  
  return BNULL;
  }


/*************************************************************************
**
**  sysf_append_data 
** This macro appends the data from buffer 2 to the end of 
** the data in buffer 1.  This may be accomplished by chaining
** buffer 2 onto buffer 1.  This is used to extend a frame.
** After this macro is called, buffer 2 is no longer valid and 
** should not be accessed.  Macro has no return value.
**
************************************************************************/

void sysf_append_data(USRBUF *p1, USRBUF *p2)
  {
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (p1 != NULL) );
  
  while (p1->link != (USRBUF*)0)
    {
    p1 = p1->link;  /* advance to the last one, if nec. */
    }
  p1->link = p2;
  }


/*************************************************************************
**
**  sysf_frag_bufr 
**
** The routine fragments the existing payload of the USRBUF (chain) 
** pointed to by the contents of the USRBUF **ppb.  Each fragment 
** is of size frag_size, except for the last one, which might 
** be the same or smaller. It returns the number of fragments  
** created. A zero value indicates a failure.
**
** Note: If the frame passed in happens to be in a queue 
** (its next pointer is not NULL), this macro will only
** fragment the first frame.   
**
** The user of this routine is responsible for freeing all created 
** fragments so far in case of errors.
**
************************************************************************/
#define APS_NONE     0
#define NCS_SPLIT_IT 1
#define NCS_LEAVE_IT 2
#define NCS_ADD_ONE  3
unsigned int sysf_frag_bufr(USRBUF *ppb, unsigned int frag_size, SYSF_UBQ *ubq)
{
   USRBUF *pcur, *psaved, *psaved_next = NULL;
   USRBUF *pnew = NULL, *pfirst = NULL;
   unsigned long total_len;
   unsigned long bufsize;
   unsigned long needsize = 0;
   unsigned char action = APS_NONE;
   unsigned char fragmenting = 0;

   /* varify that the passed arguments are reasonably healthy */
   m_NCSSYSM_BUF_ASSERT( (ubq != NULL) );

   if ((ppb == BNULL) || (frag_size == 0))
      return (ubq->count);

   total_len = sysf_get_chain_len(ppb);
   ubq->count = 1;  /* start out with 1 fragment */
   pcur = ubq->head = ubq->tail = ppb;

   if (total_len <= frag_size) 
      return (ubq->count); /* no fragmentation needed */


   /** Just in case a queue of usrbuf has been passed in.
    ** We only fragment the first frame. 
    **/
   if (pcur->next != BNULL)
   {
      psaved_next = pcur->next;
      pcur->next = BNULL;
   }

   /* Loop through the buf chain */
   while (pcur != BNULL)
   {
      bufsize = pcur->count;  /* size of the usrbuf */

      if (needsize != 0)
      {
         /* Have more than enough to complete a fragment */
         if (bufsize > needsize)
         {
            pcur->count = needsize;
            bufsize -= needsize;
            needsize = 0;

            /* Whatever left over belongs to the next fragment(s) */
            if (bufsize <= frag_size)
            {
               action = NCS_ADD_ONE;
            }
            else if (bufsize > frag_size)
            {
               action = NCS_SPLIT_IT;
            }

         } /* end if bufsize > needsize */

         else if (bufsize < needsize)  /* don't have enough to complete a fragment */
         {
            needsize -= bufsize;  /* how much more is needed ? */
            pcur = pcur->link;
            action = APS_NONE;
         }

         else  /* have enough to complete a fragment */
         {
            needsize = 0;
            action = APS_NONE;
            if (pfirst != NULL)
            {
               pfirst->next = pcur->link;
               pcur->link = NULL;
               pcur = pfirst->next;
               pfirst = NULL;
            }
            else
            {
               pcur->next = pcur->link;
               pcur->link = NULL;
               pcur = pcur->next;
            }
            if (pcur != NULL)
               ubq->count++;
         }
      }  /* end if (needsize) */

      else if (bufsize == frag_size)  /* this usrbuf itself is a fragment */
      {
         action = NCS_LEAVE_IT;
      }

      /* This usrbuf must be chained to another usrbuf 
         to create a fragment of size frag_size */
      else if(bufsize < frag_size)  
      {
         needsize = frag_size - bufsize; /* need this much from the next usrbuf */
         pfirst = ubq->tail = pcur;
         /* go to the next usrbuf in the chain */
         pcur = pcur->link;
         action = APS_NONE;
      }

      else     /* This usrbuf must be splitted up */
      {
         pcur->count = frag_size;
         bufsize -= frag_size;
         action = NCS_SPLIT_IT;
      }

      switch (action)
      {
         case NCS_LEAVE_IT:
            pcur->next = pcur->link;
            pcur->link = NULL;  /* complete fragment */
            ubq->tail  = pcur;
            pcur       = pcur->next;  /* go to the next usrbuf in the chain */
            if (pcur != NULL)
               ubq->count++;

            break;

         case NCS_SPLIT_IT:
   
            /* save the pointer to this usrbuf */
            psaved = pcur;

            /* Set the fragmenting flag */
            fragmenting = 1;

            while (fragmenting != 0)
            {
               pnew = (USRBUF*)m_NCS_MEM_ALLOC(sizeof(USRBUF),
                               NCS_MEM_REGION_IO_DATA_HDR,
                               NCS_SERVICE_ID_OS_SVCS, 2);
               if (pnew == (USRBUF*)0)
               {
                  if (psaved_next != NULL)
                     ubq->tail->next = psaved_next;
                  return (0);
               }

               memset((char *)pnew, '\0', sizeof(USRBUF));
               if (pfirst != NULL)
               {
                  pfirst->next = pnew;
                  pfirst = NULL;
               }
               else
                  pcur->next = pnew;
               ubq->count++;
               pnew->pool_ops = pcur->pool_ops; /* inherit pool stuff */
               pnew->payload  = pcur->payload;  /* point to the same payload */
               pnew->payload->RefCnt++;         /* Increment the refcount */
               
               /* Can be splitted up again */
               if (bufsize > frag_size)
               {
                  pnew->count = frag_size;
                  bufsize -= frag_size;
               }

               /* This is the last fragment created for this userbuf */
               else
               {
                  pnew->count = bufsize;
                  fragmenting = 0;
               }

          pnew->pool_ops   = pcur->pool_ops;
               pnew->start      = pcur->start + pcur->count;
               pcur = ubq->tail = pnew;
            }  /* end while */

            /* There are more usrbufs in the chain, calculate needsize */
            if (psaved->link != NULL)
            {
               needsize = frag_size - pcur->count;

               /* Determine if this is a complete fragment or not */
               if (needsize > 0)
               {
                  ubq->tail    = pfirst = pnew;
                  pcur->link   = psaved->link;
                  psaved->link = NULL;
                  pcur = pcur->link;
               }
               else  /* the next usrbuf is a new fragment */
               {
                  ubq->tail = pcur->next = psaved->link;
                  psaved->link = NULL;
                  pcur = pcur->next;
                  if (pcur != NULL)
                     ubq->count++;
               }
            }
            else
            {
               /* That's it, end of the usrbuf chain */
               ubq->tail = pcur;
               pcur = NULL;
               needsize = 0;
            }
            break;

         /* This is the case where bufsize <= frag_size */
         case NCS_ADD_ONE:
            pnew = (USRBUF*)m_NCS_MEM_ALLOC(sizeof(USRBUF),
                                               NCS_MEM_REGION_IO_DATA_HDR,
                                               NCS_SERVICE_ID_OS_SVCS, 2);

            if (pnew == (USRBUF*)0)
            {
               if (psaved_next != NULL)
                  ubq->tail->next = psaved_next;
               return (0);
            }

            memset((char *)pnew, '\0', sizeof(USRBUF));
            if (pfirst != NULL)
            {
               pfirst->next = pnew;
               pfirst = NULL;
            }
            
            ubq->count++;
            pnew->pool_ops = pcur->pool_ops; /* inherit pool stuff */
            pnew->payload  = pcur->payload;  /* point to the same payload */
            pnew->payload->RefCnt++;         /* Increment the refcount */
            pnew->count = bufsize;
            pnew->start = pcur->start + pcur->count;

            /* There are more usrbufs in the chain, calculate needsize */
            if (pcur->link != NULL)
            {
               needsize = frag_size - bufsize;

               /* Determine if this is a complete fragment or not */
               if (needsize > 0)
               {
                  ubq->tail = pfirst = pnew;
                  pnew->link = pcur->link;
                  pcur->link = NULL;
                  pcur = pnew->link;
               }
               else  /* the next usrbuf is a new fragment */
               {
                  ubq->tail = pnew->next = pcur->link;
                  pcur->link = NULL;
                  pcur = pnew->next;
                  if (pcur != NULL)
                     ubq->count++;
               }
            }
            else
            {
               /* That's it, end of the usrbuf chain */
               ubq->tail = pnew;
               pcur = NULL;
               needsize = 0;
            }

            break;

         default:
            break;
      }  /* end switch */
   }  /* end while */

   if (psaved_next != NULL)
      ubq->tail->next = psaved_next;

   return(ubq->count);
}


/*****************************************************************************

  PROCEDURE NAME:    sysf_copy_to_usrbuf

  DESCRIPTION:

   General purpose routine to copy data from contiguous memory to usrbuf

  PARAMETERS:

   packet:   Pointer to packet payload.
   length:   Octet count of data

  RETURNS:

  Pointer to usrbuf chain  or 0.

  NOTES:


*****************************************************************************/

USRBUF *
sysf_copy_to_usrbuf( uns8 *packet, unsigned int length)
  {
  USRBUF *uu_pdu, *first_uu_pdu;
  unsigned int len;
  uns8 *src, *dst;
  
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((packet == NULL)||(length == 0))) );
  
  src = packet;
  
  /** Move the pdu into a buffer chain ...
  **/
  if ((first_uu_pdu = (uu_pdu = m_MMGR_ALLOC_BUFR(sizeof (USRBUF)))) == BNULL)
    {
    m_MMGR_FREE_BUFR_LIST(uu_pdu);
    return BNULL;
    }
  
  do 
    {
    len = length>PAYLOAD_BUF_SIZE ? PAYLOAD_BUF_SIZE : length;
    if ((dst = m_MMGR_RESERVE_AT_END(&uu_pdu,len,uns8*)) == 0)
      {
      m_MMGR_FREE_BUFR_LIST(first_uu_pdu);
      return BNULL;
      }
    memcpy (dst, src, len);
    src += len;
    length -= len;
    
    }
    while (length > 0);
    
    return first_uu_pdu;
  }

/*****************************************************************************

  PROCEDURE NAME:    sysf_copy_from_usrbuf

  DESCRIPTION:

   General purpose routine to copy data from a usrbuf to contiguous memory

  PARAMETERS:

   packet:   Pointer to packet payload.
   buffer:   The buffer to copy the payload into

  RETURNS:

  Number of bytes written to buffer

  NOTES:


*****************************************************************************/

uns32
sysf_copy_from_usrbuf( USRBUF *packet, uns8 *buffer, uns32 buff_len)
  {
  /* varify that the passed arguments are reasonably healthy */
  m_NCSSYSM_BUF_ASSERT( (!((packet == NULL)||(buffer == NULL)||(buff_len == 0))) );

  if(NULL == m_MMGR_COPY_MID_DATA(packet, 0, buff_len, buffer))
    return 0;
  else
    return buff_len;
  }

/*****************************************************************************

  PROCEDURE NAME:    sysf_usrbuf_hexdump

  DESCRIPTION:

   General purpose routine to dump the content of a USRBUF in hexdump
   format.

  PARAMETERS:

   buf:       head of USRBUF chain to be hexdumped.
   fname:     File name to put hexdump data in. If NULL, send to CONSOLE.

  RETURNS:
   void

*****************************************************************************/
void 
sysf_usrbuf_hexdump(USRBUF *buf, char* fname)
  {
  uns32        len;        /* length of payload */
  uns32        loop;       /* how many times to loop */
  uns32        left;       /* leftover data after loops */
  uns32        offset;     /* offset for MID_DATA macro */
  char         space[200]; /* general purpose stack buffer */
  uns8*        data;       /* ptr to contiguous MID_DATA data */
  uns32        i;          /* an interator counter */
  
  if (buf == NULL)
    return;
  
  len = m_MMGR_LINK_DATA_LEN(buf);
  
  loop = len / 16;
  left = len % 16;

  sprintf (space,"\n USRBUF Payload Hex Dump......len = %d\n", len);
  sysf_pick_output(space, fname);
  
  for(i = 0,offset = 0; i < loop; i++, offset = offset + 16)
    {
    data = (uns8*)m_MMGR_PTR_MID_DATA(buf, offset, 16, space);
    
    if (sysf_str_hexdump(data,16,fname) != NCSCC_RC_SUCCESS)
      return;
    }
  
  data = (uns8*)m_MMGR_PTR_MID_DATA(buf,offset,left,space);
  sysf_str_hexdump(data,left,fname);
  sysf_pick_output("\n\n",fname);
  }

/*****************************************************************************

  PROCEDURE NAME:    sysf_str_hexdump

  DESCRIPTION:

   General purpose routine to dump the content of a string in hexdump
   format.

  PARAMETERS:

   data:      data to convert to hex output.
   size:      length of data to be converted for 1 line of output.
              convention generally puts this at 16 bytes of data.
   fname:     string name of file to put data into. If NULL, output
              is directed to CONSOLE.

  RETURNS:
   status     SUCCESS - all went well.
              FAILURE - something went wrong.

  NOTES:
*****************************************************************************/

uns32
sysf_str_hexdump(uns8* data, uns32 size, char* fname)
  {
  char         store[300] = {0};
  char*        curr       = &store[0];
  char         cstr[40];
  uns32        i;

  /* protect function from various forms of API abuse */

  if ((size > 40) || (size == 0) || (data == NULL))
    return NCSCC_RC_FAILURE; 

  /* OK, format the data up to size long */

  for (i = 0; i < size; i++)
    {
    sprintf (curr,"%02x ",data[i]);
    curr = &store[strlen(store)];
    }

  strncpy(cstr,(char*)data,size - 1); /* now as text string */
  cstr[size - 1] = 0;

  sprintf (curr, "        %s", cstr);

  return sysf_pick_output(store, fname);
  }

/*****************************************************************************

  PROCEDURE NAME:    sysf_pick_output

  DESCRIPTION:

   Send a string to a file or CONSOLE, depending on if the fname string
   is non-null (which implies its a well formed, NULL terminated string).

  PARAMETERS:

   str:       NULL terminated string to output
   fname:     string name of file to put data into. If NULL, output
              is directed to CONSOLE.

  RETURNS:
   status     SUCCESS - all went well.
              FAILURE - something went wrong.

  NOTES:
*****************************************************************************/

uns32
sysf_pick_output(char* str, char* fname)
  {
  FILE* file;

  if (fname != NULL)
    {
    if ((file = sysf_fopen(fname, "at")) == NULL)
      return NCSCC_RC_FAILURE;
    fprintf(file, "%s\n", str);
    sysf_fclose(file);
    }
  else
    printf("%s\n",str);

  return NCSCC_RC_SUCCESS;
  }


#if (USE_MY_MALLOC==1)
uns32 gl_my_malloc_curr_size = 0;
extern void *my_malloc(size_t nbytes)
{
   if ((gl_my_malloc_curr_size+=nbytes) > MY_MALLOC_SIZE)
   {
      gl_my_malloc_curr_size-= nbytes;
      printf("my_malloc FAILED: current=%d, requested=%d, allowed=%d\n", gl_my_malloc_curr_size, nbytes, MY_MALLOC_SIZE);
      return NULL;
   }
   printf("current=%d, requested=%d, allowed=%d\n", gl_my_malloc_curr_size, nbytes, MY_MALLOC_SIZE);
   return malloc(nbytes);
}

extern void my_free(void *mem_p)
{
   gl_my_malloc_curr_size-= nbytes;
   return free(mem_p);
}
#endif


