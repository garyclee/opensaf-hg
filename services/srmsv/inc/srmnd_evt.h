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

  MODULE NAME: SRMND_EVT.H

$Header: 
..............................................................................

  DESCRIPTION: This file has the data structure definitions defined for SRMND 
               specific event formats.

******************************************************************************/

#ifndef SRMND_EVT_H
#define SRMND_EVT_H

typedef enum srmnd_evt_type
{
  SRMND_SRMA_EVT_TYPE = 1,
  SRMND_AMF_EVT_TYPE,
  SRMND_TMR_EVT_TYPE,
  SRMND_HC_START_EVT_TYPE,
  SRMND_SRMA_DN_EVT_TYPE,
  SRMND_SIG_EVT_TYPE,
  SRMND_MAX_EVT_TYPE
} SRMND_EVT_TYPE;

typedef struct srmnd_evt
{
   NCS_IPC_MSG    msg;
   SRMND_EVT_TYPE evt_type;
   MDS_DEST       dest;
   uns32          cb_handle;

   union
   {
      uns32     rsrc_hdl;
      SRMA_MSG  *srma_msg;
      SRMND_MSG *srmnd_msg;
      SaInvocationT invocation;
   }info; 

} SRMND_EVT;

/****************************************************************************
                     Function Prototypes
****************************************************************************/
EXTERN_C SRMND_EVT *srmnd_evt_create(uns32 cb_hdl,
                                     MDS_DEST *dest,
                                     NCSCONTEXT msg,
                                     SRMND_EVT_TYPE evt);
EXTERN_C void srmnd_evt_process(SRMND_EVT *evt);
EXTERN_C void srmnd_evt_destroy(SRMND_EVT *evt);



#endif /* SRMND_EVT_H */
