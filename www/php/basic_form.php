<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Fruits</title>
		<style>
			*{font-family:'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;}
			html {height:100%; width: 100%;}
			body {height:100%; width:100%; background-color: #FFE99A; display:flex; flex-direction: column;}
			.header {display: flex; flex : 1; justify-content: center; align-items: center; font-size: 3vh;}
			.middle {display: flex; flex : 7; justify-content: center; align-items: center;}
			.footer {display: flex; flex : 1; justify-content: center; align-items: center;}
			form {display:flex; flex-direction: column; align-items: center; gap: 3vh;}
			input, select {border-radius: 15px; padding: 8px; background-color: #FFD586; color: black;}
			button {border-radius: 15px; background-color: #FF9898; color:black; padding: 8px}
		</style>
	</head>
	<body>
		<div class="header"> Fruits?</div>
		<div class="middle">
			<form action="form_data.php" enctype="multipart/form-data" method="post">
				<label for="firstname"> Firstname?</label>
				<input id="firstname" type="text" name="firstname" placeholder="John">

				<label for="lastname">Lastname?</label>
				<input id="lastname" type="text" name="lastname" placeholder="Doe">

				<label for="favouritefruit">Favourite Fruit?</label>
				<select id="favouritefruit" name="favouritefruit">
					<option value="I hate them all">None</option>
					<option value="Banana">Banana</option>
					<option value="Apple">Apple</option>
					<option value="Grapes">Grapes</option>
				</select>

				<label for="fruitimage">Fruit image?</label>
				<input id="fruitimage" type="file" name="fruitimage"></input>

				<button type="submit" name="submit" >Submit</button>
			</form>
		</div>
		<div class="footer"> Eat at least 5 fruits per day!</div>
	</body>
</html>

