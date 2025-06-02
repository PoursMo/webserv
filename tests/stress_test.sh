#!/bin/bash
ADDRESS_PORT="$1"
NUMBER_OF_TEST="$2"

REQUEST_DIR="../http"
for ((i = 0; i < $NUMBER_OF_TEST; i++)); do
	REQUEST_FILE=$(find "$REQUEST_DIR" -type f | shuf -n 1);
	nc $ADDRESS_PORT < "$REQUEST_FILE"
done