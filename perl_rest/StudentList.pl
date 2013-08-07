#!/usr/bin/perl
use perl5i::2;
use REST::Client;
use MIME::Base64;
use XML::Simple;

my $config = {
	endpoint => 'http://rest3api.sifassociation.org/api',
	consumerKey => 'new',
	consumerSecret => 'guest',
};

my $client = REST::Client->new();
$client->setHost($config->{endpoint});
$client->addHeader(Content_Type => 'application/xml');
$client->addHeader(Accept => 'application/xml');

# ------------------------------------------------------------------------------
# AUTHENTICATION
# ------------------------------------------------------------------------------
my $create_xml = q{<environment><solutionId>testSolution</solutionId><authenticationMethod>Basic</authenticationMethod><consumerName>Perl</consumerName><applicationInfo><applicationKey>PERL</applicationKey><supportedInfrastructureVersion>3.0</supportedInfrastructureVersion><supportedDataModel>SIF-US</supportedDataModel><supportedDataModelVersion>3.0</supportedDataModelVersion><transport>REST</transport><applicationProduct><vendorName>X</vendorName><productName>X</productName><productVersion>X</productVersion></applicationProduct></applicationInfo></environment>};

$client->POST(
	'environments/environment', 
	$create_xml, 
	{
		'Authorization' => 'Basic '. encode_base64($config->{consumerKey} . ':' . $config->{consumerSecret}),
	}
);
#say $client->responseContent();
my $ref = XMLin($client->responseContent(), ForceArray => 0);
my $key = $ref->{sessionToken};
say "'$key'";
$client->addHeader('Authorization', 'Basic '. encode_base64($key . ':' . $config->{consumerSecret}));

# ------------------------------------------------------------------------------
# STUDENT LIST
# ------------------------------------------------------------------------------
$client->GET('students');
my $students = XMLin($client->responseContent(), ForceArray => 0);
print join("\n", map { $_->{name}{nameOfRecord}{fullName} } @{$students->{student}} ) . "\n";
