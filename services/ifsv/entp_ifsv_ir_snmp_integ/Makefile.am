if CONTROLLER
lib_LTLIBRARIES = libifsv_bind_subagt.la

libifsv_bind_subagt_la_CPPFLAGS = 	-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/include/ \
				-DBBS_SNMPSUBAGT=1 -DNCS_SNMPSUBAGT_LOG=0 \
				@NCS_CORE_CPP_DEFINES@ 

libifsv_bind_subagt_la_SOURCES =./ifsv_ir_subagt_init.c \
				./ifsv_ir_subagt_init.h \
				./ncs_ifsv_bind_mib_agt_stub.c \
				./ncs_ifsv_bind_mib_agt_stub.h
endif
