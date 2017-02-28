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

#include <openssl/sha.h>
#include <cstdint>
#include <string>

namespace base {

extern const char kHashFunctionAlphabet[64];

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
// Note: if you use this function you need to link your program with -lcrypto
inline std::string Hash(const std::string& message) {
  SHA512_CTX context;
  SHA512_Init(&context);
  context.h[0] = UINT64_C(0x010176140648b233);
  context.h[1] = UINT64_C(0xdb92aeb1eebadd6f);
  context.h[2] = UINT64_C(0x83a9e27aa1d5ea62);
  context.h[3] = UINT64_C(0xec95f77eb609b4e1);
  context.h[4] = UINT64_C(0x71a99185c75caefa);
  context.h[5] = UINT64_C(0x006e8f08baf32e3c);
  context.h[6] = UINT64_C(0x6a2b21abd2db2aec);
  context.h[7] = UINT64_C(0x24926cdbd918a27f);
  SHA512_Update(&context, message.data(), message.size());
  unsigned char result[SHA512_DIGEST_LENGTH];
  SHA512_Final(result, &context);
  std::string encoded;
  encoded.reserve(32);
  for (int i = 0; i != 24; i += 3) {
    uint64_t a = (result[i] << 16) | (result[i + 1] << 8) | result[i + 2];
    encoded.push_back(kHashFunctionAlphabet[a >> 18]);
    encoded.push_back(kHashFunctionAlphabet[(a >> 12) & 63]);
    encoded.push_back(kHashFunctionAlphabet[(a >> 6) & 63]);
    encoded.push_back(kHashFunctionAlphabet[a & 63]);
  }
  return encoded;
}

}  // namespace base

#endif  // BASE_HASH_H_
