noinst_LTLIBRARIES = libmqa.la


libmqa_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
				-DNCS_MQA=1 -DNCS_DTA=1 -DNCS_MQSV_LOG=1 \
				-I ../inc/ \
              	-I $(top_srcdir)/services/leap/inc/ \
               	-I $(top_srcdir)/services/mds/inc/ \
               	-I $(top_srcdir)/services/mab/inc/ \
               	-I $(top_srcdir)/services/common/inc/ \
               	-I $(top_srcdir)/include/ 

libmqa_la_LDFLAGS = -static

libmqa_la_SOURCES = 	./mqa_api.c \
				./mqa_log.c \
				./mqa_mds.c \
				./mqa_clbk.c \
				./mqa_queue.c \
				./mqa_init.c
