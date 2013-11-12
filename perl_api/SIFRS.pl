#!/usr/bin/perl
use perl5i::2;
use SIF::REST;

my $sifrest = SIF::REST->new({
	endpoint => 'http://rest3api.sifassociation.org/api',
});
$sifrest->setupRest();
print $sifrest->get('students');
