lib_LTLIBRARIES = libifsv_logstr.la

libifsv_logstr_la_CPPFLAGS = 	-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ \
		-DNCS_EDSV_LOG=1 -DNCS_DTS=1 -DDTS_LOG=1 \
		-DDTS_DEBUG=1 -DMAB_LOG=1 -DPSR_LOG=1 \
		@NCS_CORE_CPP_DEFINES@ 

libifsv_logstr_la_SOURCES = ./ifsv_logstr.c 

libifsv_logstr_la_LDFLAGS = -version-number 2
