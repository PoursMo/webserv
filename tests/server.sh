#!/bin/bash

# curl http://localhost/index.html --trace-ascii
# siege
# stress

source ./tests/utils.sh
mkdir -p $LOGS_DIR/http
mkdir -p $LOGS_DIR/nginx
mkdir -p $LOGS_DIR/webserv
DEFAULT_ADDRESS="127.0.0.1"
DEFAULT_PORT="8000"

main() {

	info "\nTests server responses"

	for FILE_CONF in conf/*.json ; do
		local FILE_CONF_NG="$(pwd)/conf/nginx/$(basename $FILE_CONF .json).conf"
		if [ ! -f "$FILE_CONF_NG" ] ; then
			continue
		fi

		nginx -c $FILE_CONF_NG &
		local PID=$!
		info "NGINX STARTED conf=[$FILE_CONF_NG] pid=[$PID]"
		send_all_request $FILE_CONF "nginx"
		kill $PID
		info "NGINX STOPPED"

		./webserv $FILE_CONF &> /dev/null &
		local PID=$!
		info "WEBSERV STARTED conf=[$FILE_CONF] pid=[$PID]"
		send_all_request $FILE_CONF "webserv"
		kill $PID
		info "WEBSERV STOPPED"

	done
}

send_all_request() {
	local FILE_CONF=$1
	local SERVER_NAME=$2
	local CONFIG_NAME=$(basename $FILE_CONF .json)

	local ADDRESSES=$(cat $FILE_CONF | jq -M '.servers[] | .address' | tr -d '"')
	local index=$((0))
	for ADDRESS in $ADDRESSES ; do
		local PORT=$(cat $FILE_CONF | jq -M ".servers[$index] | .port")
		if [[ "$ADDRESS" = "null" ]] ; then
			ADDRESS="$DEFAULT_ADDRESS"
		fi
		if [[ "$PORT" = "null" ]] ; then
			PORT="$DEFAULT_PORT"
		fi
		info "SEND ALL REQUEST TO SERVER[$index]: $ADDRESS:$PORT"
		for FILE_HTTP in tests/http/*.http ; do
			local REQ_NAME=$(basename $FILE_HTTP .http)
			local OUTPUT_FILE="${LOGS_DIR}/http/${CONFIG_NAME}_${index}_${REQ_NAME}_${SERVER_NAME}.log"
			send $FILE_HTTP $ADDRESS $PORT > $OUTPUT_FILE
			if [ $? -eq 0 ] ; then
				info "$(printf "%-16s" "│$REQ_NAME") │ $OUTPUT_FILE"
			else
				waring "NO RESPONSE FROM $2:$3"
			fi
		done
		((index++))
	done
}

send() {
	cat $1 | nc -q 0 $2 $3
	# (cat $1; sleep 0.2) | telnet $2 $3

	if [ $? -ne 0 ] ; then
		echo "NO RESPONSE FROM $2:$3"
		return 1
	fi
}

main
