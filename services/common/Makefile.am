lib_LTLIBRARIES = libsaf_common.la 

noinst_LTLIBRARIES = libncs_common.la 

libncs_common_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ -DNCS_DTA=1 \
				-I ./inc/ \
				-I $(top_srcdir)/include/ \
				-I $(top_srcdir)/services/leap/inc \
				-I $(top_srcdir)/services/mds/inc \
				-I $(top_srcdir)/services/dtsv/inc \
				-I $(top_srcdir)/services/common/inc \
				-I $(top_srcdir)/services/vds/inc \
				-I $(top_srcdir)/services/mab/inc \
				-I $(top_srcdir)/services/avsv/inc \
				-I $(top_srcdir)/services/bam/inc \
				-I $(top_srcdir)/services/srmsv/inc/ \
				-I $(top_srcdir)/services/avsv/avm/inc/

libncs_common_la_LDFLAGS  = -static

libncs_common_la_SOURCES = 	./src/ncs_main_pub.c \
							./src/ncs_sprr.c



libsaf_common_la_CPPFLAGS =	@NCS_CORE_CPP_DEFINES@ \
				-I ./inc/ \
				-I $(top_srcdir)/include/ \
				-I $(top_srcdir)/services/leap/inc \
				-I $(top_srcdir)/services/mab/inc \
				-I $(top_srcdir)/services/common/inc

libsaf_common_la_SOURCES = 	./src/avm_avd_mem.c \
							./src/saf_edu.c
