lib_LTLIBRARIES = libpsr_clicef.la

libpsr_clicef_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_MAB=1 -DNCS_CLI=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/include/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/bam/inc/

libpsr_clicef_la_SOURCES = ./psrcli.c
