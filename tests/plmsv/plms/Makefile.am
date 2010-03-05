include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = plmtest

plmtest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/tests/plmsv/common \
	-I$(top_srcdir)/tests/unit_test_fw/inc

plmtest_SOURCES = \
	$(top_srcdir)/tests/unit_test_fw/src/utest.c \
	$(top_srcdir)/tests/unit_test_fw/src/util.c \
	$(top_srcdir)/tests/plmsv/common/plmtest.c \
	test_saPlmInitialize.c  \
	test_saPlmSelectionObjectGet.c \
	test_saPlmDispatch.c  \
	test_saPlmFinalize.c \
	test_saPlmEntityGroupCreate.c \
	test_saPlmEntityGroupAdd.c \
	test_saPlmEntityGroupDelete.c \
	test_saPlmEntityGroupRemove.c \
	test_saPlmReadinessTrack.c

plmtest_LDADD = \
	/usr/local/lib/libSaPlm.so \
	/usr/local/lib/opensaf/libopensaf_core.la \
	/usr/local/lib/opensaf/libplms_common.so  




