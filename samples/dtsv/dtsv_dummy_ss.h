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
 */

/*****************************************************************************

  DESCRIPTION:

  This module contains logging/tracing functions.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef DUMMY_LOG_H
#define DUMMY_LOG_H

#include "dta_papi.h"
#include "dts_papi.h"

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                          DTS Logging Control

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/******************************************************************************
 Logging offset indexes for canned constant strings for the ASCII SPEC
 ******************************************************************************/

#define DUMMY_INDX 0
#define DUMMY_INDX1 1
#define DUMMY_INDX_MIN 1
#define DUMMY_INDX_MAX 2

typedef enum dummy_sets
  {
    DUMMY_FC_PRT_INDX,
    DUMMY_FC_PRT_INDXL,
    DUMMY_FC_PRT_INDXLLL,
    DUMMY_FC_PRT_CHAR_STR,
    DUMMY_FC_PRT_IP_ADDR,
    DUMMY_FC_PRT_INDXM,
    DUMMY_FC_PRT_PDU,
    DUMMY_FC_PRT_MEMDUMP,
    DUMMY_FC_PRT_ILTIS,
    DUMMY_FC_PRT_TIF,
    DUMMY_FC_PRT_TIN,
    DUMMY_FC_PRT_TIU,
    DUMMY_FC_PRT_TIX,
    DUMMY_FC_PRT_INDXMLLL
  } DUMMY_SETS;


typedef enum dummy_log_ids
  {
    DUMMY_LOG_INDX,
    DUMMY_LOG_INDXI,
    DUMMY_LOG_INDXIC,
    DUMMY_LOG_INDXICC,
    DUMMY_LOG_INDXIIC,
    DUMMY_LOG_INDXIL,
    DUMMY_LOG_INDXL,
    DUMMY_LOG_INDXLLL,
    DUMMY_LOG_CHAR_STR,
    DUMMY_LOG_IP_ADDR,
    DUMMY_LOG_INDXM,
    DUMMY_LOG_PDU,
    DUMMY_LOG_MEMDUMP,
    DUMMY_LOG_ILTIS,
    DUMMY_LOG_TIII,
    DUMMY_LOG_TCIC,
    DUMMY_LOG_TILL,
    DUMMY_LOG_TILLL,
    DUMMY_LOG_TILLLL, 
    DUMMY_LOG_TILLLLL,
    DUMMY_LOG_TILLLLLL, 
    DUMMY_LOG_TILLLLLLL,
    DUMMY_LOG_TILLLLLLLL,
    DUMMY_LOG_TILLLLLLLLL,
    DUMMY_LOG_TILLLLLLLLLL,
    DUMMY_LOG_TIMM,
    DUMMY_LOG_TIAIA,
    DUMMY_LOG_TIAD, 
    DUMMY_LOG_ILTI, 
    DUMMY_LOG_ILTIM,
    DUMMY_LOG_ILTIL,
    DUMMY_LOG_ILTII,
    DUMMY_LOG_ILTIIL,
    DUMMY_LOG_ILTILL,
    DUMMY_LOG_ILTILLL,
    DUMMY_LOG_TIALAL, 
    DUMMY_LOG_TIALL, 
    DUMMY_LOG_ILLLL, 
    DUMMY_LOG_ILTL, 
    DUMMY_LOG_ILTCLIL, 
    DUMMY_LOG_ILTISC,
    DUMMY_LOG_ILTISCLC, 
    DUMMY_LOG_ILTIC, 
    DUMMY_LOG_ILTICL,
    DUMMY_LOG_ILTILI,
    DUMMY_LOG_ILTIILM,
    DUMMY_LOG_ILTIILL,
    DUMMY_LOG_ILTIILML,
    DUMMY_LOG_ILTIILLL,
    DUMMY_LOG_ILTILIL, 
    DUMMY_LOG_ILTLLLL, 
    DUMMY_LOG_ILTILLM, 
    DUMMY_LOG_TILAL, 
    DUMMY_LOG_TILLII,
    DUMMY_LOG_TILI, 
    DUMMY_LOG_ILI, 
    DUMMY_LOG_TIALALLLL,
    DUMMY_LOG_TIALALLLLL,
    DUMMY_LOG_TIALALLLLLL,
    DUMMY_LOG_TICL,
    DUMMY_LOG_TICLL,
    DUMMY_LOG_TICLLL,
    DUMMY_LOG_TICLLLL,
    DUMMY_LOG_TICLLLLL,
    DUMMY_LOG_TICCLLLL,
    DUMMY_LOG_TIIC,
    DUMMY_LOG_TIICC,
    DUMMY_LOG_TIF,
    DUMMY_LOG_TIN, 
    DUMMY_LOG_TIU, 
    DUMMY_LOG_TIX, 
    DUMMY_LOG_TIDNUX,
    DUMMY_LOG_TINUXD
  } DUMMY_LOG_IDS;


EXTERN_C void log_messages1(void);
EXTERN_C void reg_dummy1_ss(void);
EXTERN_C void de_reg_dummy1_ss(void);

#define LOG_DELAY           1000


#endif /* DUMMY_LOG_H */

