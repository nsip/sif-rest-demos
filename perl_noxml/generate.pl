use perl5i::2;

use XML::Pastor;

my $pastor = XML::Pastor->new();
$pastor->generate(
	mode =>'offline',
	style => 'multiple',
	schema=>'sifau_1.3.xsd',
	class_prefix=>'SIF::AU::',
	destination=>'lib',
);  


