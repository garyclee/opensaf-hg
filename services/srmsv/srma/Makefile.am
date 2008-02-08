lib_LTLIBRARIES = libsrma.la

libsrma_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_SRMA=1 -DNCS_SRMA_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libsrma_la_SOURCES =	./srma_api.c \
			./srma_cbk.c \
			./srma_db.c \
			./srma_init.c \
			./srma_log.c \
			./srma_mds.c \
			./srma_msg.c \
			./srma_proc.c \
			./srma_tmr.c \
			../common/srmsv_edumsg.c \
			../srmst/srmst_stat.c \
			../srmst/srmst_util.c
