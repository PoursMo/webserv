#!bin/bash

##This is the testing script
RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
GRAY='\033[0;37m'
NC='\033[0m'
mkdir -p logs
LOG_FILE='logs/response_request_test.log'
##Launching both server
echo -e "${GRAY}--------------------------Make--------------------------${NC}"
make
echo -e "${GRAY}--------------------------Launching both servers--------------------------${NC}"
echo -e "${GRAY}--------------------------Starting NGINX server---------------------------${NC}"

nginx -c /app/conf/nginx/tester.conf
echo -e "${GRAY}--------------------------Starting OUR server-----------------------------${NC}"
./webserv ./conf/tester.json &
echo -e "${GRAY}--------------------------All servers ready-------------------------------${NC}"
echo "NGINX listening on :    127.0.0.1:8081"
echo "Webserv listening on :  127.0.0.2:8080"
echo "--------------------TEST RESULT LOGS--------------------" > $LOG_FILE
echo -e "${GRAY}--------------------------Starting tests----------------------------------${NC}"

for req in ./tests/requests/*; do
	##basic test
	echo "------------------------------Test $req------------------------------" >> $LOG_FILE
	echo "--------Request-------" >> $LOG_FILE
	cat $req >> $LOG_FILE
	echo "------------------------------------------------------------" >> $LOG_FILE
	echo "Sending to NGINX..."
	echo "---NGINX response---" >> $LOG_FILE
	(cat $req; sleep 1) | telnet 127.0.0.1 8081 > response_NGINX.log
	sed -i '1,3d' response_NGINX.log
	cat response_NGINX.log >> $LOG_FILE
	echo "------------------------------------------------------------" >> $LOG_FILE
	echo "" >> $LOG_FILE
	echo "Sending to Webserv..."
	echo "---Webserv response---" >> $LOG_FILE
	(cat $req; sleep 1) | telnet 127.0.0.2 8080 > response_Webserv.log
	sed -i '1,3d' response_Webserv.log
	cat response_Webserv.log >> $LOG_FILE
	echo "------------------------------------------------------------" >> $LOG_FILE
	echo "" >> $LOG_FILE
	echo "" >> $LOG_FILE
	echo -e "${GRAY}--------------------------Current request:--------------------------------${NC}"
	cat ./$req
	echo -e "${GRAY}--------------------------------------------------------------------------${NC}"
	echo -e "${CYAN}----------------------------------NGINX response----------------------------------"
	cat ./response_NGINX.log
	echo -e "--------------------------------------------------------------------------${NC}"
	echo -e "${PURPLE}----------------------------------Webserv response----------------------------------"
	cat ./response_Webserv.log
	echo -e "--------------------------------------------------------------------------${NC}"
	if diff response_NGINX.log response_Webserv.log > /dev/null;
		then echo -e "${GREEN}THE TWO RESPONSES ARE THE SAME${NC}";
	else
		echo -e "${RED}THE TWO RESPONSES ARE DIFFERENT";
		echo -e "-----------------------------------DIFFERENCE-------------------------------------"
		diff response_NGINX.log response_Webserv.log
		echo -e "--------------------------------------------------------------------------${NC}"
	fi

	echo "" > response_Webserv.log
	echo "" > response_NGINX.log
done