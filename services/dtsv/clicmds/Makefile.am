lib_LTLIBRARIES = libdts_clicef.la

libdts_clicef_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ -DNCS_CLI=1 \
			-DNCSCLI_DEBUG=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libdts_clicef_la_SOURCES =	./dtsvcli.c 
