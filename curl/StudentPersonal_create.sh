#/bin/sh

#curl -v -i -H "Content-Type:application/xml" -H "Accept:application/xml" -X POST -d @inputs/post_StudentPersonals_StudentPersonal.xml http://localhost:3000/StudentPersonals/StudentPersonal
curl -v -i -H "Content-Type:application/xml" -H "Accept:application/xml" -X POST -d @inputs/post_StudentPersonals_StudentPersonal.xml http://siftraining.dd.com.au/api/StudentPersonals/StudentPersonal
