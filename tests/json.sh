#!/bin/bash

source ./tests/utils.sh

test_json() {

	info "\nTests json parsing"
	make tests/json.test > /dev/null

	each_json_file "jq ." > $LOGS_DIR/res_jq.log
	each_json_file "./tests/json.test" > $LOGS_DIR/res_parser.log
	get_diff json $LOGS_DIR/res_jq.log $LOGS_DIR/res_parser.log

	valg ./tests/json.test ./tests/json/058.json &> /dev/null
	check_leaks json_good

	valg ./tests/json.test ./tests/json/037.json &> /dev/null
	check_leaks json_bad

	return $?
}

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

test_json

