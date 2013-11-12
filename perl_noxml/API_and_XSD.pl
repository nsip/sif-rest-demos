#!/usr/bin/perl
use perl5i::2;
use SIF::REST;
use SIF::AU;

# PUPOSE: Create a School using Data Objects

my $tg = SIF::AU::TeachingGroup->new();
$tg->SchoolYear('2007');
$tg->ShortName('P');
$tg->LongName('Prep');

print "BEFORE\n" . $tg->to_xml_string();
$tg = _create($tg);
print "AFTER\n" . $tg->to_xml_string();

exit 0;

# ======================================================================
# CREATE form an Object and return the object
sub _create {
	my ($obj) = @_;
	# TODO support Multiple create
	
	my $class = ref($obj);
	my $name = $class;
	$name =~ s/^SIF::AU:://g;

	# POST / CREATE
	my $xml;
	my $ret = eval {
		$xml = _rest()->post($name . 's', $name, $obj->to_xml_string());
		return $class->from_xml($xml);
	};
	if ($@) {
		die "ERROR $@. Original XML = $xml\n";
	}
	return $ret;
}

sub _rest {
	our $sifrest;
	if (! $sifrest) {
		# SIF REST Client
		$sifrest = SIF::REST->new({
			endpoint => 'http://siftraining.dd.com.au/api',
		});
		$sifrest->setupRest();
	}
	return $sifrest;
}

