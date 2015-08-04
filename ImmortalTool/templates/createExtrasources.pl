#!/usr/bin/perl -w 

use strict;
use warnings;

if ($#ARGV < 1) {
    print STDERR "you have to specify two arguments: \n";
    print STDERR "  1: filter \n";
    print STDERR "  2: target directory\n";
    exit(1);
}

my $filter = $ARGV[0];
my $dir    = $ARGV[1];

my $output = "$dir/extrasources.make";

opendir(DIR,$dir) || die "could not open $dir for reading";
my @entries = readdir(DIR);
closedir(DIR);

my @selected;
foreach my $entry (@entries) {
  next if $entry =~ /^\./;
  next if $entry =~ /^main/;

  if ($entry =~ /^$filter$/) {
      #print "found: $entry\n";
      push(@selected, $entry);
  } #elsif (-d "$dir/$entry") {
    #  addSubDir($dir, $entry, $filter, \@selected);
  #}
}



if ($#selected > -1) {
    @selected = sort(@selected);

    open (OUT, ">$output") || die "could not open $output for writing";

    for (my $i = 0; $i < $#selected; $i++) {
	print OUT "$selected[$i]\n";
    }
    print OUT "$selected[$#selected]\n";


    close (OUT);
}

sub addSubDir {
    my $basedir = shift;
    my $dir = shift;
    my $filter = shift;
    my $refselected = shift;

    my $subdir = "$basedir/$dir";
    opendir(DIR,$subdir) || die "could not open $subdir for reading";
    my @subentries = readdir(DIR);
    closedir(DIR);
    foreach my $subentry (@subentries) {
	next if $subentry =~ /^\./;
	if ($subentry =~ /^$filter$/) {
	    #print "found: $dir/$subentry\n";
	    push(@{$refselected}, "$dir/$subentry");
	} elsif (-d "$basedir/$dir/$subentry") {
	    addSubDir($basedir, "$dir/$subentry", $filter, $refselected);
	}
    }
}
