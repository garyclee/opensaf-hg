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

*******************************************************************************/

/*
 * Module Inclusion Control...
 */
#ifndef DTA_TGT_H
#define DTA_TGT_H

/*
 * m_DTA_DBG_SINK
 *
 * If DTA fails an if-conditional or other test that we would not expect
 * under normal conditions, it will call this macro. 
 * 
 * If DTA_DEBUG == 1, fall into the sink function. A customer can put
 * a breakpoint there or leave the CONSOLE print statement, etc.
 *
 * If DTA_DEBUG == 0, just echo the value passed. It is typically a 
 * return code or a NULL.
 *
 * DTA_DEBUG can be enabled in dta_opt,h
 */

#if (DTA_DEBUG == 1)

uns32 dta_dbg_sink(uns32, char *, uns32, char *);
uns32 dta_dbg_sink_svc(uns32, char *, uns32, char *, uns32);

/* m_DTA_DBG_VOID() used to keep compiler happy @ void return functions */

#define m_DTA_DBG_SINK(r, s)  dta_dbg_sink(__LINE__,__FILE__,(uns32)r, (char*)s)
#define m_DTA_DBG_SINK_SVC(r, s, svc)  dta_dbg_sink_svc(__LINE__,__FILE__,(uns32)r, (char*)s, svc)
#define m_DTA_DBG_VOID     dta_dbg_sink(__LINE__,__FILE__,1)
#define m_DTA_DBG_SINK_VOID(r, s)  dta_dbg_sink(__LINE__,__FILE__,(uns32)r, (char*)s)
#else

#define m_DTA_DBG_SINK(r, s)  r
#define m_DTA_DBG_SINK_SVC(r, s, svc) r
#define m_DTA_DBG_VOID
#define m_DTA_DBG_SINK_VOID(r, s)
#endif

#endif   /* DTA_TGT_H */
