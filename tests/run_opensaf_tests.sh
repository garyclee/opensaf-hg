#! /bin/bash
#set -x

#      -*- OpenSAF  -*-
#
# (C) Copyright 2008 The OpenSAF Foundation
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
#
#
#
# modification:
# 2008-11   Jony <jiangnan_hw@huawei.com>  Huawei Technologies CO., LTD.
#  1.  function usage_comment()
#     + Show the usage of run_opensaf_test.sh
#  2.  function build_reg_cfg_file()
#     + Create a configure file required by TETware while test executing.
#  3.  function prepare_multi_test()
#     + Prepare multiple tests, including dependent tests and all tests.
#     + Create a single scenario file and configure file as required by TETware
#       while it executing.
#     + Copy the executable files to a temporary directory.
#     + Export some environment variables defined in file xxx_env.sh
#       as required by the tests.
#  4.  function prepare_single_test()
#     + Prepare for single test. Export some environment variables that are required
#       by the test. The environment variables are defined in file xxx_env.sh.
#  5.  function run_tests()
#     + Do the real work of running the tests. Determine the way of which tests are
#       executed and collect the results.
#  6.  function set_local_vars()
#     + Set some variables used in this script.
#  7.  function set_global_vars()
#     + Export some global variables used in the tests.
#  8.  function add_new_tetmodule()
#     + Add a directory named after the new test. Create some template files for
#       this new test.
#


#
# usage_comment 
#
function usage_comment()
{
echo "$0 [-a service_name] [-d] [-e filename] [-p platform] service_name"  
}

#
# build_reg_cfg_file
#
# creates a TETware configure file
#
function build_reg_cfg_file()
{
    echo "TET_OUTPUT_CAPTURE=false" >> $TEST_SUITE/$TET_CFG_FILE
    echo "TET_EXEC_IN_PLACE=false" >> $TEST_SUITE/$TET_CFG_FILE
    echo "TET_API_COMPLIANT=true" >> $TEST_SUITE/$TET_CFG_FILE
}


#
# prepare_multi_tests
#
# Create the tetexe.cfg, tet_scen file for all the tests
#
function prepare_multi_tests()
{
    TEST_SUITE=$TET_BASE_DIR/${TEST_NAME}_suites
    
    rm -rf $TEST_SUITE 2> /dev/null
    mkdir -p $TEST_SUITE
    
    #create file tet_scen
    echo "all" >> $TEST_SUITE/$TET_SCEN_FILE
    echo "    :parallel:" >> $TEST_SUITE/$TET_SCEN_FILE

    for i in $TEST_LIST; do
        if [ -d $i/suites ]; then
            TMP_SCEN=reg_${i}.scen
            TMP_ENV=${i}_env.sh
            
            if [ -f  $i/suites/$TMP_SCEN ]; then
                TMP_EXE=$(awk '/^[[:space:]]*\// {sub(/\//,""); print $1}' $i/suites/$TMP_SCEN)

                for j in $TMP_EXE; do                
                
                    if [ -f $i/suites/$j ]; then
                        echo "   /$j" >> $TEST_SUITE/$TET_SCEN_FILE
                        cp $i/suites/$j $TEST_SUITE
                    fi
                done

                if [ -f $i/suites/$TMP_ENV ]; then
                    source $i/suites/$TMP_ENV
                fi
            fi
        fi
    done

    echo "    :endparallel:" >> $TEST_SUITE/$TET_SCEN_FILE

    #create file tetexe.cfg 
    build_reg_cfg_file
}


#
# prepare_single_test
#
# Export envionment parameters used in service test
#
function prepare_single_test()
{
    TEST_SUITE=$TET_BASE_DIR/$SERVICE/suites
      
    if [ -f $TEST_SUITE/$TET_SCEN_FILE ]; then
        if [ -f $TEST_SUITE/$SRV_ENV_FILE ]; then
            source $TEST_SUITE/$SRV_ENV_FILE
        fi
        if [ ! -f $TEST_SUITE/$TET_CFG_FILE ]; then
            build_reg_cfg_file
        fi
    else
        echo "Without file $TET_SCEN_FILE"
        exit 2
    fi
}

