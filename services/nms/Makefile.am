if CONTROLLER
SUBDIRS = ncs-snmp-subagt

bin_PROGRAMS = ncs_snmp_subagt

ncs_snmp_subagt_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 -DNCS_MAC=1 \
                            -I ./inc/ \
                            -I $(top_srcdir)/services/leap/inc/ \
                            -I $(top_srcdir)/services/mds/inc/ \
                            -I $(top_srcdir)/services/mab/inc/ \
                            -I $(top_srcdir)/services/vds/inc/ \
                            -I $(top_srcdir)/include/ \
                            -I $(top_srcdir)/services/common/inc/ 


ncs_snmp_subagt_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_snmp_subagt_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
	$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
	$(top_srcdir)/lib/lib_SaEvt/libSaEvt.la \
	$(top_srcdir)/services/mab/mac/libmaa.la 
endif
