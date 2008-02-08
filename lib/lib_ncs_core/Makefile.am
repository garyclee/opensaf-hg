lib_LTLIBRARIES = libncs_core.la 

libncs_core_la_SOURCES  = 
libncs_core_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ 
libncs_core_la_LDFLAGS  =   -lpthread -lc -ldl -lm -lrt 
libncs_core_la_LIBADD   =   $(top_srcdir)/services/leap/libleap.la \
			$(top_srcdir)/services/common/libncs_common.la \
			$(top_srcdir)/services/mds/libmds.la \
			$(top_srcdir)/services/vds/vda/libvda.la \
			$(top_srcdir)/services/mab/oac/liboaa.la \
			$(top_srcdir)/services/dtsv/dta/libdta.la
