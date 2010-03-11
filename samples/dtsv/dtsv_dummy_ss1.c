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
..............................................................................


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  DESCRIPTION:

  This module contains the logging/tracing functions.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  FUNCTIONS INCLUDED in this module:

  log_init_ncsft..........General function to print log init message
  log_ncsft...............NCSFTlayer logging facility

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
*/

#include "dtsv_dummy_ss.h"

/*****************************************************************************

  PROCEDURE NAME:    log_messages

  DESCRIPTION:       Function used for logging dummy log's log. 

*****************************************************************************/
void log_messages1()
{
        int i;
    NCS_IP_ADDR ipadr;
    NCSFL_PDU   pdu;
    NCSFL_MEM   mdump;
 
    for (i = 0; i <= 1; i++)
    {   
        mdump.addr = "1010101";
        mdump.dump = "ABCDEFGHIJKLMNOPQRSTUVWXYZ--PARAG";
        mdump.len  = 33;
   
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_MEMDUMP, 8, 
            NCSFL_LC_API, NCSFL_SEV_ALERT, NCSFL_TYPE_TID, 0, mdump);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILLLL, 1, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILLLL, 0, 
            500, 501, 502, 503);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILI, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILI, 0, 
            500, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTL, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILTL, DUMMY_INDX, 
            500, 501);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTLLLL, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILTLLLL, DUMMY_INDX, 
            500, 501, 502, 503, 504);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTCLIL, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILTCLIL, 1, 
            500, "printing ILTCLIL", 600, DUMMY_INDX, 700, 800, "HI");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTISCLC, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILTISCLC, DUMMY_INDX, 
            500, DUMMY_INDX, 10, "1 - printing ILTISCLC", 100, "I am string 2");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTISC, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_ILTISC, DUMMY_INDX, 
            500, DUMMY_INDX, 10, "String of ILTISC");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXM, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIM, DUMMY_INDX, 500);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIMM, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIMM, DUMMY_INDX, 500, 600);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDX, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TI, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXI, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TII, DUMMY_INDX, DUMMY_INDX1);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIII, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIII, 
            DUMMY_INDX, DUMMY_INDX1, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TCIC, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TCIC, 
            "First string of TCIC ", DUMMY_INDX, "second string of TCIC ");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TICL, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TICL, 
            DUMMY_INDX, "string of TICL ", 100);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TICLL, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TICLL, 
            DUMMY_INDX, "string of TICL ", 100, 101);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TICLLL, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TICLLL, 
            DUMMY_INDX, "string of TICL ", 100, 101, 102);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TICLLLL, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TICLLLL, 
            DUMMY_INDX, "string of TICLLLL ", 100, 101, 102, 103);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TICLLLLL, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TICLLLLL, 
            DUMMY_INDX, "string of TICL ", 100, 101, 102, 103, 104);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXIC, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIIC, 
            DUMMY_INDX, DUMMY_INDX1, "I am a string of TIIC");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXICC, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIICC, 
            DUMMY_INDX, DUMMY_INDX1, "I am a string 1 of TIICC",
            "I am a string 2 of TIICC");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXIIC, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIIIC, 
            DUMMY_INDX, DUMMY_INDX1, DUMMY_INDX, "I am a string of TIIIC");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXIL, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_API, NCSFL_SEV_INFO, NCSFL_TYPE_TIIL, DUMMY_INDX, DUMMY_INDX1, 10);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL, DUMMY_INDX, 100);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TILI, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TILI, 
            DUMMY_INDX, 200, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLL, DUMMY_INDX, 
            10, 11, 12, 13);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLII, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLII, DUMMY_INDX, 
            10, 11, DUMMY_INDX, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLLL, DUMMY_INDX, 
            10, 11, 12, 13, 14);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLLLL,
            DUMMY_INDX, 10, 11, 12, 13, 14, 15);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLLLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLLLLL, 
            DUMMY_INDX, 10, 11, 12, 13, 14, 15, 16);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLLLLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLLLLLL, 
            DUMMY_INDX, 10, 11, 12, 13, 14, 15, 16, 17);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLLLLLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLLLLLLL, 
            DUMMY_INDX, 10, 11, 12, 13, 14, 15, 16, 17, 18);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILLLLLLLLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLLLLLLLLL,
            DUMMY_INDX, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TILL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILL, DUMMY_INDX, 10, 11);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 10, 11, 12);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_CHAR_STR, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TC,  
            "DUMMY 1: Print this string please ");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDX, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TI, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 20, 21, 22);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_CHAR_STR, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TC, 
            "DUMMY 1: Print this string please ");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ipadr.type = NCS_IP_ADDR_TYPE_IPV4;
        ipadr.info.v4 = 0x78798082;
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDX, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TI, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TILAL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TILAL, 
            DUMMY_INDX, 1000, ipadr, 2000);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 10, 11, 12);\
            m_NCS_TASK_SLEEP(rand() % 10);
        
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_CHAR_STR, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TC,  
            "DUMMY 1: Print this string please ");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_IP_ADDR, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIA, DUMMY_INDX, 
            ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_IP_ADDR, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIA, DUMMY_INDX, 
            ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIAIA, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIAIA, DUMMY_INDX, 
            ipadr, DUMMY_INDX, ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIALAL, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIALAL, DUMMY_INDX, 
            ipadr, 1, ipadr, 2);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIALALLLL, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIALALLLL, DUMMY_INDX, 
            ipadr, 1, ipadr, 2, 3, 4, 5);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIALALLLLL, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIALALLLLL, DUMMY_INDX, 
            ipadr, 1, ipadr, 2, 3, 4, 5, 6);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIALALLLLLL, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIALALLLLLL, DUMMY_INDX, 
            ipadr, 1, ipadr, 2, 3, 4, 5, 6, 7);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIAD, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIAD, DUMMY_INDX, 
            ipadr, mdump);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_TIALL, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIALL, DUMMY_INDX, 
            ipadr, 100, 200);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_IP_ADDR, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIA, DUMMY_INDX, 
            ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        pdu.len = 26;
        pdu.dump = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_PDU, DUMMY_FC_PRT_PDU, 
            NCSFL_LC_DATA, NCSFL_SEV_ALERT, NCSFL_TYPE_TIP, DUMMY_INDX, 
            pdu);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDX, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TI, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 20, 21, 22);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDX, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TI, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 10, 11, 12);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_CHAR_STR, DUMMY_FC_PRT_CHAR_STR, 
            NCSFL_LC_MISC, NCSFL_SEV_ALERT, NCSFL_TYPE_TC, 
            "DUMMY 1: Print this string please ");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_IP_ADDR, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIA, DUMMY_INDX, 
            ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_IP_ADDR, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIA, DUMMY_INDX, 
            ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_IP_ADDR, DUMMY_FC_PRT_IP_ADDR, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIA, DUMMY_INDX, 
            ipadr);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_PDU, DUMMY_FC_PRT_PDU, 
            NCSFL_LC_DATA, NCSFL_SEV_ALERT, NCSFL_TYPE_TIP, DUMMY_INDX, 
            pdu);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDX, DUMMY_FC_PRT_INDX, 
            NCSFL_LC_HEADLINE, NCSFL_SEV_INFO, NCSFL_TYPE_TI, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_INDXL, DUMMY_FC_PRT_INDXL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_EMERGENCY, NCSFL_TYPE_TIL, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 20, 21, 22);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1,DUMMY_LOG_INDXLLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_TILLL, DUMMY_INDX, 20, 21, 22);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTI, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTI, 
            DUMMY_INDX, 20, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIM, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIM, 
            DUMMY_INDX, 20, DUMMY_INDX, 1);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIL, 
            DUMMY_INDX, 20, DUMMY_INDX, 1);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTILI, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTILI, 
            DUMMY_INDX, 20, DUMMY_INDX, 1, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTILIL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTILIL, 
            DUMMY_INDX, 20, DUMMY_INDX, 1, DUMMY_INDX, 1000);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIC, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIC, 
            DUMMY_INDX, 20, DUMMY_INDX, "printing string of ILTIC");
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTICL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTICL, 
            DUMMY_INDX, 20, DUMMY_INDX, "printing string of ILTICL", 1000);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTII, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTII, 
            DUMMY_INDX, 20, DUMMY_INDX, DUMMY_INDX);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIIL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIIL, 
            DUMMY_INDX, 20, DUMMY_INDX, DUMMY_INDX, 2);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIILM, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIILM, 
            DUMMY_INDX, 20, DUMMY_INDX, DUMMY_INDX, 2, 200);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIILML, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIILML, 
            DUMMY_INDX, 20, DUMMY_INDX, DUMMY_INDX, 2, 200, 3000);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIILL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIILL, 
            DUMMY_INDX, 20, DUMMY_INDX, DUMMY_INDX, 2, 200);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTIILLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTIILLL, 
            DUMMY_INDX, 20, DUMMY_INDX, DUMMY_INDX, 2, 200, 3000);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTILL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTILL, 
            DUMMY_INDX, 20, DUMMY_INDX, 1, 2);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTILLM, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTILLM, 
            DUMMY_INDX, 20, DUMMY_INDX, 1, 2, 200);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_ILTILLL, DUMMY_FC_PRT_INDXLLL, 
            NCSFL_LC_FUNC_ENTRY, NCSFL_SEV_ALERT, NCSFL_TYPE_ILTILLL, 
            DUMMY_INDX, 20, DUMMY_INDX, 1, 2, 3);
        m_NCS_TASK_SLEEP(rand() % 10);

        /* Added logging for float values */
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIF, DUMMY_FC_PRT_TIF,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIF,
            DUMMY_INDX, 1212.3432);
        m_NCS_TASK_SLEEP(rand() % 10);
        
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIF, DUMMY_FC_PRT_TIF,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIF,
            DUMMY_INDX, (double)18446744073709551615uLL);
        m_NCS_TASK_SLEEP(rand() % 10);

        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIN, DUMMY_FC_PRT_TIN,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIN,
            DUMMY_INDX, (long long)-1234567891011LL);
        m_NCS_TASK_SLEEP(rand() % 10);

        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIU, DUMMY_FC_PRT_TIU,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIU,
            DUMMY_INDX, (unsigned long long)1234567891011ULL);
        m_NCS_TASK_SLEEP(rand() % 10);

        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIX, DUMMY_FC_PRT_TIX,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIX,
            DUMMY_INDX, (long long)0xeeeeeeeeffffffffLL);
        m_NCS_TASK_SLEEP(rand() % 10);

