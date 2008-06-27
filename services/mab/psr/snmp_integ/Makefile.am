if CONTROLLER
lib_LTLIBRARIES = libpssv_subagt.la

libpssv_subagt_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		 	-DNCS_MAC=1 -DMAB_LOG=1 \
			-I ../inc/ \
		        -I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/include/ \
			-I $(top_srcdir)/services/common/inc/

libpssv_subagt_la_SOURCES =	./ncs_psr_mib_agt_stub.c \
				./ncs_psr_mib_agt_stub.h \
				./psr_subagt_init.c \
				./psr_subagt_init.h
endif
