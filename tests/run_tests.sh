#!/bin/bash

source ./lib_path.sh $1
OPENSAF_TETWARE_DIR=/opt/opensaf/tetware


function build_reg_scen_file()
{
   echo "all" > $OPENSAF_SCEN_FILE
   echo "   :parallel:" >> $OPENSAF_SCEN_FILE
   echo "   /$OPENSAF_REG_SVC_FILE" >> $OPENSAF_SCEN_FILE
   echo "   :endparallel:" >> $OPENSAF_SCEN_FILE
}

function build_ifsv_reg_scen_file()
{
   echo "all" > $OPENSAF_SCEN_FILE
      echo "   /reg_vip.sh" >> $OPENSAF_SCEN_FILE
      echo "   :parallel:" >> $OPENSAF_SCEN_FILE
      echo "   /$OPENSAF_REG_SVC_FILE" >> $OPENSAF_SCEN_FILE
      echo "   /$OPENSAF_REG_IFSV_DRIVER_FILE" >> $OPENSAF_SCEN_FILE
      echo "   :endparallel:" >> $OPENSAF_SCEN_FILE
}




function single_node_test()
{ 
   export TET_ROOT=/usr/local/tet
   export PATH=$PATH:/usr/local/tet/bin

   cd $OPENSAF_TETWARE_DIR/$1/suites
   if [ ":$REG_TYPE" == ":" ]; then
      OPENSAF_REG_SVC_FILE=reg_$1.sh
   else
      OPENSAF_REG_SVC_FILE=reg_$1_$REG_TYPE.sh
   fi
   OPENSAF_SCEN_FILE=reg_$1_auto.scen
   mkdir -p $RESULTS_DIR
   RES_DIR=$RESULTS_DIR
   if [ "$1" == "ifsv" ]; then
      if [ ":$REG_TYPE" == ":" ]; then
         OPENSAF_REG_IFSV_DRIVER_FILE=reg_ifsv_driver.sh
      else
         OPENSAF_REG_IFSV_DRIVER_FILE=reg_ifsv_"$REG_TYPE"_driver.sh
      fi
      build_ifsv_reg_scen_file  
   else
      build_reg_scen_file 
   fi
   if [ -f $RES_DIR/$1_jrnl ]; then
      echo "The file $RES_DIR/$1_jrnl already exists"
      export MOVE_NOW=`date +%F_%T`
      echo "Moving $1_jrnl $1.txt to $RESULTS_DIR/$MOVE_NOW"
      mkdir $RESULTS_DIR/$MOVE_NOW/
      mv $RES_DIR/$1_jrnl $RES_DIR/$1.txt $RESULTS_DIR/$MOVE_NOW/.
   fi
   echo "Starting Tests for $1 service..."  
   tcc -e -s $OPENSAF_SCEN_FILE -j $RES_DIR/$1_jrnl $PWD
   echo "Done."
   rm -f $OPENSAF_SCEN_FILE
   cd $RES_DIR
   echo "Generating result in text format..."
   grw -c 6 -f text -o $1.txt $1_jrnl
   echo "Check $1.txt for the results."
}


function run_single_node()
{
   SVC=$2

   case "$1" in
      reg)
          OPENSAF_REG_SVCS_COUNT=8
          declare -ar opensaf_reg_svcs='([0]="cpsv" [1]="edsv" [2]="mqsv" [3]="glsv" [4]="ifsv" [5]="srmsv" [6]="mds" [7]="mbcsv" )'
          unset REG_TYPE
          ;;
   esac

   if [ $# -le 1 ]; then
      echo "Get the <svc_name> from the following"
      echo "************************"
      for ((i=0; i<$OPENSAF_REG_SVCS_COUNT; i++))
      do
         echo "${opensaf_reg_svcs[$i]}"
      done
      echo "************************"
      return;
   fi

   if [ "$SVC" != "all" ]; then
      status=false;

      for ((i=0; i<$OPENSAF_REG_SVCS_COUNT; i++))
      do
         if [ ${opensaf_reg_svcs[i]} == $SVC ]; then
            status=true;
            break;
         fi
      done

      if [ $status = "false" ]; then
         echo " Invalid service name "
         return;
      fi
   fi


   #source $OPENSAF_TETWARE_DIR/lib_path.sh
   source /opt/opensaf/tetware/setup.sh
   if [ ":$RESULTS_DIR" == ":" ]; then
      RESULTS_DIR=/root/test_results
   fi

   if [ "$SVC" == "all" ]; then

      for ((i=0; i<$OPENSAF_REG_SVCS_COUNT; i++))
      do
         single_node_test ${opensaf_reg_svcs[$i]}  $1
      done

   else
      single_node_test $SVC  $1
   fi
}


function run_reg()
{
   if [ $# -lt 1 ]; then
      echo "Invalid input - Options <svc_name/all> "
      echo ""
      run_single_node reg 
   else
      run_single_node reg $1
   fi
}


function show_cmds()
{
cat << EOF
=========== CMD-SET ==============
run_reg
==================================
EOF
}
