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

#include <endian.h>
#include <cstddef>
#include <random>
#include <string>
#include "base/hash.h"
#include "gtest/gtest.h"

struct TestVector {
  const char* key_data;
  size_t key_size;
  const char* hash_value;
};

#define TEST_VECTOR(key, hash) \
  { key, sizeof(key) - 1, hash }

static const TestVector test_vectors[] = {
    TEST_VECTOR("", "mJbyfHPNxOzI7KPhb27rY6_gS2wNOSds"),
    TEST_VECTOR("\000", "U-4lHisMY1zsvi2ry5wqlYKUMhhb_MX8"),
    TEST_VECTOR("\001", "7R0FXuGlh60oucbCoEIyp97oPDI34hNn"),
    TEST_VECTOR("\376", "X-7TtfTMWhw7T9CK1-yk8bRAlF_544MB"),
    TEST_VECTOR("\377", "OMGVy2Bhf3bPvd_yYKNVn8AK-EhkSJN4"),
    TEST_VECTOR(" ", "LOR-HJ8c_bFoH7LVc45B67zrxvqNp2ih"),
    TEST_VECTOR("\t", "jwSZSrsTHsB4kB2nUvpebPSpvDdt8d7V"),
    TEST_VECTOR("\n", "WlgTrOkNrltdky0QZv1B3jMG0D73h3YC"),
    TEST_VECTOR("\r\n", "pAv_qrHWSbOLxiJmXB2ETosp_FiE-G4H"),
    TEST_VECTOR("\\/:", "wyC3ktDwB4tvyzXBO5zx_iH76GKFmjxB"),
    TEST_VECTOR("$abc", "xSHQCvFJpVfx4ofdholD-mzgxyJJHjYg"),
    TEST_VECTOR("\000\000\000\000\000", "mzklNJ6l6vWVe73PvthH61RRsl90ygC_"),
    TEST_VECTOR("123456", "E_7JXvueRDhde3Wq3W5jYSsts_LOWJNZ"),
    TEST_VECTOR("1234567", "0cAgD2NO2ASyz39axMrnBdV_VuHn2PhC"),
    TEST_VECTOR("12345678", "Uzmag7xvTQg40VBwOumXaugZxFZyRpB8"),
    TEST_VECTOR("123456789", "3YfQdE3kVpdmBEaBHM4uCj1feXvixjYA"),
    TEST_VECTOR(
        "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
        "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
        "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
        "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
        "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
        "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
        "\340\341\342\343\344\345\346\347\350\351\352\353\354\355",
        "z2V3p2mUUigcBkYWSEw0yhz2f1v4M8Nb"),
    TEST_VECTOR(
        "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
        "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
        "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
        "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
        "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
        "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
        "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356",
        "WprfN6xOSNY9RHaNCkR12yEEx5hGfwHj"),
    TEST_VECTOR(
        "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
        "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
        "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
        "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
        "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
        "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
        "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356"
        "\357",
        "K3wQd-ZX5k7_AZ7p1qOwrehV5vkaJq9E"),
    TEST_VECTOR(
        "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
        "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
        "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
        "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
        "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
        "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
        "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357"
        "\360",
        "O-ERJdo-W-iCV4b2QogLJmtR1gMIhlkx")};

TEST(BaseHash, TestVectors) {
  base::InitializeHashFunction();
  for (size_t i = 0; i < sizeof(test_vectors) / sizeof(test_vectors[0]); ++i) {
    std::string key =
        std::string(test_vectors[i].key_data, test_vectors[i].key_size);
    std::string hash = std::string(test_vectors[i].hash_value);
    std::string computed_hash = base::Hash(key);
    EXPECT_EQ(hash, computed_hash);
  }
}

TEST(BaseHash, LongRandomString) {
  base::InitializeHashFunction();
  std::string key;
  key.reserve(8000);
  std::mt19937_64 generator(4711);
  for (size_t i = 0; i != 1000; ++i) {
    union {
      char bytes[sizeof(uint64_t)];
      uint64_t word;
    };
    word = htobe64(generator());
    key.append(bytes, sizeof(uint64_t));
  }
  std::string hash = std::string("C_r2XVQjnVbnpQTMHO2OQJ2sn0YZi1py");
  std::string computed_hash = base::Hash(key);
  EXPECT_EQ(hash, computed_hash);
}