/*To check for maximum value in 64 bits in TIX format*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIX, DUMMY_FC_PRT_TIX,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIX,
            DUMMY_INDX_MAX, 18446744073709551615ULL);

/*To check for minimum value in 64 bits in TIX format*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIX, DUMMY_FC_PRT_TIX,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIX,
            DUMMY_INDX_MIN, 0x8000000000000000LL);

/*To check for maximum value of unsigned long long  in TIU format*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIU, DUMMY_FC_PRT_TIU,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIU,
            DUMMY_INDX_MAX, (unsigned long long)18446744073709551615ULL);

/*To check for minimum value of unsigned long long in TIU format*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIU, DUMMY_FC_PRT_TIU,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIU,
            DUMMY_INDX_MIN, (unsigned long long)0ULL);

/*To check for maximum value of signed long long  in TIN format*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIN, DUMMY_FC_PRT_TIN,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIN,
            DUMMY_INDX_MAX, (long long)9223372036854775807LL);

/*To check for minimum value of signed long long  in TIN format*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIN, DUMMY_FC_PRT_TIN,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIN,
            DUMMY_INDX_MIN, 0x8000000000000000LL);

/*To check for combination of D & NUX format specifiers handling*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TIDNUX, DUMMY_FC_PRT_INDXMLLL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TIDNUX, DUMMY_INDX,
            mdump, (long long)223372036854775808LL,(unsigned long long)1234567891011ULL,(long long)223372036854775808LL );
   
/*To check for combination of  NUX & D format specifiers handling*/
        ncs_logmsg(UD_DTSV_DEMO_SERVICE1, DUMMY_LOG_TINUXD, DUMMY_FC_PRT_INDXMLLL,
            NCSFL_LC_HEADLINE, NCSFL_SEV_ALERT, NCSFL_TYPE_TINUXD, DUMMY_INDX1,
            (long long)223372036854775808LL,(unsigned long long)1234567891011ULL,(long long)223372036854775808LL, mdump);

        m_NCS_TASK_SLEEP(LOG_DELAY * 2);
        
    }
}


void reg_dummy1_ss()
{

   NCS_DTSV_RQ        reg;

   reg.i_op = NCS_DTSV_OP_BIND;
   reg.info.bind_svc.svc_id = UD_DTSV_DEMO_SERVICE1;
   reg.info.bind_svc.version = 2;
   strcpy(reg.info.bind_svc.svc_name, "dummy");
   ncs_dtsv_su_req(&reg);
}

void de_reg_dummy1_ss()
{
   NCS_DTSV_RQ        reg;

   reg.i_op = NCS_DTSV_OP_UNBIND;
   reg.info.unbind_svc.svc_id = UD_DTSV_DEMO_SERVICE1;
   ncs_dtsv_su_req(&reg);

}



