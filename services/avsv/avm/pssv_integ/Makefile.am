lib_LTLIBRARIES = libavm_pssv.la

libavm_pssv_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_PSR=1 -DPSR_LOG=1 \
			-DNCS_PSS_RED=1 -DNCS_PSS_DEBUG=0 \
			-I ../inc/ \
			-I ../inc/avm_snmp_inc \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/hisv/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/platform/ \
			-I $(top_srcdir)/include/

libavm_pssv_la_SOURCES =./avm_pssv_init.c \
			./avm_pssv_init.h \
			./ncs_avm_mib_pssv_mib.h \
			./ncsavmentdeploytableentry_pssv_mib.c \
			./ncsavmentupgradetableentry_pssv_mib.c