#
# run_tests
#
# The argument to this function is either "all" or
# the name of one of the services in TEST_LIST.
#
function run_tests()
{    
    cd $TET_BASE_DIR

    if [ "$SERVICE" = "all" ]; then
        TEST_LIST=$(ls -d */ | awk -F / '{print $1}')
        prepare_multi_tests
    else
        if [ "$DEP_TEST" != "" ]; then
            if [ -f $SERVICE/suites/$SRV_DEP_FILE ]; then
                TEST_LIST=$(awk -F ":" '/^'"$SERVICE"'/ {print $2}' $SERVICE/suites/$SRV_DEP_FILE)
                
                if [ "$TEST_LIST" = "" ]; then 
                    echo "Without dependent tests for $SERVICE. Check file $SERVICE/suites/$SRV_DEP_FILE. "
                fi
                
                TEST_LIST+=" $SERVICE"
                prepare_multi_tests   
            else
                echo "Without file $SERVICE/suites/$SRV_DEP_FILE. "
                exit 2
            fi                    
        else
            prepare_single_test
        fi
    fi

    # create resulte directory
    TEST_TIME=$(date +%F_%H-%M-%S)
    RESULT_NAME=${TEST_NAME}_${TEST_TIME}
    RESULT_DIR=${OPENSAF_TET_RESULT}/${RESULT_NAME}
    mkdir -p $RESULT_DIR
    if [ ! -d $RESULT_DIR ]; then 
        echo "Check directory $RESULT_DIR"
        exit 2
    fi
    
    cd $TEST_SUITE

    if [ ! -f $OPENSAF_TET_LOG ]; then
      touch $OPENSAF_TET_LOG
    fi  
    typeset -i LINE_BEGIN=($(wc -l $OPENSAF_TET_LOG | awk '{print $1}') + 1)
    
    echo "Starting Tests for service..."  
    ${TET_ROOT}/bin/tcc -e -s $TET_SCEN_FILE -x $TET_CFG_FILE -i $RESULT_DIR -j $RESULT_DIR/${TEST_NAME}.jrnl $PWD
  
    if [ $? -ne 0 ]; then
       echo "Tests for service did not run."
       return
    else
       echo "Done."
    fi

    # Copy the test log to result dictory
    typeset -i LINE_END=$(wc -l $OPENSAF_TET_LOG | awk '{print $1}')
    sed -n "$LINE_BEGIN,${LINE_END}p" $OPENSAF_TET_LOG > $RESULT_DIR/${TEST_NAME}.log

    echo "Generating result in html format..."
    ${TET_ROOT}/bin/grw -c bceTBIpif -f chtml -o $RESULT_DIR/${TEST_NAME}.html $RESULT_DIR/${TEST_NAME}.jrnl
    echo "Check ${RESULT_DIR}/${TEST_NAME}.html for the results."

    if [ "$DEP_TEST" != "" -o "$SERVICE" = "all" ]; then
        rm -rf $TEST_SUITE 
    fi
}


#
# set the variables used in this script
#
function set_local_vars()
{
    if [ "$DEP_TEST" != "" ]; then
        SRV_DEP_FILE=reg_${SERVICE}.dep
        TEST_NAME=${SERVICE}_dep
    else
        TEST_NAME=$SERVICE
    fi   
    SRV_ENV_FILE=${TEST_NAME}_env.sh
    TET_CFG_FILE=reg_${TEST_NAME}.cfg
    TET_SCEN_FILE=reg_${TEST_NAME}.scen

}

#
# Export some env variables required by the tet tests.
# This script MUST export or re-export any environment
# variables that are used in the */suites programs and scripts.
#
function set_global_vars()
{
    echo "Set global variables"

    if [ -f $ENV_FILE ]; then
        source $ENV_FILE
    else
        echo "Without file $ENV_FILE. Use default environment variables."
    fi

    # where OpenSAF is installed:
    export OPENSAF_ROOT=${OPENSAF_ROOT:-/opt/opensaf}
    export OPENSAF_CONF=${OPENSAF_CONF:-/etc/opt/opensaf}
    export OPENSAF_VAR=${OPENSAF_VAR:-/var/opt/opensaf}
 
    # where tetware objects are installed:
    export TET_ROOT=${TET_ROOT:-/usr/local/tet}
    if [ ! -d $TET_ROOT/bin ]; then
        echo "$TET_ROOT/bin does not exist"
        echo "Install Tetware in $TET_ROOT,"
        echo "then re-run this script."
        exit 1
    else
        echo "TET_ROOT = $TET_ROOT"
    fi
 
    export PATH=$PATH:$TET_ROOT/bin

    # where opensaf tests are installed:
    export TET_BASE_DIR=${TET_BASE_DIR:-$PWD}
    if [ ! -d $TET_BASE_DIR ]; then
        echo "$TET_BASE_DIR does not exist"
        exit 1
    else 
        echo "TET_BASE_DIR = $TET_BASE_DIR"
    fi
 
    # where test results are saved 
    OPENSAF_TET_RESULT=${OPENSAF_TET_RESULT:-$PWD/results}
    mkdir -p $OPENSAF_TET_RESULT 
    if [ ! -d $OPENSAF_TET_RESULT ]; then
        echo "Check directory $OPENSAF_TET_RESULT"     
        exit 1
    fi
    echo "OPENSAF_TET_RESULT = $OPENSAF_TET_RESULT"

    # where test log are saved
    OPENSAF_TET_LOG=${OPENSAF_TET_LOG:-/tmp/tccdlog}
    echo "OPENSAF_TET_LOG=$OPENSAF_TET_LOG"
 
    # Tests look for executables with names of the form "*_${TARGET_ARCH}.exe"
    #export TARGET_ARCH=${TARGET_ARCH:-i386}
 
    export TET_TMP_DIR=/tmp
    #export TET_MDS_IF_INDEX=1
    #export TET_CLUSTER_ID=6007  
}

