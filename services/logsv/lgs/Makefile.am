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
# Author(s): Ericsson AB
#
#

bin_PROGRAMS = opensaf_saflogd

opensaf_saflogd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_MAIN_EXTERNAL \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

opensaf_saflogd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c \
			./lgs_amf.c \
			./lgs_evt.c \
			./lgs_mbcsv.c \
			./lgs_fmt.c \
			./lgs_main.c \
			./lgs_mds.c \
			./lgs_util.c \
			./lgs_stream.c

opensaf_saflogd_LDADD = \
	$(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
	$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_srcdir)/services/mbcsv/src/libmbca.la

