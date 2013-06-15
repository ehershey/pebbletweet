#!/usr/bin/perl
use CGI qw(header);
use Data::Dumper;

my $LOG = "./pebble.log";
open (LOG,">> $LOG") || die($!);
print header;
print "<html><body><pre>\n";
print Dumper(\%ENV);
print LOG Dumper(\%ENV);
print "</pre></body></html>\n";
close(LOG);
