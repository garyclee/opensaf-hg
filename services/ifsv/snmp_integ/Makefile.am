if CONTROLLER

lib_LTLIBRARIES = libifsv_subagt.la

libifsv_subagt_la_CPPFLAGS = 	-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/include/ \
				-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0 \
				@NCS_CORE_CPP_DEFINES@ 

libifsv_subagt_la_SOURCES = 	./if_mib_agt_stub.c \
				./if_mib_agt_stub.h \
				./ifsv_subagt_init.c \
				./ifsv_subagt_init.h

endif
