if CONTROLLER
SUBDIRS = src logstr

bin_PROGRAMS = ncs_pdrbd

ncs_pdrbd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_PDRBD=1 -DPDRBD_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/services/vds/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ 

ncs_pdrbd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_pdrbd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la  \
                ./src/libpdrbd.la
endif
