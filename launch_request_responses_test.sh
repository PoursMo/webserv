#!/bin/bash
## This is used to launch the start the request response test container.sh##


## building and running docker image
if [[ $(docker images -q test/nginx) == "" ]];
	then docker build -t test/nginx .
fi
docker run --rm -v .:/app test/nginx ./request_response_test.sh
rm -f response_NGINX.log
rm -f response_Webserv.log

# echo "-----Stopping and removing containers-----"
# echo "Finished."
# echo "Logs available at \"./test_logs.txt\""