bin_PROGRAMS = hisv_power_ctrl \
				hisv_reset_ctrl \
				hpl_entity_lookup

hisv_power_ctrl_CPPFLAGS= -DNCS_SAF=1 \
		@TEST_INC_DIR@

hisv_power_ctrl_SOURCES =	hisv_test.h \
				hisv_test_common.c \
				hisv_power_ctrl.c

hisv_power_ctrl_LDADD =	/usr/lib/libSaAmf.so \
				/usr/lib/libedsv_common.so \
				/usr/lib/libSaEvt.so \
				/usr/lib/libhpl.so

hisv_reset_ctrl_CPPFLAGS= -DNCS_SAF=1 \
		@TEST_INC_DIR@

hisv_reset_ctrl_SOURCES =	hisv_test.h \
				hisv_test_common.c \
				hisv_reset_ctrl.c

hisv_reset_ctrl_LDADD =	/usr/lib/libSaAmf.so \
				/usr/lib/libedsv_common.so \
				/usr/lib/libSaEvt.so \
				/usr/lib/libhpl.so

hpl_entity_lookup_CPPFLAGS= -DNCS_SAF=1 \
				-I /usr/include/openhpi \
				-I /usr/include/opensaf

hpl_entity_lookup_SOURCES =	hisv_test.h \
				hisv_test_common.c \
				hpl_entity_lookup.c

hpl_entity_lookup_LDADD =	/usr/lib/libSaAmf.so \
				/usr/lib/libedsv_common.so \
				/usr/lib/libSaEvt.so \
				/usr/lib/libhpl.so

#
# vim: tabstop=4
