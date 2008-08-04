lib_LTLIBRARIES = libbam_logstr.la

libbam_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_BAM=1 -DNCS_DTS=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/dtsv/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/bam/inc/ \
		-I $(top_srcdir)/include/

libbam_logstr_la_SOURCES =	./bam_logstr.c

libbam_logstr_la_LDFLAGS = -version-number 2
