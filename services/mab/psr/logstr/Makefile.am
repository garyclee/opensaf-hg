lib_LTLIBRARIES = libpssv_logstr.la

libpssv_logstr_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
			-DNCS_MAC=1 -DMAB_LOG=1 -DNCS_DTS=1 \
			-I ../../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/include/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/mab/inc/

libpssv_logstr_la_SOURCES = ./psr_logstr.c

libpssv_logstr_la_LDFLAGS = -version-number 3
