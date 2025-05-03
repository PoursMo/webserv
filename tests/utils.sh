#!/bin/bash

SRC_DIR="./srcs"

LEAKS_CMD="valgrind"
LEAKS_CMD+=" --leak-check=full"
LEAKS_CMD+=" --track-fds=yes"
LEAKS_CMD+=" --show-leak-kinds=all"
LEAKS_CMD+=" --track-origins=yes"
LEAKS_CMD+=" --log-file=leaks.log"
#LEAKS_CMD+=" --trace-children=yes"

sync_sources() {
	SOURCES=$(ls $SRC_DIR/*.c $SRC_DIR/**/*.c | sed "s;$SRC_DIR/;;g" | tr '\n' ' ')
	SED_COMMAND="s;^SOURCES	.*;SOURCES			=	$SOURCES;"
	if [ $(uname) = "Linux" ];  then
		sed -i -e "$SED_COMMAND" "./Makefile"
	else
		sed -i "" "$SED_COMMAND" "./Makefile"
	fi
}

check_leaks() {
	LEAKS_DETECTED=$(cat ./leaks.log | grep "ERROR SUMMARY" | awk '{printf "%s", $4}' | tr -d "0")
	if [[ $LEAKS_DETECTED == "" ]] ; then
		success "LEAKS\t\tOK"
	else
		error "LEAKS\t\tERROR\t./leaks.log"
	fi
}

get_diff() {
	local TEST_NAME=$1
	local FILE_A=$2
	local FILE_B=$3
	local FILE_OUT="$TEST_NAME.diff"

	if [[ TEST_NAME == "" || FILE_A == "" || FILE_B == "" ]] ; then
		warning "get_diff() need tree args. TEST_NAME FILE_A AND FILE_B"
		return 1
	fi

	local DIFF=$(diff -U 2 $FILE_A $FILE_B)
	echo "$DIFF" > "$FILE_OUT"

	if [[ $DIFF == "" ]] ; then
		success "DIFF\t\tOK"
	else
		error "DIFF\t\tERROR\t$(pwd)/$FILE_OUT"
	fi

}

info() {
	echo -e "\033[36m$1\033[0m"
}

error() {
	echo -e "\033[31m$1\033[0m"
}

success() {
	echo -e "\033[32m$1\033[0m"
}

warning() {
	echo -e "\033[33m$1\033[0m"
}

