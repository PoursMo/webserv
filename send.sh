sed 's/^/> /' $1; echo; echo
(cat $1; sleep 0.1) | telnet 127.0.0.1 8000 | sed 's/^/< /'