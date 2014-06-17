include $(top_srcdir)/Makefile.common

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = plmtest

plmtest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/tests/plmsv/common \
	-I$(top_srcdir)/tests/unit_test_fw/inc

plmtest_SOURCES = \
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
	$(top_builddir)/osaf/libs/saf/libSaPlm/libSaPlm.la \
	$(top_builddir)/osaf/libs/core/libopensaf_core.la \
	$(top_builddir)/tests/unit_test_fw/src/libutest.la \
	$(top_builddir)/tests/plmsv/common/libplmtest.la
