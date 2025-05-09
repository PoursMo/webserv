#!/bin/bash

source ./tests/utils.sh

info "TODO: test server"

start_nginx() {
	nginx -c /app/tests/nginx/default.conf -g "daemon off;"
}
