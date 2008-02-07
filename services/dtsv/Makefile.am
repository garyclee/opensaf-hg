if CONTROLLER

SUBDIRS = clicmds dts logstr pssv_integ 

bin_PROGRAMS = ncs_dts

ncs_dts_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_DTS=1 -DDTA_FLOW=1 -DNCS_DTA=1 \
		-I ./inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ 

ncs_dts_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_dts_LDFLAGS = -Wl,--export-dynamic

ncs_dts_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		$(top_srcdir)/services/mbcsv/src/libmbca.la \
		./dts/libdts.la

endif
