if CONTROLLER
lib_LTLIBRARIES = libedsv_subagt.la 

libedsv_subagt_la_CPPFLAGS = 	@NCS_CORE_CPP_DEFINES@ \
				-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
			   	-I $(top_srcdir)/services/mds/inc/ \
			       	-I $(top_srcdir)/include/ \
 				-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0

libedsv_subagt_la_SOURCES =	./edsv_subagt_init.c \
				./edsv_subagt_init.h \
				./saf_event_svc_mib_agt_stub.c \
				./saf_event_svc_mib_agt_stub.h

endif
