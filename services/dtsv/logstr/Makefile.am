lib_LTLIBRARIES = libdts_logstr.la

libdts_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DDTS_LOG=1 -DDTA_LOG=1 -DNCS_DTS=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include

libdts_logstr_la_SOURCES =	./dts_logstr.c

libdts_logstr_la_LDFLAGS = -version-number 2
