#!/bin/bash

if [[ $(docker images -q webserv) == "" ]]; then
	docker build -t webserv .
fi

docker run --rm -v .:/app webserv ./test.sh
