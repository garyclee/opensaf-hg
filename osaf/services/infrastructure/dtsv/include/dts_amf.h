/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
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

/*****************************************************************************
FILE NAME: DTS_AMF.H
DESCRIPTION: Function prototypes used for DTS used in AMF.
******************************************************************************/
#ifndef DTS_AMF_H
#define DTS_AMF_H
/*** Macro used to get the AMF version used ****/
uns32 dts_amf_init(struct dts_cb *dts_cb_inst);
uns32 dts_amf_finalize(struct dts_cb *dts_cb_inst);
void dts_amf_sigusr1_handler(int i_sig_num);
uns32 dts_amf_register(DTS_CB *inst);

#define m_DTS_GET_AMF_VER(amf_ver) amf_ver.releaseCode='B'; amf_ver.majorVersion=0x01; amf_ver.minorVersion=0x01;

#endif
