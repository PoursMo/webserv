#!/bin/bash

source ./tests/utils.sh

if compilation ; then
	rm -rf $LOGS_DIR
	# ./tests/json.sh
	# ./tests/conf.sh
	./tests/server.sh locations
fi
