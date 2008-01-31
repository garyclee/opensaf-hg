#! /bin/sh

#      -*- OpenSAF  -*-
#
#  Copyright (c) 2007, Ericsson AB
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
# file and program are licensed under High-Availability Operating 
# Environment Software License Version 1.4.
# Complete License can be accesseble from below location.
# http://www.opensaf.org/license 
# See the Copying file included with the OpenSAF distribution for
# full licensing terms.
#
# Author(s):
#   
#

# UML System startup script

die() {
    echo "ERROR: $*"
    exit 1
}

test -f "$PWD/opensaf.sh" || die 'Must execute opensaf.sh from directory where it is located'
export CLUSTER_SIM_UML_DIR=$PWD
test -d ./root.controller || die "No shadow root at [./root]"
test -d ./root.payload || die "No shadow root at [./root]"

UML_DIR=$CLUSTER_SIM_UML_DIR/uml
REPL_DIR=repl-opensaf
test -x $UML_DIR/bin/uml_start || die "Not executable [$UML_DIR/bin/uml_start]"

NUMNODES=${2:-5}
IPADDRBASE=${IPADDRBASE:-192.168.0}
PIDFILEDIR=/tmp/${USER}_opensaf

cluster_start() {
    numnodes=$1
    x=5
    y=5
    dy=340

    # Clean-up
    rm -rf $PIDFILEDIR
    mkdir $PIDFILEDIR

    if ! test -d $CLUSTER_SIM_UML_DIR/$REPL_DIR; then
        mkdir -p $CLUSTER_SIM_UML_DIR/$REPL_DIR/pssv_store
        mkdir -p $CLUSTER_SIM_UML_DIR/$REPL_DIR/log
    fi

    # Create /etc/hosts file
    echo "127.0.0.1       localhost" > root.controller/etc/hosts

    # Start network if necessary
    if ! pgrep uml_switch > /dev/null; then
        xterm -iconic -geometry 60x8-0+110 \
            -T UML-network -e $UML_DIR/bin/uml_switch &
        echo $! >> $PIDFILEDIR/uml_pids
    fi

    # Start the processors
    node_name_prefix=SC_2_
    rootdir=root.controller
    for ((p=1; p <= $numnodes; p++)); do
        # First two nodes are system controllers, the rest are payloads
        if ((p == 3)); then
            x=660
            y=5
            node_name_prefix=PL_2_
            rootdir=root.payload
        fi
        hostname=$node_name_prefix$p
        xterm -hold -geometry 90x20+$x+$y -T $hostname -e \
            $UML_DIR/bin/uml_start $p umid=$hostname mem=128M hostname=$hostname shadowroot=$rootdir &
        echo $! >> $PIDFILEDIR/uml_pids
        y=$((y+dy))

        # Append host to /etc/hosts file
        echo "$IPADDRBASE.$p       $hostname.opensaf.org  $hostname" >> root.controller/etc/hosts
    done

    cp root.controller/etc/hosts root.payload/etc/hosts
}

cluster_stop() {
    echo "Stopping opensaf services"
    nodes=`ls $HOME/.uml`
    for node in $nodes; do
        $UML_DIR/bin/uml_mconsole $node halt > /dev/null 2>&1
        sleep 1
        rm -rf $HOME/.uml/$node
    done
    if [ -f $PIDFILEDIR/uml_pids ]; then
        for i in $(cat $PIDFILEDIR/uml_pids); do
            kill -9 $i > /dev/null 2>&1
        done
        rm -f $PIDFILEDIR/uml_pids
    fi
}

delete_persistent_store() {
    echo "Deleting persistent store contents"
    rm -rf $CLUSTER_SIM_UML_DIR/root.controller/var/*
    rm -rf $CLUSTER_SIM_UML_DIR/root.payload/var/*
    rm -rf $CLUSTER_SIM_UML_DIR/$REPL_DIR/log/*
    rm -rf $CLUSTER_SIM_UML_DIR/$REPL_DIR/pssv_store/*
}

case "$1" in
    start)
        echo "Cold starting opensaf services"
        cluster_stop
        delete_persistent_store
        cluster_start $NUMNODES
        ;;
    stop)
        cluster_stop
        ;;
    clean)
        delete_persistent_store
        ;;
    restart)
        echo "Warm starting opensaf services"
        echo "Reusing persistent store contents"
        cluster_stop
        cluster_start $NUMNODES
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
	
esac
exit 0
