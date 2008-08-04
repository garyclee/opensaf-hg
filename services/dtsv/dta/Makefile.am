noinst_LTLIBRARIES = libdta.la

libdta_la_CPPFLAGS=	@NCS_CORE_CPP_DEFINES@ \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/ 

libdta_la_LDFLAGS =	-static

libdta_la_SOURCES =	./dta_dl_api.c \
			./dta_api.c \
			./dta_mds.c \
			./dta_tgt.c
