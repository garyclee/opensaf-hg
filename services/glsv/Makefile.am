if CONTROLLER

SUBDIRS = gld glnd logstr
bin_PROGRAMS = ncs_gld ncs_glnd

else

SUBDIRS = glnd
bin_PROGRAMS = ncs_glnd
endif


ncs_gld_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_GLD=1 \
                -I ./inc/ \
                -I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/mds/inc/ \
                -I $(top_srcdir)/services/mab/inc/ \
                -I $(top_srcdir)/services/vds/inc/ \
                -I $(top_srcdir)/services/common/inc/ \
                -I $(top_srcdir)/include/

ncs_gld_SOURCES =  $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_gld_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
		 $(top_srcdir)/lib/lib_SaClm/libSaClm.la \
                 $(top_srcdir)/services/mbcsv/src/libmbca.la \
		./gld/libgld.la \
		./common/libglsv_common.la


ncs_glnd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_GLND=1 \
                -I ./inc/ \
                -I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/mds/inc/ \
                -I $(top_srcdir)/services/mab/inc/ \
                -I $(top_srcdir)/services/vds/inc/ \
                -I $(top_srcdir)/services/common/inc/ \
                -I $(top_srcdir)/include/

ncs_glnd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_glnd_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
                 $(top_srcdir)/lib/lib_SaCkpt/libSaCkpt.la \
		./glnd/libglnd.la \
		./common/libglsv_common.la
