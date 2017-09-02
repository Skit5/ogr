<?php
ini_set('display_errors', 'On');
require_once('dao/add.php');
require_once('dao/list.php');
require_once('dao/read.php');
require_once('dao/remove.php');
require_once('dao/search.php');
require_once('ogr.php');
require_once('conf.php');
header('Content-Type: application/json');
$results = array();
if($_POST) {
	$post = $_POST;
	if($post['q'] == 'list')
		$results = getList($pdo);
	if($post['q'] == 'read'){
		if($post['vhc'])
			$results = getDataPoints($pdo, $post['vhc']);
	}
}
echo json_encode($results);
?>