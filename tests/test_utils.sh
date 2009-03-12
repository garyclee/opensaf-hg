#!/bin/bash

PCSERVER=127.0.0.1
PCSERVER_HOSTNAME=`hostname`
TET_ROOT=/usr/local/tet
TETWARE_DIR=/opt/opensaf/tetware
HOSTNAME=`hostname`
ARCH=`uname -m`

edit_passwd_file()
{
   echo "Taking a backup of the /etc/passwd file as /etc/passwd.bkp ....."
   cp /etc/passwd /etc/passwd.bkp
   echo "Updating the /etc/passwd file ...."
   cat /etc/passwd.bkp |perl -e '{ while(<>){if(/tet:/){print "tet:x:0:0::/home/tet:/bin/bash\n"}else{print } }  }' >/etc/passwd
}

create_users_groups () {
  echo "Adding required groups and users"
# Now, create the user and group required for TET
    userdel tet 2>/dev/null
    groupdel testers 2>/dev/null
    groupdel tet 2>/dev/null
 
    /usr/sbin/groupadd testers
    /usr/sbin/groupadd tet
 
    mkdir -p /home/tet
    /usr/sbin/useradd -g tet -G testers -p tet -d /home/tet tet
#    cp /root/systems.equiv /home/tet
    chown -R tet /home/tet
    chgrp -R tet /home/tet
 
 
}


create_tet_env()
{
   edit_passwd_file

   grep "export PATH=\$PATH:/usr/local/tet/bin" /root/.bashrc
   if [ $? -ne 0 ]; then echo "export PATH=\$PATH:/usr/local/tet/bin" >> /root/.bashrc; fi
   grep "export TET_ROOT=/usr/local/tet" /root/.bashrc
   if [ $? -ne 0 ]; then echo "export TET_ROOT=/usr/local/tet" >> /root/.bashrc; fi
   grep "export TET_TMP_DIR=/tmp" /root/.bashrc
   if [ $? -ne 0 ]; then echo "export TET_TMP_DIR=/tmp" >> /root/.bashrc; fi
   grep "source /root/.bashrc" /etc/profile
   if [ $? -ne 0 ]; then echo "source /root/.bashrc" >> /etc/profile; fi

   export PATH=$PATH:/usr/local/tet/bin
   export TET_ROOT=/usr/local/tet
   export TET_TMP_DIR=/tmp

   if [ -f /untar.log ]; then
      rm /untar.log
   fi
}

setup_tware_env()
{
   create_users_groups
   ./install_tccd.sh
   create_tet_env
   do_initial_configuration
}

create_setup()
{
   echo "#================ Tetware automation cfg =============" > /tmp/setup
   echo "#<Ip-addr>        <blade>" >> /tmp/setup
   echo "#=====================================================" >> /tmp/setup
   if [ $HOSTNAME != $PCSERVER_HOSTNAME ]; then
      echo "$PCSERVER $PCSERVER_HOSTNAME" >> /tmp/setup
   fi
   echo "#=====================================================" >> /tmp/setup
}

create_systems_files()
{
   echo "000 $1" > /usr/local/tet/systems
   echo "$1" > /usr/local/tet/systems.equiv
   if [ -f /home/tet/systems ]; then
      rm /home/tet/systems /home/tet/systems.equiv
   fi
   ln -s /usr/local/tet/systems /home/tet/systems
   ln -s /usr/local/tet/systems.equiv /home/tet/systems.equiv
}

CLEANHOSTS='
use File::Copy;
$fileName = "/etc/hosts";
$outFileName = "/etc/hosts.new";
$startPattern = "#================ Tetware automation cfg =============";
$endPattern1  = "#<Ip-addr>";
$endPattern   = "#=====================================================";

open (INFILE, $fileName) || die "Cannot open $fileName: $!\n";
open (OUTFILE, ">$outFileName") || die "Cannot open $outFileName: $!\n";

while (<INFILE>) {
        next if (/$startPattern/ ... /$endPattern1/);
        next if (/$endPattern/  ... /$endPattern/);
        print  OUTFILE $_;
}
close (INFILE);
close (OUTFILE);

move ($outFileName, $fileName);
'

PERLCOMMAND='
$infile = "/etc/hosts";
$outfile = "/tmp/tmp_hosts";
$setupfile="/tmp/setup";

open (INFILE, $infile)  or die "cant open $infile $!\n";
open (OUTFILE, ">$outfile") or die "cant open test_hosts $!\n";
open (SETUPFILE, $setupfile) or die "cant open setup $!\n";

