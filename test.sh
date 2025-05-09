#!/bin/bash

source ./tests/utils.sh

run_tests() {

	compilation
	./tests/json.sh

}

run_tests