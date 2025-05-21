sed 's/^/> /' $1; echo; echo
if [ "$2" != "" ] ; then
	(cat $1; sleep 0.2) | telnet 127.0.0.1 8000 | sed 's/^/< /'
else 
	< $1 nc 127.0.0.1 8000 | sed 's/^/< /'
fi