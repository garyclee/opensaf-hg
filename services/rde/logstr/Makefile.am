lib_LTLIBRARIES = librde_logstr.la

librde_logstr_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
				-DNCS_DTS=1 \
				-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/dtsv/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/

librde_logstr_la_SOURCES = ./rde_log_str.c

librde_logstr_la_LDFLAGS = -version-number 1
