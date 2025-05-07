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
cd app
echo -e "${GRAY}--------------------------Launching both servers--------------------------${NC}"
echo -e "${GRAY}--------------------------Starting NGINX server---------------------------${NC}"
service nginx start

echo -e "${GRAY}--------------------------Starting OUR server-----------------------------${NC}"
./webserv ./config/tester.json &
echo -e "${GRAY}--------------------------All servers ready-------------------------------${NC}"
echo "NGINX listening on :    127.0.0.1:8081"
echo "Webserv listening on :  127.0.0.2:8080"
echo -e "${GRAY}--------------------------Starting tests----------------------------------${NC}"
##basic test
echo "Sending to NGINX..."
(cat ./request/basic_request; sleep 1) | telnet 127.0.0.1 8081 > response_NGINX.txt
sed -i '1,3d' response_NGINX.txt
echo "Sending to Webserv..."
(cat ./request/basic_request; sleep 1) | telnet 127.0.0.2 8080 > response_Webserv.txt
sed -i '1,3d' response_Webserv.txt
echo -e "${GRAY}--------------------------Current request:--------------------------------${NC}"
cat ./request/basic_request
echo -e "${GRAY}--------------------------------------------------------------------------${NC}"
echo -e "${CYAN}----------------------------------NGINX response----------------------------------"
cat ./response_NGINX.txt
echo -e "--------------------------------------------------------------------------${NC}"
echo -e "${PURPLE}----------------------------------Webserv response----------------------------------"
cat ./response_Webserv.txt
echo ""
echo -e "--------------------------------------------------------------------------${NC}"
if diff response_NGINX.txt response_Webserv.txt > /dev/null;
	then echo -e "${GREEN}THE TWO RESPONSES ARE THE SAME${NC}";
else
	echo -e "${RED}THE TWO RESPONSES ARE DIFFERENT";
	echo -e "-----------------------------------DIFFERENCE-------------------------------------"
	diff response_NGINX.txt response_Webserv.txt
	echo -e "--------------------------------------------------------------------------${NC}"
fi