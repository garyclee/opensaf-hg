#      OpenSAF 
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
#            Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = scripts

noinst_HEADERS = \
	inc/smfnd.h \
	inc/smfnd_evt.h

pkglib_PROGRAMS = opensaf_smfnd

opensaf_smfnd_CPPFLAGS = \
	-DNCS_MAIN_EXTERNAL \
	$(AM_CPPFLAGS) \
	-I $(top_srcdir)/osaf/services/saf/smfsv/inc \
	-I $(top_srcdir)/osaf/services/saf/smfsv/smfnd/inc \
	-I $(top_srcdir)/osaf/libs/core/common/include \
	-I $(top_srcdir)/osaf/libs/core/include \
	-I $(top_srcdir)/osaf/services/infrastructure/masv/include

opensaf_smfnd_SOURCES = \
	$(top_srcdir)/osaf/libs/core/common/ncs_main_pvt.c \
	smfnd_main.c \
	smfnd_amf.c \
	smfnd_evt.c \
	smfnd_mds.c

opensaf_smfnd_LDADD = \
	$(top_builddir)/osaf/libs/saf/libSaAmf/libSaAmf.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/osaf/libs/common/avsv/libavsv_common.la \
	$(top_builddir)/osaf/libs/common/smfsv/libsmfsv_common.la
