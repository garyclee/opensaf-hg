bin_PROGRAMS = ncs_nid

ncs_nid_CPPFLAGS= -I ../inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/common/inc \
		-I $(top_srcdir)/services/rde/common/inc \
		@NCS_CORE_CPP_DEFINES@ -DNID_LINUX_PC -DNCS_SAF=1
		

ncs_nid_SOURCES = ./nid_ipmc_cmd.c \
		  ./nodeinit.c 
 

ncs_nid_LDADD = $(top_srcdir)/lib/lib_ncs_core/libncs_core.la \
		$(top_srcdir)/services/rde/rda/librda.la

