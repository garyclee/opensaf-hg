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
 * Author(s): Ericsson AB
 *
 */


#ifndef util_h
#define util_h

extern const char *saf_notification_types[];
extern const char *saf_error[];
extern SaTimeT getSaTimeT(void);
extern void create_dn(char *rdn, char *parent, SaNameT *dn);
extern void sa_namet_init(char *value, SaNameT *namet);
extern const char *get_saf_error(SaAisErrorT rc);

extern void safassert_impl(const char* file, unsigned int line, SaAisErrorT actual, SaAisErrorT expected, int die);
extern void assertvalue_impl(__const char *__assertion, __const char *__file,
		   unsigned int __line, __const char *__function);

/**
 * Pre and post condition checking. Will print information and
 * terminate program if actual and expected status code differ.
 * @param actual
 * @param expected
 */
#define safassert(actual, expected) safassert_impl(__FILE__, __LINE__, actual, expected, 1)

#define safassertNice(actual, expected) \
	((actual == expected)? 0: (safassert_impl(__FILE__, __LINE__, actual, expected, 0),1))

#define assertvalue(expr) \
  (((expr) ? 0 : (assertvalue_impl (__STRING(expr), __FILE__, __LINE__, __FUNCTION__), 1)))

#endif
