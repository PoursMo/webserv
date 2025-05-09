#!/bin/bash

source ./tests/utils.sh

info "\nTests configurations parsing"
mkdir -p "$LOGS_DIR/conf"

PROG="./tests/conf.test"
rm -f $PROG
make $PROG > /dev/null
if [ ! -x $PROG ] ; then
	error "PROGRAME TEST COMPILATION FAILED: $PROG"
	return 1
fi

for CONF_FILE in tests/bad-conf/*.json ; do
	valg $PROG $CONF_FILE 2> /dev/null
	if [ $? -eq 0 ] ; then
		warning "ERROR NOT CATCHED\t$CONF_FILE"
	fi
	check_leaks "conf/bad-$(basename $CONF_FILE .json)"
done

for CONF_FILE in conf/*.json ; do
	valg $PROG $CONF_FILE 2> /dev/null
	check_leaks "conf/$(basename $CONF_FILE .json)"
done

