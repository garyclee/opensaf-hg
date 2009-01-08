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
# Author(s): Vishal Soni (vishal.soni@emerson.com) 
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = ncs_fms

noinst_HEADERS = \
      $(top_srcdir)/services/fm/fms/inc/fm_amf.h \
      $(top_srcdir)/services/fm/fms/inc/fm_cb.h \
      $(top_srcdir)/services/fm/fms/inc/fm_evt.h \
      $(top_srcdir)/services/fm/fms/inc/fm.h \
      $(top_srcdir)/services/fm/fms/inc/fm_mds.h \
      $(top_srcdir)/services/fm/fms/inc/fm_mem.h \
      $(top_srcdir)/services/fm/common/inc/fma_fm_intf.h

ncs_fms_CPPFLAGS= \
                -I $(top_srcdir)/services/fm/fms/inc/ \
                -I $(top_srcdir)/include/ \
                -I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/common/inc \
                -I $(top_srcdir)/services/fm/common/inc \
                @NCS_CORE_CPP_DEFINES@


ncs_fms_SOURCES = $(top_srcdir)/services/fm/fms/src/fm_main.c \
		  $(top_srcdir)/services/fm/fms/src/fm_mds.c \
		  $(top_srcdir)/services/fm/fms/src/fm_rda.c \
		  $(top_srcdir)/services/fm/fms/src/fm_amf.c \
		  $(top_srcdir)/services/fm/common/src/fma_fm_intf.c


ncs_fms_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/services/rde/rda/librda.la \
		$(top_srcdir)/services/fm/fma/libfma.la \
                $(top_srcdir)/services/hisv/hpl/libhpl.la \
                $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la 
