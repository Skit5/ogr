<?php
require_once('conf.php');

function getList($sortBy=0, $orderAsc = true){
	$sortField = '';
	switch($sortBy){
		case 0:
			$sortField = 'nom';
			break;
		case 1:
			$sortField = 'details';
			break;
		case 2:
			$sortField = 'insertDate';
			break;
		default:
			$sortField = 'nom';
			break;
	}
	$orderType = (!$orderAsc)?'DESC':'ASC';

	$query = $pdo->query('SELECT * FROM vehicules ORDER BY ? ?');
	$query->execute(array($sortField,$orderType));

	return $query->fetchAll(PDO::FETCH_ASSOC);

}
?>