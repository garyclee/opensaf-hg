#      -*- OpenSAF  -*-
#
# (C) Copyright 2010 The OpenSAF Foundation
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

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	compile \
	config.guess \
	config.h.in \
	config.sub \
	configure \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	Makefile.in \
	m4/libtool.m4 \
	m4/ltoptions.m4 \
	m4/ltsugar.m4 \
	m4/ltversion.m4 \
	m4/lt~obsolete.m4

include $(top_srcdir)/Makefile.common

EXTRA_DIST = \
	AUTHORS \
	ChangeLog \
	COPYING.LIB \
	INSTALL \
	NEWS \
	THANKS \
	README \
	m4

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = config include lib plmcd scripts

install-data-hook:
	@for i in $$(grep -lr -e '$${exec_prefix}' $(DESTDIR)$(sysconfdir)/init.d/plmc*) ; do \
		sed -i 's|$${exec_prefix}|$(exec_prefix)|g' "$$i"; \
	done

clean-local:
	-rm -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
