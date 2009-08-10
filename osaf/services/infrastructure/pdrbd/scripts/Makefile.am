#      -*- OpenSAF  -*-
#
# (C) Copyright 2009 The OpenSAF Foundation
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

dist_pkglib_SCRIPTS = \
	drbd_clean

nodist_pkglib_SCRIPTS = \
	$(top_builddir)/osaf/services/infrastructure/pdrbd/scripts/drbdctrl \
	$(top_builddir)/osaf/services/infrastructure/pdrbd/scripts/drbd_start \
	$(top_builddir)/osaf/services/infrastructure/pdrbd/scripts/ncs_drbd_r0_clean \
	$(top_builddir)/osaf/services/infrastructure/pdrbd/scripts/pdrbdsts \
	$(top_builddir)/osaf/services/infrastructure/pdrbd/scripts/pdrbdctrl
