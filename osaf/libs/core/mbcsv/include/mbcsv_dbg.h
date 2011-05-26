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
#ifndef MBCSV_DBG_H
#define MBCSV_DBG_H

/*
 * m_MBCSV_DBG_SINK
 *
 * If MBCSV fails an if-conditional or other test that we would not expect
 * under normal conditions, it will call this macro. 
 * 
 * If MBCSV_DEBUG == 1, fall into the sink function. A customer can put
 * a breakpoint there or leave the CONSOLE print statement, etc.
 *
 * If MBCSV_DEBUG == 0, just echo the value passed. It is typically a 
 * return code or a NULL.
 *
 * MBCSV_DEBUG can be enabled in mbcsv_opt,h
 */

#if (MBCSV_LOG == 1)

uns32 mbcsv_dbg_sink(uns32, char *, long, char *);
uns32 mbcsv_dbg_sink_svc(uns32, char *, uns32, char *, uns32);

/* m_MBCSV_DBG_VOID() used to keep compiler happy @ void return functions */

#define m_MBCSV_DBG_SINK(r, s)  mbcsv_dbg_sink(__LINE__,__FILE__,(long)r, (char*)s)
#define m_MBCSV_DBG_SINK_SVC(r, s, sid)  mbcsv_dbg_sink_svc(__LINE__,__FILE__,(uns32)r, (char*)s, (uns32)sid)
#else

#define m_MBCSV_DBG_SINK(r, s)  r
#define m_MBCSV_DBG_SINK_SVC(r, s, sid) r
#endif   /*MBCSV_LOG == 1 */

#endif   /* MBCSV_DBG_H */
