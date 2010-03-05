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
# Author(s):  Emerson Network Power
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

pkglib_PROGRAMS = opensaf_clmna

opensaf_clmna_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/osaf/libs/common/clmsv/include \
	-I$(top_srcdir)/osaf/services/infrastructure/masv/include

opensaf_clmna_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	$(top_srcdir)/osaf/libs/common/clmsv/clmsv_enc_dec.c \
	clms_nodeagent.c

opensaf_clmna_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaClm/libSaClm.la 
