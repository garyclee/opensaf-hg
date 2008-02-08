bin_PROGRAMS = ncs_rde

ncs_rde_CPPFLAGS= -I ../inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc \
		-I $(top_srcdir)/services/rde/common/inc \
		@NCS_CORE_CPP_DEFINES@
		

ncs_rde_SOURCES = ./rde_amf.c \
		./rde_init.c \
		./rde_log.c \
		./rde_main.c \
		./rde_rda.c \
		./rde_rde.c
 

ncs_rde_LDADD = $(top_srcdir)/lib/lib_SaAmf/libSaAmf.la 

