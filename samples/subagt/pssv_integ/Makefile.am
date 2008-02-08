lib_LTLIBRARIES = libsnmptm_pssv.la

libsnmptm_pssv_la_CPPFLAGS= -DNCS_SAF=1 \
            -I ../inc/ \
	@DEMO_INC_DIR@

libsnmptm_pssv_la_SOURCES =	./mcg_ncs_test_mib_pssv_mib.h \
							./snmptm_pssv_init.h \
							./ncstestscalars_pssv_mib.c \
							./ncstesttableeightentry_pssv_mib.c \
							./ncstesttablefourentry_pssv_mib.c \
							./ncstesttablenineentry_pssv_mib.c \
							./ncstesttableoneentry_pssv_mib.c \
							./ncstesttablesevenentry_pssv_mib.c \
							./ncstesttablesixentry_pssv_mib.c  \
							./ncstesttabletenentry_pssv_mib.c \
							./ncstesttablethreeentry_pssv_mib.c \
							./snmptm_pssv_init.c

libsnmptm_pssv_la_LIBADD = @DEMO_LIBS@/libncs_core.so \
                    @DEMO_LIBS@/libsaf_common.so \
                    @DEMO_LIBS@/libSaEvt.so \
                    @DEMO_LIBS@/libedsv_common.so
