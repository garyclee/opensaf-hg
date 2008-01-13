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

  
  .....................................................................  
  
  DESCRIPTION: This file describes the Memory management macros 
  
  ***************************************************************************/ 
#ifndef SUBAGT_MEM_H
#define SUBAGT_MEM_H

typedef enum
{
    NCS_SERVICE_SNMPSUBAGT_CB, 
    NCS_SERVICE_SNMPSUBAGT_OID_DB_ELEM, 
    NCS_SERVICE_SNMPSUBAGT_OID_DB_NODE, 
    NCS_SERVICE_SNMPSUBAGT_MBX,
    NCS_SERVICE_SNMPSUBAGT_MAX 
}NCS_SERVICE_SNMPSUBAGT_ID;

/* Memory mgmt macros */

/* For Control Block */
#define m_MMGR_NCSSA_CB_ALLOC   (NCSSA_CB *)m_NCS_MEM_ALLOC(sizeof(NCSSA_CB),\
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_SNMPSUBAGT, \
                                            NCS_SERVICE_SNMPSUBAGT_CB)

#define m_MMGR_NCSSA_CB_FREE(p)  m_NCS_MEM_FREE(p, \
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_SNMPSUBAGT,\
                                            NCS_SERVICE_SNMPSUBAGT_CB)

/* for the base oid */
#define m_MMGR_NCSSA_OID_DB_ELEM_ALLOC(size) \
                                 (oid*) m_NCS_MEM_ALLOC(((size)*sizeof(oid)), \
                                           NCS_MEM_REGION_PERSISTENT, \
                                           NCS_SERVICE_ID_SNMPSUBAGT, \
                                           NCS_SERVICE_SNMPSUBAGT_OID_DB_ELEM)

#define m_MMGR_NCSSA_OID_DB_ELEM_FREE(p) m_NCS_MEM_FREE(p, \
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_SNMPSUBAGT,\
                                            NCS_SERVICE_SNMPSUBAGT_OID_DB_ELEM)

/* for the oid to table-id data base node*/
#define m_MMGR_NCSSA_OID_DB_NODE_ALLOC \
   (NCSSA_OID_DATABASE_NODE*)m_NCS_MEM_ALLOC(sizeof(NCSSA_OID_DATABASE_NODE), \
                                             NCS_MEM_REGION_PERSISTENT, \
                                             NCS_SERVICE_ID_SNMPSUBAGT, \
                                             NCS_SERVICE_SNMPSUBAGT_OID_DB_NODE) 
#define m_MMGR_NCSSA_OID_DB_NODE_FREE(p) m_NCS_MEM_FREE(p, \
                                            NCS_MEM_REGION_PERSISTENT, \
                                            NCS_SERVICE_ID_SNMPSUBAGT,\
                                            NCS_SERVICE_SNMPSUBAGT_OID_DB_NODE)

/* for the SNMP-subagent mailbox data structure */
#define m_MMGR_SNMPSUBAGT_MBX_MSG_ALLOC\
    (SNMPSUBAGT_MBX_MSG*)m_NCS_MEM_ALLOC(sizeof(SNMPSUBAGT_MBX_MSG), \
                                         NCS_MEM_REGION_PERSISTENT,\
                                         NCS_SERVICE_ID_SNMPSUBAGT, \
                                         NCS_SERVICE_SNMPSUBAGT_MBX)
    
#define m_MMGR_SNMPSUBAGT_MBX_MSG_FREE(p) m_NCS_MEM_FREE(p, \
                                         NCS_MEM_REGION_PERSISTENT,\
                                         NCS_SERVICE_ID_SNMPSUBAGT, \
                                         NCS_SERVICE_SNMPSUBAGT_MBX)

/* To add to the list of MIBs registered */
#define m_MMGR_SNMPSUBAGT_MIBS_REG_LIST_ALLOC\
    (SNMPSUBAGT_MIBS_REG_LIST*)m_NCS_MEM_ALLOC(sizeof(SNMPSUBAGT_MIBS_REG_LIST), \
                                         NCS_MEM_REGION_PERSISTENT,\
                                         NCS_SERVICE_ID_SNMPSUBAGT, \
                                         NCS_SERVICE_SNMPSUBAGT_MBX)
    
#define m_MMGR_SNMPSUBAGT_MIBS_REG_LIST_FREE(p) m_NCS_MEM_FREE(p, \
                                         NCS_MEM_REGION_PERSISTENT,\
                                         NCS_SERVICE_ID_SNMPSUBAGT, \
                                         NCS_SERVICE_SNMPSUBAGT_MBX)

#endif


