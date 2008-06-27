noinst_LTLIBRARIES = libmqd.la


libmqd_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
			-DNCS_MQD=1 -DASAPi_DEBUG=1 -DNCS_MQSV_LOG=1 -DNCS_DTA=1 \
			-I ../inc/ \
			-I ../inc/snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/

libmqd_la_LDFLAGS = -static

libmqd_la_SOURCES = 	./mqd_sbevt.c \
			./mqd_mbcsv.c \
			./mqd_mbedu.c \
			./mqd_util.c \
			./mqd_asapi.c \
			./mqd_api.c \
			./mqd_clm.c \
			./mqd_evt.c \
			./mqd_log.c \
			./mqd_mds.c \
			./mqd_mib.c \
			./mqd_red.c \
			./mqd_saf.c \
			./mqd_tmr.c \
			./mqd_qgrpmemsentry.c \
			./mqd_grpentry.c \
			./mqd_scalarentry.c \
			./miblib_gen/safmsgscalarobject_mib.c \
			./miblib_gen/samsgqueuegroupmembersentry_mib.c \
			./miblib_gen/samsgqueuegroupentry_mib.c
