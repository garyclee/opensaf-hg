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

  MODULE NAME: SNMPTM_DEF.H


..............................................................................

  DESCRIPTION:  This file describes all the MACRO, ENUMs.. definitions which
                are required by SNMPTM application. 

******************************************************************************
*/
#ifndef SNMPTM_DEF_H
#define SNMPTM_DEF_H

/* Macro definition for SNMPTM service id,
 * ensure no service IDs are defined beyond UD_SERVICE_ID_END
 * which is defined in ncs_svd.h.
 */
#define NCS_SERVICE_ID_SNMPTM   (UD_SERVICE_ID_MAX - 1)

/* The following macro's are used for MOVEROW operations, to test the move row 
 * across two different OAC instances.
 */
#define SNMPTM_VCARD_ID1   (NCSVDA_UNNAMED_MAX - 10)
#define SNMPTM_VCARD_ID2   (NCSVDA_UNNAMED_MAX - 20)

/* If this macro's is SET to one then the MOVEROW operation moves the row
 * to the self OAC else move row will be done across 2 different OAC's
 * from SNMPTM_VCARD_ID1 to SNMPTM_VCARD_ID2 & vice-versa
 */ 
#define SNMPTM_MOVEROW_SELF  FALSE   /* TRUE or FALSE */

/* Instance name, used as an index to register with OAC */
#define SNMPTM_SERVICE_NAME  "snmptm"
#define SNMPTM_SERVICE_NAME_LEN  6
 
/* Function prototype used for MIB objects registration. */
typedef uns32 (*SNMPTM_MIBREG_FNC)(void);

/* Macro definitions for SNMPTM locks */
#define m_SNMPTM_LOCK(lock, type)    m_NCS_LOCK(lock, type)
#define m_SNMPTM_UNLOCK(lock, type)  m_NCS_UNLOCK(lock, type)

/**************************************************************************
                       Memory specific Macro definitions
**************************************************************************/
/* Service Sub IDs for SNMPTM */
typedef enum
{
   NCS_SERVICE_SNMPTM_SUB_ID_SNMPTM_CB = 1,
   NCS_SERVICE_SNMPTM_SUB_ID_TBLONE,
   NCS_SERVICE_SNMPTM_SUB_ID_TBLTHREE,
   NCS_SERVICE_SNMPTM_SUB_ID_TBLFOUR,
   NCS_SERVICE_SNMPTM_SUB_ID_TBLFIVE,
   NCS_SERVICE_SNMPTM_SUB_ID_TBLEIGHT,
   NCS_SERVICE_SNMPTM_SUB_ID_MAX
} NCS_SERVICE_SNMPTM_SUB_ID;

/* Memory allocation and release macros */

/* SNMPTM Control Block */
#define m_MMGR_ALLOC_SNMPTM_CB          malloc(sizeof(SNMPTM_CB))
#define m_MMGR_FREE_SNMPTM_CB(p)        free(p)

/* SNMPTM TBLONE */
#define m_MMGR_ALLOC_SNMPTM_TBLONE      malloc(sizeof(SNMPTM_TBLONE))
#define m_MMGR_FREE_SNMPTM_TBLONE(p)    free(p)

/* SNMPTM TBLTHREE */
#define m_MMGR_ALLOC_SNMPTM_TBLTHREE    malloc(sizeof(SNMPTM_TBLTHREE))
#define m_MMGR_FREE_SNMPTM_TBLTHREE(p)  free(p)

/* SNMPTM TBLFOUR */
#define m_MMGR_ALLOC_SNMPTM_TBLFOUR     malloc(sizeof(SNMPTM_TBLFOUR))
#define m_MMGR_FREE_SNMPTM_TBLFOUR(p)   free(p)

/* SNMPTM TBLFIVE */
#define m_MMGR_ALLOC_SNMPTM_TBLFIVE     malloc(sizeof(SNMPTM_TBLFIVE))
#define m_MMGR_FREE_SNMPTM_TBLFIVE(p)   free(p)

/* SNMPTM TBLSIX */
#define m_MMGR_ALLOC_SNMPTM_TBLSIX     malloc(sizeof(SNMPTM_TBLSIX))
#define m_MMGR_FREE_SNMPTM_TBLSIX(p)   free(p)

/* SNMPTM TBLSEVEN */
#define m_MMGR_ALLOC_SNMPTM_TBLSEVEN   malloc(sizeof(SNMPTM_TBLSEVEN))
#define m_MMGR_FREE_SNMPTM_TBLSEVEN(p) free(p)

/* SNMPTM TBLEIGHT */
#define m_MMGR_ALLOC_SNMPTM_TBLEIGHT      malloc(sizeof(SNMPTM_TBLEIGHT))
#define m_MMGR_FREE_SNMPTM_TBLEIGHT(p)    free(p)

/* SNMPTM TBLTEN */
#define m_MMGR_ALLOC_SNMPTM_TBLTEN      malloc(sizeof(SNMPTM_TBLTEN))
#define m_MMGR_FREE_SNMPTM_TBLTEN(p)    free(p)

/***************************************************************************
                            SNMPTM TRAP info                              
***************************************************************************/
/* subscription id for traps */
#define SNMPTM_EDA_SUBSCRIPTION_ID_TRAPS m_SNMP_EDA_SUBSCRIPTION_ID_TRAPS

/* Event Filter pattern */
#define SNMPTM_EDA_EVT_FILTER_PATTERN m_SNMP_TRAP_FILTER_PATTERN 

/* Filter pattern length */
#define SNMPTM_EDA_EVT_FILTER_PATTERN_LEN strlen(SNMPTM_EDA_EVT_FILTER_PATTERN)

/* Channel Name */
#define SNMPTM_EDA_EVT_CHANNEL_NAME    m_SNMP_EDA_EVT_CHANNEL_NAME    

/* Publisher Name */
#define SNMPTM_EDA_EVT_PUBLISHER_NAME  "SNMPTM"
#define SNMPTM_TRAP_PATTERN_ARRAY_LEN  3

#define m_SNMPTM_UNS64_TO_PARAM(param,buffer,val64) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = 8; \
   param->info.i_oct = (uns8 *)buffer; \
   m_NCS_OS_HTONLL_P(param->info.i_oct,val64); \
}

#define m_SNMPTM_STR_TO_PARAM(param,str) \
{\
   param->i_fmat_id = NCSMIB_FMAT_OCT; \
   param->i_length = strlen(str); \
   param->info.i_oct = (uns8 *)str; \
}

/* Function prototype */
EXTERN_C void  snmptm_main(int argc, char **argv);

#endif /* SNMPTM_DEF_H */


