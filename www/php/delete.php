<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>File_deletion</title>
</head>
<body>
	<div>
		<span>Method: <?php echo $_SERVER['REQUEST_METHOD']; ?></span>
	<?php
	if ($_SERVER['REQUEST_METHOD'] === "DELETE") {
		$file = $_GET['file'];
		$uploadDir = $_SERVER["PATH_INFO"];
		$targetPath = $uploadDir . '/' . $file;
		echo "file:	" . $file . "\n";
		echo "targetDir:	" . $uploadDir . "\n";
		echo "targetPath	" . $targetPath . "\n";
		if (file_exists($targetPath)) {
			if (unlink($targetPath)) {
				echo "deletion succeeded";
			}
		}
		else
		{
			echo "File does not exist";
		}
	}
	?>
	</div>
</body>
</html>