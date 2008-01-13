noinst_LTLIBRARIES = libmds.la

libmds_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		      @MDS_WORD_ARCH_FLAGS@ \
			-I ./inc/ \
			-I $(top_srcdir)/include/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/mab/inc/

libmds_la_LDFLAGS = -static

libmds_la_SOURCES =	./src/mds_dt_tipc.c \
			./src/mds_c_db.c \
			./src/mds_main.c \
			./src/mds_papi.c \
			./src/mds_c_sndrcv.c \
			./src/mds_dt.c \
			./src/mds_c_api.c \
			./src/mds_log.c
