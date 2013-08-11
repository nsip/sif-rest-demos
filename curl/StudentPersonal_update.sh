#/bin/sh

curl -v -i -H "Content-Type:application/xml" -H "Accept:application/xml" -X PUT -d @inputs/put_StudentPersonals_X.xml http://localhost:3000/StudentPersonals/$1
