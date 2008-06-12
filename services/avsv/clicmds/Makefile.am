lib_LTLIBRARIES = libavsv_clicef.la

libavsv_clicef_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CLI=1 -DNCSCLI_FILE=1 -DNCSCLI_LOG=1 \
			-I ../inc/ \
			-I ../inc/avsv_snmp_inc \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libavsv_clicef_la_SOURCES =	./avsvcli.c 
