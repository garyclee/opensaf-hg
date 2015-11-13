/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2015 The OpenSAF Foundation
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
#include <iostream>
#include "mbcsv_papi.h"
#include "cb.h"
#include "app.h"
#include "gtest/gtest.h"
#include "ncssysf_mem.h"

// Singleton Control Block. Statically allocated
static AVD_CL_CB _control_block;

// Global reference to the control block
AVD_CL_CB *avd_cb = &_control_block;

// The fixture for testing encode decode for mbcsv
class CkptEncDecTest : public ::testing::Test {

 protected:

  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  const SaNameT* asSaNameT(const std::string& name) {
    sa_name_t.length = name.size();
    memcpy((char*)sa_name_t.value, name.c_str(), sa_name_t.length);
    return &sa_name_t;
  }

  bool isLittleEndian() const {
    union {
      int i;
      char c[sizeof(int)];
    } x;
    x.i = 1;
    if(x.c[0] == 1) {
      return true;
    } else {
      return false;
    }
  }

  SaNameT sa_name_t {};
  NCS_MBCSV_CB_DEC dec {};
  NCS_MBCSV_CB_ENC enc {};
  NCS_UBAID uba {};
};

TEST_F(CkptEncDecTest, testEncDecAvdApp) {
  int rc = 0;
  std::string app_name {"AppName"};
  AVD_APP app(asSaNameT(app_name));
  app.saAmfApplicationAdminState = SA_AMF_ADMIN_LOCKED;
  app.saAmfApplicationCurrNumSGs = 0x44332211;

  rc = ncs_enc_init_space(&enc.io_uba);
  ASSERT_TRUE(rc == NCSCC_RC_SUCCESS);

  enc.io_msg_type = NCS_MBCSV_MSG_ASYNC_UPDATE;
  enc.io_action = NCS_MBCSV_ACT_UPDATE;
  enc.io_reo_hdl = (MBCSV_REO_HDL) &app;
  enc.io_reo_type = AVSV_CKPT_AVD_APP_CONFIG;
  enc.i_peer_version = AVD_MBCSV_SUB_PART_VERSION_3;

  encode_app(&enc.io_uba, &app);

  // retrieve saAmfApplicationCurrNumSGs encoded from the USR buf
  int32_t size = enc.io_uba.ttl;
  char *tmpData = new char[size];
  char *buf = sysf_data_at_start(enc.io_uba.ub, size, tmpData);
  uint32_t offset = sizeof(SaNameT) + sizeof(uint32_t);
  uint32_t *fld = (uint32_t*) &buf[offset];
  delete [] tmpData;

  // verify that the encoded value is in network byte order
  if (isLittleEndian()) {
    ASSERT_EQ(*fld, (uint32_t) 0x11223344);
  } else {
    ASSERT_EQ(*fld, (uint32_t) 0x44332211);
  }

  memset(&app, '\0', sizeof(AVD_APP));
  decode_app(&enc.io_uba, &app);

  ASSERT_EQ(Amf::to_string(&app.name), "AppName");
  ASSERT_EQ(app.saAmfApplicationAdminState, SA_AMF_ADMIN_LOCKED);
  ASSERT_EQ(app.saAmfApplicationCurrNumSGs, (uint32_t) 0x44332211);
}
