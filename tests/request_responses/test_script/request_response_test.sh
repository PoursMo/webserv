#!bin/bash

##This is the testing script

##Launching both server
cd app
echo "--------------------------Launching both servers--------------------------"
echo "--------------------------Starting NGINX server---------------------------"
service nginx start

echo "--------------------------Starting OUR server-----------------------------"
./webserv ./config/tester.json &
echo "--------------------------All servers ready-------------------------------"
echo "NGINX listening on :    127.0.0.1:8081"
echo "Webserv listening on :  127.0.0.1:8080"

##basic test
cat ./request/basic_request | nc -v 127.0.0.1 8081 > log_1.txt &
cat ./request/basic_request | nc -v 127.0.0.1 8080 > log_2.txt &
if diff log_1.txt log_2.txt > /dev/null;
	then echo "success";
else
	echo "failure";
fi