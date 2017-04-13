/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 * Copyright (C) 2017, Oracle and/or its affiliates. All rights reserved.
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
 * Author(s): Emerson Network Power
 *
 */
/****************************************************************************

  DESCRIPTION:

  This module is the include file for IMM integration.

****************************************************************************/
#ifndef AMF_AMFD_IMM_H_
#define AMF_AMFD_IMM_H_

#include "amf/amfd/cb.h"
#include "osaf/immutil/immutil.h"
#include <queue>
#include <string>

typedef void (*AvdImmOiCcbApplyCallbackT)(CcbUtilOperationData_t *opdata);
typedef SaAisErrorT (*AvdImmOiCcbCompletedCallbackT)(
    CcbUtilOperationData_t *opdata);

extern SaAisErrorT avd_imm_init(void *avd_cb);
extern void avd_imm_impl_set_task_create(void);
extern SaAisErrorT avd_imm_impl_set(void);
extern SaAisErrorT avd_imm_applier_set(void);
extern void avd_imm_update_runtime_attrs(void);

/**
 * Possible return values from avd_job_fifo_dequeue
 */
typedef enum {
  JOB_EXECUTED = 1,  /* A job was successfully executed */
  JOB_ETRYAGAIN = 2, /* A job exist but execution was rejected with TRY_AGAIN */
  JOB_ENOTEXIST = 3, /* No job exist */
  JOB_EINVH = 4,     /* Invalid handle */
  JOB_ERR = 5        /* Other error than TRY_AGAIN, job was removed/deleted */

} AvdJobDequeueResultT;

typedef enum {
  JOB_TYPE_IMM = 1,  /* A IMM job */
  JOB_TYPE_NTF = 2, /* A NTF job */
  JOB_TYPE_ANY
} AvdJobTypeT;

// TODO HANO Write comments
// @todo move this into job.h
class Job {
 public:
  virtual AvdJobDequeueResultT exec(const AVD_CL_CB *cb) = 0;
  virtual ~Job() = 0;
  virtual AvdJobTypeT getJobType() = 0;
  virtual bool isRunnable(const AVD_CL_CB *cb) = 0;
};

class ImmJob : public Job {
public:
  bool implementer;
  ImmJob():implementer(true) {};
  AvdJobTypeT getJobType() { return JOB_TYPE_IMM; }
  bool isRunnable(const AVD_CL_CB *cb);
};

//
class ImmObjCreate : public ImmJob {
 public:
  SaImmClassNameT className_;
  std::string parentName_;
  const SaImmAttrValuesT_2 **attrValues_;

  ImmObjCreate() : ImmJob(){};
  bool immobj_update_required();
  AvdJobDequeueResultT exec(const AVD_CL_CB *cb);

  ~ImmObjCreate();
};

//
class ImmObjUpdate : public ImmJob {
 public:
  std::string dn;
  SaImmAttrNameT attributeName_;
  SaImmValueTypeT attrValueType_;
  void *value_;

  ImmObjUpdate() : ImmJob(){};
  bool immobj_update_required();
  AvdJobDequeueResultT exec(const AVD_CL_CB *cb);
  bool si_get_attr_value();
  bool siass_get_attr_value();
  bool csiass_get_attr_value();
  bool comp_get_attr_value();
  bool su_get_attr_value();

  ~ImmObjUpdate();
};

//
class ImmObjDelete : public ImmJob {
 public:
  std::string dn;

  ImmObjDelete() : ImmJob(){};
  bool immobj_update_required();
  bool is_csiass_exist();
  bool is_siass_exist();
  AvdJobDequeueResultT exec(const AVD_CL_CB *cb);

  ~ImmObjDelete();
};

class ImmAdminResponse : public ImmJob {
 public:
  ImmAdminResponse(const SaInvocationT invocation, const SaAisErrorT result) {
    this->invocation_ = invocation;
    this->result_ = result;
  }
  AvdJobDequeueResultT exec(const AVD_CL_CB *cb);

  ~ImmAdminResponse() {}

 private:
  ImmAdminResponse();
  SaInvocationT invocation_;
  SaAisErrorT result_;
};

