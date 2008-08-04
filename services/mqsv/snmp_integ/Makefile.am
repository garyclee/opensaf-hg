if CONTROLLER
lib_LTLIBRARIES = libmqsv_subagt.la

libmqsv_subagt_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 \
			-DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DNCS_EDA=1 \
                        -I ../inc/ \
                        -I $(top_srcdir)/services/leap/inc/ \
                        -I $(top_srcdir)/services/mds/inc/ \
                        -I $(top_srcdir)/services/common/inc/ \
                        -I $(top_srcdir)/services/mab/inc/ \
                        -I $(top_srcdir)/include/

libmqsv_subagt_la_SOURCES =	./mqsv_subagt_init.c \
				./mqsv_subagt_init.h \
				./saf_msg_svc_mib_agt_stub.c \
				./saf_msg_svc_mib_agt_stub.h
endif
