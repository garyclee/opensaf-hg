#      -*- OpenSAF  -*-
#
# (C) Copyright 2008 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
# Author(s): Wind River Systems
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = include

noinst_LTLIBRARIES = libavd.la

libavd_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/avsv/include \
	-I$(top_srcdir)/osaf/services/saf/avsv/avd/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include \
	-I$(top_srcdir)/osaf/snmp/mapi/include \
        -I$(top_srcdir)/osaf/libs/common/immsv/include

libavd_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

libavd_la_LDFLAGS = -static

libavd_la_SOURCES = \
	avd_app.c \
	avd_apptype.c \
	avd_avm.c \
	avd_avmmsg.c \
	avd.c \
	avd_chkop.c \
	avd_ckpt_dec.c \
	avd_ckpt_edu.c \
	avd_ckpt_enc.c \
	avd_ckpt_updt.c \
	avd_cluster.c \
	avd_comp.c \
	avd_compcstype.c \
	avd_comptype.c \
	avd_csi.c \
	avd_csiattr.c \
	avd_cstype.c \
	avd_ctcstype.c \
	avd_dblog.c \
	avd_dmsg.c \
	avd_hb.c \
	avd_hlt.c \
	avd_hlttype.c \
	avd_imm.c \
	avd_mds.c \
	avd_ndfsm.c \
	avd_ndmsg.c \
	avd_ndproc.c \
	avd_node.c \
	avd_nodegroup.c \
	avd_nodeswbundle.c \
	avd_pg.c \
	avd_proc.c \
	avd_role.c \
	avd_sg2Nfsm.c \
	avd_sg.c \
	avd_sgtype.c \
	avd_sgNAcvRfsm.c \
	avd_sgNoRfsm.c \
	avd_sgNpMfsm.c \
	avd_sgNWayfsm.c \
	avd_sgproc.c \
	avd_si.c \
	avd_siDep.c \
	avd_su.c \
	avd_susi.c \
	avd_sutcomptype.c \
	avd_sutype.c \
	avd_svctype.c \
	avd_svctypecstypes.c \
	avd_tmr.c \
	avd_ntf.c \
	avd_util.c \
	$(top_srcdir)/osaf/tools/safimm/src/immutil.c

