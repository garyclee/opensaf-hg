if CONTROLLER

SUBDIRS = vds logstr
bin_PROGRAMS = ncs_vds

ncs_vds_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_VDS=1 -DNCS_VDS_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/ 

ncs_vds_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_vds_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/lib/lib_SaCkpt/libSaCkpt.la \
		./vds/libvds.la
endif
