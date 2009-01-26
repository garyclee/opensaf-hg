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

include $(top_srcdir)/samples/avsv/avsv.mk
include $(top_srcdir)/samples/avsv/pxy_pxd_support/pxy_pxd.mk
include $(top_srcdir)/samples/cli/cli.mk
include $(top_srcdir)/samples/cpsv/cpsv.mk
include $(top_srcdir)/samples/dtsv/dtsv.mk
include $(top_srcdir)/samples/edsv/edsv.mk
include $(top_srcdir)/samples/glsv/glsv.mk
include $(top_srcdir)/samples/leap/leap.mk
include $(top_srcdir)/samples/masv/masv.mk
include $(top_srcdir)/samples/mbcsv/mbcsv.mk
include $(top_srcdir)/samples/mds/mds.mk
include $(top_srcdir)/samples/mqsv/mqsv.mk
include $(top_srcdir)/samples/srmsv/srmsv.mk
include $(top_srcdir)/samples/subagt/subagt.mk

samples_sources = \
   $(avsv_sources) \
   $(pxy_pxd_sources) \
   $(cli_sources) \
   $(cpsv_sources) \
   $(dtsv_sources) \
   $(edsv_sources) \
   $(glsv_sources) \
   $(leap_sources) \
   $(masv_sources) \
   $(mbcsv_sources) \
   $(mds_sources) \
   $(mqsv_sources) \
   $(srmsv_sources) \
   $(subagt_sources)
