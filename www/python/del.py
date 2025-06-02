import os
from urllib.parse import parse_qs

def main():
	if os.environ.get("REQUEST_METHOD") != "DELETE":
		print("Status: 405")
		print()
		return
	qs = parse_qs(os.environ.get('QUERY_STRING'))
	todoIds = qs.get("todoId")
	if todoIds:
		todoPath = os.environ.get('PATH_INFO') + '/' + todoIds[0]
		os.remove(todoPath)
	print("Location: /python/app.py")
	print("Status: 302")
	print()

main()