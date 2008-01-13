if CONTROLLER

SUBDIRS = logstr

noinst_LTLIBRARIES = libbam.la

libbam_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_BAM=1 -DNCS_MAB=1 -DNCS_DTA=1 -DNCS_BAM_LOG=1 \
		-I ./inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libbam_la_LDFLAGS =	-static

libbam_la_SOURCES =	./src/amfConfig.cpp \
		./src/bam_avd.c \
		./src/bam_avm.c \
		./src/bamError.cpp \
		./src/bam_hw_deploy.cpp \
		./src/bam_hw_util.c \
		./src/bam_hw_vld.cpp \
		./src/bam_init.cpp \
		./src/bam_log.c \
		./src/bam_mds.c \
		./src/bam_mibs.cpp \
		./src/bamParser.cpp \
		./src/bam_psr.c \
		./src/bam_rda.c \
		./src/bam_sgs.cpp \
		./src/psr_bam.c

endif
