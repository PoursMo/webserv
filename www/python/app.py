import os
from urllib.parse import parse_qs

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

def printRemoveTodo(username, todoId):
	print("<button onclick=\"handleDelete('" + username + "/" + todoId, "')\">")
	print('<span class="button_content">Fini !</span>')
	print('</button>')

def printTodoList(username):
	print('<h3>Todo:</h3>')
	userDir = getUserDir(username)
	if not os.path.exists(userDir):
		print("<p>Pas de todo</p>")
	else:
		print('<ul style="max-width: 400px;">')
		for filename in os.listdir(userDir):
			print('<li style="display: flex; align-items: center; gap: 1rem;">')
			with open(userDir + filename, 'r') as f:
				print('<span style="flex-grow: 1; padding: 0.3em 0.5em;">')
				print(f.read())
				print('</span>')
			printRemoveTodo(username, filename)
			print('</li>')
		print('</ul>')

def main():
	name = getCookie('name')
	print("Content-Type: text/html")
	print()
	if not name or name == '':
		print('<h1>Connexion requise !</h1>')
		print('<a href="/python">Login</a>')
		return
	print('<h1>Hey', name, '</h1>')
	printTodoList(name)
	print('<form action="add.py" method="post">')
	print('<input name="todo" placeholder="Nouvelle tache" autofocus="on" />')
	print('<input type="submit" input>')
	qs = parse_qs(os.environ.get('QUERY_STRING'))
	errors = qs.get("error")
	if errors:
		print(errors[0])
	print('</form>')
	print('<script src="handleDelete.js" ></script>')
	# TODO: list tasks contained in $path_info/todo_${name}/4.txt
	print('<a href="/python/logout.py">Logout</a>')

main()
