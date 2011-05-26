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
#ifndef DTS_TGT_H
#define DTS_TGT_H

/*
 * m_DTS_DBG_SINK
 *
 * If DTS fails an if-conditional or other test that we would not expect
 * under normal conditions, it will call this macro. 
 * 
 * If DTS_DEBUG == 1, fall into the sink function. A customer can put
 * a breakpoint there or leave the CONSOLE print statement, etc.
 *
 * If DTS_DEBUG == 0, just echo the value passed. It is typically a 
 * return code or a NULL.
 *
 * DTS_DEBUG can be enabled in dts_opt,h
 */

#if ((DTS_DEBUG == 1) || (DTS_LOG == 1))

uns32 dts_dbg_sink(uns32, char *, uns32, char *);
uns32 dts_dbg_sink_svc(uns32, char *, uns32, char *, uns32);
/* Added new function to display service name */
uns32 dts_dbg_sink_svc_name(uns32, char *, uns32, char *, char *);
/* m_DTS_DBG_VOID() used to keep compiler happy @ void return functions */

#define m_DTS_DBG_SINK(r, s)  dts_dbg_sink(__LINE__,__FILE__,(uns32)r, (char*)s)
#define m_DTS_DBG_SINK_SVC(r, s, svc)  dts_dbg_sink_svc(__LINE__,__FILE__,(uns32)r, (char*)s, svc)
/* Added new macro to map to the new function */
#define m_DTS_DBG_SINK_SVC_NAME(r, s, svc)  dts_dbg_sink_svc_name(__LINE__,__FILE__,(uns32)r, (char*)s, (char*)svc)
#define m_DTS_DBG_VOID     dts_dbg_sink(__LINE__,__FILE__,1)
#else

#define m_DTS_DBG_SINK(r, s)  r
#define m_DTS_DBG_SINK_SVC(r, s, svc)
#define m_DTS_DBG_SINK_SVC_NAME(r, s, svc)
#define m_DTS_DBG_VOID
#endif

/*
 * m_<DTS>_VALIDATE_HDL
 *
 * VALIDATE_HDL   : in the LM create service for DTS, they each
 *                  return an NCSCONTEXT value called o_<DTS>_hdl (such as
 *                  o_mac_hdl. This value represents the internal control block
 *                  of that DTS sub-part, scoped by a particalar virtual router.
 *
 *                  When this macro is invoked, DTS wants the target service to
 *                  provide that handle value back. DTS does not know or care
 *                  how the value is stored or recovered. This is the business
 *                  of the target system.
 *   
 * NOTE: The default implementation here, is only an example. A real system can
 *       store this however it wants.
 */

void *sysf_dts_validate(uns32 k);

/* The DTS validate macro */

#define m_DTS_VALIDATE_HDL(k)  sysf_dts_validate(k)

#endif   /* DTS_TGT_H */
