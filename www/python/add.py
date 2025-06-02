import sys
import os
import uuid
import cgi

# sys.stdin
# shutil.copyfile(sys.stdin, )

def getCookie(key):
	cookies = os.environ.get('HTTP_COOKIE')
	if cookies:
		for cookie in cookies.split(';'):
			(kkey, value ) = cookie.split('=')
			if kkey == key:
				return value
	return ''

def getUserDir(username):
	pathInfo = os.environ.get('PATH_INFO')
	return pathInfo + '/' + username + '/'

def getTodoContent():
	if os.environ.get('CONTENT_TYPE') == 'application/x-www-form-urlencoded':
		form = cgi.FieldStorage()
		return form.getvalue("todo")
	return sys.stdin.read()

def main():
	if os.environ.get("REQUEST_METHOD") != "POST":
		print("Status: 405")
		print()
		return
	name = getCookie('name')
	if not name or name == '':
			print("Content-Type: text/html")
			print()
			print('<h1>Connexion requise !</h1>')
			print('<a href="/python">Login</a>')
			return
	userDir = getUserDir(name) 
	os.makedirs(userDir, 511, True)
	todoFile = userDir + str(uuid.uuid1())
	todoContent = getTodoContent()
	if not todoContent:
		print("Location: /python/app.py?error=content_empty")
		print("Status: 302")
		print()
		return
	with open(todoFile, 'w') as outputFile:
		outputFile.write(todoContent)
	print("Location: /python/app.py?success=1")
	print("Status: 302")
	print()

main()
# form = cgi.FieldStorage()
# form["name"].value

# TODO: apped task in path_info/user.json
# {tasks: [{id: 1, content: "prout", isChecked: false }]}