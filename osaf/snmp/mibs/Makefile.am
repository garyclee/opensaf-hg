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
include $(top_srcdir)/osaf/snmp/mibs/Makefile.mibs

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = $(mibs_files)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/snmp/mibs
	@for file in $(mibs_files); do \
		$(INSTALL) -m 644 $(top_srcdir)/osaf/snmp/mibs/$$file $(DESTDIR)$(datadir)/snmp/mibs/$$file.txt; \
	done

uninstall-local:
	@for file in $(mibs_files); do \
		rm -f $(DESTDIR)$(datadir)/snmp/mibs/$$file.txt; \
	done
