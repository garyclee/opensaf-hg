#!/usr/bin/perl
#
#  TestUtils: Test utility routines for HISv tests.
#
#  Routines:
#    ErrorMessage
#    Error
#    Warn
#    Message
#    ParseCmdline
#    Done
#    RunCmd
#    CheckRunTest
#
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
# Author(s):
#           Hewlett-Packard Company
#
#
################################################################################
package TestUtils;

################################################################################
#                             Globals and Defaults
################################################################################
use strict 'vars';
use Getopt::Long;
use Globals;
use Exporter;
use vars qw($VERBOSE $DRYRUN $GLOBAL_H %TET_EXIT @EXPORT @ISA);

@ISA = qw(Exporter);
@EXPORT = qw(ErrorMessage Error Warn Message ParseCmdline Done
             RunCmd RunBG ReadTestDataFile);

################################################################################
#                             Globals and Defaults
################################################################################
 # The name of the calling test
$GLOBAL_H->{MyName} = $0;
   $GLOBAL_H->{MyName} =~ s:.*::;

 # To show commands, not execute them
$DRYRUN = 0;

 # To read and write test parameters
$GLOBAL_H->{TestDataFile} = "hisvTest.dat";

 # Level of informational test output
$VERBOSE = 0;

 # Set TET exit values
%TET_EXIT = ("PASS" => 0,
			 "FAIL" => 1,
			 "UNRESOLVED" => 2,
			 "NOTINUSE" => 3,
			 "UNSUPPORTED", 4,
			 "UNTESTED", 5,
			 "UNINITIATED", 6,
			 "NORESULT", 7);

################################################################################
#                                 Subroutines
################################################################################
#*******************************************************************************
#
#  ErrorMessage: Prints out an error message to stderr and exits with the
#                specified error code.
#  Input:   $msg     -> Message to print
#           $exitVal -> Error code for exit
#
#  Return:
#           Exits with the error code if a message is specified
#           If $msg and $exitVal are undefined, print a usage message and
#           exit with value 0 (shell success).
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub ErrorMessage
{
    my ($msg, $exitVal) = @_;

    $exitVal =  $TET_EXIT{"FAIL"} if (defined($msg) && !defined($exitVal));
    my $myName = $GLOBAL_H->{MyName};

      # Remove one trailing newline, since we'll provide one for free
    if (defined($msg) && $msg !~ /^\s*$/) {
       $msg =~ s/\n$//;
    }

      # If just printing a usage message, set the exit value to zero.
    if (defined($msg) && $msg =~ /^\s*$/) {
       $exitVal = 0;
       print "\nUsage: $myName [-v[=level]] [-dryrun] [-f testDataFile]\n";
       print "Where:\n";
       print "     -v\t\tPrint progress messages.\n";
       print "       \t\tIf the level is greater than 1, print debugging messages.\n";
       print "     -dryrun\tOnly show commands that would be run, don't execute them.\n";
       print "  Test Options:\n";
       print "     -f\t\tSpecify the file that contains the test data\n";
       print "       \t\tDefault: $GLOBAL_H->{TestDataFile}\n";
       print "\n";
    }
    else {
       print STDERR "\nERROR: $msg\n";
       print STDERR "\n";
   }
   Done($exitVal) if ($exitVal);
   exit($exitVal);
}

#*******************************************************************************
#
#  Error: Prints out an error message to stderr, and returns
#  Input:   $msg     -> Message to print
#
#  Return: void
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub Error
{
    my ($msg) = @_;

      # Remove one trailing newline, since we'll provide one for free
    if (defined($msg) && $msg !~ /^\s*$/) {
       $msg =~ s/\n$//;
    }
    print STDERR "ERROR: $msg\n";
    return;
}  # End Error();

#*******************************************************************************
#
#  Warn: Prints out warninig message to stderr.
#  Input:   $msg     -> Message to print
#
#  Return: void
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub Warn
{
   my ($msg) = @_;

    # Remove one trailing newline, since we'll provide one for free
   if (defined($msg) && $msg !~ /^\s*$/) {
      $msg =~ s/\n$//;
   }
   print STDERR "\nWARNING: $msg\n";
   return;
}  # End Warn

#*******************************************************************************
#
#  Message: Prints out an informational message.
#  Input:   $msg     -> Message to print
#           $level   -> Verbosity level to print message.
#
#  Return: void
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub Message
{
   my ($msg, $level) = @_;
   $level = 0 if (!defined($level));

    # Remove one trailing newline, since we'll provide one for free
   if (defined($msg) && $msg !~ /^\s*$/) {
      $msg =~ s/\n$//;
   }
   if ($VERBOSE >= $level) {
      print "INFO: $msg\n";
   }
   return;
}  # End Message

