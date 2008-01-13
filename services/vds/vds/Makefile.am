noinst_LTLIBRARIES = libvds.la

libvds_la_CPPFLAGS= @NCS_CORE_CPP_DEFINES@ \
		-DNCS_VDS=1 -DNCS_VDS_LOG=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/

libvds_la_LDFLAGS = -static

libvds_la_SOURCES =	./vds_main.c \
			./vds_amf.c \
			./vds_evt.c \
			./vds_log.c \
			./vds_mds.c \
			./vds_db.c \
			./vds_ckpt.c
