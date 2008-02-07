noinst_LTLIBRARIES = libmqnd.la

libmqnd_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
			-DNCS_MQND=1 -DASAPi_DEBUG=1 -DNCS_MQSV_LOG=1 -DNCS_DTA=1 \
			-I ../inc/ \
			-I ../inc/snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libmqnd_la_LDFLAGS =	-static

libmqnd_la_SOURCES =	./mqnd_qentry.c \
			./mqnd_proc.c \
			./mqnd_util.c \
			./mqnd_db.c \
			./mqnd_mq.c \
			./mqnd_namedb.c \
			./mqnd_qprentry.c \
			./mqnd_amf.c \
			./mqnd_evt.c \
			./mqnd_log.c \
			./mqnd_mds.c \
			./mqnd_mib.c \
			./mqnd_shm.c \
			./mqnd_tmr.c \
			./mqnd_restart.c \
			./mqnd_init.c \
			./miblib_gen/samsgqueueentry_mib.c \
			./miblib_gen/samsgqueuepriorityentry_mib.c
