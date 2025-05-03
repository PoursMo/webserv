#!/bin/bash

source ./tests/utils.sh

run_tests() {
	make fclean > /dev/null

	info "TEST JSON PARSING"
	make tests/json/json.test > /dev/null
	./tests/json/json.sh

	info "TEST CONFIG"
	warning "TODO"

}

run_tests