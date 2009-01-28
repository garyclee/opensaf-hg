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
                       Makefile.in


include $(top_srcdir)/include.mk
include $(top_srcdir)/samples/samples.mk
include $(top_srcdir)/mibs/mibs.mk

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


dist_doc_DATA = \
	$(top_srcdir)/AUTHORS \
	$(top_srcdir)/ChangeLog \
	$(top_srcdir)/COPYING.LIB \
	$(top_srcdir)/INSTALL \
	$(top_srcdir)/NEWS \
	$(top_srcdir)/OpenSAFCopyRights \
	$(top_srcdir)/THANKS \
	$(top_srcdir)/README \
	$(top_srcdir)/README.uml \
	$(top_srcdir)/README.conf \
	$(top_srcdir)/README.samples


dist_pkgdata_DATA = \
	$(top_srcdir)/config/drbd.1.conf \
	$(top_srcdir)/config/drbd.2.conf

nobase_dist_pkgdata_DATA = $(samples_sources)

pkginclude_HEADERS = \
	include/cpsv_papi.h \
	include/dta_papi.h \
	include/dts_papi.h \
	include/hpl_api.h \
	include/hpl_msg.h \
	include/ifa_papi.h \
	include/ifsv_cli_papi.h \
	include/ifsv_papi.h \
	include/ip_defs.h \
	include/ipxs_papi.h \
	include/lfm_avm_intf.h \
	include/mab_penv.h \
	include/mac_papi.h \
	include/mbcsv_papi.h \
	include/mds_papi.h \
	include/ncs_cli.h \
	include/ncs_dl.h \
	include/ncs_edu_pub.h \
	include/ncsencdec_pub.h \
	include/ncsgl_defs.h \
	include/ncs_hdl_pub.h \
	include/ncs_ip.h \
	include/ncs_iplib.h \
	include/ncs_ipprm.h \
	include/ncs_ipv4.h \
	include/ncs_ipv6.h \
	include/ncs_lib.h \
	include/ncs_log.h \
	include/ncs_main_papi.h \
	include/ncs_mda_papi.h \
	include/ncs_mds_def.h \
	include/ncsmiblib.h \
	include/ncsmib_mem.h \
	include/ncs_mib_pub.h \
	include/ncs_mtbl.h  \
	include/ncs_mtree.h \
	include/ncs_osprm.h \
	include/ncspatricia.h \
	include/ncs_posix_tmr.h \
	include/ncs_queue.h \
	include/ncs_saf.h \
	include/ncs_scktprm.h \
	include/ncs_sprr_papi.h \
	include/ncs_stack_pub.h \
	include/ncs_svd.h \
	include/ncssysf_def.h \
	include/ncssysf_ipc.h \
	include/ncssysf_lck.h \
	include/ncssysf_mem.h \
	include/ncssysfpool.h \
	include/ncssysf_sem.h \
	include/ncssysf_tmr.h \
	include/ncssysf_tsk.h \
	include/ncs_tmr.h \
	include/ncs_trap.h \
	include/ncs_ubaid.h \
	include/ncsusrbuf.h \
	include/ncs_xmtre.h \
	include/nid_api.h \
	include/nid_err.h \
	include/oac_papi.h \
	include/os_defs.h \
	include/psr_mib_load.h \
	include/psr_papi.h \
	include/rda_papi.h \
	include/sckt_defs.h \
	include/srma_papi.h \
	include/srmsv_papi.h \
	include/subagt_mab.h \
	include/subagt_oid_db.h \
	include/sysf_ip.h \
	include/trg_defs.h

include_HEADERS = \
	include/saAis.h \
	include/saAmf.h \
	include/saCkpt.h \
	include/saClm.h \
	include/saEvt.h \
	include/saLck.h \
	include/saLog.h \
	include/saMsg.h \
	include/saNtf.h


BASE_SUBDIRS = \
	services/leap \
	services/common \
	services/mds \
	services/vds/vda \
	services/mab/oac \
	services/dtsv/dta \
	lib/lib_ncs_core \
	services/avsv/ava \
	services/avsv/common \
	lib/lib_SaAmf \
	services/cpsv/common \
	services/cpsv/cpa \
	lib/lib_SaCkpt \
	services/avsv/cla \
	lib/lib_SaClm \
	services/edsv/common \
	services/edsv/eda \
	lib/lib_SaEvt \
	services/glsv/gla \
	lib/lib_SaLck \
	services/logsv/lga \
	lib/lib_SaLog \
	services/mqsv/common \
	services/mqsv/mqa \
	lib/lib_SaMsg \
	services/mab/mac \
	services/mbcsv/src \
	services/srmsv/srma \
	services/ifsv/ifa \
	services/ifsv/common

