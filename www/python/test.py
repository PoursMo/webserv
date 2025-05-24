import cgi

form = cgi.FieldStorage()
name = form.getvalue('name', 'World')

print("Content-Type: text/html\n")
print(f"<h1>Hello {name}!</h1>")