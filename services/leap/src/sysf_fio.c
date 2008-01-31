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

  ncs_fio.h


..............................................................................

  DESCRIPTION: Abstractions and APIs for NCS_FIO service, a file I/O service

******************************************************************************
*/



#include "ncs_opt.h"
#include "gl_defs.h"
#include "ncs_osprm.h"

#include "ncs_fio.h"
#include "ncssysf_def.h"

uns32
ncs_fileio(NCSCONTEXT context, NCS_VRID id, NCS_FILEIO_REQUEST *request)
{
    uns32 retval = NCSCC_RC_FAILURE;

    m_NCS_ASSERT(NCS_FILEIO_CMD_SENTINAL > request->i_request);

    USE(context);
    USE(id);

    switch(request->i_request)
    {
    case NCS_FILEIO_CMD_OPEN:
        {
            uns8 *filename = request->fileio_open.i_filename;
            uns8 *openmode = request->fileio_open.i_openmode;
            FILE *fileptr = NULL;

            m_NCS_ASSERT(NULL != filename);
            if(NULL != filename)
            {
                fileptr = sysf_fopen((char *)filename, (char *)openmode);
                if(NULL != fileptr)
                {
                    request->fileio_open.o_fileptr = fileptr;
                    retval = NCSCC_RC_SUCCESS;
                }
            }
        }
        break;

    case NCS_FILEIO_CMD_CLOSE:
        {
            m_NCS_ASSERT(NULL != request->fileio_close.i_fileptr);
            if(NULL != request->fileio_close.i_fileptr)
            {
                sysf_fclose(request->fileio_close.i_fileptr);
            }
            retval = NCSCC_RC_SUCCESS;
        }
        break;

    case NCS_FILEIO_CMD_READLINE:
        {
            m_NCS_ASSERT(NULL != request->fileio_read.i_fileptr);
            if(NULL != request->fileio_read.i_fileptr)
            {
                if(NULL != fgets((char*)request->fileio_read.io_buffer,
                                 (int32)request->fileio_read.i_readsize,
                                 request->fileio_read.i_fileptr))
                {
                    request->fileio_read.o_bytecnt = m_NCS_STRLEN((char*)request->fileio_read.io_buffer);
                    retval = NCSCC_RC_SUCCESS;
                }
            }
        }
        break;

    case NCS_FILEIO_CMD_WRITELINE:
        {
            m_NCS_ASSERT(NULL != request->fileio_write.i_fileptr);
            USE(request->fileio_write.i_writesize);
            if(NULL != request->fileio_write.i_fileptr)
            {
                if(EOF != fputs((char*)request->fileio_write.i_buffer, request->fileio_write.i_fileptr))
                {
                    request->fileio_write.o_bytecnt = m_NCS_STRLEN((char*)request->fileio_write.i_buffer);
                    retval = NCSCC_RC_SUCCESS;
                }
            }
        }
        break;

    case NCS_FILEIO_CMD_SEEK:
        USE(request->fileio_seek.i_fileptr);
        USE(request->fileio_seek.i_amount);
        USE(request->fileio_seek.i_direction);
        break;

    case NCS_FILEIO_CMD_SENTINAL:
    default:
        m_NCS_ASSERT(NCS_FILEIO_CMD_SENTINAL <= request->i_request);
        break;
    }
    return retval;
}
