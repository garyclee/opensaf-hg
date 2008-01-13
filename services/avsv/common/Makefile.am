lib_LTLIBRARIES = libavsv_common.la

libavsv_common_la_CPPFLAGS =  -I ../inc/ \
			-I ../inc/avsv_snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ \
			@NCS_CORE_CPP_DEFINES@ -DNCS_EDU=1

libavsv_common_la_SOURCES =	./avsv_d2nedu.c \
				./avsv_d2nmsg.c \
				./avsv_n2avamsg.c \
				./avsv_n2clamsg.c \
				./avsv_n2avaedu.c \
				./avsv_n2claedu.c \
				./avsv_util.c \
				./avsv_eduutil.c \
				../../common/src/saf_edu.c
