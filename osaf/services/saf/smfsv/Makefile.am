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
#            Ericsson AB
#

include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = config smfd smfnd

noinst_HEADERS = \
	inc/smfsv_defs.h \
	inc/smfsv_evt.h

dist_pkglib_SCRIPTS = \
        interface_commands/smf_backup_create \
        interface_commands/smf_bundle_check \
        interface_commands/smf_cluster_reboot \
        interface_commands/smf_node_check \
        interface_commands/smf_repository_check
