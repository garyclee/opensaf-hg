lib_LTLIBRARIES = libmbca.la

libmbca_la_CPPFLAGS =  @NCS_CORE_CPP_DEFINES@ \
			-DNCS_MBCSV=1 -DMBCSV_DEBUG=1 -DMBCSV_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
			-I $(top_srcdir)/include/ 

libmbca_la_SOURCES =	./mbcsv_act.c \
						./mbcsv_api.c \
						./mbcsv_dbg.c \
						./mbcsv_dl_api.c \
						./mbcsv_fsm.c \
						./mbcsv_inv.c \
						./mbcsv_log.c \
						./mbcsv_mbx.c \
						./mbcsv_mds.c \
						./mbcsv_peer.c \
						./mbcsv_pr_evts.c \
						./mbcsv_pwe_anc.c \
						./mbcsv_queue.c \
						./mbcsv_tmr.c \
						./mbcsv_util.c 
