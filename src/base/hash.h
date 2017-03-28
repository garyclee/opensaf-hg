/*      -*- OpenSAF  -*-
 *
 * Copyright Ericsson AB 2017 - All Rights Reserved.
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
 */

#ifndef BASE_HASH_H_
#define BASE_HASH_H_

#include <string>

namespace base {

extern const char kHashFunctionAlphabet[64];

// Initialize the hash function. This must be done at least once before calling
// the function Hash() below. It is safe to call this function multiple times,
// since this function does nothing if the hash function has already been
// initialized.
void InitializeHashFunction();

// This function takes an arbitrary string as input parameter and returns
// another string which is a hash of the input string generated using a
// collision-resistant hash function. The probability that two different inputs
// would generate the same output is small enough that it can be safely ignored
// for all practical purposes.
//
// The return value of this function is a string of printable ASCII characters.
// The length of the returned string will always be exactly 32 characters, and
// the returned string is guaranteed to not contain any character that would be
// illegal for use in a file name.
//
// NOTE: You must have called InitializeHashFunction() at least once before
//       calling this function.
std::string Hash(const std::string& message);

}  // namespace base

#endif  // BASE_HASH_H_
