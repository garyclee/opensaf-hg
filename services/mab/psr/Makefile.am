SUBDIRS = logstr

noinst_LTLIBRARIES = libpsr.la

libpsr_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_PSR=1 -DNCS_MAB=1 \
		-DMAB_LOG=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/bam/inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/common/inc/

libpsr_la_LDFLAGS =	-static -Wl,--export-dynamic

libpsr_la_SOURCES =	./psr_amf.c \
			./psr_api.c \
			./psr_ext.c \
			./psr_init.c \
			./psr_log.c \
			./psr_mds.c \
			./psr_mib.c \
			./psr_pvt.c \
			./pss_mbcsv.c \
			./psr_rfmt.c \
			../mas_pss_common/app_amf.c \
			./pss_miblib_gen/ncspssvprofiletableentry_mib.c \
			./pss_miblib_gen/ncspssvscalars_mib.c 

libpsr_la_LIBADD = $(top_srcdir)/services/rde/rda/librda.la
