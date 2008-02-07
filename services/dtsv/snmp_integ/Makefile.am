if CONTROLLER
lib_LTLIBRARIES = libdtsv_subagt.la

libdtsv_subagt_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 -DNCS_AVA=1 \
			-DNCS_AVSV_LOG=1 -DNCS_EDA=1 \
                        -I ../inc/ \
                        -I $(top_srcdir)/services/leap/inc/ \
                        -I $(top_srcdir)/services/mds/inc/ \
                        -I $(top_srcdir)/services/common/inc/ \
                        -I $(top_srcdir)/services/mab/inc/ \
                        -I $(top_srcdir)/include/

libdtsv_subagt_la_SOURCES =	./dts_mapi.h \
				./dtsv_subagt_init.c \
				./dtsv_subagt_init.h \
				./ncs_dtsv_mib_agt_stub.c \
				./ncs_dtsv_mib_agt_stub.h
endif
