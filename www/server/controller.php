<?php
ini_set('display_errors', 'On');
require_once('dao/add.php');
require_once('dao/list.php');
require_once('dao/read.php');
require_once('dao/remove.php');
require_once('dao/search.php');
require_once('ogr.php');

if($_POST) {
	$data = json_decode(file_get_contents('php://input'), true);
	print_r($data);
}else{
	echo "Yo!";
}
?>