class NtfSend : public Job {
 public:
  NtfSend() : already_sent(false) {}
  AvdJobDequeueResultT exec(const AVD_CL_CB *cb);
  AvdJobTypeT getJobType() { return JOB_TYPE_NTF; }
  bool isRunnable(const AVD_CL_CB *cb) { return true;}
  SaNtfNotificationsT myntf;
  bool already_sent;
  ~NtfSend();
};

// @todo move this into job.h
//
class Fifo {
 public:
  static Job *peek();

  static void queue(Job *job);

  static Job *dequeue();

  static AvdJobDequeueResultT execute(const AVD_CL_CB *cb);
  static AvdJobDequeueResultT executeAll(const AVD_CL_CB *cb,
      AvdJobTypeT job_type = JOB_TYPE_ANY);
  static AvdJobDequeueResultT executeAdminResp(const AVD_CL_CB *cb);

  static void empty();

  static uint32_t size();

  static void trim_to_size(const uint32_t size);

 private:
  static std::queue<Job *> job_;
};
//

/**
 * Install callbacks associated with classNames
 * @param className
 * @param rtattr_cb
 * @param adminop_cb
 * @param ccb_compl_cb
 * @param ccb_apply_cb
 * @param ccb_abort_cb
 */
void avd_class_impl_set(const std::string &className,
                        SaImmOiRtAttrUpdateCallbackT rtattr_cb,
                        SaImmOiAdminOperationCallbackT_2 adminop_cb,
                        AvdImmOiCcbCompletedCallbackT ccb_compl_cb,
                        AvdImmOiCcbApplyCallbackT ccb_apply_cb);
/**
 * Default callback that can be uses for e.g. base types, will return OK for
 * create & delete ops. Modify returns error.
 *
 * @param opdata
 *
 * @return SaAisErrorT
 */
SaAisErrorT avd_imm_default_OK_completed_cb(CcbUtilOperationData_t *opdata);

extern unsigned int avd_imm_config_get(void);
extern SaAisErrorT avd_saImmOiRtObjectUpdate_sync(
    const std::string &dn, SaImmAttrNameT attributeName,
    SaImmValueTypeT attrValueType, void *value,
    SaImmAttrModificationTypeT modifyType = SA_IMM_ATTR_VALUES_REPLACE);
extern SaAisErrorT avd_saImmOiRtObjectUpdate_multival_sync(
    const std::string &dn, SaImmAttrNameT attributeName,
    SaImmValueTypeT attrValueType, SaImmAttrValueT *value, uint32_t assigned_si,
    SaImmAttrModificationTypeT modifyType = SA_IMM_ATTR_VALUES_REPLACE);
extern SaAisErrorT avd_saImmOiRtObjectUpdate_replace_sync(
    const std::string &dn, SaImmAttrNameT attributeName,
    SaImmValueTypeT attrValueType, void *value,
    SaImmAttrModificationTypeT modifyType = SA_IMM_ATTR_VALUES_REPLACE);
extern void avd_saImmOiRtObjectUpdate(const std::string &dn,
                                      const std::string &attributeName,
                                      SaImmValueTypeT attrValueType,
                                      void *value);
extern void avd_saImmOiRtObjectCreate(const std::string &lassName,
                                      const std::string &parentName,
                                      const SaImmAttrValuesT_2 **attrValues);
extern void avd_saImmOiRtObjectDelete(const std::string &objectName);

extern void avd_imm_reinit_bg(void);
extern void avd_saImmOiAdminOperationResult(SaImmOiHandleT immOiHandle,
                                            SaInvocationT invocation,
                                            SaAisErrorT result);
void report_ccb_validation_error(const CcbUtilOperationData_t *opdata,
                                 const char *format, ...)
    __attribute__((format(printf, 2, 3)));
void report_admin_op_error(SaImmOiHandleT immOiHandle, SaInvocationT invocation,
                           SaAisErrorT result, struct admin_oper_cbk *pend_cbk,
                           const char *format, ...)
    __attribute__((format(printf, 5, 6)));
extern void check_and_flush_job_queue_standby_amfd(void);
void ckpt_job_queue_size();

#endif  // AMF_AMFD_IMM_H_
