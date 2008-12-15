#! /usr/bin/perl
$data_file = "/var/opt/opensaf/pid/comp_pid";

$COMP_NAME = $ENV{'SA_AMF_COMPONENT_NAME'};
print "CLEANUP COMMAND: $COMP_NAME\n";

open(DAT,"+<",$data_file) || die("Could not open file!");
@raw_data=<DAT>;

foreach $wrestler (@raw_data)
{
 chop($wrestler);
 chop($pid);
 ($w_name,$pid)=split(/\:/,$wrestler);

 print "$w_name $pid \n";

 if("$COMP_NAME" eq "$w_name") {
 	print "Killing the Component $w_name with pid = $pid\n\n";
        delete @raw_data{$wrestler,@raw_data};
        undef $wrestler;
        kill 10, $pid; #SIGUSR1
 }
}

$ENV{'TWARE_NCS_EXEC'}=2;
close(DAT);

# Sleep for few second to give time for processing of SIGUSR1
sleep 3;



