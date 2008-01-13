noinst_LTLIBRARIES = libcla.la

libcla_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ \
			-I ../inc/ \
			-I ../inc/avsv_snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libcla_la_LDFLAGS = -static

libcla_la_SOURCES =	./cla_api.c \
			./cla_hdl.c \
			./cla_init.c \
			./cla_log.c \
			./cla_mds.c \
			./cla_op.c
