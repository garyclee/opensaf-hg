noinst_LTLIBRARIES = libifnd.la

libifnd_la_CPPFLAGS = 	-I ../inc/ \
			-I ../inc/ifsv_snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ \
			-DNCS_IFND=1 -DNCS_IFSV_LOG=1 \
			-DNCS_VIP=1 -DNCS_MAB=1 -DVIP_HALS_SUPPORT=1 \
			-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_IPXS=1 \
			@NCS_CORE_CPP_DEFINES@ 

libifnd_la_LDFLAGS = -static

libifnd_la_SOURCES = 	./ifnd_ipxs.c \
			./ifnd_api.c \
			./ifnd_evt.c \
			./ifnd_mds.c \
			./ifnd_mib.c \
			./ifnd_red.c \
			./ifnd_saf.c \
			./ifnddevt.c \
			./ifnddprc.c \
			./ifnd_vipproc.c \
			./ifndproc.c
