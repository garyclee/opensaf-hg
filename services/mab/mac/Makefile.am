lib_LTLIBRARIES = libmaa.la

libmaa_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_MAC=1 -DMAB_LOG=1 -DNCS_MAB=1 -DNCS_CLI_MAA=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/common/inc/

libmaa_la_SOURCES = ./mac_api.c \
		./mac_lib.c \
		./mac_mds.c \
		./mac_pvt.c
