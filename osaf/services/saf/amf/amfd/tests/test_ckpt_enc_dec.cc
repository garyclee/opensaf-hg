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
#include "susi.h"
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
  uint32_t *fld = reinterpret_cast<uint32_t*>(&buf[offset]);

  // verify that the encoded value is in network byte order
  if (isLittleEndian()) {
    // saAmfApplicationCurrNumSGs
    ASSERT_EQ(*fld, static_cast<uint32_t>(0x11223344));
  } else {
    // saAmfApplicationCurrNumSGs
    ASSERT_EQ(*fld, static_cast<uint32_t>(0x44332211));
  }

  delete [] tmpData;

  memset(&app, '\0', sizeof(AVD_APP));
  decode_app(&enc.io_uba, &app);

  ASSERT_EQ(Amf::to_string(&app.name), "AppName");
  ASSERT_EQ(app.saAmfApplicationAdminState, SA_AMF_ADMIN_LOCKED);
  ASSERT_EQ(app.saAmfApplicationCurrNumSGs, static_cast<uint32_t>(0x44332211));
}

TEST_F(CkptEncDecTest, testEncDecAvdComp) {
  int rc = 0;
  std::string comp_name{"CompName"};
  std::string comp_proxy_name{"CompProxyName"};
  AVD_COMP comp(asSaNameT(comp_name));

  comp.saAmfCompOperState = static_cast<SaAmfOperationalStateT>(0x44332211);
  comp.saAmfCompReadinessState = static_cast<SaAmfReadinessStateT>(0x55443322);
  comp.saAmfCompPresenceState = static_cast<SaAmfPresenceStateT>(0x66554433);
  comp.saAmfCompRestartCount = 0x77665544;
  comp.saAmfCompCurrProxyName = *(asSaNameT(comp_proxy_name));

  rc = ncs_enc_init_space(&enc.io_uba);
  ASSERT_TRUE(rc == NCSCC_RC_SUCCESS);

  enc.io_msg_type = NCS_MBCSV_MSG_ASYNC_UPDATE;
  enc.io_action = NCS_MBCSV_ACT_UPDATE;
  enc.io_reo_hdl = (MBCSV_REO_HDL) & comp;
  enc.io_reo_type = AVSV_CKPT_AVD_COMP_CONFIG;
  enc.i_peer_version = AVD_MBCSV_SUB_PART_VERSION_3;

  encode_comp(&enc.io_uba, &comp);
  
  // retrieve AVD_COMP encoded data from the USR buf
  int32_t size = enc.io_uba.ttl;
  char *tmpData = new char[size];
  char *buf = sysf_data_at_start(enc.io_uba.ub, size, tmpData);
  uint32_t offset = sizeof(SaNameT);
  uint32_t *fld = reinterpret_cast<uint32_t*>(&buf[offset]);
  
  // verify that the encoded value is in network byte order
  if (isLittleEndian()) {
    // saAmfCompOperState
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x11223344));
    // saAmfCompReadinessState
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x22334455));
    // saAmfCompPresenceState
    ASSERT_EQ(*fld, static_cast<uint32_t>(0x33445566));
  } else {
    // saAmfCompOperState
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x44332211));
    // saAmfCompReadinessState
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x55443322));
    // saAmfCompPresenceState
    ASSERT_EQ(*fld, static_cast<uint32_t>(0x66554433));
  }

  delete [] tmpData;

  memset(&comp, '\0', sizeof (AVD_COMP));
  decode_comp(&enc.io_uba, &comp);

  ASSERT_EQ(Amf::to_string(&comp.comp_info.name), "CompName");
  ASSERT_EQ(comp.saAmfCompOperState, static_cast<SaAmfOperationalStateT>(0x44332211));
  ASSERT_EQ(comp.saAmfCompReadinessState, static_cast<SaAmfReadinessStateT>(0x55443322));
  ASSERT_EQ(comp.saAmfCompPresenceState, static_cast<SaAmfPresenceStateT>(0x66554433));
  ASSERT_EQ(comp.saAmfCompRestartCount, static_cast<uint32_t>(0x77665544));
  ASSERT_EQ(Amf::to_string(&comp.saAmfCompCurrProxyName), "CompProxyName");
}

TEST_F(CkptEncDecTest, testEncDecAvdSiAss) {
  int rc = 0;
  AVD_SU_SI_REL susi;
  AVSV_SU_SI_REL_CKPT_MSG susi_ckpt;
  AVD_SU su;
  AVD_SI si;
  std::string su_name("su_name");
  std::string si_name("si_name");
  std::string comp_name("comp_name");
  std::string csi_name("csi_name");

  rc = ncs_enc_init_space(&enc.io_uba);
  ASSERT_TRUE(rc == NCSCC_RC_SUCCESS);

  su.name = *(asSaNameT(su_name));
  si.name = *(asSaNameT(si_name));
  susi.su = &su;
  susi.si = &si;
  susi.state = SA_AMF_HA_ACTIVE;
  susi.fsm = AVD_SU_SI_STATE_ABSENT;
  susi.csi_add_rem = SA_FALSE; 
  susi.comp_name = *(asSaNameT(comp_name));
  susi.csi_name = *(asSaNameT(csi_name));

  enc.io_msg_type = NCS_MBCSV_MSG_ASYNC_UPDATE;
  enc.io_action = NCS_MBCSV_ACT_UPDATE;
  enc.io_reo_hdl = (MBCSV_REO_HDL)&susi;
  enc.io_reo_type = AVSV_CKPT_AVD_SI_ASS;
  enc.i_peer_version = AVD_MBCSV_SUB_PART_VERSION_4;

  encode_siass(&enc.io_uba, &susi, enc.i_peer_version);
  decode_siass(&enc.io_uba, &susi_ckpt, enc.i_peer_version);

  ASSERT_EQ(Amf::to_string(&susi_ckpt.su_name), su_name);
  ASSERT_EQ(Amf::to_string(&susi_ckpt.si_name), si_name);
  ASSERT_EQ(susi_ckpt.state, SA_AMF_HA_ACTIVE);
  ASSERT_EQ(susi_ckpt.fsm, AVD_SU_SI_STATE_ABSENT);
  ASSERT_EQ(susi_ckpt.csi_add_rem, SA_FALSE);
  ASSERT_EQ(Amf::to_string(&susi_ckpt.comp_name), comp_name);
  ASSERT_EQ(Amf::to_string(&susi_ckpt.csi_name), csi_name);
}

