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

MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.h.in config.sub \
                       configure depcomp install-sh ltmain.sh missing \
                       Makefile.in m4/libtool.m4 m4/ltoptions.m4 m4/ltsugar.m4 \
		       m4/ltversion.m4 m4/lt~obsolete.m4


include $(top_srcdir)/Makefile.common
include $(top_srcdir)/samples/samples.mk

ACLOCAL_AMFLAGS = -I m4

# Retrieve values of the variables through 'configure' followed by
# 'make', not directly through 'configure', so that a user who
# sets some of these variables consistently on the 'make' command
# line gets correct results.
#
# One advantage of this approach, compared to the classical
# approach of adding -DLIBDIR=\"$(libdir)\" etc. to AM_CPPFLAGS,
# is that it protects against the use of undefined variables.
# If, say, $(libdir) is not set in the Makefile, LIBDIR is not
# defined by this module, and code using LIBDIR gives a
# compilation error.
#
# Another advantage is that 'make' output is shorter.
#
# Listed in the same order as the GNU makefile conventions.
# The Automake-defined pkg* macros are appended, in the order
# listed in the Automake 1.10a+ documentation.
configmake.h: Makefile
	rm -f $@-t $@
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	echo '#define PREFIX "$(prefix)"'; \
	echo '#define EXEC_PREFIX "$(exec_prefix)"'; \
	echo '#define BINDIR "$(bindir)"'; \
	echo '#define SBINDIR "$(sbindir)"'; \
	echo '#define LIBEXECDIR "$(libexecdir)"'; \
	echo '#define DATAROOTDIR "$(datarootdir)"'; \
	echo '#define DATADIR "$(datadir)"'; \
	echo '#define SYSCONFDIR "$(sysconfdir)"'; \
	echo '#define SHAREDSTATEDIR "$(sharedstatedir)"'; \
	echo '#define LOCALSTATEDIR "$(localstatedir)"'; \
	echo '#define INCLUDEDIR "$(includedir)"'; \
	echo '#define OLDINCLUDEDIR "$(oldincludedir)"'; \
	echo '#define DOCDIR "$(docdir)"'; \
	echo '#define INFODIR "$(infodir)"'; \
	echo '#define HTMLDIR "$(htmldir)"'; \
	echo '#define DVIDIR "$(dvidir)"'; \
	echo '#define PDFDIR "$(pdfdir)"'; \
	echo '#define PSDIR "$(psdir)"'; \
	echo '#define LIBDIR "$(libdir)"'; \
	echo '#define LISPDIR "$(lispdir)"'; \
	echo '#define LOCALEDIR "$(localedir)"'; \
	echo '#define MANDIR "$(mandir)"'; \
	echo '#define MANEXT "$(manext)"'; \
	echo '#define PKGSYSCONFDIR "$(sysconfdir)/$(PACKAGE_NAME)/"'; \
	echo '#define PKGLOCALSTATEDIR "$(localstatedir)/lib/$(PACKAGE_NAME)/"'; \
	echo '#define PKGDATADIR "$(pkgdatadir)"'; \
	echo '#define PKGINCLUDEDIR "$(pkgincludedir)"'; \
	echo '#define PKGLIBDIR "$(pkglibdir)"'; \
	echo '#define PKGLIBEXECDIR "$(pkglibexecdir)"'; \
	echo '#define PKGPIDDIR "$(localstatedir)/run/$(PACKAGE_NAME)/"'; \
	echo '#define PKGLOGDIR "$(localstatedir)/log/$(PACKAGE_NAME)/"'; \
	} | sed '/""/d' > $@-t
	mv $@-t $@

BUILT_SOURCES = configmake.h

CLEANFILES = configmake.h configmake.h-t

EXTRA_DIST = \
	$(top_srcdir)/tests


dist_doc_DATA = \
	$(top_srcdir)/AUTHORS \
	$(top_srcdir)/ChangeLog \
	$(top_srcdir)/COPYING.LIB \
	$(top_srcdir)/INSTALL \
	$(top_srcdir)/NEWS \
	$(top_srcdir)/OpenSAFCopyRights \
	$(top_srcdir)/THANKS \
	$(top_srcdir)/README \
	$(top_srcdir)/00-README.uml \
	$(top_srcdir)/00-README.conf \
	$(top_srcdir)/00-README.samples \
	$(top_srcdir)/00-README.debug \
	$(top_srcdir)/00-README.drbd


dist_pkgdata_DATA = 

nobase_dist_pkgdata_DATA = $(samples_sources)

SUBDIRS = osaf pkgconfig

if ENABLE_JAVA

SUBDIRS += java

endif

if ENABLE_TESTS

SUBDIRS += \
	tests/avsv/amfoitest \
	tests/logsv \
	tests/ntfsv \
	tests/immsv/implementer \
	tests/immsv/management
endif


nodist_pkgsysconf_DATA = \
	$(top_builddir)/config/script.conf


dist_pkglocalstate_DATA = \
	$(top_srcdir)/config/.drbd_sync_state_0 \
	$(top_srcdir)/config/.drbd_sync_state_1 \
	$(top_srcdir)/config/.drbd_sync_state_2 \
	$(top_srcdir)/config/.drbd_sync_state_3 \
	$(top_srcdir)/config/.drbd_sync_state_4 \
	$(top_srcdir)/config/node_ha_state

