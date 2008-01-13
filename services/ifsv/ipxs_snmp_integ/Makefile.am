lib_LTLIBRARIES = libipxs_subagt.la

libipxs_subagt_la_CPPFLAGS = 	-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/include/ \
				-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0 \
				@NCS_CORE_CPP_DEFINES@ 

libipxs_subagt_la_SOURCES = 	./ipxs_subagt_init.c \
				./ipxs_subagt_init.h \
				./ncs_ifsv_ifip_mib_agt_stub.c \
				./ncs_ifsv_ifip_mib_agt_stub.h
