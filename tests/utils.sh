#!/bin/bash

SRC_DIR="./srcs"
LOGS_DIR="logs"
LEAKS_FILE="leaks.log"

mkdir -p "$LOGS_DIR"

valg() {
	local VALGRIND="valgrind"
	local VALGRIND+=" --leak-check=full"
	local VALGRIND+=" --track-fds=yes"
	local VALGRIND+=" --show-leak-kinds=all"
	local VALGRIND+=" --track-origins=yes"
	local VALGRIND+=" --log-file=$LEAKS_FILE"
	#local VALGRIND+=" --trace-children=yes"

	$VALGRIND "$@"
}

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
	LEAKS_DETECTED=$(cat $LEAKS_FILE | grep "ERROR SUMMARY" | awk '{printf "%s", $4}' | tr -d "0")
	LEAKS_FILE_NAME="$LOGS_DIR/$1_leaks.log"
	mv $LEAKS_FILE $LEAKS_FILE_NAME
	if [[ $LEAKS_DETECTED == "" ]] ; then
		success "LEAKS\t[$NAME]\tOK"
	else
		error "LEAKS\t[$NAME]\t$LEAKS_FILE_NAME"
	fi
}

get_diff() {
	local TEST_NAME=$1
	local FILE_A=$2
	local FILE_B=$3
	local FILE_OUT="$LOGS_DIR/$TEST_NAME.diff"

	if [[ TEST_NAME == "" || FILE_A == "" || FILE_B == "" ]] ; then
		warning "get_diff() need tree args. TEST_NAME FILE_A AND FILE_B"
		return 1
	fi

	local DIFF=$(diff -U 2 $FILE_A $FILE_B)
	echo "$DIFF" > "$FILE_OUT"

	if [[ $DIFF == "" ]] ; then
		success "DIFF\t[$TEST_NAME]\t\tOK"
	else
		error "DIFF DETECTED [$TEST_NAME]\t$FILE_OUT"
		info "$FILE_A | $FILE_B"
	fi
}

get_prog_name() {
	cat Makefile | grep "^NAME " | cut -d'=' -f2 | xargs
}

compilation() {
	PROG_NAME=$(get_prog_name)

	info "\nCompilation of '$PROG_NAME'..."
	make re > /dev/null
	if [ ! -x $PROG_NAME ] ; then
		error "COMPILATION FAILED"
		info "Executable '$PROG_NAME' not found"
		return 1
	fi
	success "COMPILATION OK"
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

