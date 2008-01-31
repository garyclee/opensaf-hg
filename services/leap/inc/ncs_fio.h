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

/*
 * Module Inclusion Control...
 */
#ifndef NCS_FIO_H
#define NCS_FIO_H


/*****************************************************************************
*   Commands for FILE IO                                                     *
*****************************************************************************/
typedef enum ncs_fileio_cmd
{
    NCS_FILEIO_CMD_OPEN,           /* open a file                  */
    NCS_FILEIO_CMD_CLOSE,          /* close a file                 */
    NCS_FILEIO_CMD_READLINE,       /* read a line from a file      */
    NCS_FILEIO_CMD_WRITELINE,      /* write a line from a file     */
    NCS_FILEIO_CMD_SEEK,           /* move around in a file        */
    NCS_FILEIO_CMD_SENTINAL        /* no comma                     */
} NCS_FILEIO_CMD;



/*****************************************************************************
*   File Open API                                                            *
*****************************************************************************/
typedef struct ncs_fileio_open
{
    uns8 *i_filename;             /* name of file to open         */
    uns8 *i_openmode;             /* mode (r,w,a,+) to open file  */
    FILE *o_fileptr;              /* file pointer to pass back in */
} NCS_FILEIO_OPEN;


/*****************************************************************************
*   File Close API                                                           *
*****************************************************************************/
typedef struct ncs_fileio_close
{
    FILE *i_fileptr;              /* file pointer of open file    */
} NCS_FILEIO_CLOSE;



/*****************************************************************************
*   File Read API                                                            *
*****************************************************************************/
typedef struct ncs_fileio_read
{
    FILE *i_fileptr;              /* file pointer of open file    */
    uns32 i_readsize;             /* buffer to fill from read     */
    uns8 *io_buffer;              /* size of buffer to recv read  */
    uns32 o_bytecnt;              /* byte count actually read     */
} NCS_FILEIO_READ;


/*****************************************************************************
*   File Read API                                                            *
*****************************************************************************/
typedef struct ncs_fileio_write
{
    FILE *i_fileptr;              /* file pointer of open file    */
    uns8 *i_buffer;               /* buffer to write to file      */
    uns32 i_writesize;            /* size of buffer to write      */
    uns32 o_bytecnt;              /* byte count written           */
} NCS_FILEIO_WRITE;


/*****************************************************************************
*   File Seek API                                                            *
*****************************************************************************/
typedef struct ncs_fileio_seek
{
    FILE *i_fileptr;              /* file pointer of open file    */
    int32 i_amount;               /* number of bytes to move      */
    int32 i_direction;            /* direction of move            */
} NCS_FILEIO_SEEK;



/*****************************************************************************
*   File IO API                                                              *
*****************************************************************************/
typedef struct ncs_fileio_request
{
    NCS_FILEIO_CMD   i_request;         /* request initiated by user */

    /* FILE IO requests */
    NCS_FILEIO_OPEN  fileio_open;       /* file open   */
    NCS_FILEIO_CLOSE fileio_close;      /* file close  */
    NCS_FILEIO_READ  fileio_read;       /* file read   */
    NCS_FILEIO_WRITE fileio_write;      /* file write  */
    NCS_FILEIO_SEEK  fileio_seek;       /* file seek   */
} NCS_FILEIO_REQUEST;

typedef uns32(*NCS_FILEIO)(NCSCONTEXT, NCS_VRID id, NCS_FILEIO_REQUEST *request);

uns32 ncs_fileio(NCSCONTEXT context, NCS_VRID id, NCS_FILEIO_REQUEST *request);

#endif /* NCS_FIO_H */

