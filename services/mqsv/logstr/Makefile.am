lib_LTLIBRARIES = libmqsv_logstr.la

libmqsv_logstr_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
				-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/ \
				-DNCS_EDSV_LOG=1 -DNCS_DTS=1 -DDTS_LOG=1 \
				-DDTS_DEBUG=1 -DMAB_LOG=1 -DPSR_LOG=1

libmqsv_logstr_la_SOURCES =	./mqa_logstr.c \
				./mqnd_logstr.c \
				./mqd_logstr.c 

libmqsv_logstr_la_LDFLAGS = -version-number 4
