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

	local CONF_NAME="$1"
	local REQ_NAME_FILTER="$2"

	info "\nTests http request\n"

	for FILE_CONF in conf/*.json ; do
		local FILE_CONF_NG="$(pwd)/conf/nginx/$(basename $FILE_CONF .json).conf"
		if [ ! -f "$FILE_CONF_NG" ] ; then
			continue
		fi
		if [[ "$CONF_NAME" != "" &&  "$CONF_NAME" != "$(basename $FILE_CONF .json)" ]] ; then
			continue
		fi

		# NGINX
		nginx -c $FILE_CONF_NG &
		local PID=$!
		info "[NGINX] $FILE_CONF_NG"
		send_all_request $FILE_CONF "nginx" $REQ_NAME_FILTER
		kill $PID
		info "[NGINX]\n"

		# WEBSERV
		# ./webserv $FILE_CONF &> /dev/null &
		./webserv $FILE_CONF > ./webserv.log 2> ./webserv_err.log &
		local PID=$!
		info "[WEBSERV] $FILE_CONF"
		send_all_request $FILE_CONF "webserv" $REQ_NAME_FILTER
		kill $PID
		info "[WEBSERV]\n"
		
	done

	compare_response
}

send_all_request() {
	local FILE_CONF=$1
	local SERVER_NAME=$2
	local REQ_NAME_FILTER=$3
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
			if [[ $REQ_NAME_FILTER != "" && $REQ_NAME_FILTER != $REQ_NAME ]] ; then
				continue
			fi
			if [[ $REQ_NAME = _* ]] ; then 
				warning "│ IGNORED REQUEST $REQ_NAME"
				continue
			fi
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
	
	# NETCAT MODE
	# nc -q 1 $2 $3 < $1
	# if [ $? -ne 0 ] ; then
	# 	echo "NO RESPONSE FROM $2:$3"
	# 	return 1
	# fi

	# TELNET MODE
	(cat $1; sleep 0.1) | telnet $2 $3 2> /dev/null
	if [ $? -ne 1 ] ; then
		echo "NO RESPONSE FROM $2:$3"
		return 1
	fi

}

compare_response() {
	for LOG_FILE_WS in $LOGS_DIR/http/*webserv.log ; do
		local LOG_FILE_NG=$(echo $LOG_FILE_WS | sed 's/webserv.log$/nginx.log/')

		local RES_WS=$(grep "^HTTP/1.1" $LOG_FILE_WS | head -n 1 | sed 's/\r/↵/g')
		local RES_NG=$(grep "^HTTP/1.1" $LOG_FILE_NG | head -n 1 | sed 's/\r/↵/g')
		
		#diff $LOG_FILE_WS $LOG_FILE_NG > $(echo $LOG_FILE_WS | sed 's/_webserv.log$/.diff/')

		if [[ $RES_WS == $RES_NG ]] ; then
			success "$(printf "%-42s" $LOG_FILE_WS) $RES_WS"
		else
			error "$LOG_FILE_WS\nWEBSERV:\t$RES_WS\nNGINX:\t\t$RES_NG"
		fi
	done
}

main "$@"
