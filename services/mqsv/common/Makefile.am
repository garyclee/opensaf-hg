lib_LTLIBRARIES = libmqsv_common.la

libmqsv_common_la_CPPFLAGS =    @NCS_CORE_CPP_DEFINES@ \
				-DNCS_MQA=1 -DNCS_MQSV_LOG=1 \
				-I ../inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/ 

libmqsv_common_la_SOURCES = ./mqsv_common.c \
			./mqsv_asapi_dec.c \
			./mqsv_asapi_enc.c \
			./mqsv_asapi.c \
			./mqsv_edu.c \
			./posix.c
