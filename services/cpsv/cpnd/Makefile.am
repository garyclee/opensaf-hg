noinst_LTLIBRARIES = libcpnd.la

libcpnd_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPND=1 -DNCS_CPSV_LOG=1 \
			-I ../inc/ \
			-I ../inc/snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libcpnd_la_LDFLAGS = -static

libcpnd_la_SOURCES =	./cpnd_amf.c \
			./cpnd_db.c \
			./cpnd_evt.c \
			./cpnd_init.c \
			./cpnd_log.c \
			./cpnd_mds.c \
			./cpnd_proc.c \
			./cpnd_res.c \
			./cpnd_tmr.c \
			./miblibgen/