dist_pkgimmxml_DATA = \
	$(top_srcdir)/osaf/tools/safimm/immxml/README


dist_bin_SCRIPTS = \
	$(top_srcdir)/scripts/amf-adm \
	$(top_srcdir)/scripts/amf-find \
	$(top_srcdir)/scripts/amf-state \
	$(top_srcdir)/scripts/opensaf_mem_dump

dist_pkgimmxml_SCRIPTS = \
	$(top_srcdir)/osaf/tools/safimm/immxml/baseimm.py \
	$(top_srcdir)/osaf/tools/safimm/immxml/immxml-configure \
	$(top_srcdir)/osaf/tools/safimm/immxml/immxml-merge \
	$(top_srcdir)/osaf/tools/safimm/immxml/immxml-nodegen \
	$(top_srcdir)/osaf/tools/safimm/immxml/immxml-validate \
	$(top_srcdir)/osaf/tools/safimm/immxml/immxml-clustersize \
	$(top_srcdir)/osaf/tools/safimm/immxml/verify.sh

nodist_bin_SCRIPTS = \
	$(top_builddir)/scripts/collect_logs_controller \
	$(top_builddir)/scripts/collect_logs_payload \
	$(top_builddir)/scripts/get_ha_state 


dist_pkglib_SCRIPTS = \
	$(top_srcdir)/scripts/find_pid \
	$(top_srcdir)/scripts/opensaf_reboot 

if ENABLE_RPM_TARGET

RPMTOPDIR = `pwd`/rpms
RPMSOURCEDIR = $(RPMTOPDIR)/SOURCES
RPMSPECDIR = $(RPMTOPDIR)/SPECS
SRPMDIR = $(RPMTOPDIR)/SRPMS
RPMDIR = $(RPMTOPDIR)/RPMS

prep-rpm-dir: dist
	mkdir -p $(RPMTOPDIR)
	mkdir -p $(RPMTOPDIR)/{BUILD,RPMS,SOURCES,SPECS,SRPMS,tmp}
	cp $(top_srcdir)/$(PACKAGE_NAME).spec $(RPMSPECDIR)
	cp $(top_srcdir)/$(PACKAGE_NAME)-$(VERSION).tar.gz $(RPMSOURCEDIR)

srpm: prep-rpm-dir
	rpmbuild -bs --rmspec --rmsource \
		--define "_topdir $(RPMTOPDIR)" --define "_tmppath $(RPMTOPDIR)/tmp" \
		$(RPMSPECDIR)/$(PACKAGE_NAME).spec

rpm: prep-rpm-dir
	rpmbuild -bb --clean --rmspec --rmsource \
		--define "_topdir $(RPMTOPDIR)" --define "_tmppath $(RPMTOPDIR)/tmp" \
		$(RPMSPECDIR)/$(PACKAGE_NAME).spec

endif

update-ldconfig:
	echo "$(pkglibdir)" > $(top_builddir)/config/$(PACKAGE_NAME)-$(host_cpu).conf
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/ld.so.conf.d
	$(INSTALL_DATA) $(top_builddir)/config/$(PACKAGE_NAME)-$(host_cpu).conf $(DESTDIR)$(sysconfdir)/ld.so.conf.d

install-data-local: update-ldconfig
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/log/$(PACKAGE_NAME)
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/run/$(PACKAGE_NAME)

install-data-hook:
	@for i in $$(grep -lr -e 'xxLIBDIRxx' $(DESTDIR)$(pkgimmxml_svcdir)/*.xml) ; do \
		sed -i 's|xxLIBDIRxx|$(pkglibdir)|g' "$$i"; \
	done
	@for i in $$(grep -lr -e 'xxBINDIRxx' -e 'xxLIBDIRxx' $(DESTDIR)$(pkgsysconfdir)/*) ; do \
		sed -i 's|xxBINDIRxx|$(bindir)|g' "$$i"; \
		sed -i 's|xxLIBDIRxx|$(pkglibdir)|g' "$$i"; \
	done

uninstall-local:
	-rm -rf $(DESTDIR)$(pkgsysconfdir)
	-rm -rf $(DESTDIR)$(pkgincludedir)
	-rm -rf $(DESTDIR)$(pkglibdir)
	-rm -rf $(DESTDIR)$(docdir)
	-rm -rf $(DESTDIR)$(pkgdatadir)
	-rm -rf $(DESTDIR)$(pkglocalstatedir)
	-rm -rf $(DESTDIR)$(localstatedir)/log/$(PACKAGE_NAME)
	-rm -rf $(DESTDIR)$(localstatedir)/run/$(PACKAGE_NAME)
	-rm -f $(DESTDIR)$(sysconfdir)/ld.so.conf.d/$(PACKAGE_NAME)-$(host_cpu).conf

docs:
	doxygen

clean-local:
	-rm -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
if ENABLE_RPM_TARGET
	-rm -rf $(RPMTOPDIR)/{BUILD,RPMS,SOURCES,SPECS,SRPMS,tmp}
endif

distclean-local:
	-rm -f $(top_builddir)/config/$(PACKAGE_NAME)-$(host_cpu).conf
	-rm -rf docs/
