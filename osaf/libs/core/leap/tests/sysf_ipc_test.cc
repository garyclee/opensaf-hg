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
#include <string>
#include <thread>
#include <mutex>
#include <sched.h>
#include <poll.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "sysf_ipc.h"

#include "ncs_main_papi.h"
#include "gtest/gtest.h"

std::mutex send_ctr_mutex;

typedef struct message_ {
  struct message_ *next;
  NCS_IPC_PRIORITY prio;
  int seq_no;
} Message;

bool mbox_clean(NCSCONTEXT arg, NCSCONTEXT msg) {
  Message *curr;
  Message *temp;

  /* clean the entire mailbox */
  for (curr = (Message *)msg; curr;) {
    temp = curr;
    curr = curr->next;

    delete temp;
  }
  return true;
}

// The fixture for testing c-function sysf_ipc
class SysfIpcTest : public ::testing::Test {
 public:

 protected:

  SysfIpcTest() {
    // Setup work can be done here for each test.
    no_of_msgs_sent = 0;
    no_of_msgs_received = 0;
  }

  virtual ~SysfIpcTest() {
    // Cleanup work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
    int rc = ncs_leap_startup();
    // see ticket #1629, return code should be ok
    //ASSERT_EQ(rc, NCSCC_RC_SUCCESS); 

    rc = m_NCS_IPC_CREATE(&mbox);
    ASSERT_EQ(rc, NCSCC_RC_SUCCESS);

    rc = m_NCS_IPC_ATTACH(&mbox);
    ASSERT_EQ(rc, NCSCC_RC_SUCCESS);
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
    int rc = m_NCS_IPC_DETACH(&mbox, mbox_clean, 0);
    ASSERT_EQ(rc, NCSCC_RC_SUCCESS);

    rc = m_NCS_IPC_RELEASE(&mbox, 0);
    ASSERT_EQ(rc, NCSCC_RC_SUCCESS);

    // see ticket #1629, calling ncs_leap_shutdown causes a segv.
    // ncs_leap_shutdown();
  }

  void send_msg(NCS_IPC_PRIORITY prio, int seq_no) {
    Message *msg;

    msg = new Message;
    msg->prio = prio;
    msg->seq_no = seq_no;
    int rc = m_NCS_IPC_SEND(&mbox, msg, prio);
    ASSERT_EQ(rc, NCSCC_RC_SUCCESS);
  }

  void recv_msg(NCS_IPC_PRIORITY prio, int seq_no) {
    Message *msg;

    msg = reinterpret_cast<Message*>(ncs_ipc_non_blk_recv(&mbox));
    ASSERT_TRUE(msg != NULL);
    ASSERT_EQ(msg->prio, prio);
    ASSERT_EQ(msg->seq_no, seq_no);
    delete msg;
  }

  //
  static void MessageReceiver();

  //
  static void MessageSender() {
    Message *msg;
    int prio;
    srand(time(NULL));
    int rc = NCSCC_RC_SUCCESS;

    for (int i = 0; i < 60; ++i) {
      msg = new Message;

      prio = (random() % 3) + 1;
      msg->prio = (NCS_IPC_PRIORITY) prio;
      msg->seq_no = i;

      rc = m_NCS_IPC_SEND(&mbox, msg, msg->prio);
      EXPECT_EQ(rc, NCSCC_RC_SUCCESS);

      {
        std::lock_guard<std::mutex> lck(send_ctr_mutex);
        no_of_msgs_sent++;
      }

      sched_yield();
    }
  }

  // Objects declared here can be used by all tests in the test case.

  static SYSF_MBX mbox;
  static uint32_t no_of_msgs_sent;
  static uint32_t no_of_msgs_received;
};

SYSF_MBX SysfIpcTest::mbox = 0;
uint32_t SysfIpcTest::no_of_msgs_sent = 0;
uint32_t SysfIpcTest::no_of_msgs_received = 0;

void SysfIpcTest::MessageReceiver() {
  NCS_SEL_OBJ mbox_fd;
  pollfd fds;
  bool done = false;
  Message *msg;

  mbox_fd = ncs_ipc_get_sel_obj(&mbox);

  fds.fd = mbox_fd.rmv_obj;
  fds.events = POLLIN;

  while (!done) {
    int rc = poll(&fds, 1, -1);

    if (rc == -1) {
      if (errno == EINTR) {
        continue;
      }
      ASSERT_EQ(rc, 0);
    }

    if (fds.revents & POLLIN) {
      while ((msg = reinterpret_cast<Message*>(ncs_ipc_non_blk_recv(&mbox))) != NULL) {
        no_of_msgs_received++;

        if (msg->seq_no == 4711) {
           done = true;
        }

        delete msg;
      }
    }
  }

  {
    std::lock_guard<std::mutex> lck(send_ctr_mutex);
    ASSERT_EQ(no_of_msgs_received, no_of_msgs_sent);
  }
} 

// Tests send and receive
TEST_F(SysfIpcTest, TestSendReceiveMessage) {
  Message *msg;

  // send messages
  send_msg(NCS_IPC_PRIORITY_LOW, 1);
  send_msg(NCS_IPC_PRIORITY_LOW, 2);
  send_msg(NCS_IPC_PRIORITY_NORMAL, 1);
  send_msg(NCS_IPC_PRIORITY_NORMAL, 2);
  send_msg(NCS_IPC_PRIORITY_HIGH, 1);
  send_msg(NCS_IPC_PRIORITY_HIGH, 2);
  send_msg(NCS_IPC_PRIORITY_VERY_HIGH, 1);
  send_msg(NCS_IPC_PRIORITY_VERY_HIGH, 2);

  // receive messages
  recv_msg(NCS_IPC_PRIORITY_VERY_HIGH, 1);
  recv_msg(NCS_IPC_PRIORITY_VERY_HIGH, 2);
  recv_msg(NCS_IPC_PRIORITY_HIGH, 1);
  recv_msg(NCS_IPC_PRIORITY_HIGH, 2);
  recv_msg(NCS_IPC_PRIORITY_NORMAL, 1);
  recv_msg(NCS_IPC_PRIORITY_NORMAL, 2);
  recv_msg(NCS_IPC_PRIORITY_LOW, 1);
  recv_msg(NCS_IPC_PRIORITY_LOW, 2);

  msg = reinterpret_cast<Message*>(ncs_ipc_non_blk_recv(&mbox));
  EXPECT_TRUE(msg == NULL);
}

// Tests threads sending and receiving messages
TEST_F(SysfIpcTest, TestThreadsSendReceiveMessage) {
  std::thread sndr_thread[5];
  srand(time(NULL));

  std::thread msg_receiver(MessageReceiver);

  ASSERT_EQ(msg_receiver.joinable(), true);

  for (int i = 0; i < 5; ++i) {
    sndr_thread[i] = std::thread(MessageSender);
  }

  for (int i = 0; i < 5; ++i) {
    sndr_thread[i].join();
  }

  sched_yield();

  {
    std::lock_guard<std::mutex> lck(send_ctr_mutex);
    no_of_msgs_sent++;
  }

  send_msg(NCS_IPC_PRIORITY_LOW, 4711);

  msg_receiver.join();
}
