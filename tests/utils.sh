#!/bin/bash

SRC_DIR="./srcs"

VALGRIND="valgrind"
VALGRIND+=" --leak-check=full"
VALGRIND+=" --track-fds=yes"
VALGRIND+=" --show-leak-kinds=all"
VALGRIND+=" --track-origins=yes"
VALGRIND+=" --log-file=leaks.log"
#VALGRIND+=" --trace-children=yes"

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
	NAME=$1
	LEAKS_DETECTED=$(cat ./leaks.log | grep "ERROR SUMMARY" | awk '{printf "%s", $4}' | tr -d "0")
	LEAKS_FILE="$1_leaks.log"
	mv ./leaks.log "$LEAKS_FILE"
	if [[ $LEAKS_DETECTED == "" ]] ; then
		success "LEAKS [$NAME]\tOK"
	else
		error "LEAKS [$NAME]\tERROR\t$LEAKS_FILE"
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

array_contains () { 
    local array="$1[@]"
    local seeking=$2
    local in=1
    for element in "${!array}"; do
        if [[ $element == "$seeking" ]]; then
            in=0
            break
        fi
    done
    return $in
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

