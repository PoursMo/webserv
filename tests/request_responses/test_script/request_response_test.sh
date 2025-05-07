#!bin/bash

##This is the testing script
RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
GRAY='\033[0;37m'
NC='\033[0m'
##Launching both server
echo -e "${GRAY}--------------------------Launching both servers--------------------------${NC}"
	echo -e "${GRAY}--------------------------Starting NGINX server---------------------------${NC}"
	service nginx start

	echo -e "${GRAY}--------------------------Starting OUR server-----------------------------${NC}"
	./webserv ./config/tester.json &
	echo -e "${GRAY}--------------------------All servers ready-------------------------------${NC}"
	echo "NGINX listening on :    127.0.0.1:8081"
	echo "Webserv listening on :  127.0.0.2:8080"

test_files=(request/*)
echo "--------------------TEST RESULT LOGS--------------------" > test_logs.txt
echo -e "${GRAY}--------------------------Starting tests----------------------------------${NC}"
for i in ${test_files[@]}
{
	##basic test
	echo "------------------------------Test $i------------------------------" >> test_logs.txt
	echo "--------Request-------" >> test_logs.txt
	cat $i >> test_logs.txt
	echo "------------------------------------------------------------" >> test_logs.txt
	echo "Sending to NGINX..."
	echo "---NGINX response---" >> test_logs.txt
	(cat ./$i; sleep 1) | telnet 127.0.0.1 8081 > response_NGINX.txt
	sed -i '1,3d' response_NGINX.txt
	cat response_NGINX.txt >> test_logs.txt
	echo "------------------------------------------------------------" >> test_logs.txt
	echo "" >> test_logs.txt
	echo "Sending to Webserv..."
	echo "---Webserv response---" >> test_logs.txt
	(cat ./$i; sleep 1) | telnet 127.0.0.2 8080 > response_Webserv.txt
	sed -i '1,3d' response_Webserv.txt
	cat response_Webserv.txt >> test_logs.txt
	echo "------------------------------------------------------------" >> test_logs.txt
	echo "" >> test_logs.txt
	echo "" >> test_logs.txt
	echo -e "${GRAY}--------------------------Current request:--------------------------------${NC}"
	cat ./$i
	echo -e "${GRAY}--------------------------------------------------------------------------${NC}"
	echo -e "${CYAN}----------------------------------NGINX response----------------------------------"
	cat ./response_NGINX.txt
	echo -e "--------------------------------------------------------------------------${NC}"
	echo -e "${PURPLE}----------------------------------Webserv response----------------------------------"
	cat ./response_Webserv.txt
	echo -e "--------------------------------------------------------------------------${NC}"
	if diff response_NGINX.txt response_Webserv.txt > /dev/null;
		then echo -e "${GREEN}THE TWO RESPONSES ARE THE SAME${NC}";
	else
		echo -e "${RED}THE TWO RESPONSES ARE DIFFERENT";
		echo -e "-----------------------------------DIFFERENCE-------------------------------------"
		diff response_NGINX.txt response_Webserv.txt
		echo -e "--------------------------------------------------------------------------${NC}"
	fi
}