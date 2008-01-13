noinst_LTLIBRARIES = libgla.la

libgla_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCS_GLA=1 -DNCS_GLSV_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/ \
	                -I $(top_srcdir)/services/common/inc/

libgla_la_LDFLAGS = -static

libgla_la_SOURCES =	./gla_api.c \
			./gla_clbk.c \
			./gla_init.c \
			./gla_log.c \
			./gla_mds.c \
			./gla_queue.c
