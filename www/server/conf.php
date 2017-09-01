<?php
	$dbAccess['user'] = "root";
	$dbAccess['pswd'] = "password";
	$dbAccess['db'] = "dbogr";

	//$pdo = new PDO('mysql:host=localhost;dbname='.$dbAccess['db'],$dbAccess['user'],$dbAccess['pswd']);
	$pdo = new PDO('mysql:host=localhost;dbname=dbogr','root','password');

?>