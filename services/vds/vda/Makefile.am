noinst_LTLIBRARIES = libvda.la

libvda_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/services/common/inc/ \
		-I $(top_srcdir)/include/ 

libvda_la_SOURCES = ./ncs_ada.c \
					./ncs_mda.c \
					./ncs_vda.c

libvda_la_LDFLAGS = -static
