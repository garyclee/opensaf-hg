BUILT_SOURCES = snmp_stubs

snmp_stubs:
	./generate_snmp_stubs.sh

SUBDIRS = snmp_integ pssv_integ

bin_PROGRAMS = snmptm_demo

snmptm_demo_CPPFLAGS= -DNCS_SAF=1 \
			-I ./inc/ \
			@DEMO_INC_DIR@

snmptm_demo_SOURCES =	./snmptm_main.c \
			./src/snmptm_api.c \
			./src/snmptm_bgn.c \
			./src/snmptm_cb.c \
			./src/snmptm_mib.c \
			./src/snmptm_scalar_mib.c \
			./src/snmptm_tblcmd.c \
			./src/snmptm_tbleight.c \
			./src/snmptm_tblfive.c \
			./src/snmptm_tblfive_mib.c \
			./src/snmptm_tblfour.c \
			./src/snmptm_tblone.c \
			./src/snmptm_tblone_mib.c \
			./src/snmptm_tblseven.c \
			./src/snmptm_tblsix.c \
			./src/snmptm_tblsix_mib.c \
			./src/snmptm_tblten.c \
			./src/snmptm_tblthree.c \
			./src/snmptm_tblthree_mib.c \
			./snmptm_miblibgen/ncstestscalars_mib.c \
			./snmptm_miblibgen/ncstesttableeightentry_mib.c \
			./snmptm_miblibgen/ncstesttablefiveentry_mib.c \
			./snmptm_miblibgen/ncstesttablefourentry_mib.c \
			./snmptm_miblibgen/ncstesttablenineentry_mib.c \
			./snmptm_miblibgen/ncstesttableoneentry_mib.c \
			./snmptm_miblibgen/ncstesttablesevenentry_mib.c \
			./snmptm_miblibgen/ncstesttablesixentry_mib.c \
			./snmptm_miblibgen/ncstesttabletenentry_mib.c \
			./snmptm_miblibgen/ncstesttablethreeentry_mib.c

snmptm_demo_LDADD = @DEMO_LIBS@/libncs_core.so \
					@DEMO_LIBS@/libsaf_common.so \
					@DEMO_LIBS@/libSaEvt.so \
					@DEMO_LIBS@/libedsv_common.so
