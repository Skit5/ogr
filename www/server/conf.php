<?php
	$dbAccess['user'] = "root";
	$dbAccess['pswd'] = "password";
	$dbAccess['db'] = "dbogr";

	$pdo = new PDO('mysql:host=localhost;charset=utf8;dbname='.$dbAccess['db'],$dbAccess['user'],$dbAccess['pswd']);
	//$pdo = new PDO('mysql:host=localhost;dbname=dbogr','root','password');
	$pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

?>