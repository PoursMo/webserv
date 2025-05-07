#!/bin/bash
## This is used to launch the start the request response test container.sh##


## building and running docker image
make -C ../../.
docker build -t test/nginx . >/dev/null 2>&1
docker run -dit --name request_response_test test/nginx >/dev/null 2>&1

##copy files to container
echo "-----Copying files-----"
docker exec -it request_response_test mkdir scripts >/dev/null 2>&1
docker exec -it request_response_test mkdir config >/dev/null 2>&1
docker exec -it request_response_test mkdir request >/dev/null 2>&1
docker cp ./test_config/. request_response_test:/app/config/ >/dev/null 2>&1
docker cp ./test_request/. request_response_test:/app/request/ >/dev/null 2>&1
docker cp ./test_script/request_response_test.sh request_response_test:/app/scripts/ >/dev/null 2>&1
docker cp ../../webserv request_response_test:/app/ >/dev/null 2>&1

##add nginx_custom config to container
echo "-----Adding NGINX custom config files-----"
docker rm request_response_test:/etc/nginx/nginx.conf >/dev/null 2>&1
docker cp ./nginx_config/nginx.conf request_response_test:/etc/nginx/nginx.conf >/dev/null 2>&1

sleep 1

##start tests
docker exec request_response_test bash /app/scripts/request_response_test.sh

container_id=$(docker ps -a -qf "name=^request_response_test$")

echo "-----Retrieving logs-----"
docker cp request_response_test:/app/test_logs.txt ./test_logs.txt >/dev/null 2>&1
echo "-----Stopping and removing containers-----"
docker stop "$container_id" >/dev/null 2>&1
docker container rm "$container_id" >/dev/null 2>&1
echo "Finished."
echo "Logs available at \"./test_logs.txt\""