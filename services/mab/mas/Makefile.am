noinst_LTLIBRARIES = libmas.la

libmas_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAS=1 -DNCS_MAB=1 -DMAB_LOG=1 \
		-DNCS_AVA=1 -DNCS_AVSV_LOG=1 -DNCS_MAS_RED=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/common/inc/

libmas_la_LDFLAGS = -static

libmas_la_SOURCES =	./mas_amf.c \
			./mas_api.c \
			./mas_lib.c \
			./mas_mbcsv.c \
			./mas_mds.c \
			./mas_pvt.c \
			../mas_pss_common/app_amf.c 

libmas_la_LIBADD = $(top_srcdir)/services/rde/rda/librda.la
