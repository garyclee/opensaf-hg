noinst_LTLIBRARIES = libeda.la

libeda_la_CPPFLAGS = 	@NCS_CORE_CPP_DEFINES@ -DNCS_EDA=1 -DNCS_DTA=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libeda_la_LDFLAGS = -static

libeda_la_SOURCES = 	./eda_util.c \
			./eda_saf_api.c \
			./eda_hdl.c \
			./eda_log.c \
			./eda_mds.c \
			./eda_init.c
