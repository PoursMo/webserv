#!/bin/bash
ADDRESS="$1"
PORT="$2"
NUMBER_OF_TEST="$3"

REQUEST_DIR="./http"
for ((i = 0; i < $NUMBER_OF_TEST; i++)); do
	REQUEST_FILE=$(find "$REQUEST_DIR" -type f | shuf -n 1);
	(cat $REQUEST_FILE; sleep 0.001) | telnet $ADDRESS $PORT
done