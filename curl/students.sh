# NOTE: export SIFSESION=X taken from sessionToken in Environment Create
curl -v -i \
	--basic -u $SIFSESSION:guest \
	-H "Content-Type:application/xml" -H "Accept:application/xml" \
	-X GET  \
	http://rest3api.sifassociation.org/api/students/