#
# Add a new test module
#
function add_new_tetmodule()
{
    MODNAME=$1
    TEMPLATE_TAR=template.tar.z
    #create new test module directory
    if [ -d $MODNAME ]; then
        echo "new module $MODNAME has already exist!"
        exit 1
    fi

    mkdir $MODNAME

    #untar template.tar to new module directory
    if [ ! -f $TEMPLATE_TAR ]; then
        echo "new template file $TEMPLATE_TAR dosn't exist!"
        exit 1
    fi
    cp $TEMPLATE_TAR ./$MODNAME/
    cd $MODNAME
    tar -xvzf $TEMPLATE_TAR

    #modify abc module name to new module name
    rename "xxx" "${MODNAME}" ./inc/*
    rename "xxx" "${MODNAME}" ./src/*
    rename "xxx" "${MODNAME}" ./suites/*

    sed -i "s/xxx/$MODNAME/g" `grep xxx -rl ./`
    
    UP_MODNAME=`echo $MODNAME|tr 'a-z' 'A-Z'`
    sed -i "s/XXX/$UP_MODNAME/g" `grep XXX -rl ./`

    #add new module Makefile.am to project Makefile.am

    rm $TEMPLATE_TAR
    cd ..
    echo "add new test module $MODNAME complete!"
    
}


################################
# main program
################################

while getopts a:de:p:t: option; do
    case $option in
        a) 
            add_new_tetmodule $OPTARG
            exit 0
            ;;
        d) 
            DEP_TEST=yes
            ;;  
        e) 
            ENV_FILE=$OPTARG
            
            ;;  
        p) 
            PLATFORM=$OPTARG
            if [ "$PLATFORM" = "" ]; then
                PLATFORM=linux
            else
                if [ "$PLATFORM" != "linux" -a "$PLATFORM" != "linux64" -a "$PLATFORM" != "solaris" ]; then
                    echo "Error while processing platform name"
                    usage_comment
                    exit 2
                fi
            fi
            ;;   
        t) 
            TEST_CASE=$OPTARG
            if [ "$TEST_CASE" = "" ]; then
                TEST_CASE=bas
            else
                if [ "$TEST_CASE" != "bas" -a "$TEST_CASE" != "ext" ]; then
                    temp=$(echo $TEST_CASE | grep [^0-9,])
                    if [ "$temp" != "" ]; then 
                        echo "Error while processing test cace type"
                        usage_comment
                        exit 2
                    fi
                fi
            fi
            ;;
        *)
            echo "Unknown error while processing options"
            usage_comment
            exit 2
            ;;
    esac
done

SERVICE=${@:$OPTIND}

if [ "$SERVICE" = "" ]; then
    echo "Invalid service name"
    usage_comment
    exit 1
fi

if [ "$ENV_FILE" = "" ]; then
     ENV_FILE=opensaf_tet_env.sh
fi

# initialize some global variables
set_global_vars
    
if [ ! -d $TET_BASE_DIR/$SERVICE/suites ]; then
    if [ "$SERVICE" != "all" ]; then
        echo "Invalid service name"
        usage_comment
        exit 1
    else
        DEP_TEST=""
    fi
fi

# initialize some local variables used in this script
set_local_vars


# do the real work of running the test(s)
run_tests

exit 0
