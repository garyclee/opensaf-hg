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

  This module contains routines related H&J Counting Semaphores.
..............................................................................

  FUNCTIONS INCLUDED in this module:

  ncs_sem_create....create/initialize a semaphore
  ncs_sem_release...release a semaphore
  ncs_sem_give......increment the semaphore by 1
  ncs_sem_take......wait for semaphore to be greater than 0

 ******************************************************************************
 */

#include <ncsgl_defs.h>
#include "ncs_osprm.h"
#include "ncssysf_def.h"

/****************************************************************************
  PROCEDURE NAME:   sysf_strrcspn

  DESCRIPTION:
     finds the delimiter, left of the start positions in the string.
     See also strcspn, strspn.

  RETURNS:
     index into string where delimiter is left of start position.
*****************************************************************************/
int32 sysf_strrcspn(const uint8_t *s, const int32 start_pos, const uint8_t *reject)
{
	int32 i;
	int32 j;
	uns32 rej_len = strlen((char *)reject);

	for (i = (int32)start_pos; i >= 0; i--) {
		for (j = 0; j < (int32)rej_len; j++) {
			if (s[i] == reject[j]) {
				return i;
			}
		}
	}
	return EOF;
}

/****************************************************************************
  PROCEDURE NAME:   sysf_strincmp

  DESCRIPTION:
  case insensitive string compare, but no more than to n characters.
  See also stricmp and strncmp

  RETURNS:
     index into string where delimiter is left of start position.
*****************************************************************************/
int32 sysf_strincmp(const uint8_t *s1, const uint8_t *s2, uns32 n)
{
	uint8_t c1 = '\0';
	uint8_t c2 = '\0';

	while (n > 0) {
		c1 = (unsigned char)*s1++;
		c2 = (unsigned char)*s2++;

		if ('a' <= c1 && 'z' >= c1) {
			c1 -= 'a' - 'A';
		}

		if ('a' <= c2 && 'z' >= c2) {
			c2 -= 'a' - 'A';
		}

		if (c1 == '\0' || c1 != c2) {
			return c1 - c2;
		}
		n--;
	}

	return c1 - c2;
}
