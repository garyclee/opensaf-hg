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

/*
 * Module Inclusion Control...
 */
#ifndef NCS_CIO_H
#define NCS_CIO_H

typedef enum ncs_conio_keys {
	kbNone,
	kbF1 = NCS_OS_KEY_F1,
	kbF2 = NCS_OS_KEY_F2,
	kbF3 = NCS_OS_KEY_F3,
	kbF4 = NCS_OS_KEY_F4,
	kbF5 = NCS_OS_KEY_F5,
	kbF6 = NCS_OS_KEY_F6,
	kbF7 = NCS_OS_KEY_F7,
	kbF8 = NCS_OS_KEY_F8,
	kbF9 = NCS_OS_KEY_F9,
	kbF10 = NCS_OS_KEY_F10,
	kbF11 = NCS_OS_KEY_F11,
	kbF12 = NCS_OS_KEY_F12,
	kbHome = NCS_OS_KEY_HOME,
	kbEnd = NCS_OS_KEY_END,
	kbPgUp = NCS_OS_KEY_PGUP,
	kbPgDn = NCS_OS_KEY_PGDN,
	kbIns = NCS_OS_KEY_INS,
	kbDel = NCS_OS_KEY_DEL,
	kbUp = NCS_OS_KEY_UP,
	kbDown = NCS_OS_KEY_DOWN,
	kbLeft = NCS_OS_KEY_LEFT,
	kbRight = NCS_OS_KEY_RIGHT,
	kbEnter = NCS_OS_KEY_ENTER,
	kbEsc = NCS_OS_KEY_ESC,
	kbTab = NCS_OS_KEY_TAB,
	kbBackSp = NCS_OS_KEY_BACKSP,
	kbCtrl = NCS_OS_KEY_MOD_CTRL,
	kbAlt = NCS_OS_KEY_MOD_ALT,
	kbShift = NCS_OS_KEY_MOD_SHIFT,
	kbSentinal		/* no comma */
} NCS_CONIO_KEYS;

/*****************************************************************************
*   Commands for CON IO                                                     *
*****************************************************************************/
typedef enum ncs_conio_cmd {
	NCS_CONIO_CMD_PUTCHR,	/* put a character to console   */
	NCS_CONIO_CMD_GETCHR,	/* get a character from console */
	NCS_CONIO_CMD_PUTSTR,	/* put a string to console      */
	NCS_CONIO_CMD_GETSTR,	/* get a string from console    */
	NCS_CONIO_CMD_UNBUF_PUTCHR,	/* put an unbuffered character  */
	NCS_CONIO_CMD_UNBUF_GETCHR,	/* get an unbuffered character  */
	NCS_CONIO_CMD_FFLUSH,	/* flush a buffered console     */
	NCS_CONIO_CMD_SENTINAL	/* no comma                     */
} NCS_CONIO_CMD;

/*****************************************************************************
*   File Open API                                                            *
*****************************************************************************/
typedef struct ncs_conio_putchr {
	int32 i_char;		/* character to send to console   */
} NCS_CONIO_PUTCHR;

/*****************************************************************************
*   File Close API                                                           *
*****************************************************************************/
typedef struct ncs_conio_getchr {
	int32 o_char;		/* character read from console   */
} NCS_CONIO_GETCHR;

/*****************************************************************************
*   File Read API                                                            *
*****************************************************************************/
typedef struct ncs_conio_putstr {
	uns8 *i_string;		/* string to send to console  */
	uns32 i_strlen;		/* size of string to send     */
	int32 o_bytecnt;	/* byte count actually sent   */
} NCS_CONIO_PUTSTR;

/*****************************************************************************
*   File Read API                                                            *
*****************************************************************************/
typedef struct ncs_conio_getstr {
	uns8 *i_string;		/* string read from console  */
	uns32 i_strlen;		/* size of string space       */
	int32 o_bytecnt;	/* byte count actually read   */
} NCS_CONIO_GETSTR;

/*****************************************************************************
*   File Seek API                                                            *
*****************************************************************************/
typedef struct ncs_conio_unbuf_putchr {
	int32 i_char;		/* character to send to console   */
} NCS_CONIO_UNBUF_PUTCHR;

/*****************************************************************************
*   File Seek API                                                            *
*****************************************************************************/
typedef struct ncs_conio_unbuf_getchr {
	int32 o_char;		/* character read from console   */
} NCS_CONIO_UNBUF_GETCHR;

/*****************************************************************************
*   File Seek API                                                            *
*****************************************************************************/
typedef struct ncs_conio_fflush {
	NCSCONTEXT i_stream;	/* stream to flush             */
} NCS_CONIO_FFLUSH;

/*****************************************************************************
*   File IO API                                                              *
*****************************************************************************/
typedef struct ncs_conio_request {
	NCS_CONIO_CMD i_request;	/* request initiated by user */

	/* CON IO requests */
	NCS_CONIO_PUTCHR putchr;	/* put a character to console   */
	NCS_CONIO_GETCHR getchr;	/* get a character from console */
	NCS_CONIO_PUTSTR putstr;	/* put a string to console      */
	NCS_CONIO_GETSTR getstr;	/* get a string from console    */
	NCS_CONIO_UNBUF_PUTCHR unbuf_putchr;	/* put an unbuffered character  */
	NCS_CONIO_UNBUF_GETCHR unbuf_getchr;	/* get an unbuffered character  */
	NCS_CONIO_FFLUSH fflush;	/* flush a buffered console     */

} NCS_CONIO_REQUEST;

typedef uns32 (*NCS_CONIO) (NCSCONTEXT, NCS_VRID id, NCS_CONIO_REQUEST *request);

uns32 ncs_conio(NCSCONTEXT context, NCS_VRID id, NCS_CONIO_REQUEST *request);

#endif   /* NCS_FIO_H */
