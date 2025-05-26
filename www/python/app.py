import os

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

def printRemoveTodo(todoId):
	print('<form method="DELETE" action="rm.py" style="margin: 0;">')
	print('<input type="hidden" name="todoId" value="', todoId, '"/>')
	print('<input type="submit" value="Fini !"/>')
	print('</form>')

def printTodoList(username):
	print('<h3>Todo:</h3>')
	userDir = getUserDir(username)
	os.makedirs(userDir, 511, True)
	print('<ul style="max-width: 400px;">')
	for filename in os.listdir(userDir):
		print('<li style="display: flex; align-items: center; gap: 1rem;">')
		with open(userDir + filename, 'r') as f:
			print('<span style="flex-grow: 1; padding: 0.3em 0.5em;">')
			print(f.read())
			print('</span>')
		printRemoveTodo(filename)
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
	print('<form action="add.py" method="post" enctype="text/plain">')
	print('<input name="todo" placeholder="Nouvelle tache" />')
	print('<input type="submit" input>')
	print('</form>')
	# TODO: list tasks contained in $path_info/todo_${name}/4.txt
	print('<a href="/python/logout.py">Logout</a>')

main()
