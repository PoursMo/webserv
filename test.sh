#!/bin/bash

each_json_file() {
	local COMMAND=$1
	for JSON_FILE in tests/json/*.json ; do
		echo -ne "$JSON_FILE:\t"
		cat $JSON_FILE | $COMMAND . &> /dev/null
		echo $?
	done
}

json_files_generate() {
	local num=$((0))
	while IFS= read -r line; do
		echo $line > "tests/json/$(printf %03d $num).json"
		((num++))
	done < tests/json/lines.txt
}

json_files_clean() {
	rm tests/json/*.json
}

test() {
	mkdir -p log
	json_files_generate


	# run jq on all json and print exit status
	each_json_file jq > log/res_jq.log

	# TODO: run our json parser and compare result
	# each_json_file ./tests/json/parser > log/res_parser.log
	# diff log/res_jq.log log/res_parser.log

	if [[ $1 != "" ]] ; then
		json_files_clean
	fi
}

test $@
