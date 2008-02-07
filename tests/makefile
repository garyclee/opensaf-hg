# include file  some variable stuff.

include $(TET_OPENSAF_HOME)/make_tetinc

#Power PC stuff
CPP    = c++
CC     = $(COMPILER)
#LD     = $(LD)

LD_FLAGS= -o
SYS_APP_FLAG=0

.PHONEY : clean all


# common shared libraries to be put here 
SHARED_LIBRARIES=$(OPENSAF_SHARED_LIBRARIES) $(COMMON_SHARED_LIBRARIES) $(TET_ARCHIVES)


#ifneq (,$(findstring -DTET_ALL=1,$(HJCPP_DEFINES)))
#   SRCS=$(wildcard $(TET_OPENSAF_HOME)/cpsv/src/*.c)
#   SRCS+=$(wildcard $(TET_OPENSAF_HOME)/mqsv/src/*.c)
#   SRCS+=$(wildcard $(TET_OPENSAF_HOME)/glsv/src/*.c)
#   SRCS+=$(wildcard $(TET_OPENSAF_HOME)/edsv/src/*.c)
#else
 
	SRCS=$(wildcard $(MAKE_DIR)/*.c)
	SRCS += $(wildcard $(TET_OPENSAF_HOME)/common/src/*.c)

#endif

OBJS:=$(patsubst %.c,%.o,$(SRCS))
OPENSAF_TOT_LIBS+=$(SHARED_LIBRARIES)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(HJCPP_DEFINES) $(INCLUDE) -o $@ $<



# start
TET_LIB=$(TARGET)

all:$(TET_LIB) 

$(TET_LIB): $(OBJS)
		$(CC) $(LD_FLAGS) $@ -L$(TARGET_LIB_PATH) $(OBJS) $(OPENSAF_TOT_LIBS);
	@rm -rf $(OBJS)
           

$(OBJS):$(SRCS)

clean:
	rm -rvf `find $(TET_OPENSAF_HOME)/*/src/ -name *.o -follow` `find $(TET_OPENSAF_HOME)/ -name *.exe -follow` 