TEST_F(CkptEncDecTest, testEncDecAvdCb) {
  int rc = 0;
  AVD_CL_CB cb;

  cb.init_state = AVD_APP_STATE;
  cb.cluster_init_time = 0x8877665544332211;
  cb.nodes_exit_cnt = 0x55443322;
  
  rc = ncs_enc_init_space(&enc.io_uba);
  ASSERT_TRUE(rc == NCSCC_RC_SUCCESS);

  enc.io_msg_type = NCS_MBCSV_MSG_ASYNC_UPDATE;
  enc.io_action = NCS_MBCSV_ACT_UPDATE;
  enc.io_reo_hdl = (MBCSV_REO_HDL)&cb;
  enc.io_reo_type = AVSV_CKPT_AVD_CB_CONFIG;
  enc.i_peer_version = AVD_MBCSV_SUB_PART_VERSION_3;

  encode_cb(&enc.io_uba, &cb, enc.i_peer_version);
  
  // retrieve AVD_CL_CB encoded data from the USR buf
  int32_t size = enc.io_uba.ttl;
  char *tmpData = new char[size];

  char *buf = sysf_data_at_start(enc.io_uba.ub, size, tmpData);
  uint32_t offset = 0;
  uint32_t *fld = reinterpret_cast<uint32_t*>(&buf[offset]);
  
  // verify that the encoded value is in network byte order
  if (isLittleEndian()) {
    // app_state
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x04000000));
    // cluster_init_time
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x55667788));
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x11223344));
    // nodes_exit_cnt
    ASSERT_EQ(*fld, static_cast<uint32_t>(0x22334455));
  } else {
    // app_state
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x00000004));
    // cluster_init_time
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x88776655));
    ASSERT_EQ(*fld++, static_cast<uint32_t>(0x44332211));
    // nodes_exit_cnt
    ASSERT_EQ(*fld, static_cast<uint32_t>(0x55443322));
  }

  delete [] tmpData;

  cb.init_state = AVD_INIT_BGN;
  cb.cluster_init_time = 0x0;
  cb.nodes_exit_cnt = 0x0;
 
  decode_cb(&enc.io_uba, &cb, enc.i_peer_version);

  ASSERT_EQ(cb.init_state, AVD_APP_STATE);
  ASSERT_EQ(cb.cluster_init_time, static_cast<SaTimeT>(0x8877665544332211));
  ASSERT_EQ(cb.nodes_exit_cnt, static_cast<uint32_t>(0x55443322));
}

TEST_F(CkptEncDecTest, testEncDecAvdSiTrans) {
  int rc = 0;
  SG_2N sg;
  AVD_SI tobe_redistributed;
  AVD_SU min_assigned_su;
  AVD_SU max_assigned_su;
  std::string sg_name("sg_name");
  std::string si_name("si_name");
  std::string min_name("min_name");
  std::string max_name("max_name");
  AVSV_SI_TRANS_CKPT_MSG msg;

  min_assigned_su.name = *(asSaNameT(min_name));
  max_assigned_su.name = *(asSaNameT(max_name));
  tobe_redistributed.name = *(asSaNameT(si_name));
  sg.name = *(asSaNameT(sg_name));
  sg.si_tobe_redistributed = &tobe_redistributed;
  sg.min_assigned_su = &min_assigned_su;
  sg.max_assigned_su = &max_assigned_su;

  rc = ncs_enc_init_space(&enc.io_uba);
  ASSERT_TRUE(rc == NCSCC_RC_SUCCESS);

  enc.io_msg_type = NCS_MBCSV_MSG_ASYNC_UPDATE;
  enc.io_action = NCS_MBCSV_ACT_UPDATE;
  enc.io_reo_hdl = (MBCSV_REO_HDL)&sg;
  enc.io_reo_type = AVSV_CKPT_AVD_SI_TRANS;
  enc.i_peer_version = AVD_MBCSV_SUB_PART_VERSION_4;

  encode_si_trans(&enc.io_uba, static_cast<AVD_SG*>(&sg), enc.i_peer_version);
  decode_si_trans(&enc.io_uba, &msg, enc.i_peer_version);

  ASSERT_EQ(Amf::to_string(&msg.sg_name), sg_name);
  ASSERT_EQ(Amf::to_string(&msg.si_name), si_name);
  ASSERT_EQ(Amf::to_string(&msg.min_su_name), min_name);
  ASSERT_EQ(Amf::to_string(&msg.max_su_name), max_name);
}

