/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2016 The OpenSAF Foundation
 * Copyright (C) 2017, Oracle and/or its affiliates. All rights reserved.
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
 * Author(s): Oracle
 *
 */

#include <thread>
#include "amf/amfnd/avnd.h"
#include <poll.h>
#include "amf/amfnd/imm.h"

ir_cb_t ir_cb;
struct pollfd ImmReader::fds[];
extern const AVND_EVT_HDLR g_avnd_func_list[AVND_EVT_MAX];

/**
 * This thread will read classes and object information thereby allowing main
 * thread to proceed with other important work.
 * As of now, SU instantiation needs imm data to be read, so
 * only this event is being handled here.
 * Going forward, we need to use this function for similar purposes.
 * @param _cb
 *
 * @return void*
 */
void ImmReader::imm_reader_thread() {
  AVND_EVT *evt;
  TRACE_ENTER();
  NCS_SEL_OBJ mbx_fd;

  /* create the mail box */
  uint32_t rc = m_NCS_IPC_CREATE(&ir_cb.mbx);
  if (NCSCC_RC_SUCCESS != rc) {
    LOG_CR("AvND Mailbox creation failed");
    exit(EXIT_FAILURE);
  }
  TRACE("Ir mailbox creation success");

  /* attach the mail box */
  rc = m_NCS_IPC_ATTACH(&ir_cb.mbx);
  if (NCSCC_RC_SUCCESS != rc) {
    LOG_CR("Ir mailbox attach failed");
    exit(EXIT_FAILURE);
  }

  mbx_fd = ncs_ipc_get_sel_obj(&ir_cb.mbx);
  fds[FD_MBX].fd = mbx_fd.rmv_obj;
  fds[FD_MBX].events = POLLIN;

  while (1) {
    if (poll(fds, irfds, -1) == (-1)) {
      if (errno == EINTR) {
        continue;
      }

      LOG_ER("poll Fail - %s", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (fds[FD_MBX].revents & POLLIN) {
      while (nullptr != (evt = (AVND_EVT *)ncs_ipc_non_blk_recv(&ir_cb.mbx)))
        ir_process_event(evt);
    }
  }
  TRACE_LEAVE();
}

/**
 * Start a imm reader thread.
 * @param Nothing.
 * @return Nothing.
 */
void ImmReader::imm_reader_thread_create() {
  TRACE_ENTER();

  std::thread t{imm_reader_thread};
  t.detach();

  TRACE_LEAVE();
}

/**
 * This function process events, which requires reading data from imm.
 * As of now, SU instantiation needs to read data, so diverting SU
 * instantiation in this thread. Going forward, we need to use this
 * function for similar purposes.
 * @param Event pointer.
 * @return Nothing.
 */
void ImmReader::ir_process_event(AVND_EVT *evt) {
  uint32_t res = NCSCC_RC_SUCCESS;
  AVND_SU *su = 0;
  TRACE_ENTER2("Evt type:%u", evt->type);

  /* get the su */
  su = avnd_sudb_rec_get(avnd_cb->sudb,
                         Amf::to_string(&evt->info.ir_evt.su_name));
  if (!su) {
    TRACE("SU'%s', not found in DB",
          osaf_extended_name_borrow(&evt->info.ir_evt.su_name));
    goto done;
  }

  if (false == su->is_ncs) {
    res = avnd_comp_config_get_su(su);
  } else {
    res = NCSCC_RC_FAILURE;
  }

  {
    AVND_EVT *evt_ir = 0;
    SaNameT copy_name;
    TRACE("Sending to main thread.");
    osaf_extended_name_alloc(su->name.c_str(), &copy_name);
    evt_ir =
        avnd_evt_create(avnd_cb, AVND_EVT_IR, 0, nullptr, &copy_name, 0, 0);
    osaf_extended_name_free(&copy_name);
    if (res == NCSCC_RC_SUCCESS)
      evt_ir->info.ir_evt.status = true;
    else
      evt_ir->info.ir_evt.status = false;

    res = m_NCS_IPC_SEND(&avnd_cb->mbx, evt_ir, evt_ir->priority);

    if (NCSCC_RC_SUCCESS != res)
      LOG_CR("AvND send event to main thread mailbox failed, type = %u",
             evt_ir->type);

    TRACE("Imm Read:'%d'", evt_ir->info.ir_evt.status);

    /* if failure, free the event */
    if (NCSCC_RC_SUCCESS != res) avnd_evt_destroy(evt_ir);
  }
done:
  if (evt) avnd_evt_destroy(evt);

  TRACE_LEAVE2("res '%u'", res);
}
