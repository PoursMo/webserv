#!/bin/bash

source ./tests/utils.sh

each_json_file() {
	local COMMAND=$1
	for JSON_FILE in tests/json/*.json ; do
		echo -ne "$JSON_FILE:\t"
		JSON=$(cat $JSON_FILE)
		if [[ "$JSON" == '' || "$JSON" == '{}{}' || "$JSON" == '[][]' || "$JSON" == '{"key":[], "key":[]}' ]]; then
			echo "IGNORED $JSON"
			continue
		fi
		echo $JSON | $COMMAND &> /dev/null
		echo -e "$?\t$JSON"
	done
}

info "\nTests json parsing"
PROG="./tests/json.test"
make $PROG > /dev/null
if [ ! -x $PROG ] ; then
	error "PROGRAME TEST COMPILATION FAILED: $PROG"
	exit 1
fi

mkdir -p "$LOGS_DIR/json"

each_json_file "jq ." > $LOGS_DIR/json/parser_jq.log
each_json_file "$PROG" > $LOGS_DIR/json/parser.log
get_diff json/parser $LOGS_DIR/json/parser_jq.log $LOGS_DIR/json/parser.log

valg $PROG ./tests/json/058.json &> /dev/null
check_leaks json/good

valg $PROG ./tests/json/037.json &> /dev/null
check_leaks json/bad
