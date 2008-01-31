lib_LTLIBRARIES = librda.la

INCLUDES = -I ./inc/ \
	-I $(top_srcdir)/include/ \
	-I $(top_srcdir)/services/leap/inc/ \
	-I $(top_srcdir)/services/common/inc \
	-I $(top_srcdir)/services/rde/common/inc \
	@NCS_CORE_CPP_DEFINES@

librda_la_SOURCES = ./src/rda_main.c \
		$(top_srcdir)/services/common/src/rda_papi.c

