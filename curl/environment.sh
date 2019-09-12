curl -v -i \
	--basic -u new:guest \
	-H "Content-Type:application/xml" -H "Accept:application/xml" \
	-X POST  \
	-d @environment.xml  \
	http://rest3api.sifassociation.org/api/environments/environment
