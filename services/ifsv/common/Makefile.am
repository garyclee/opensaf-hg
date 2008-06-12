lib_LTLIBRARIES = libifsv_common.la

libifsv_common_la_CPPFLAGS =	-I ../inc/ \
				-I ../inc/ifsv_snmp_inc/ \
				-I $(top_srcdir)/services/leap/inc/ \
				-I $(top_srcdir)/services/mds/inc/ \
				-I $(top_srcdir)/services/mab/inc/ \
				-I $(top_srcdir)/services/common/inc/ \
				-I $(top_srcdir)/include/ \
				-DNCS_VIP=1 -DNCS_MAB=1 -DVIP_HALS_SUPPORT=1 \
				-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_LOG=1 \
				-DNCS_IFSV_IPXS=1 \
				@NCS_CORE_CPP_DEFINES@ 

libifsv_common_la_SOURCES =	./ifsv_vip_db.c \
				./ifsv_cif.c \
				./ifsv_drv.c \
				./ifsv_log.c \
				./ifsv_mds.c \
				./ifsv_tmr.c \
				./ifsv_db.c \
				./ifsvipxs.c \
				./ipxs_edu.c \
				./ifsv_miblib_gen/ifentry_mib.c
