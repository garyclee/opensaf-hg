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

pkglib_LTLIBRARIES = libfma_logstr.la

libfma_logstr_la_CPPFLAGS = \
        @NCS_CORE_CPP_DEFINES@ \
        -DNCS_DTS=1 \
        -I $(top_srcdir)/services/fm/fma/inc/ \
        -I $(top_srcdir)/services/leap/inc/ \
        -I $(top_srcdir)/services/mds/inc/ \
        -I $(top_srcdir)/services/mab/inc/ \
        -I $(top_srcdir)/services/common/inc/ \
        -I $(top_srcdir)/include/

libfma_logstr_la_SOURCES = fma_logstr.c

libfma_logstr_la_LDFLAGS = -version-number 1

