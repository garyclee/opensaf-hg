if CONTROLLER
lib_LTLIBRARIES = libglsv_subagt.la

libglsv_subagt_la_CPPFLAGS = 	@NCS_CORE_CPP_DEFINES@ \
				-DNCS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=1 \
				-DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DNCS_EDA=1 \
                        	-I ../inc/ \
                        	-I $(top_srcdir)/services/leap/inc/ \
                        	-I $(top_srcdir)/services/mds/inc/ \
                        	-I $(top_srcdir)/include/

libglsv_subagt_la_SOURCES =	./glsv_subagt_init.c \
				./glsv_subagt_init.h \
				./saf_lck_svc_mib_agt_stub.c \
				./saf_lck_svc_mib_agt_stub.h
endif
