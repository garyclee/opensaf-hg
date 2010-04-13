include $(top_srcdir)/samples/avsv/avsv.mk
include $(top_srcdir)/samples/avsv/pxy_pxd_support/pxy_pxd.mk
include $(top_srcdir)/samples/cpsv/cpsv.mk
include $(top_srcdir)/samples/mqsv/mqsv.mk
include $(top_srcdir)/samples/edsv/edsv.mk
include $(top_srcdir)/samples/glsv/glsv.mk
include $(top_srcdir)/samples/smfsv/smfsv.mk

samples_sources = \
   $(avsv_sources) \
   $(pxy_pxd_sources) \
   $(cpsv_sources) \
   $(edsv_sources) \
   $(glsv_sources) \
   $(mqsv_sources) \
   $(smfsv_sources)
