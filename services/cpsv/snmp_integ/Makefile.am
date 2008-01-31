if CONTROLLER
lib_LTLIBRARIES = libcpsv_subagt.la 

libcpsv_subagt_la_CPPFLAGS = 	@NCS_CORE_CPP_DEFINES@ \
				-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
			   	-I $(top_srcdir)/services/mds/inc/ \
	    		       	-I $(top_srcdir)/include/ \
 				-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0

libcpsv_subagt_la_SOURCES =	./cpsv_subagt_init.c \
				./cpsv_subagt_init.h \
				./saf_chk_svc_mib_agt_stub.c \
				./saf_chk_svc_mib_agt_stub.h
endif
