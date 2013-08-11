#/bin/sh

curl -v -i -H "Content-Type:application/xml" -H "Accept:application/xml" -X GET http://localhost:3000/StudentPersonals/$1
