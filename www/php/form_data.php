<!DOCTYPE html>
<html>
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>Fruits</title>
		<style>
			*{font-family:'Trebuchet MS', 'Lucida Sans Unicode', 'Lucida Grande', 'Lucida Sans', Arial, sans-serif;}
			html {height:100%; width: 100%;}
			body {height:100%; width:99%; background-color: #FFE99A; display:flex; flex-direction: column; justify-content: center}
			div{display:flex; justify-content: center; align-items: center; padding: 15px; font-family: 'Franklin Gothic Medium', 'Arial Narrow', Arial, sans-serif;}
			div.inside {display:flex; justify-content: center; align-items: center;background-color: #FFAAAA; border-radius: 15px}
		</style>
	</head>
	<body>
		<div>
			<div class="inside">
				<?php
				if ($_SERVER["REQUEST_METHOD"] == "POST")
				{
					//GET VALUES FROM FORM
					$firstname = htmlspecialchars($_POST["firstname"]);
					$lastname = htmlspecialchars($_POST["lastname"]);
					$favouritefruits = htmlspecialchars($_POST["favouritefruit"]);
					echo "These are the data that the user submitted:";
					echo "<br>";
					echo "First name: ";
					echo $firstname;
					echo "<br>";
					echo "Last name: ";
					echo $lastname;
					echo "<br>";
					echo "Favourite fruits: ";
					echo $favouritefruits;
					echo "<br>";

					$target_dir = $_SERVER["PATH_INFO"];
					$target_file = $target_dir . "/" . basename($_FILES["fruitimage"]["name"]);
					$uploadOk = 1;
					$imageFileType = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));
					if (isset($_POST["submit"]))
					{
						$check = getimagesize($_FILES["fruitimage"]["tmp_name"]);
						if ($check !== false)
						{
							echo "File is an image - " . $check["mime"] . ".";
							echo "<br>";
							echo "Its name is : '";
							echo $_FILES["fruitimage"]["name"];
							echo "'<br>";
							$uploadOk = 1;
						}
						else
						{
							echo "File is not an image.";
							$uploadOk = 0;
						}
					}
					// Check if $uploadOk is set to 0 by an error
					if ($uploadOk == 0) {
						echo "Sorry, your file was not uploaded.";
						// if everything is ok, try to upload file
					} else {
						if (move_uploaded_file($_FILES["fruitimage"]["tmp_name"], $target_file)) {
							?>
							<span>The file has been uploaded to <?php echo htmlspecialchars($target_file); ?></span>
							<br>
							<img src="<?php echo $target_file; ?>" alt="user avatar" >
							<?php
						} else {
							echo "Sorry, there was an error uploading your file.";
						}
					}
				}
			?>
			</div>
		</div>
</html>

<?php

?>