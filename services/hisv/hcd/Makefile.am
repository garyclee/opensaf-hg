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

MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libhcd.la

libhcd_la_CPPFLAGS = \
	@NCS_CORE_CPP_DEFINES@ \
	-DNCS_HISV_LOG=1 \
	-I $(top_srcdir)/services/hisv/inc \
	-I $(top_srcdir)/services/leap/inc \
	-I $(top_srcdir)/services/mds/inc \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/mab/inc \
	-I $(top_srcdir)/include

libhcd_la_CFLAGS = @HPI_CFLAGS@

libhcd_la_LDFLAGS = -static

libhcd_la_SOURCES = \
	ham/ham_init.c \
	ham/ham.c \
	ham/ham_evt.c \
	ham/ham_mds.c \
	ham/ham_tmr.c \
	hsm/hsm.c \
	hsm/hsm_init.c \
	sim/sim.c \
	sim/sim_init.c \
	sim/sim_evt.c \
	hcd_util.c \
	hcd.c \
	hcd_amf.c \
	hcd_log.c \
	hisv_demo.c
