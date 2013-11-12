#!/usr/bin/perl
use perl5i::2;
use SIF::AU;

my $tg = SIF::AU::TeachingGroup->new();
$tg->RefId('abc123');
$tg->SchoolYear('2007');
$tg->LocalId('123');
$tg->ShortName('P');
$tg->LongName('Prep');

say $tg->to_xml_string();

