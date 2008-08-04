if CONTROLLER
#Setup Controller Targets

SUBDIRS = avd avm avnd logstr clicmds pssv_integ snmp_integ 
bin_PROGRAMS = ncs_scap
ncs_scap_CPPFLAGS=  @NCS_CORE_CPP_DEFINES@ \
            -DNCS_MAC=1 -DNCS_BAM=1 -DNCS_AVD=1 \
            -DNCS_AVND=1 -DNCS_AVM=1 -DNCS_SRMA=1 \
            -I ./inc/ \
            -I ./avm/inc/ \
            -I $(top_srcdir)/services/leap/inc/ \
            -I $(top_srcdir)/services/common/inc/ \
            -I $(top_srcdir)/services/vds/inc/ \
            -I $(top_srcdir)/services/mds/inc/ \
            -I $(top_srcdir)/services/mab/inc/ \
            -I $(top_srcdir)/services/bam/inc/ \
            -I $(top_srcdir)/services/srmsv/inc/ \
            -I $(top_srcdir)/include/

ncs_scap_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c

ncs_scap_LDFLAGS = -lxerces-c

ncs_scap_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaEvt/libSaEvt.la \
                 ../common/libsaf_common.la \
                 ./common/libavsv_common.la \
                 ../mab/mac/libmaa.la \
                 ../srmsv/srma/libsrma.la \
                 ../mbcsv/src/libmbca.la \
                 ../hisv/hpl/libhpl.la \
                 ./avd/libavd.la \
                 ./avnd/libavnd.la \
                 ./avm/libavm.la \
                 ../bam/libbam.la

else 
# Setup Payload targets

SUBDIRS = avnd
bin_PROGRAMS = ncs_pcap
ncs_pcap_CPPFLAGS= 	@NCS_CORE_CPP_DEFINES@ \
             -DNCS_AVND=1 -DNCS_SRMA=1 \
			-I ./inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/services/vds/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/srmsv/inc/ \
			-I $(top_srcdir)/include/

ncs_pcap_SOURCES = $(top_srcdir)/services/common/src/ncs_main_pvt.c
ncs_pcap_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
                 $(top_srcdir)/lib/lib_SaEvt/libSaEvt.la \
                 ../common/libsaf_common.la \
                 ./common/libavsv_common.la \
                 ../srmsv/srma/libsrma.la \
                 ./avnd/libavnd.la


endif
