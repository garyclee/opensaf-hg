#!/bin/bash

#source /opt/opensaf/tetware/lib_path.sh

export TET_PID_FILE=/var/opt/opensaf/pid/comp_pid

if [ ! -d "/var/opt/opensaf/pid" ] ;then
	mkdir  /var/opt/opensaf/pid
fi
                                                                                
echo "INSTANTIATE COMMAND: $SA_AMF_COMPONENT_NAME";

#/usr/bin/X11/xterm -title "`echo $SA_AMF_COMPONENT_NAME| cut -f 2 -d,|cut -f 2 -d=|awk -F_ '{printf($1"_"$3"\n")}'`"  -fg black  -bg '#efffff' -e $TET_BASE_DIR/ncs/avsv/avsv_a_$TARGET_ARCH.exe & 


if [ ! -d "/tmp/avsv" ] ;then
	mkdir /tmp/avsv
fi

xterm -e $TET_BASE_DIR/avsv/avsv_a_$TARGET_ARCH.exe > /tmp/avsv/"$SA_AMF_COMPONENT_NAME" 2>&1
#$TET_BASE_DIR/avsv/avsv_a_$TARGET_ARCH.exe > /tmp/avsv/"$SA_AMF_COMPONENT_NAME" 2>&1
