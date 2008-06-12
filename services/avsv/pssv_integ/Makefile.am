lib_LTLIBRARIES = libavsv_pssv.la

libavsv_pssv_la_CPPFLAGS = @NCS_CORE_CPP_DEFINES@ -DNCS_PSR=1 -DPSR_LOG=1 \
			-DNCS_PSS_RED=1 -DNCS_PSS_DEBUG=0 -DNCS_MBCSV=1 -DMBCSV_LOG=1\
                        -I ../inc/ \
                        -I ../inc/avsv_snmp_inc \
                        -I $(top_srcdir)/services/leap/inc/ \
                        -I $(top_srcdir)/services/mds/inc/ \
                        -I $(top_srcdir)/services/mab/inc/ \
                        -I $(top_srcdir)/include/ \
                        -I $(top_srcdir)/services/common/inc/

libavsv_pssv_la_SOURCES =./avsv_pssv_init.c \
			./avsv_pssv_init.h \
			./avsvscalars_pssv_mib.c \
			./ncs_avsv_mib_pssv_mib.h \
			./ncsndtableentry_pssv_mib.c \
			./ncssgtableentry_pssv_mib.c \
			./ncssitableentry_pssv_mib.c \
			./ncssutableentry_pssv_mib.c \
			./saamfcompcstypesupportedtableentry_pssv_mib.c \
			./saamfcomptableentry_pssv_mib.c \
			./saamfcsinamevaluetableentry_pssv_mib.c \
			./saamfcsitableentry_pssv_mib.c \
			./saamfcstypeparamentry_pssv_mib.c \
			./saamfhealthchecktableentry_pssv_mib.c \
			./saamfnodetableentry_pssv_mib.c \
			./saamfscalars_pssv_mib.c \
			./saamfsgtableentry_pssv_mib.c \
			./saamfsitableentry_pssv_mib.c \
			./saamfsuspersirankentry_pssv_mib.c \
			./saamfsutableentry_pssv_mib.c \
			./saclmnodetableentry_pssv_mib.c \
			./saf_amf_mib_pssv_mib.h \
			./saf_cluster_mib_pssv_mib.h \
			./safclmscalarobject_pssv_mib.c
