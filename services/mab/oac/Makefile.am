noinst_LTLIBRARIES = liboaa.la

liboaa_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DMAB_LOG=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/bam/inc/

liboaa_la_LDFLAGS = -static

liboaa_la_SOURCES = ../common/mab_log.c \
		../common/mab_mds.c \
		../src/mab_def.c \
		../src/mab_tgt.c \
		./oac_api.c \
		./oac_lib.c \
		./oac_mds.c \
		./oac_pvt.c
