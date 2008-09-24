if CONTROLLER

SUBDIRS = eds logstr

bin_PROGRAMS = ncs_eds

ncs_eds_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_EDS=1 -DNCS_EDSV_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

ncs_eds_SOURCES =  $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_eds_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
                 $(top_srcdir)/lib/lib_SaClm/libSaClm.la \
                 $(top_srcdir)/services/mbcsv/src/libmbca.la \
		./common/libedsv_common.la \
		./eds/libeds.la

endif
