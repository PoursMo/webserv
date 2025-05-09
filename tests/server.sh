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

	info "\nTests http request\n"

	for FILE_CONF in conf/*.json ; do
		local FILE_CONF_NG="$(pwd)/conf/nginx/$(basename $FILE_CONF .json).conf"
		if [ ! -f "$FILE_CONF_NG" ] ; then
			continue
		fi

		nginx -c $FILE_CONF_NG &
		local PID=$!
		info "[NGINX] $FILE_CONF_NG"
		send_all_request $FILE_CONF "nginx"
		kill $PID
		info "[NGINX]\n"

		./webserv $FILE_CONF &> /dev/null &
		local PID=$!
		info "[WEBSERV] $FILE_CONF"
		send_all_request $FILE_CONF "webserv"
		kill $PID
		info "[WEBSERV]\n"
	done

	compare_response
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
		info "│ server[$index] $ADDRESS:$PORT"
		for FILE_HTTP in tests/http/*.http ; do
			local REQ_NAME=$(basename $FILE_HTTP .http)
			local OUTPUT_FILE="${LOGS_DIR}/http/${CONFIG_NAME}_${index}_${REQ_NAME}_${SERVER_NAME}.log"
			send $FILE_HTTP $ADDRESS $PORT > $OUTPUT_FILE
			if [ $? -eq 0 ] ; then
				success "│ $(printf "%-26s" "$FILE_HTTP") -> $OUTPUT_FILE"
			else
				warning "│ $(printf "%-26s" "$FILE_HTTP") -> NO RESPONSE"
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

compare_response() {
	for LOG_FILE_WS in $LOGS_DIR/http/*webserv.log ; do
		local LOG_FILE_NG=$(echo $LOG_FILE_WS | sed 's/webserv.log$/nginx.log/')
		local RES_WS=$(head -n 1 $LOG_FILE_WS)
		local RES_NG=$(head -n 1 $LOG_FILE_NG)
		
		# TODO remove CRLF bypass
		if [[ $RES_WS == ${RES_NG%$'\r'} ]] ; then
			success "$(printf "%-42s" $LOG_FILE_WS) '$RES_WS'"
		else
			error "$(printf "%-42s" $LOG_FILE_WS) '$RES_WS' instead of '${RES_NG%$'\r'}'"
		fi
	done
}

main