CONTROLLER_SUBDIRS = \
	services/dtsv/snmp_integ \
	services/mab/psr/snmp_integ \
	services/mqsv/snmp_integ \
	services/edsv/snmp_integ \
	services/bam \
	services/cpsv/snmp_integ \
	services/glsv/snmp_integ \
	services/glsv/common \
	services/ifsv/snmp_integ \
	services/ifsv/ipxs_snmp_integ \
	services/ifsv/entp_snmp_integ \
	services/ifsv/entp_ifsv_ir_snmp_integ \
	services/rde/rda \
	tools/pdrbd \
	services

SUBDIRS = $(BASE_SUBDIRS) $(CONTROLLER_SUBDIRS)

if ENABLE_JAVA

SUBDIRS += java

endif

if ENABLE_HPI

SUBDIRS += tools/hisv

endif

common_sysconf = \
	$(top_srcdir)/config/chassis_id \
	$(top_srcdir)/config/slot_id

controller_sysconf = \
	$(top_srcdir)/config/AppConfig.xsd \
	$(top_srcdir)/config/cli_cefslib_conf \
	$(top_srcdir)/config/NCSConfig.xsd \
	$(top_srcdir)/config/ncsSnmpSubagt.conf \
	$(top_srcdir)/config/NCSSystemBOM.xml \
	$(top_srcdir)/config/NCSSystemBOM.xml.hp.c-class \
	$(top_srcdir)/config/pssv_lib_conf \
	$(top_srcdir)/config/subagt_lib_conf \
	$(top_srcdir)/config/ValidationConfig.xml \
	$(top_srcdir)/config/ValidationConfig.xsd \
	$(top_srcdir)/config/vipsample.conf \
	$(top_srcdir)/config/nodeinit.conf.controller \
	$(top_srcdir)/config/.drbd_sync_state_0 \
	$(top_srcdir)/config/.drbd_sync_state_1 \
	$(top_srcdir)/config/.drbd_sync_state_2 \
	$(top_srcdir)/config/.drbd_sync_state_3 \
	$(top_srcdir)/config/.drbd_sync_state_4 \
	$(top_srcdir)/config/saflog.conf \
	$(top_srcdir)/config/rde.conf

payload_sysconf = \
	$(top_srcdir)/config/nodeinit.conf.payload

dist_pkgsysconf_DATA = \
	$(common_sysconf) \
	$(controller_sysconf) \
	$(payload_sysconf)

nodist_pkgsysconf_DATA = \
	$(top_builddir)/config/script.conf

dist_pkglocalstate_DATA = \
	$(top_srcdir)/config/node_ha_state \
	$(top_srcdir)/config/pssv_spcn_list

dist_bin_SCRIPTS = \
	$(top_srcdir)/scripts/drbd_clean \
	$(top_srcdir)/scripts/find_pid \
	$(top_srcdir)/scripts/ncs_cpd_clean \
	$(top_srcdir)/scripts/ncs_cpnd_clean \
	$(top_srcdir)/scripts/ncs_drbd_r0_clean \
	$(top_srcdir)/scripts/ncs_gld_clean \
	$(top_srcdir)/scripts/ncs_glnd_clean \
	$(top_srcdir)/scripts/ncs_mqd_clean \
	$(top_srcdir)/scripts/ncs_mqnd_clean \
	$(top_srcdir)/scripts/ncs_srmnd_clean \
	$(top_srcdir)/scripts/ncs_vds_clean \
	$(top_srcdir)/scripts/opensaf_mem_dump_tool \
	$(top_srcdir)/scripts/pdrbdsts

if ENABLE_HPI
dist_bin_SCRIPTS += $(top_srcdir)/scripts/ncs_hisv_clean
endif

