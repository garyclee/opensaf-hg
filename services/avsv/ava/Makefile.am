noinst_LTLIBRARIES = libava.la

libava_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCS_AVA=1 -DNCS_AVSV_LOG=1 \
		-I ../inc/ \
		-I ../inc/avsv_snmp_inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libava_la_LDFLAGS =	-static

libava_la_SOURCES =	./ava_api.c \
		./ava_hdl.c \
		./ava_init.c \
		./ava_log.c \
		./ava_mds.c \
		./ava_op.c
