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

  DESCRIPTION:

  This module contains declarations pertaining to implementation of 
  patricia tree search/add/delete functions.

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef _PATRICIA_H_
#define _PATRICIA_H_

#include "ncspatricia.h"

#define m_KEY_CMP(t, k1, k2) memcmp(k1, k2, (size_t)(t)->params.key_size)
#define m_GET_BIT(key, bit)  ((bit < 0) ? 0 : ((int)((*((key) + (bit >> 3))) >> (7 - (bit & 0x07))) & 0x01))

#endif
