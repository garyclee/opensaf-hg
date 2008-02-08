if CONTROLLER
SUBDIRS= srmnd logstr
else
SUBDIRS= srmnd
endif

bin_PROGRAMS = ncs_srmnd
ncs_srmnd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_SRMND=1 \
                -I ./inc/ \
                -I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/mds/inc/ \
                -I $(top_srcdir)/services/common/inc/ \
                -I $(top_srcdir)/services/mab/inc/ \
                -I $(top_srcdir)/services/vds/inc/ \
                -I $(top_srcdir)/include/


ncs_srmnd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_srmnd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		./srmnd/libsrmnd.la
