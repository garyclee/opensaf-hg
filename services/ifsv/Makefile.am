if CONTROLLER

SUBDIRS = clicmds ifd ifnd ifsv_ip_installer logstr
bin_PROGRAMS = ncs_ifd ncs_ifnd

else

SUBDIRS = ifnd ifsv_ip_installer 
bin_PROGRAMS = ncs_ifnd

endif

ncs_ifd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_IFD=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 \
		-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_LOG=1\
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ 

ncs_ifd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_ifd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/services/mbcsv/src/libmbca.la \
		./ifd/libifd.la \
		./common/libifsv_common.la


ncs_ifnd_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_IFND=1 -DNCS_VIP=1 -DVIP_HALS_SUPPORT=1 \
		-DNCS_IFSV_BOND_INTF=1 -DNCS_IFSV_LOG=1\
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ 

ncs_ifnd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c	

ncs_ifnd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		./ifnd/libifnd.la \
		./common/libifsv_common.la
