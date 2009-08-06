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

 MODULE NAME:  CLI_MEM.H

..............................................................................

  DESCRIPTION:

  Header file for Command Tree and its utility functions.

******************************************************************************
*/

#ifndef CLI_MEM_H
#define CLI_MEM_H

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                Common Include Files.

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*NCS_SERVICE_CLI_SUB_ID_OUTOFBOUNDARY is added to fix the bug 59854*/
/* Service Sub IDs for LMS */
typedef enum 
{
    NCS_SERVICE_CLI_SUB_ID_CLI_CB = 1,
    NCS_SERVICE_CLI_SUB_ID_CLI_CMD_HISTORY,
    NCS_SERVICE_CLI_SUB_ID_CLI_CMD_NODE,
    NCS_SERVICE_CLI_SUB_ID_CLI_CMD_ELEMENT,
    NCS_SERVICE_CLI_SUB_ID_CLI_RANGE,
    NCS_SERVICE_CLI_SUB_ID_CLI_DEFAULT_VAL,
    NCS_SERVICE_CLI_SUB_ID_NCSCLI_BINDERY,
    NCS_SERVICE_SHIM_SHIM_MSG,    
    NCS_SERVICE_CLI_SUB_ID_OUTOFBOUNDARY_TOKEN,
} NCS_SERVICE_CLI_SUB_ID;
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

                        Memory Allocation and Release Macros 

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#define m_MMGR_ALLOC_CLI_CB    (CLI_CB*)m_NCS_MEM_ALLOC(sizeof(CLI_CB), \
                                    NCS_MEM_REGION_PERSISTENT, \
                                    NCS_SERVICE_ID_CLI, \
                                    NCS_SERVICE_CLI_SUB_ID_CLI_CB)
#define m_MMGR_FREE_CLI_CB(p)  m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                    NCS_SERVICE_ID_CLI, \
                                    NCS_SERVICE_CLI_SUB_ID_CLI_CB)

#define m_MMGR_ALLOC_CLI_CMD_HISTORY    (CLI_CMD_HISTORY*)m_NCS_MEM_ALLOC(sizeof(CLI_CMD_HISTORY), \
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_CLI, \
                                            NCS_SERVICE_CLI_SUB_ID_CLI_CMD_HISTORY)
#define m_MMGR_FREE_CLI_CMD_HISTORY(p)  m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_CLI, \
                                            NCS_SERVICE_CLI_SUB_ID_CLI_CMD_HISTORY)

#define m_MMGR_ALLOC_CLI_CMD_NODE           (CLI_CMD_NODE*)m_NCS_MEM_ALLOC(sizeof(CLI_CMD_NODE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                NCS_SERVICE_CLI_SUB_ID_CLI_CMD_NODE)
#define m_MMGR_FREE_CLI_CMD_NODE(p)         m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                NCS_SERVICE_CLI_SUB_ID_CLI_CMD_NODE)

#define m_MMGR_ALLOC_CLI_CMD_ELEMENT        (CLI_CMD_ELEMENT*)m_NCS_MEM_ALLOC(sizeof(CLI_CMD_ELEMENT), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                NCS_SERVICE_CLI_SUB_ID_CLI_CMD_ELEMENT)
#define m_MMGR_FREE_CLI_CMD_ELEMENT(p)      m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                NCS_SERVICE_CLI_SUB_ID_CLI_CMD_ELEMENT)

#define m_MMGR_ALLOC_CLI_RANGE              (CLI_RANGE*)m_NCS_MEM_ALLOC(sizeof(CLI_RANGE), \
                                                NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                NCS_SERVICE_CLI_SUB_ID_CLI_RANGE)
#define m_MMGR_FREE_CLI_RANGE(p)            m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                NCS_SERVICE_ID_CLI, \
                                                NCS_SERVICE_CLI_SUB_ID_CLI_RANGE)

#define m_MMGR_ALLOC_NCSCLI_BINDERY              (NCSCLI_BINDERY*)m_NCS_MEM_ALLOC(sizeof(NCSCLI_BINDERY), \
                                                    NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_CLI, \
                                                    NCS_SERVICE_CLI_SUB_ID_NCSCLI_BINDERY)
#define m_MMGR_FREE_NCSCLI_BINDERY(p)            m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_CLI, \
                                                    NCS_SERVICE_CLI_SUB_ID_NCSCLI_BINDERY)

#define m_MMGR_ALLOC_SHIM_MSG              (SHIM_MSG*)m_NCS_MEM_ALLOC(sizeof(SHIM_MSG), \
                                                    NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_CLI, \
                                                    NCS_SERVICE_SHIM_SHIM_MSG)
#define m_MMGR_FREE_SHIM_MSG(p)            m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_CLI, \
                                                    NCS_SERVICE_SHIM_SHIM_MSG)
/* OUTOFBOUNDARY_TOKEN aoolc & free are added while fixing the bug 59854 */
#define m_MMGR_ALLOC_OUTOFBOUNDARY_TOKEN(numofbytes) (int8*)m_NCS_MEM_ALLOC(numofbytes, \
                                                    NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_CLI, \
                                                    NCS_SERVICE_CLI_SUB_ID_OUTOFBOUNDARY_TOKEN)
#define m_MMGR_FREE_OUTOFBOUNDARY_TOKEN(p)            m_NCS_MEM_FREE(p,NCS_MEM_REGION_PERSISTENT, \
                                                    NCS_SERVICE_ID_CLI, \
                                                    NCS_SERVICE_CLI_SUB_ID_OUTOFBOUNDARY_TOKEN)
#endif
