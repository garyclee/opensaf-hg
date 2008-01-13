if CONTROLLER
lib_LTLIBRARIES = libavsv_subagt.la

libavsv_subagt_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCS_AVA=1 \
			-DNCS_AVSV_LOG=1 -DNCS_EDA=1 -DNCS_SNMPSUBAGT=1 \
			-DNCS_SNMPSUBAGT_LOG=1 \
                        -I ../inc/ \
                        -I $(top_srcdir)/services/leap/inc/ \
                        -I $(top_srcdir)/services/mds/inc/ \
                        -I $(top_srcdir)/include/

libavsv_subagt_la_SOURCES =	./avsv_subagt_init.c \
				./avsv_subagt_init.h \
				./ncs_avsv_mib_agt_stub.c \
				./ncs_avsv_mib_agt_stub.h \
				./saf_amf_mib_agt_stub.c \
				./saf_amf_mib_agt_stub.h \
				./saf_cluster_mib_agt_stub.c \
				./saf_cluster_mib_agt_stub.h 
endif
