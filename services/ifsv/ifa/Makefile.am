lib_LTLIBRARIES = libifa.la

libifa_la_CPPFLAGS = 	-I ../inc/ \
			-I ../inc/ifsv_snmp_inc \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/ \
			-DNCS_IFA=1 -DNCS_IFSV_LOG=1 -DNCS_VIP=1 \
			-DNCS_IFSV_IPXS=1 -DVIP_HALS_SUPPORT=1 \
			-DNCS_IFSV_BOND_INTF=1 \
			@NCS_CORE_CPP_DEFINES@ 

libifa_la_SOURCES =	./ifa_proc.c \
			./ifa_api.c \
			./ifa_mds.c \
			./ifa_vipproc.c \
			./ipxs_aproc.c
