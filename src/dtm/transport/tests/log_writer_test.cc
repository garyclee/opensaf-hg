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

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include "dtm/transport/log_writer.h"
#include "gtest/gtest.h"

class LogWriterTest : public ::testing::Test {
 protected:
  LogWriterTest() :
      tmpdir_{} {
  }

  virtual ~LogWriterTest() {
  }

  virtual void SetUp() {
    char tmpdir[] = "/tmp/log_writer_test_XXXXXX";
    char* result = mkdtemp(tmpdir);
    ASSERT_NE(result, nullptr);
    tmpdir_ = result;
    int retval = setenv("pkglogdir", tmpdir_.c_str(), 1);
    ASSERT_EQ(retval, 0);
  }

  virtual void TearDown() {
    std::string cmd = std::string("rm -f ") + tmpdir_ + std::string("/*");
    int result = system(cmd.c_str());
    ASSERT_EQ(result, 0);
    result = rmdir(tmpdir_.c_str());
    ASSERT_EQ(result, 0);
    result = unsetenv("pkglogdir");
    ASSERT_EQ(result, 0);
  }

  std::string tmpdir_;
};

TEST_F(LogWriterTest, ExistingFileShouldBeAppended) {
  const char first_line[] = "first line";
  const char second_line[] = "second line";
  std::ofstream ostr(tmpdir_ + std::string("/mds.log"));
  ostr << first_line << std::endl;
  ostr.close();
  LogWriter* log_writer = new LogWriter();
  memcpy(log_writer->current_buffer_position(), second_line,
         sizeof(second_line) - 1);
  log_writer->current_buffer_position()[sizeof(second_line) - 1] = '\n';
  log_writer->Write(sizeof(second_line));
  delete log_writer;
  std::ifstream istr(tmpdir_ + std::string("/mds.log"));
  std::string line1;
  std::string line2;
  std::getline(istr, line1);
  EXPECT_FALSE(istr.fail());
  std::getline(istr, line2);
  EXPECT_FALSE(istr.fail());
  EXPECT_EQ(line1, std::string(first_line));
  EXPECT_EQ(line2, std::string(second_line));
}
