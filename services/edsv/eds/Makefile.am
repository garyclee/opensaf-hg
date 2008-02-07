noinst_LTLIBRARIES = libeds.la

libeds_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ -DNCS_EDS=1 -DNCS_EDSV_LOG=1 -DNCS_DTA=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
                -I $(top_srcdir)/include/ \
                -I $(top_srcdir)/services/common/inc/

libeds_la_LDFLAGS = -static 

libeds_la_SOURCES =		./eds_amf.c \
				./eds_api.c \
				./eds_cb.c \
				./eds_ckpt.c \
				./eds_debug.c \
				./eds_evt.c \
				./eds_ll.c \
				./eds_log.c \
				./eds_mds.c \
				./eds_mib.c \
				./eds_tmr.c \
				./eds_util.c \
				../edsv_miblibgen/saevtchannelentry_mib.c \
				../edsv_miblibgen/safevtscalarobject_mib.c 

libeds_la_LIBADD = $(top_srcdir)/services/rde/rda/librda.la

