#!/usr/bin/perl
use perl5i::2;
use SIF::REST;

my $sifrest = SIF::REST->new({
	endpoint => 'http://siftraining.dd.com.au/api',
});
$sifrest->setupRest();
print $sifrest->get('SchoolInfos');

