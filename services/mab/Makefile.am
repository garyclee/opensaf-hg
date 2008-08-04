if CONTROLLER

SUBDIRS = mas psr psrcli logstr

bin_PROGRAMS = ncs_cli_maa ncs_mas ncs_psr ncs_snmp_subagt

ncs_mas_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAS=1 -DNCS_MAB=1 -DMAB_LOG=1 -DNCS_AVA=1 \
		-DNCS_AVSV_LOG=1 -DNCS_MAS_RED=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/avsv/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/include/

ncs_mas_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_mas_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
                $(top_srcdir)/services/mbcsv/src/libmbca.la \
                $(top_srcdir)/services/common/libsaf_common.la \
		./mas/libmas.la

ncs_psr_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAC=1 -DNCS_PSR=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 -DNCS_MBCSV=1 -DMBCSV_DEBUG=1 -DMBCSV_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/avsv/inc/ \
		-I $(top_srcdir)/services/mbcsv/inc/ \
		-I $(top_srcdir)/services/bam/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

ncs_psr_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c 

ncs_psr_LDFLAGS = -Wl,--export-dynamic 

ncs_psr_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
                $(top_srcdir)/services/mbcsv/src/libmbca.la \
		./mac/libmaa.la \
		./psr/libpsr.la

ncs_cli_maa_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAC=1 -DNCS_CLI_MAA  -DNCS_CLI=1 -DNCSCLI_DEBUG=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 -DMAB_LOG=1 -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/avsv/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

ncs_cli_maa_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c 

ncs_cli_maa_LDFLAGS = -Wl,--export-dynamic

ncs_cli_maa_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/services/cli/libcli.la \
		./mac/libmaa.la

ncs_snmp_subagt_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAC=1 -DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/avsv/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

ncs_snmp_subagt_SOURCES =  $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_snmp_subagt_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/lib/lib_SaEvt/libSaEvt.la \
                $(top_srcdir)/services/mab/mac/libmaa.la

endif
