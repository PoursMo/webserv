#!/bin/bash

cd $(dirname ${BASH_SOURCE[0]})
source ../utils.sh

test_json() {
	json_files_generate

	each_json_file jq . > res_jq.log
	each_json_file ./json.test > res_parser.log

	$VALGRIND ./json.test 058.json &> /dev/null
	check_leaks json_good

	$VALGRIND ./json.test 037.json &> /dev/null
	check_leaks json_bad

	rm -f *.json
	get_diff json res_jq.log res_parser.log
	return $?
}

each_json_file() {
	local COMMAND=$1
	for JSON_FILE in *.json ; do
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

json_files_generate() {
	local num=$((1))
	while IFS= read -r line; do
		echo $line > "$(printf %03d $num).json"
		((num++))
	done < lines.txt
}

test_json
