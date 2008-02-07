lib_LTLIBRARIES = libmbcsv_logstr.la

libmbcsv_logstr_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
			-DNCS_MBCSV=1 -DMBCSV_DEBUG=1 -DMBCSV_LOG=1 -DNCS_DTS=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/ \
			-I $(top_srcdir)/services/common/inc/

libmbcsv_logstr_la_SOURCES = ./mbcsv_logstr.c

libmbcsv_logstr_la_LDFLAGS = -version-number 2
