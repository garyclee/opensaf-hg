lib_LTLIBRARIES = libpdrbd_logstr.la

libpdrbd_logstr_la_CPPFLAGS = 	-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ \
		-DNCS_PDRBD=1 -DPDRBD_LOG=1 \
		@NCS_CORE_CPP_DEFINES@ 

libpdrbd_logstr_la_SOURCES = ./pdrbd_logstr.c 

libpdrbd_logstr_la_LDFLAGS = -version-number 1
