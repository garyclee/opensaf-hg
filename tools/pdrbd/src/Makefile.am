noinst_LTLIBRARIES = libpdrbd.la

libpdrbd_la_CPPFLAGS = 	-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ \
			-DNCS_PDRBD=1 -DPDRBD_LOG=1 \
			@NCS_CORE_CPP_DEFINES@ 

libpdrbd_la_LDFLAGS = -static

libpdrbd_la_SOURCES = ./pdrbd_amf.c \
                      ./pdrbd_log.c  \
                      ./pdrbd_mds.c  \
                      ./pdrbd_proc.c  \
                      ./pdrbd_proxied_amf.c \
                      ./pdrbd_util.c 
