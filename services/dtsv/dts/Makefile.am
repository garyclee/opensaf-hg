noinst_LTLIBRARIES = libdts.la

libdts_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
		-DNCS_DTS=1 -DDTS_LOG=1 -DNCS_DTA=1 -DDTS_FLOW=1 \
		-I ../inc/ \
		-I $(top_srcdir)/services/leap/inc/ \
		-I $(top_srcdir)/services/mds/inc/ \
		-I $(top_srcdir)/services/mab/inc/ \
		-I $(top_srcdir)/include/ \
		-I $(top_srcdir)/services/common/inc/
		

libdts_la_LDFLAGS = -static -Wl,--export-dynamic

libdts_la_SOURCES =	./dts_amf.c \
			./dts_api.c \
			./dts_cbop.c \
			./dts_chkop.c \
			./dts_ckpt_dec.c \
			./dts_ckpt_edu.c \
			./dts_ckpt_enc.c \
			./dts_ckpt_updt.c \
			./dts_def.c \
			./dts_dl_api.c \
			./dts_glpolc.c \
			./dts_log.c \
			./dts_mds.c \
			./dts_mib.c \
			./dts_nodepolc.c \
			./dts_pvt.c \
			./dts_seq.c \
			./dts_svcpolc.c \
			./dts_tgt.c \
			./sysf_log.c \
			./dts_miblib_gen/ncsdtsvnodelogpolicyentry_mib.c \
			./dts_miblib_gen/ncsdtsvservicelogpolicyentry_mib.c \
			./dts_miblib_gen/ncsdtsvscalars_mib.c 

libdts_la_LIBADD = $(top_srcdir)/services/rde/rda/librda.la
