noinst_LTLIBRARIES = libsrmnd.la

libsrmnd_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_SRMND=1 -DNCS_SRMND_LOG=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libsrmnd_la_LDFLAGS = -static

libsrmnd_la_SOURCES =	./srmnd_amf.c \
			./srmnd_db.c \
			./srmnd_evt.c \
			./srmnd_init.c \
			./srmnd_log.c \
			./srmnd_mds.c \
			./srmnd_mon.c \
			./srmnd_msg.c \
			./srmnd_proc.c \
			./srmnd_stat.c \
			./srmnd_tmr.c \
			../common/srmsv_edumsg.c \
			../srmst/srmst_stat.c \
			../srmst/srmst_util.c
