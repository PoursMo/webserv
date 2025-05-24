from os import environ

def getCookie(key):
	cookies = environ.get('HTTP_COOKIE')
	if cookies:
		for cookie in cookies.split(';'):
			(kkey, value ) = cookie.split('=')
			if kkey == key:
				return value
	return ''

def main():
	name = getCookie('name')
	print("Content-Type: text/html")
	print()
	if not name:
		print('<h1>Connexion requise !</h1>')
		print('<a href="/python">Login</a>')
		return
	print('<h1>Hey', name, '</h1>')
	print('<p>Bienvenu dans ton espace personnel.</p>')
	print('<a href="/python/logout.py">Logout</a>')

main()
