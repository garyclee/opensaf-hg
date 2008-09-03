
OPENSAF_ROOT = $(PWD)/../../..
ARCH := $(shell arch)
ifeq ($(ARCH),i686)
	LIB = lib
endif
ifeq ($(ARCH),x86_64)
	LIB = lib64
endif

SAF_LIBS = $(OPENSAF_ROOT)/targets/$(shell ls $(OPENSAF_ROOT)/targets/)/$(LIB)
INCDIR +=       -I $(OPENSAF_ROOT)/include
LIBDIR +=	$(SAF_LIBS)
LDFLAGS +=	-lpthread -ldl -lncs_core -lSaLog

SRC_FILES =	tet_log_main.c \
		tet_saLogInitialize.c \
		tet_saLogSelectionObjectGet.c \
		tet_saLogDispatch.c \
		tet_saLogFinalize.c \
		tet_saLogStreamOpen_2.c \
		tet_saLogStreamOpenAsync_2.c \
		tet_saLogStreamOpenCallbackT.c \
		tet_saLogWriteLog.c \
		tet_saLogWriteLogAsync.c \
		tet_saLogWriteLogCallbackT.c \
		tet_saLogFilterSetCallbackT.c \
		tet_saLogStreamClose.c \
		tet_saLogLimitGet.c \

FMT_SRC_FILES = lgs_fmt_main.c \
		$(OPENSAF_ROOT)/services/logsv/lgs/lgs_fmt.c

FMT_INC_DIR = -I $(OPENSAF_ROOT)/services/logsv/inc -I $(OPENSAF_ROOT)/services/logsv/lgs

CC = gcc -Wall -g $(INCDIR)

default: formatter logtest

formatter: $(FMT_SRC_FILES)
	$(CC) $(FMT_INC_DIR) $(FMT_SRC_FILES) -o $@

logtest: $(SRC_FILES)
	$(CC) $(SRC_FILES) $(LDFLAGS) -L$(LIBDIR) -o $@
	cp logtest $(OPENSAF_ROOT)/tools/cluster_sim_uml

clean:
	rm -f logtest formatter *.o

