
OPENSAF_ROOT = $(PWD)/../../..
SAF_LIBS = $(OPENSAF_ROOT)/targets/i686-suse-linux/lib
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

CC = gcc -Wall -g $(INCDIR)

default: logtest

logtest: $(SRC_FILES)
	$(CC) $(SRC_FILES) $(LDFLAGS) -L$(LIBDIR) -o $@
	cp logtest $(OPENSAF_ROOT)/tools/cluster_sim_uml

clean:
	rm -f logtest *.o

