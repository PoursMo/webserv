#!/bin/bash
container_id=$(docker ps -a -qf "name=^request_response_test$")

docker stop "$container_id"
docker container rm "$container_id"