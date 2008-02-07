lib_LTLIBRARIES = libavm_subagt.la

libavm_subagt_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DNCS_EDA=1 -DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libavm_subagt_la_SOURCES =	./avm_subagt_init.c \
				./avm_subagt_init.h \
				./ncs_avm_mib_agt_stub.c \
				./ncs_avm_mib_agt_stub.h

