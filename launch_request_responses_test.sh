#!/bin/bash
## This is used to launch the start the request response test container.sh##


## building and running docker image
if [[ $(docker images -q test/nginx) == "" ]];
	then docker build -t test/nginx .
fi
# docker run --rm -v .:/app --name request_response_test test/nginx >/dev/null

# echo "-----Stopping and removing containers-----"
# echo "Finished."
# echo "Logs available at \"./test_logs.txt\""