#*******************************************************************************
#
#  ParseCmdline: Parse the user's command line.
#  Input:   (global) @ARGV
#
#  Return: void
#  Output: Sets global variables, $VERBOSE, $DRYRUN, $GLOBAL_H->{TestDataFile}
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub ParseCmdline
{
   my $help;
   
   GetOptions(
		"dryrun"				=> \$DRYRUN,
		"file|data=s"			=> \$GLOBAL_H->{TestDataFile},
		"verbose|v:i"			=> \$VERBOSE,
		"h:help:?"				=> \$help,
   );

   ErrorMessage() if $help;

   $VERBOSE = 1 if (defined($VERBOSE) && !$VERBOSE);
   $VERBOSE = 3 if ($DRYRUN);
}   # End ParseCmdline

#*******************************************************************************
#
#  Done: Cleanup and exit
#  Input:  code -> exit value
#
#  Return: void
#          Exits with the requested value.
#
#  Note: Kill any processes spawned by tests calling RunCmd using the PID list
#        in $GLOBAL_H->{pids}
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub Done
{
   my ($code) = @_;
     # Kill any processes that we started that haven't completed
   if ($GLOBAL_H->{pids}) {
      if (ref($GLOBAL_H->{pids}) =~ /ARRAY/ &&
         scalar(@{$GLOBAL_H->{pids}})) {
            Message "Killing @{$GLOBAL_H->{pids}}\n";
            kill "KILL" => @{$GLOBAL_H->{pids}};
            Message "Waiting a moment to insure that all of the processes are killed";
            sleep 2;
      }
   }
   exit($code);
}  # End Done

