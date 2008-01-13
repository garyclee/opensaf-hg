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
  ------------
  H&J Queue Pointers

  Where-as the ncs_queue facility assumes that the thing to be enqueued
 
   1) has a field dedicated to linking instances together (next), and
   2) the thing linked will persist over the life of being enqueued

  This queuing mechanism assumes neither. It simply enqueues and dequeues
  a void*  worth of data and assumes the client knows the semantics of what
  is participating. 

  A few support functions are also provided, as per the needs of signalling.

  - -  W A R N I N G  - - 

        the NCS_QPTRS service has these touchy points, or disclaimers that 
        any user ought to be aware of.....

        1) The invoker cannot enqueue A) NULL ptrs or B) ptrs with the 
           value 0xffffffff, because:

           A) NULL is what is reported when the queue is empty.
           B) 0xffffffff means 'dead-slot' in this implementation. It will
              be skipped over in dequeue operations.

        2) Microsoft C++ does not like putting void* in a queue, so passed
           void* pointers are cast to uns32* for enqueue/dequeue operations.


******************************************************************************
*/

#ifndef NCS_QPTRS_H
#define NCS_QPTRS_H

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                       P O I N T E R    Q U E U E

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

#ifndef NCS_QSPACE_SLOTS_MAX
#define NCS_QSPACE_SLOTS_MAX     100
#endif

typedef struct ncs_qlink {
  struct ncs_qlink  *next;                      /* link to the next one, if needed        */
  uns32            *slot[NCS_QSPACE_SLOTS_MAX]; /* hold up to 100 element entries         */
  } NCS_QLINK;

typedef struct ncs_qspace {
  char*            file;
  uns32            line;
  NCS_QLINK         *front;  /* front buffer, from which to dequeue        */
  NCS_QLINK         *back;   /* back buffer, from which to enqueue         */
  uns16            slots;   /* # of elements that can live in an NCS_QLINK */
  uns16            f_idx;   /* front index from which to read next        */
  uns16            b_idx;   /* back index to which we write next          */
  int32            count;
  int32            max_size;
} NCS_QSPACE;

#define NCS_QSPACE_DEAD 0xffffffff /* RMV puts this in place of found value */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                   P R O T O T Y P E S
         
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/* constructor & destructor */

EXTERN_C LEAPDLL_API void    ncs_qspace_construct(NCS_QSPACE* qs);
EXTERN_C LEAPDLL_API void    ncs_circ_qspace_construct(NCS_QSPACE* qs, int32 max_size);
EXTERN_C LEAPDLL_API void    ncs_qspace_delete(NCS_QSPACE* qs);

/* Commit resources and prepare for enqueue & dequeue */

EXTERN_C LEAPDLL_API void    ncs_qspace_init  (NCS_QSPACE* qs);
EXTERN_C LEAPDLL_API void    ncs_qspace_enq   (NCS_QSPACE* qs, void* it);
EXTERN_C LEAPDLL_API void*   ncs_qspace_deq   (NCS_QSPACE* qs);
EXTERN_C LEAPDLL_API void*   ncs_qspace_pop   (NCS_QSPACE* qs);


/* find & purge */

EXTERN_C LEAPDLL_API NCS_BOOL ncs_qspace_hunt  (NCS_QSPACE *qs, void* tgt);
EXTERN_C LEAPDLL_API NCS_BOOL ncs_qspace_remove(NCS_QSPACE *qs, void* tgt, NCS_BOOL find_all);
EXTERN_C LEAPDLL_API void*   ncs_qspace_peek(NCS_QSPACE *qs, int32 index);

/* change max size */
EXTERN_C LEAPDLL_API uns32 ncs_circ_qspace_set_maxsize(NCS_QSPACE *qs, int32 max_size);


/* yea, we could use ncs_qspace_enq instead of ncs_qspace_push,
   but isn't the symmetry nice? */
#define ncs_qspace_push            ncs_qspace_enq

#define ncs_qspace_count(qs)       (qs)->count


#define m_NCS_QSPACE_INIT(q)           (q)->file = __FILE__; \
                                      (q)->line = __LINE__; \
                                      ncs_qspace_init(q); 

#define m_QSPACE_STUFF_OWNER(qs,a)  ncs_mem_dbg_loc(a,qs->line,qs->file)

#endif /* NCS_QPTR_H */


