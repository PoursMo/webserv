

<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Awesome document</title>
</head>
<body>

	<h1>Salut</h1>

	<?php
		echo 'Hello ' . htmlspecialchars($_GET["name"]) . "!\n";
		echo 'Script filename: ' . htmlspecialchars($_SERVER['SCRIPT_FILENAME']) . "\n";
		echo 'Server port: ' . $_SERVER['SERVER_PORT'] . "\n";
		echo 'Request time: ' . $_SERVER['REQUEST_TIME'] . "\n";
		echo 'Server: ' . $_SERVER['SERVER_SOFTWARE'] . "\n";
	?>

</body>
</html>


