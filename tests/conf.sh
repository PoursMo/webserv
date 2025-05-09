#!/bin/bash

source ./tests/utils.sh

test_config() {

	info "\nTests configurations parsing"

	local PROG="./tests/conf.test"
	rm -f $PROG
	make $PROG > /dev/null
	if [ ! -x $PROG ] ; then
		error "PROGRAME TEST COMPILATION FAILED: $PROG"
		return 1
	fi

	for CONF_FILE in tests/bad-conf/*.json ; do
		valg $PROG $CONF_FILE 2> /dev/null
		check_leaks $(basename $CONF_FILE .json)
	done
}

test_config
