

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

		// Takes raw data from the request
		$resource = isset($_GET['resource']) ? $_GET['resource'] : null;
		echo $resource;

		// $json = file_get_contents('php://input');

		// Converts it into a PHP object
		// $data = json_decode($json);
		// echo 'data.name:' . $data->name . "\n";

		// TODO: use application/x-www-form-urlencoded instead application/json
		// echo 'Hello ' . htmlspecialchars($_POST["name"]) . "!\n";
		// echo 'Script filename: ' . htmlspecialchars($_SERVER['SCRIPT_FILENAME']) . "\n";
		// echo 'Server port: ' . $_SERVER['SERVER_PORT'] . "\n";
		// echo 'Request time: ' . $_SERVER['REQUEST_TIME'] . "\n";
		// echo 'Server: ' . $_SERVER['SERVER_SOFTWARE'] . "\n";
	?>

</body>
</html>


