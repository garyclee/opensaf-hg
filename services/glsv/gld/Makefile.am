noinst_LTLIBRARIES = libgld.la

libgld_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_GLSV_LOG=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/cpsv/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/ 

libgld_la_LDFLAGS = -static

libgld_la_SOURCES =	./gld_amf.c \
			./gld_api.c \
			./gld_evt.c \
			./gld_log.c \
			./gld_mbcsv.c \
			./gld_mds.c \
			./gld_mib.c \
			./gld_red.c \
			./gld_rsc.c \
			./gld_scalar.c \
			./gld_standby.c \
			./gld_tbl.c \
			./gld_tmr.c \
			./miblib_gen/saflckscalarobject_mib.c \
			./miblib_gen/salckresourceentry_mib.c
