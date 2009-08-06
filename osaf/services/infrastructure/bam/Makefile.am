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

SUBDIRS = include logstr

noinst_LTLIBRARIES = libbam.la

libbam_la_CPPFLAGS = \
	-DNCS_BAM=1 -DNCS_MAB=1 -DNCS_DTA=1 -DNCS_BAM_LOG=1 \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/services/infrastructure/bam/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

libbam_la_CFLAGS = $(AM_CFLAGS) @HPI_CFLAGS@

libbam_la_CXXFLAGS = $(AM_CXXFLAGS) @HPI_CFLAGS@

libbam_la_LDFLAGS = -static

libbam_la_SOURCES = \
	amfConfig.cpp \
	bam_avd.c \
	bam_avm.c \
	bamError.cpp \
	bam_hw_deploy.cpp \
	bam_hw_util.c \
	bam_hw_vld.cpp \
	bam_init.cpp \
	bam_log.c \
	bam_mds.c \
	bam_mibs.cpp \
	bamParser.cpp \
	bam_psr.c \
	bam_rda.c \
	bam_sgs.cpp \
	psr_bam.c
