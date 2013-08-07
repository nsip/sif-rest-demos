#!/usr/bin/perl
use HTTP::Request::Common qw(GET POST);  
use LWP::UserAgent; 
my $ua = LWP::UserAgent->new();  

$create_xml = q{<environment>
  <solutionId>testSolution</solutionId>
  <authenticationMethod>Basic</authenticationMethod>
  <instanceId>Instance</instanceId>
  <userToken>User</userToken>
  <consumerName>Perl example</consumerName>
  <applicationInfo>
    <applicationKey>Perl</applicationKey>
    <supportedInfrastructureVersion>3.0</supportedInfrastructureVersion>
    <supportedDataModel>SIF-US</supportedDataModel>
    <supportedDataModelVersion>3.0</supportedDataModelVersion>
    <transport>REST</transport>
    <applicationProduct>
      <vendorName>Vendor</vendorName>
      <productName>Product</productName>
      <productVersion>Version</productVersion>
    </applicationProduct>
  </applicationInfo>
</environment>};

my $req = POST 
	'http://rest3api.sifassociation.org/api/environments/environment', 
	Content_Type => 'application/xml',
	Accept => 'application/xml',
	Content => $create_xml,
;
$req->authorization_basic('new', 'guest');
my $response = $ua->request($req);
print $response->as_string; 
