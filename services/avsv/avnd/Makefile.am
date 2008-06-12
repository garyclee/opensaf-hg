noinst_LTLIBRARIES = libavnd.la

libavnd_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_AVND=1 -DNCS_AVSV_LOG=1 \
		-I ../inc/ \
		-I ../inc/avsv_snmp_inc \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libavnd_la_LDFLAGS = -static

libavnd_la_SOURCES =	./avnd.c \
			./avnd_cam.c\
			./avnd_cbq.c \
			./avnd_chc.c \
			./avnd_clc.c \
			./avnd_clm.c \
			./avnd_clmdb.c \
			./avnd_comp.c \
			./avnd_compdb.c \
			./avnd_cpm.c \
			./avnd_di.c \
			./avnd_err.c \
			./avnd_evt.c \
			./avnd_evtlog.c \
			./avnd_hcdb.c \
			./avnd_hpi.c \
			./avnd_log.c \
			./avnd_mab.c \
			./avnd_mds.c \
			./avnd_pg.c \
			./avnd_pgdb.c\
			./avnd_proc.c \
			./avnd_sidb.c \
			./avnd_srm.c \
			./avnd_su.c \
			./avnd_sudb.c\
			./avnd_susm.c\
			./avnd_term.c\
			./avnd_tmr.c\
			./avnd_trap.c\
			./avnd_util.c\
			./avnd_verify.c\
			./avnd_miblib_gen/ncsscomptableentry_mib.c \
			./avnd_miblib_gen/ncssndtableentry_mib.c \
			./avnd_miblib_gen/ncsssutableentry_mib.c \
			./avnd_miblib_gen/saamfscompcsitableentry_mib.c

