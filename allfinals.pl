#!/usr/bin/perl -w


# interpret command-line parameters

$cleanup = 0;
$mcifile = "";

while ($#ARGV >= 0) {
  $arg = shift @ARGV;
  if ($arg eq "-f") { $cleanup = 1; }
  else { $mcifile = $arg; }
}
  
usage() unless $mcifile;

# test presence and absence of files;

die "$mcifile not found" unless -r $mcifile;
die "input is not mci file" unless $mcifile =~ /\.mci$/;
$satfile = $mcifile; $satfile =~ s/mci$/sat/;
$resfile = $mcifile; $resfile =~ s/mci$/res/;

cleanup() if $cleanup;
testfile($satfile);
testfile($resfile);

# first step, generate sat formula from mci file
#print "$mcifile\n";
system "./mci2sat -c -f $mcifile" || die "could not execute mci2sat";
die "cannot open $satfile" unless -r $satfile;

# obtain number of (non-cutoff) events and correspondence between
# conditions and places
open FD,$satfile;
while (<FD>) {
  $numev = $1 if /^c event e\d* . var (\d*)/;
  $place{$2} = $1 if /^c condition c\d* \((.*?)\) . var (\d*)/;
}
close FD;

# foreach my $key (keys %place) {
#   my $value = $place{$key};
#   print "Key: $key, Value: $value\n";
# }

# append a commentary to the SAT formula (just for readability)
open FF,">>$satfile";
print FF "\nc previously found configurations that we wish to exclude\n";
close FF;

# repeat the following until the SAT solver comes up with "unsatisfiable"

LABEL:

# execute minisat (and get it to shut up)

system "minisat $satfile $resfile > /dev/null 2> /dev/null "
  || die "could not execute minisat";

die "cannot open $resfile" unless -r $resfile;

# evaluate the result of minisat
open FD,$resfile;
$line = <FD>;

# if there are no more solutions -> quit
if ($line =~ /UNSAT/) { cleanup(); exit 0; }

# otherwise, obtain the satisfying assignment
$line = <FD>;
@vars = split (/ /,$line);

# We generate a blocking clause that forbids the configuration.
# Listing the (positive) events is enough because the configuration is maximal.
# Moreover, we output the corresponding marking (list of marked places).
@events = ();
@places = ();

for $i (@vars) {
  next unless $i > 0;
  push @events,-$i if $i <= $numev;
  push @places,$place{$i} if $i > $numev;
}

# append the blocking clause to the sat formula
# FIXME: I'm not updating the number of clauses in the DIMACS header.
# 	 However, minisat can deal with that. Other solvers might not.
open FF,">>$satfile";
print FF join(" ",@events)," 0\n";
close FF;

# print marking to stdout
print join(" ",@places),"\n";

# and repeat
goto LABEL;

sub usage {
  print STDERR "Syntax: allfinals.pl [-f] example.mci\n";
  print STDERR "Outputs all markings of final configurations\n";
  print STDERR "The script requires minisat and mci2sat.\n";
  print STDERR "The -f option removes example.mci and example.sat ",
      "before starting.\n";
  exit 0;
}

sub cleanup {
  unlink ($satfile,$resfile);
}

sub testfile {
  my $file = shift;
  die "$file is present; remove or start script with -f" if -e $file;
}
