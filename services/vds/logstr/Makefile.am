lib_LTLIBRARIES = libvds_logstr.la

libvds_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_VDS=1 -DNCS_DTS=1 -DNCS_VDS_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libvds_logstr_la_SOURCES =	./vds_logstr.c

libvds_logstr_la_LDFLAGS = -version-number 3
