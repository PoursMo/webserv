<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Fruits</title>
		<link rel="stylesheet" href="style.css">
	</head>
	<?php
		$onSuccess = isset($_GET['success']);
		$onError = isset($_GET['error']);
	?>
	<body>
		<div class="header">Ma gallerie</div>
	
		<form action="add.php" enctype="multipart/form-data" method="post">
			<input type="file" name="file"></input>
			<button type="submit" name="submit" >
				<span class="button_content">Ajouter</span>
			</button>
			<?php
				if (isset($_GET['success'])) {?>
					<div>Success 😁</div>
				<?php } else if (isset($_GET['error'])) {?>
					<div>Oups, 😥</div>
				<?php }
			?>
		</form>

		
		<div class="gallerie">
		<?php
			$uploadDir = $_SERVER["PATH_INFO"];
			$files = scandir($uploadDir, SCANDIR_SORT_DESCENDING, null);
			$files = array_diff($files, array('.', '..'));
			if ($files === false)
				echo "No images found";
			else
			{
				foreach($files as $file)
				{
					?><div class="gallerie_element"><?php
						$fileType = strtolower(pathinfo($file, PATHINFO_EXTENSION));
						if ($fileType == "mov") { ?>
							<video
							controls
							type="video/<?php echo $fileType ?>"
							src="<?php echo  "/upload/" . $file ?>"
							></video>
						<?php } else { ?>
							<a href="<?php echo  "/upload/" . $file ?>" target="_blank">
								<img src="<?php echo  "/upload/" . $file ?>" alt="<?php echo $file ?>">
							</a>
						<?php } ?>
							<button onclick="handleDelete('<?php echo $file ?>')">
								<span class="button_content">Supprimer</span>
							</button>
					</div> <?php
				}
			}
		?>
		</div>
		
		<div class="footer">Webserv compagnie ©(TM)</div>

		<script>
			async function handleDelete(file){
				const res = await fetch(`delete.php?file=${file}`, {
					method: 'DELETE'
				})
				if (!res.ok)
					return window.location = 'gallery.php?error=1'
				return window.location = 'gallery.php?success=1'
			}
		</script>
	</body>
</html>

