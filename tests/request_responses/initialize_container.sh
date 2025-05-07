#!/bin/bash
## This is used to launch the start the request response test container.sh##

## building and running docker image
docker build -t test/nginx .
docker run -dit --name request_response_test test/nginx