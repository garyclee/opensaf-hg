lib_LTLIBRARIES = libsrmsv_logstr.la

libsrmsv_logstr_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_DTS=1 -DNCS_SRMSV_LOG=1 -DNCS_SRMA_LOG=1 \
			-DNCS_SRMND_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include

libsrmsv_logstr_la_SOURCES =	./srmsv_logstr.c \
				./srma_logstr.c \
				./srmnd_logstr.c

libsrmsv_logstr_la_LDFLAGS = -version-number 3
