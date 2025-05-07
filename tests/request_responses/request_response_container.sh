#!/bin/bash

#Create folders and copy configs et script
docker exec -it request_response_test mkdir scripts
docker exec -it request_response_test mkdir config
docker cp ./test_config/. request_response_test:/app/config/

#