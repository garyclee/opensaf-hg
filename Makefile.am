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
	echo '#define OSAF_SYSCONFDIR "$(sysconfdir)/$(PACKAGE_NAME)/"'; \
	echo '#define SHAREDSTATEDIR "$(sharedstatedir)"'; \
	echo '#define LOCALSTATEDIR "$(localstatedir)"'; \
	echo '#define OSAF_LOCALSTATEDIR "$(localstatedir)/lib/$(PACKAGE_NAME)/"'; \
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
	echo '#define PKGDATADIR "$(pkgdatadir)"'; \
	echo '#define PKGINCLUDEDIR "$(pkgincludedir)"'; \
	echo '#define PKGLIBDIR "$(pkglibdir)"'; \
	echo '#define PKGLIBEXECDIR "$(pkglibexecdir)"'; \
	echo '#define PIDPATH "$(localstatedir)/run/$(PACKAGE_NAME)/"'; \
	} | sed '/""/d' > $@-t
	mv $@-t $@

BUILT_SOURCES = configmake.h

CLEANFILES = configmake.h configmake.h-t


if ENABLE_SMIDUMP

BUILT_SOURCES += build_smidump

build_smidump:
	@if [ ! $(top_srcdir) -ef $(top_builddir) ]; then \
		cp -R $(top_srcdir)/tools/smidump $(top_builddir)/tools; \
	fi
	@$(top_builddir)/tools/smidump/generate_smidump.sh

endif


EXTRA_DIST = \
	$(top_srcdir)/initd/opensafd.initd \
	$(top_srcdir)/tools/smidump \
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


# FIXME Like this until SNMP references removed
SUBDIRS = osaf/snmp/mapi osaf/snmp/miblib osaf
#SUBDIRS = osaf

if ENABLE_JAVA

SUBDIRS += java

endif


dist_pkgsysconf_DATA = \
	$(top_srcdir)/config/imm.conf \
	$(top_srcdir)/config/AppConfig.xsd \
	$(top_srcdir)/config/NCSConfig.xsd \
	$(top_srcdir)/config/NCSSystemBOM.xml \
	$(top_srcdir)/config/NCSSystemBOM.xml.hp.c-class \
	$(top_srcdir)/config/ValidationConfig.xml \
	$(top_srcdir)/config/ValidationConfig.xsd \
	$(top_srcdir)/config/vipsample.conf \
	$(top_srcdir)/config/nodeinit.conf.controller \
	$(top_srcdir)/config/nodeinit.conf.payload \
	$(top_srcdir)/config/saflog.conf \
	$(top_srcdir)/config/ntf.conf \
	$(top_srcdir)/config/rde.conf \
	$(top_srcdir)/config/fms.conf \
	$(top_srcdir)/config/imm.xml \
	$(top_srcdir)/config/pdrbd_proxied.conf

nodist_pkgsysconf_DATA = \
	$(top_builddir)/config/script.conf

dist_sysconf_DATA = \
	$(top_srcdir)/config/drbd.conf


dist_pkglocalstate_DATA = \
	$(top_srcdir)/config/.drbd_sync_state_0 \
	$(top_srcdir)/config/.drbd_sync_state_1 \
	$(top_srcdir)/config/.drbd_sync_state_2 \
	$(top_srcdir)/config/.drbd_sync_state_3 \
	$(top_srcdir)/config/.drbd_sync_state_4 \
	$(top_srcdir)/config/node_ha_state


dist_bin_SCRIPTS = \
	$(top_srcdir)/scripts/opensaf_mem_dump

nodist_bin_SCRIPTS = \
	$(top_builddir)/scripts/collect_logs_controller \
	$(top_builddir)/scripts/collect_logs_payload \
	$(top_builddir)/scripts/get_ha_state 


dist_pkglib_SCRIPTS = \
	$(top_srcdir)/scripts/drbd_clean \
	$(top_srcdir)/scripts/find_pid \
	$(top_srcdir)/scripts/ncs_cpd_clean \
	$(top_srcdir)/scripts/ncs_cpnd_clean \
	$(top_srcdir)/scripts/ncs_gld_clean \
	$(top_srcdir)/scripts/ncs_glnd_clean \
	$(top_srcdir)/scripts/ncs_mqd_clean \
	$(top_srcdir)/scripts/ncs_mqnd_clean \
	$(top_srcdir)/scripts/ncs_srmnd_clean \
	$(top_srcdir)/scripts/ncs_vds_clean \
	$(top_srcdir)/scripts/opensaf_reboot 

if ENABLE_HPI

dist_pkglib_SCRIPTS += \
	$(top_srcdir)/scripts/ncs_hisv_clean

endif

