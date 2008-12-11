#!/usr/bin/perl
$|++;  #flush stdoutput
use File::Copy;
use Sys::Hostname;
my $host = hostname();
my $srcFileName = "/etc/hosts";
my $outFileName = "/etc/hosts.new";
my $ipAddress = $ARGV[0];
my $numFound = 0;
my $args = @ARGV;
my $fileModified = 0;


if (!($args)) {
	print ("Usage: perl alignEtcHosts.pl <IP Address>\n");
	exit 0;
}
print ("ipAddress = $ipAddress\n");
print ("Hostname = $host\n");

open (INFILE, $srcFileName) || die "Cannot open $srcFileName: $!\n";

#Grep to see if you have <IP ADDRESS  HOSTNAME.**** appears twice###
while (<INFILE>) {
	if (/^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\s+$host/) {
		$numFound += 1;
	}
	last if (/^#<Ip-addr>/);
}
print ("numFound = $numFound\n");

#Exit of <IP ADDRESS  HOSTNAME.**** appears only once #####
if ($numFound < 2) {
	close (INFILE);
	close (OUTFILE);
	exit 0;
}
#Rewind the file handle to the beginning of the file
seek (INFILE, 0, 0);

#Store the stings in an array; Assuming that there are only two occurences
while (<INFILE>) { 
	if (/^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\s+$host\.?/) {
		push(@stringArray, $_);
	}
}

print ("$stringArray[0]");
print ("$stringArray[1]");

#Modify the file so that the required string the first line
#Write the output to a new file and then move the new file to the actual file
open (OUTFILE, ">$outFileName") || die "Cannot open $outFileName: $!\n";
seek (INFILE, 0, 0);
while (<INFILE>) {
	if ($stringArray[0] =~ /^$ipAddress/) {
		print ("File $srcFileName is Ok\n");
		last;
	}
	$fileModified += 1;
	if (/$stringArray[0]/) {
		print OUTFILE $stringArray[1];
	} elsif (/$stringArray[1]/) {
		print OUTFILE $stringArray[0];
	} else {
		print OUTFILE $_;
	}
}
close (INFILE);
close (OUTFILE);

#If modified then Move the new file to the actual file
#If file is ok then Don't modify the source file and delete the new file created.
if ($fileModified) {
	print ("File $srcFileName is Modified\n");
	move ($outFileName, $srcFileName);
} else {
	unlink $outFileName;
}
