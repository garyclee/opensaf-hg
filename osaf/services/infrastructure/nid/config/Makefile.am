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

dist_pkgsysconf_DATA = \
	$(top_srcdir)/osaf/services/infrastructure/nid/config/nodeinit.conf.controller \
	$(top_srcdir)/osaf/services/infrastructure/nid/config/nodeinit.conf.payload \
	$(top_srcdir)/osaf/services/infrastructure/nid/config/nid.conf

set-default-node-config:
	@echo "*** Setting default controller node configuration ***"
	echo "2" > $(DESTDIR)$(pkgsysconfdir)/chassis_id
	echo "1" > $(DESTDIR)$(pkgsysconfdir)/slot_id
	echo "15" > $(DESTDIR)$(pkgsysconfdir)/subslot_id
	echo "controller" > $(DESTDIR)$(pkgsysconfdir)/node_type

install-data-hook: set-default-node-config
if !ENABLE_AIS_PLM
	sed -i '/plmd/d'  $(DESTDIR)$(pkgsysconfdir)/nodeinit.conf.controller
endif
if !ENABLE_TIPC_TRANSPORT
	sed -i '/TIPC/d'  $(DESTDIR)$(pkgsysconfdir)/nodeinit.conf.controller
	sed -i '/TIPC/d'  $(DESTDIR)$(pkgsysconfdir)/nodeinit.conf.payload
	sed -i 's/MDS_TRANSPORT=TIPC/MDS_TRANSPORT=TCP/g' $(DESTDIR)$(pkgsysconfdir)/nid.conf
endif
