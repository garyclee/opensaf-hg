noinst_LTLIBRARIES = libifd.la

libifd_la_CPPFLAGS = 	-I ../inc/ \
			-I ../inc/ifsv_snmp_inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ \
			-DNCS_IFD=1 -DNCS_MAB=1 -DNCS_IFSV_LOG=1 \
			-DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 \
			-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_IPXS=1 \
			@NCS_CORE_CPP_DEFINES@ 

libifd_la_LDFLAGS = -static

libifd_la_SOURCES = ./ifd_ir_mib.c \
		./ifsvifap.c \
		./ifd_mbcsv.c \
		./ipxs_cip.c \
		./ifsv_cifmap.c \
		./ifd_ir.c \
		./vip_reg_oac.c \
		./ifd_vipproc.c \
		./ifd_api.c \
		./ifd_evt.c \
		./ifd_mds.c \
		./ifd_mib.c \
		./ifd_red.c \
		./ifd_saf.c \
		./ifd_cgbl.c \
		./vip_mib.c \
		./ifd_ipxs.c \
		./ifsv_cxif.c \
		./ifdproc.c \
		./ifd_miblib_gen/ifmibobjects_mib.c \
		./ifd_miblib_gen/ncsifsvifipentry_mib.c \
		./ifd_miblib_gen/ncsifsvifxentry_mib.c \
		./ifd_miblib_gen/intf_mib.c \
		./ifd_miblib_gen/ncsifsvifmapentry_mib.c \
		./ifd_miblib_gen/ncsifsvipentry_mib.c \
		./ifsv_ir_miblib_gen/ncsifsvbindifentry_mib.c \
		./vip_miblib_gen/ncsvipentry_mib.c
