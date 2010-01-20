include $(top_srcdir)/samples/avsv/avsv.mk
include $(top_srcdir)/samples/avsv/pxy_pxd_support/pxy_pxd.mk
include $(top_srcdir)/samples/cpsv/cpsv.mk
include $(top_srcdir)/samples/dtsv/dtsv.mk
include $(top_srcdir)/samples/edsv/edsv.mk
include $(top_srcdir)/samples/glsv/glsv.mk
include $(top_srcdir)/samples/leap/leap.mk
include $(top_srcdir)/samples/mbcsv/mbcsv.mk
include $(top_srcdir)/samples/mds/mds.mk
include $(top_srcdir)/samples/mqsv/mqsv.mk

samples_sources = \
   $(avsv_sources) \
   $(pxy_pxd_sources) \
   $(cpsv_sources) \
   $(dtsv_sources) \
   $(edsv_sources) \
   $(glsv_sources) \
   $(leap_sources) \
   $(mbcsv_sources) \
   $(mds_sources) \
   $(mqsv_sources)
