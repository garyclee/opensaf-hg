#ifndef CLMSV_ENC_DEC_H
#define CLMSV_ENC_DEC_H

#include <saClm.h>
#include <ncsgl_defs.h>
#include <ncs_log.h>
#include <ncs_lib.h>
#include <mds_papi.h>
#include <ncs_mda_pvt.h>
#include <mbcsv_papi.h>
#include <ncs_edu_pub.h>
#include <ncs_util.h>
#include <logtrace.h>
#include <clmsv_msg.h>

EXTERN_C uns32 decodeSaNameT(NCS_UBAID *uba,SaNameT *name);
EXTERN_C uns32 decodeNodeAddressT(NCS_UBAID *uba,SaClmNodeAddressT * nodeAddress);
EXTERN_C uns32 encodeSaNameT(NCS_UBAID *uba,SaNameT *name);
#endif
