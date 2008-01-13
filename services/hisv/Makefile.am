if CONTROLLER

SUBDIRS = hcd hpl logstr

if HW_MGMT
bin_PROGRAMS = ncs_hisv

ncs_hisv_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ -DNCS_HISV=1 -DNCS_HISV_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/

ncs_hisv_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c	

ncs_hisv_LDADD = -l@HPI_LIB@ -lopenhpimarshal -lglib-2.0 -lltdl \
		$(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/lib/lib_SaEvt/libSaEvt.la \
		$(top_srcdir)/services/edsv/common/libedsv_common.la \
		./hcd/libhcd.la \
		./hpl/libhpl.la
endif
endif
