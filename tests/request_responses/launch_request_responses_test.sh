#!/bin/bash
## This is used to launch the start the request response test container.sh##

## building and running docker image
docker build -t test/nginx .
docker run -dit --name request_response_test test/nginx

##copy files to container
docker exec -it request_response_test mkdir scripts
docker exec -it request_response_test mkdir config
docker exec -it request_response_test mkdir request
docker cp ./test_config/. request_response_test:/app/config/
docker cp ./test_request/. request_response_test:/app/request/
docker cp ./test_script/request_response_test.sh request_response_test:/app/scripts/
docker cp ../../webserv request_response_test:/app/

##add nginx_custom config to container
docker rm request_response_test:/etc/nginx/nginx.conf
docker cp ./nginx_config/nginx.conf request_response_test:/etc/nginx/nginx.conf

sleep 1

##start tests
docker exec request_response_test bash /app/scripts/request_response_test.sh

container_id=$(docker ps -a -qf "name=^request_response_test$")

docker cp request_response_test:/app/test_logs.txt ./test_logs.txt
docker stop "$container_id"
docker container rm "$container_id"