my $var="";
my $tmp_var="";

while (<INFILE>) {
    if (/^127\.0\.0\.1/) {
       if ($tmp_var eq "") {
          $tmp_var="$_"
       }
    } else {
      print OUTFILE $_
    }
}

close (INFILE);

while(<SETUPFILE>) {
    print OUTFILE $_
}

close (SETUPFILE);
close (OUTFILE);'

do_initial_configuration()
{
  source setup.sh
   perl -e "{$CLEANHOSTS}"
   create_systems_files $HOSTNAME
   touch /tmp/setup
   perl -e "{$PERLCOMMAND}"
   cp /tmp/tmp_hosts /etc/hosts
   echo "127.0.0.1 $HOSTNAME localhost" >> /etc/hosts
   rm /tmp/setup /tmp/tmp_hosts
   /etc/init.d/xinetd restart
}

configure_tetware()
{ 
   source setup.sh
   rm -rf /opt/opensaf/tetware
   rm -rf /usr/local/tet
 
   ln -s  $PWD  /opt/opensaf/tetware 
   ln -s  $OPEN_TWARE_HOME   /usr/local/tet
 
   setup_tware_env
   perl -e "{$CLEANHOSTS}"

   create_systems_files $PCSERVER_HOSTNAME
   create_setup
   echo "Updating the /etc/hosts file  ...."
   if [ $HOSTNAME == $PCSERVER_HOSTNAME ]; then
      grep $PCSERVER /etc/hosts
      if [ $? -ne 0 ]; then
         echo "Adding entry to /etc/hosts"
         echo "$PCSERVER   $PCSERVER_HOSTNAME" >> /etc/hosts
      fi
   fi
   perl -e "{$PERLCOMMAND}"
   cp /tmp/tmp_hosts /etc/hosts
   echo "127.0.0.1 $HOSTNAME localhost" >> /etc/hosts
   if [ $HOSTNAME == $PCSERVER_HOSTNAME ]; then
      cp /etc/hosts /etc/hosts.bak
      ./alignEtcHosts.pl $PCSERVER
   fi
   /etc/init.d/xinetd restart
   rm /tmp/tmp_hosts /tmp/setup
   source lib_path.sh $1
   #cp maa_switch/libmaa_switch.so $LD_LIBRARY_PATH/libmaa_switch.so
}

tet_setup_run_env()
{
   source setup.sh
   rm -rf /opt/opensaf/tetware
   rm -rf /usr/local/tet
 
   ln -s  $PWD  /opt/opensaf/tetware 
   ln -s  $OPEN_TWARE_HOME   /usr/local/tet

   cd /opt/opensaf/tetware/
   # source lib_path.sh
   ./build_all.sh  $1


   #cp maa_switch/libmaa_switch_linux.so $LD_LIBRARY_PATH/libmaa_switch.so
}

tet_pack()
{
 cd $OPENSAF_HOME 
 tar -zcf tests/tet_suites$TARGET_HOST.tgz  tests/avsv/*.exe tests/avsv/suites/* tests/cpsv/*.exe tests/cpsv/suites/* tests/cpsv/src/tet_cpa.c tests/edsv/*.exe  tests/edsv/suites/*  tests/glsv/*.exe tests/glsv/suites/* tests/ifsv/*.exe tests/ifsv/suites/*   tests/mds/suites/* tests/mds/*.exe  tests/mqsv/*.exe tests/mqsv/suites/*  tests/mbcsv/*.exe tests/mbcsv/suites/*  tests/srmsv/suites/* tests/srmsv/*.exe tests/lib_path.sh tests/run_tests.sh tests/test_utils.sh tests/alignEtcHosts.pl tests/setup.sh tests/install_tccd.sh
}

case "$1" in
   setup_env)
      if [ ":"$2 == ":" ]; then 
          echo "Error: USAGE: ./test_utils.sh <build/setup_env> <linux/linux-64>";
          exit;
      fi
      configure_tetware $2
      ;;

   build)
      if [ ":"$2 == ":" ]; then
          echo "Error: USAGE: ./test_utils.sh <build/setup_env> <linux/linux-64>";
          exit;
      fi
      tet_setup_run_env $2;
      tet_pack
      ;;
   *)
      echo "Error: USAGE: ./test_utils.sh <build/setup_env> <linux/linux-64>";;
esac

