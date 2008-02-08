if HW_MGMT

lib_LTLIBRARIES = libhisv_logstr.la

libhisv_logstr_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
				-DNCS_HISV_LOG=1 -DNCS_DTS=1 -DDTS_LOG=1 \
				-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
				-I $(top_srcdir)/services/dtsv/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/

libhisv_logstr_la_SOURCES = ./hcd_logstr.c

libhisv_logstr_la_LDFLAGS = -version-number 2

endif

