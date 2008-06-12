noinst_LTLIBRARIES = libcpa.la

libcpa_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_CPA=1 -DNCS_CPSV_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libcpa_la_LDFLAGS = -static

libcpa_la_SOURCES =./cpa_api.c \
			./cpa_db.c \
			./cpa_init.c \
			./cpa_log.c \
			./cpa_mds.c \
			./cpa_proc.c \
			./cpa_tmr.c 
