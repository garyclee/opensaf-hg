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


  ncs_cio.h


..............................................................................

  DESCRIPTION: Abstractions and APIs for NCS_CIO service, a console I/O service

******************************************************************************
*/


#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"
#include "ncssysf_def.h"
#include "ncs_cio.h"

uns32
ncs_conio(NCSCONTEXT context, NCS_VRID id, NCS_CONIO_REQUEST *request)
{
    uns32 retval = NCSCC_RC_FAILURE;

    assert(NCS_CONIO_CMD_SENTINAL > request->i_request);

    USE(context);
    USE(id);

    switch(request->i_request)
    {
    case NCS_CONIO_CMD_PUTCHR:            /* put a character to console   */
        m_NCS_CONS_PUTCHAR(request->putchr.i_char);
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_GETCHR:            /* get a character from console */
        request->getchr.o_char = m_NCS_CONS_GETCHAR();
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_PUTSTR:            /* put a string to console      */
        request->putstr.o_bytecnt = m_NCS_CONS_PRINTF((const char *)"%s",
                                                     (char *)request->putstr.i_string);
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_GETSTR:            /* get a string from console    */
        fgets((char *)request->getstr.i_string, request->getstr.i_strlen, stdin);
        request->getstr.o_bytecnt = (int32)strlen((char*)request->getstr.i_string);
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_UNBUF_PUTCHR:      /* put an unbuffered character  */
        m_NCS_CONS_PUTCHAR(request->unbuf_putchr.i_char);
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_UNBUF_GETCHR:      /* get an unbuffered character  */
        request->unbuf_getchr.o_char = m_NCS_CONS_UNBUF_GETCHAR();
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_FFLUSH:            /* flush a buffered console     */
        fflush((FILE*)request->fflush.i_stream);
        retval = NCSCC_RC_SUCCESS;
        break;

    case NCS_CONIO_CMD_SENTINAL:
    default:
        assert(NCS_CONIO_CMD_SENTINAL <= request->i_request);
        break;
    }
    return retval;
}





