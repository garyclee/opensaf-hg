if CONTROLLER

SUBDIRS = mqd mqnd logstr
bin_PROGRAMS = ncs_mqd ncs_mqnd

else
SUBDIRS = mqnd
bin_PROGRAMS = ncs_mqnd

endif

ncs_mqd_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_MQD=1 \
                -I ./inc/ \
                -I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/mds/inc/ \
                -I $(top_srcdir)/services/vds/inc/ \
                -I $(top_srcdir)/services/common/inc/ \
                -I $(top_srcdir)/services/mab/inc/ \
                -I $(top_srcdir)/include/

ncs_mqd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_mqd_LDADD =  $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
                 $(top_srcdir)/lib/lib_SaClm/libSaClm.la \
                 $(top_srcdir)/services/common/libsaf_common.la \
                 $(top_srcdir)/services/mbcsv/src/libmbca.la \
                 ./mqd/libmqd.la \
                 ./common/libmqsv_common.la

ncs_mqnd_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ -DNCS_MQND=1 \
                -I ./inc/ \
                -I $(top_srcdir)/services/leap/inc/ \
                -I $(top_srcdir)/services/mds/inc/ \
                -I $(top_srcdir)/services/vds/inc/ \
                -I $(top_srcdir)/services/common/inc/ \
                -I $(top_srcdir)/services/mab/inc/ \
                -I $(top_srcdir)/include/

ncs_mqnd_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_mqnd_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la \
                 $(top_srcdir)/lib/lib_SaClm/libSaClm.la \
                 ./mqnd/libmqnd.la \
                 ./common/libmqsv_common.la
