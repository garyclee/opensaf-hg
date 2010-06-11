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

MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/Makefile.common

BUILT_SOURCES = build-javadoc

EXTRA_DIST = \
	src \
	overview.html \
	build.xml

build-javadoc:
	$(ANT) -f build.xml javadocAPI

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(javadocdir)/$(PACKAGE_NAME)
	@cd javadoc; \
	for file in `find . -name \*`; do \
		$(INSTALL) -D -m 644 ../$(top_builddir)/java/ais_api/javadoc/$$file $(DESTDIR)$(javadocdir)/$(PACKAGE_NAME)/$$file.txt; \
	done; \
	cd -

uninstall-local:
	-rm -rf $(DESTDIR)$(javadocdir)/$(PACKAGE_NAME)

clean-local:
	$(ANT) -f build.xml clean
	-rm -rf $(top_builddir)/java/ais_api/javadoc

