<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>File_deletion</title>
</head>
<body>
	<div>
	<?php
	if ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
		$resource = isset($_GET['resource']) ? $_GET['resource'] : null;
		$targetDir = $_SERVER["PATH_INFO"];
		$targetPath = $targetDir . '/' . $resource;
		echo "resource:	" . $resource . "\n";
		echo "targetDir:	" . $targetDir . "\n";
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