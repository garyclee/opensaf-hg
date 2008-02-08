noinst_LTLIBRARIES = libcpd.la

libcpd_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPD=1 -DNCS_CPSV_LOG=1 \
		-I ../inc/ \
		-I ../inc/snmp_inc \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libcpd_la_LDFLAGS = -static

libcpd_la_SOURCES =	./cpd_amf.c \
			./cpd_ckptentry.c \
			./cpd_ckptscalar.c \
			./cpd_db.c \
			./cpd_evt.c \
			./cpd_init.c \
			./cpd_log.c \
			./cpd_mbcsv.c \
			./cpd_mds.c \
			./cpd_mib.c \
			./cpd_proc.c \
			./cpd_red.c \
			./cpd_reploc.c \
			./cpd_sbevt.c \
			./cpd_tmr.c \
			./miblib_gen/sackptcheckpointentry_mib.c \
			./miblib_gen/safckptscalarobject_mib.c \
			./miblib_gen/sackptnodereplicalocentry_mib.c
