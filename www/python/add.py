import sys
import os
import uuid
import shutil



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

def main():
	name = getCookie('name')
	if not name or name == '':
			print('<h1>Connexion requise !</h1>')
			print('<a href="/python">Login</a>')
			return
	file = getUserDir(name) + str(uuid.uuid1())
	with os.fdopen(sys.stdin.fileno(), 'rb') as input_file, open(file, 'wb') as output_file:
			shutil.copyfileobj(input_file, output_file)

main()
# form = cgi.FieldStorage()
# form["name"].value

# TODO: apped task in path_info/user.json
# {tasks: [{id: 1, content: "prout", isChecked: false }]}