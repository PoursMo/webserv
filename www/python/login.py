import cgi
from http import cookies

def main():
	form = cgi.FieldStorage()

	if "name" not in form :
		print("Content-Type: text/html\n")
		print()
		print("<H1>Error</H1>")
		print("<p>Please fill in the name field.</p>")
		return
	
	name = form["name"].value
	cookie = cookies.SimpleCookie()
	cookie["name"] = name
	print(cookie)
	print("Content-Type: text/html")
	print("Location: /python/app.py")
	print("Status: 302")
	print()

main()
