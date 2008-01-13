lib_LTLIBRARIES = libcli_logstr.la

libcli_logstr_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ -DNCSCLI_LOG=1 -DNCS_DTS=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libcli_logstr_la_SOURCES =	./cli_logstr.c

libcli_logstr_la_LDFLAGS = -version-number 1
