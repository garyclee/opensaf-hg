lib_LTLIBRARIES = libmab_logstr.la

libmab_logstr_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAC=1 -DMAB_LOG=1 -DNCS_DTS=1 \
		-I ../inc/ \
	    -I $(top_srcdir)/services/leap/inc/ \
	    -I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/common/inc/

libmab_logstr_la_SOURCES = ./mab_logstr.c

libmab_logstr_la_LDFLAGS = -version-number 3