#*******************************************************************************
#
#  RunCmd: Run a command, checking to see if it succeeds
#           If the command fails, parse the command output
#           for "Error:" messages
#
#  Input:   $cmd     ->  command to run
#           $timeout ->  timeout (default is 10 sec)
#
#  Return:  1 => command success
#           0 => command failure
#           Sets $GLOBAL_H->{cmd_output} with output from the
#           output so that it can be parsed.
#           Sets $GLOBAL_H->{pids} with the process' ID
#
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
sub RunCmd
{
    my ($cmd, $timeout) = @_;
    $timeout = 10 if (!defined($timeout));

    Message("Running $cmd", 2);

    return 1 if ($DRYRUN);

    $GLOBAL_H->{cmd_output} = undef;
    my $perlRtn;

     # Set a timeout to keep processes from running away
    local $SIG{ALRM} = sub{die "timeout";};
    if (my $pid = open(CMD, "$cmd 2>&1 |")) {
       push (@{$GLOBAL_H->{pids}}, $pid);
       Message("Command Output:\n", 3);
       eval {
          alarm($timeout);
          while (<CMD>) {
             print if ($VERBOSE >= 3);
             push(@{$GLOBAL_H->{cmd_output}}, $_);
          }
          close(CMD);
          alarm(0);
      };  # End eval
      $perlRtn = $?;
        # The process completed, remove the PID from the list
      pop(@{$GLOBAL_H->{pids}});
    }  # End if ($pid=open...)

    my $shellRtn = $perlRtn >> 8;
    if ($shellRtn && ($VERBOSE < 3)) {
       print "@{$GLOBAL_H->{cnd_output}}";
       print "\n\nCommand returned an error value, $shellRtn\n";
    }
    if ($shellRtn == 14) {
       Error("$cmd timed out in $timeout seconds");
    }
    elsif ($shellRtn == 139) {
       Error("$cmd dumped core");
    }

    my $success = $shellRtn ? 0 : 1;
    return $success;
}  # End RunCmd

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#  CheckRunTest: Check to see if this test should be run.
#                 Checks to see if SKIP_TEST file exists.
#
#  Input:   None
#
#  Return:  0, unless SKIP_TEST file exists, then exit UNTESTED.
#
######################################################################
sub CheckRunTest
{
   my $MyName = $GLOBAL_H->{MyName};
   if ( -f "SKIP_TEST") {
       print STDERR "\nSkipping $MyName";
       exit($TET_EXIT{UNTESTED});
   }
}

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#
#  ReadTestData: Read the test data file to populate 
#                 Checks to see if SKIP_TEST file exists.
#
#  Input:   Data file
#
#
#  Return:  Pointer to hash with data to run tests
#
#  Notes:
#	 The format of the data file is:
#		 oa {
#			   OA_hostname 		=  hostname | ip of the OA
#			   OA_User_Name		=  user with admin privileges
#			   OA_Password 		=  OA password for the above user
#			   entity_root 		=  chassis controlled by the OA
#			}
#
#		 controller {
#			   system_hostname	= hostname | ip
#			   system_bay	 	= bay number (same as the
#								  OA slot displayed using
#								  the OA command "show server list")
#			   system_ilo_name	= hostname | ip of the system
#			   system_ilo_user	= iLO2 user with admin privileges
#			   system_ilo_password	= password of the above user
#			   system_user		= user with root privileges
#			   system_password	= password of that system
#			}
#
#		 payload {
#			   system_hostname	= hostname | ip
#			   system_bay	 	= bay number (same as the
#								  OA slot displayed using
#								  the OA command "show server list")
#			   system_ilo_name	= hostname | ip of the system
#			   system_user		= user with root privileges
#			   system_password	= password of that system
#              ilo_user			= ilo user with admin privileges
#              ilo_password		= ilo user with admin privileges
#			}
#
#	 The format of the returned hash is:
#	 $tdata->{oa}->{ip}			= IP/hostname for the OA supervising
#									 the enclosure for the systems.
#				 ->{user}		= OA user with admin privileges
#				 ->{passwd}		= OA password for user with admin
#									 privileges
#				 ->{entity_root}= Chassis ID in the format
#                                 "{SYSTEM_CHASSIS,2}"
#
#
#	 $testSystems->{role}->$systemData
#	 where role is one of (active, standby, payload_1, payload_2),
#	 and $systemData is a hash pointer with the following data
#	 for each blade:
#	  $systemData->{hostname}  = hostname | ip
#				 ->{bay} 	  = blade_id
#				 ->{IP}		= IP/hostname for eth0
#
######################################################################
{
my @foundList;      # keep track of data types we've found
sub ReadTestDataFile
{
   my ($tdataFile) = @_;
   my @types = qw(liboa_soap libilo2_ribcl controller payload);
   my @dataTags = qw(entity_root OA_User_Name OA_Password ACTIVE_OA
					system_hostname system_bay system_ilo_name
					system_user system_password
                  );

   Message("Reading $tdataFile");
   my $dataHash = {};  # The return value

   open(CFG, "<$tdataFile") or ErrorMessage("Couldn't open test-data file, $tdataFile", $TET_EXIT{NORESULT});

   # For debug/error printing
   my $lineNo = 0;
   while (<CFG>) {
        $lineNo++;
		# Get rid fo new line at the end
        chomp;
        # Get rid of comments
        s/\s*#.*$//;
        next if /^\s*$/;   # Skip blank lines
        foreach my $rawType (@types) {
           my $type = lc($rawType);
             # This will convert liboa_soap to oa
           $type =~ s/lib//;
             # This will convert system_user to user
           $type =~ s/system_//;
           $type =~ s/_[^_]*//;
           Message("Checking for $type on line $lineNo");
             # Note that this match forces the opening brace to be
             # on the same line to match....
           if ( (/^\s*handler\s+$rawType\s*\{/i) || (/^\s*$type\s*\{/i) ) {
             # Ignore repetitive entries if they aren't controller or
             #  payload
              next if ( (($type ne "controller") && ($type ne "payload")) && 
                         grep(@foundList, /$type/) );
               # Now read the file to the closing '}' for this data
               #  definition
              while (my $line = <CFG>) {
				  $lineNo++;
					# Get rid fo new line at the end
				  chomp($line);
        			# Get rid of comments
				  $line =~ s/\s*#.*$//;
				  next if /^\s*$/;   # Skip blank lines
					# Done when we encounter the closing brace
				  last if /^\s*\}/;
				  foreach my $datum (@dataTags) {
                    $datum = lc($datum);
                    Message("Checking for $datum on line $lineNo");
					if ($line =~ /^\s*$datum\s*=\s*(.*)\s*$/i) {
						$dataHash->{$type}->{$datum} = $1;
                        next;
                    }
                  }    # End foreach $datum (@dataTags)
              }        # End while ($line = <CFG>
           }  # End if matching type
       
        }     # End foreach $type
   }          # End while (<CFG>) 
   return $dataHash;
}    # End ReadTestDataFile
}    # End static variables for ReadTestDataFile

#
# vim: tabstop=4
#
1;
