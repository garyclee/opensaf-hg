export TET_TMP_DIR=/tmp
export MBCSTM_SYS_ID=0
export DISTRIBUTED=0
export SINGLE=1
export TET_CASE=-1

#source /opt/opensaf/tetware/lib_path.sh

#... invoke your exe
#xterm -T INST1 -geometry 120x40 -e gdb $TET_BASE_DIR/mbcsv/mbcsv_$TARGET_ARCH.exe
 $TET_BASE_DIR/mbcsv/mbcsv_$TARGET_ARCH.exe