nodist_pkglib_SCRIPTS = \
	$(top_builddir)/scripts/dhclnt_clean \
	$(top_builddir)/scripts/dhclnt_start \
	$(top_builddir)/scripts/drbdctrl \
	$(top_builddir)/scripts/drbd_start \
	$(top_builddir)/scripts/ncs_drbd_r0_clean \
	$(top_builddir)/scripts/pdrbdsts \
	$(top_builddir)/scripts/ncs_cpd_start \
	$(top_builddir)/scripts/ncs_cpnd_start \
	$(top_builddir)/scripts/ncs_dts_clean \
	$(top_builddir)/scripts/ncs_dts_start \
	$(top_builddir)/scripts/ncs_eds_clean \
	$(top_builddir)/scripts/ncs_eds_start \
	$(top_builddir)/scripts/opensaf_fms \
	$(top_builddir)/scripts/ncs_gld_start \
	$(top_builddir)/scripts/ncs_glnd_start \
	$(top_builddir)/scripts/ncs_ifd_clean \
	$(top_builddir)/scripts/ncs_ifd_start \
	$(top_builddir)/scripts/ncs_ifnd_clean \
	$(top_builddir)/scripts/ncs_ifnd_start \
	$(top_builddir)/scripts/ncs_mas_clean \
	$(top_builddir)/scripts/ncs_mas_start \
	$(top_builddir)/scripts/ncs_mqd_start \
	$(top_builddir)/scripts/ncs_mqnd_start \
	$(top_builddir)/scripts/ncs_pcap_clean \
	$(top_builddir)/scripts/ncs_pcap_start \
	$(top_builddir)/scripts/ncs_pss_clean \
	$(top_builddir)/scripts/ncs_pss_start \
	$(top_builddir)/scripts/ncs_rde_clean \
	$(top_builddir)/scripts/ncs_rde_start \
	$(top_builddir)/scripts/ncs_scap_clean \
	$(top_builddir)/scripts/ncs_scap_start \
	$(top_builddir)/scripts/ncs_srmnd_start \
	$(top_builddir)/scripts/ncs_vds_start \
	$(top_builddir)/scripts/nid_tipc \
	$(top_builddir)/scripts/immd_script \
	$(top_builddir)/scripts/immnd_script \
	$(top_builddir)/scripts/opensaf_lgs \
	$(top_builddir)/scripts/opensaf_ntfs \
	$(top_builddir)/scripts/pdrbdctrl \
	$(top_builddir)/scripts/rde_script \
	$(top_builddir)/scripts/tipc_reset.sh 

if ENABLE_HPI

nodist_pkglib_SCRIPTS += \
	$(top_builddir)/scripts/ncs_hisv_start

endif

if WITH_OPENHPI

nodist_pkglib_SCRIPTS += \
	$(top_builddir)/scripts/openhpid_clean \
	$(top_builddir)/scripts/openhpid_start \
	$(top_builddir)/scripts/openhpisubagt_clean \
	$(top_builddir)/scripts/openhpisubagt_start

endif


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

update-pkgconfig:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(top_builddir)/pkgconfig/opensaf-*.pc $(DESTDIR)$(libdir)/pkgconfig

set-default-node-config:
	@echo "*** Setting default controller node configuration ***"
	echo "2" > $(DESTDIR)$(pkgsysconfdir)/chassis_id
	echo "1" > $(DESTDIR)$(pkgsysconfdir)/slot_id
	echo "15" > $(DESTDIR)$(pkgsysconfdir)/subslot_id
	echo "controller" > $(DESTDIR)$(pkgsysconfdir)/node_type

install-data-local: update-ldconfig update-pkgconfig
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/log/$(PACKAGE_NAME)
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/run/$(PACKAGE_NAME)
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/init.d
	$(INSTALL) -m 755 $(top_srcdir)/initd/opensafd.initd $(DESTDIR)$(sysconfdir)/init.d/opensafd

install-data-hook: set-default-node-config
	@for i in $$(grep -lr -e 'xxBINDIRxx' -e 'xxLIBDIRxx' -e 'xxPIDPATHxx'  $(DESTDIR)$(pkgsysconfdir)/*) ; do \
		sed -i 's|xxPIDPATHxx|$(localstatedir)/run/$(PACKAGE_NAME)|g' "$$i"; \
		sed -i 's|xxBINDIRxx|$(bindir)|g' "$$i"; \
		sed -i 's|xxLIBDIRxx|$(pkglibdir)|g' "$$i"; \
	done
	cd $(DESTDIR)$(pkgsysconfdir) && $(LN_S) -f nodeinit.conf.controller nodeinit.conf

uninstall-local:
	-rm -rf $(DESTDIR)$(pkgsysconfdir)
	-rm -rf $(DESTDIR)$(pkgincludedir)
	-rm -rf $(DESTDIR)$(pkglibdir)
	-rm -rf $(DESTDIR)$(docdir)
	-rm -rf $(DESTDIR)$(pkgdatadir)
	-rm -rf $(DESTDIR)$(pkglocalstatedir)
	-rm -rf $(DESTDIR)$(localstatedir)/log/$(PACKAGE_NAME)
	-rm -rf $(DESTDIR)$(localstatedir)/run/$(PACKAGE_NAME)
	-rm -f $(DESTDIR)$(sysconfdir)/init.d/opensafd
	-rm -f $(DESTDIR)$(sysconfdir)/ld.so.conf.d/$(PACKAGE_NAME)-$(host_cpu).conf
	-rm -f $(DESTDIR)$(libdir)/pkgconfig/opensaf-*.pc

clean-local:
	-rm -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
if ENABLE_RPM_TARGET
	-rm -rf $(RPMTOPDIR)/{BUILD,RPMS,SOURCES,SPECS,SRPMS,tmp}
endif

distclean-local:
	-rm -f $(top_builddir)/config/$(PACKAGE_NAME)-$(host_cpu).conf
if ENABLE_SMIDUMP
	@if [ -f $(top_builddir)/tools/smidump/bin/smidump ]; then \
		rm -rf $(top_builddir)/tools/smidump/bin; \
		make -C $(top_builddir)/tools/smidump/src distclean; \
	fi
endif
