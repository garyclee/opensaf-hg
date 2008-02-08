lib_LTLIBRARIES = libsubagt_logstr.la

libsubagt_logstr_la_CPPFLAGS =	-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/ \
				-DNCS_EDSV_LOG=1 -DNCS_DTS=1 \
				@NCS_CORE_CPP_DEFINES@ 
								
libsubagt_logstr_la_SOURCES =	./subagt_logstr.c	

libsubagt_logstr_la_LDFLAGS = -version-number 2
