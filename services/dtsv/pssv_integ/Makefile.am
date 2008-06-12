lib_LTLIBRARIES = libdts_pssv.la

libdts_pssv_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ \
			-DNCS_PSR=1 -DPSR_LOG=1 -DNCS_PSS_RED=1 \
			-DNCS_PSS_DEBUG=0 -DNCS_MBCSV=1 \
			-DMBCSV_DEBUG=1 -DMBCSV_LOG=1 \
                        -I ../inc/ \
                        -I $(top_srcdir)/services/leap/inc/ \
                        -I $(top_srcdir)/services/mds/inc/ \
                        -I $(top_srcdir)/services/common/inc/ \
                        -I $(top_srcdir)/services/mab/inc/ \
                        -I $(top_srcdir)/include/

libdts_pssv_la_SOURCES =./dts_pssv_init.c \
			./dts_pssv_init.h \
			./ncs_dtsv_mib_pssv_mib.h \
			./ncsdtsvnodelogpolicyentry_pssv_mib.c \
			./ncsdtsvscalars_pssv_mib.c \
			./ncsdtsvservicelogpolicyentry_pssv_mib.c