nodist_bin_SCRIPTS = \
	$(top_builddir)/scripts/collect_logs_controller \
	$(top_builddir)/scripts/collect_logs_payload \
	$(top_builddir)/scripts/dhclnt_clean \
	$(top_builddir)/scripts/dhclnt_start \
	$(top_builddir)/scripts/drbdctrl \
	$(top_builddir)/scripts/drbd_start \
	$(top_builddir)/scripts/get_ha_state \
	$(top_builddir)/scripts/ip_inst_script \
	$(top_builddir)/scripts/ncs_cpd_start \
	$(top_builddir)/scripts/ncs_cpnd_start \
	$(top_builddir)/scripts/ncs_dts_clean \
	$(top_builddir)/scripts/ncs_dts_start \
	$(top_builddir)/scripts/ncs_eds_clean \
	$(top_builddir)/scripts/ncs_eds_start \
	$(top_builddir)/scripts/ncs_gld_start \
	$(top_builddir)/scripts/ncs_glnd_start \
	$(top_builddir)/scripts/ncs_ifd_clean \
	$(top_builddir)/scripts/ncs_ifd_start \
	$(top_builddir)/scripts/ncs_ifnd_clean \
	$(top_builddir)/scripts/ncs_ifnd_start \
	$(top_builddir)/scripts/ncs_ifsv_ip_ins_script \
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
	$(top_builddir)/scripts/ncs_snmpsubagt_clean \
	$(top_builddir)/scripts/ncs_snmpsubagt_reload \
	$(top_builddir)/scripts/ncs_snmpsubagt_start \
	$(top_builddir)/scripts/ncs_srmnd_start \
	$(top_builddir)/scripts/ncs_vds_start \
	$(top_builddir)/scripts/nid_tipc \
	$(top_builddir)/scripts/nis_snmpd_clean \
	$(top_builddir)/scripts/nis_snmpd_start \
	$(top_builddir)/scripts/opensaf_lgs \
	$(top_builddir)/scripts/pdrbdctrl \
	$(top_builddir)/scripts/rde_script \
	$(top_builddir)/scripts/tipc_reset.sh

if ENABLE_HPI
nodist_bin_SCRIPTS += $(top_builddir)/scripts/ncs_hisv_start
endif

if WITH_OPENHPI
nodist_bin_SCRIPTS += \
	$(top_builddir)/scripts/openhpid_clean \
	$(top_builddir)/scripts/openhpid_start \
	$(top_builddir)/scripts/openhpisubagt_clean \
	$(top_builddir)/scripts/openhpisubagt_start
endif


EXTRA_DIST = \
	$(top_srcdir)/initd/opensafd.initd \
	$(top_srcdir)/tools/smidump \
	$(top_srcdir)/tests \
	$(top_srcdir)/mibs


RPMTOPDIR = `pwd`/rpms
RPMSOURCEDIR = $(RPMTOPDIR)/SOURCES
RPMSPECDIR = $(RPMTOPDIR)/SPECS
SRPMDIR = $(RPMTOPDIR)/SRPMS
RPMDIR = $(RPMTOPDIR)/RPMS

update-ldconfig:
	echo "$(pkglibdir)" > $(top_builddir)/config/$(PACKAGE_NAME)-$(target_cpu).conf
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/ld.so.conf.d
	$(INSTALL_DATA) $(top_builddir)/config/$(PACKAGE_NAME)-$(target_cpu).conf $(DESTDIR)$(sysconfdir)/ld.so.conf.d

update-pkgconfig:
	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(top_builddir)/pkgconfig/opensaf-*.pc $(DESTDIR)$(libdir)/pkgconfig

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

install-data-local: update-ldconfig update-pkgconfig
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/log/$(PACKAGE_NAME)
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/run/$(PACKAGE_NAME)
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/init.d
	$(mkinstalldirs) $(DESTDIR)$(datadir)/snmp/mibs
	$(INSTALL) -m 755 $(top_srcdir)/initd/opensafd.initd $(DESTDIR)$(sysconfdir)/init.d/opensafd
	@for file in $(mibs_files); do \
		$(INSTALL) -m 644 $(top_srcdir)/$$file $(DESTDIR)$(datadir)/snmp/$$file.txt; \
	done


install-data-hook:
	@for i in $$(grep -lr -e 'xxBINDIRxx' -e 'xxLIBDIRxx' -e 'xxPIDPATHxx'  $(DESTDIR)$(pkgsysconfdir)/*) ; do \
		sed -i 's|xxPIDPATHxx|$(localstatedir)/run/$(PACKAGE_NAME)|g' "$$i"; \
		sed -i 's|xxBINDIRxx|$(bindir)|g' "$$i"; \
		sed -i 's|xxLIBDIRxx|$(libdir)|g' "$$i"; \
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
	-rm -f $(DESTDIR)$(sysconfdir)/ld.so.conf.d/$(PACKAGE_NAME)-$(target_cpu).conf
	-rm -f $(DESTDIR)$(libdir)/pkgconfig/opensaf-*.pc
	@for file in $(mibs_files); do \
		rm -f $(DESTDIR)$(datadir)/snmp/$$file.txt; \
	done

clean-local:
	-rm -f $(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
	-rm -rf $(RPMTOPDIR)/{BUILD,RPMS,SOURCES,SPECS,SRPMS,tmp}

distclean-local:
	-rm -f $(top_builddir)/config/$(PACKAGE_NAME)-$(target_cpu).conf
if ENABLE_SMIDUMP
	@if [ -f $(top_builddir)/tools/smidump/bin/smidump ]; then \
		rm -rf $(top_builddir)/tools/smidump/bin; \
		make -C $(top_builddir)/tools/smidump/src distclean; \
	fi
endif

