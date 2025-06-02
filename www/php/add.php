<!DOCTYPE html>
<html>

<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>Add file</title>
</head>

<body>

	<?php
	if ($_SERVER["REQUEST_METHOD"] == "POST") {

		$uploadDir = $_SERVER["PATH_INFO"];
		$targetFile = $uploadDir . "/" . basename($_FILES["file"]["name"]);
		$fileType = strtolower(pathinfo($targetFile, PATHINFO_EXTENSION));
		// TODO: do something with fileType
	?>
		<ul>
			<li>targetDir: <?php echo $uploadDir ?></li>
			<li>targetFile: <?php echo $targetFile ?></li>
			<li>fileType: <?php echo $fileType ?></li>
		</ul>
	<?php

		if (move_uploaded_file($_FILES["file"]["tmp_name"], $targetFile)) {
			echo "file uploaded on: ";
			echo $targetFile;
			header('Location: /php/gallery.php?success=1', true, 302);
		} else {
			echo "Fail to load file on " . $targetFile;
			header('Location: /php/gallery.php?error=1', true, 302);
		}
	}
	?>
</body>

</html>