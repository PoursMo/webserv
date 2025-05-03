#!/bin/bash

cd $(dirname ${BASH_SOURCE[0]})
source ../utils.sh

test_json() {
	json_files_generate
	each_json_file jq . > res_jq.log
	each_json_file ./json.test > res_parser.log
	rm -f *.json
	get_diff json res_jq.log res_parser.log
	return $?
}

each_json_file() {
	local COMMAND=$1
	for JSON_FILE in *.json ; do
		echo -ne "$JSON_FILE:\t"
		cat $JSON_FILE | $COMMAND &> /dev/null
		echo $? $(cat $JSON_FILE)
	done
}

json_files_generate() {
	local num=$((0))
	while IFS= read -r line; do
		echo $line > "$(printf %03d $num).json"
		((num++))
	done < lines.txt
}

test_json
