noinst_LTLIBRARIES = libhcd.la

libhcd_la_CPPFLAGS =  	@NCS_CORE_CPP_DEFINES@ -DNCS_HISV_LOG=1 \
			-I ../inc/ \
			-I $(top_srcdir)/services/leap/inc/ \
			-I $(top_srcdir)/services/mds/inc/ \
			-I $(top_srcdir)/services/common/inc/ \
                        -I $(top_srcdir)/services/mab/inc/ \
			-I $(top_srcdir)/include/

libhcd_la_LDFLAGS = -static

libhcd_la_SOURCES =	./ham/ham_init.c \
			./ham/ham.c \
			./ham/ham_evt.c \
			./ham/ham_mds.c \
			./ham/ham_tmr.c \
			./hsm/hsm.c \
			./hsm/hsm_init.c \
			./sim/sim.c \
			./sim/sim_init.c \
			./sim/sim_evt.c \
			./hcd_util.c \
			./hcd.c \
			./hcd_amf.c \
			./hcd_log.c \
			./hisv_demo.c
