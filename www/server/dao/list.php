<?php

function getList($pdo, $sortBy=0, $orderAsc = true){
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

	$query = $pdo->prepare('SELECT * FROM vehicules ORDER BY :field :type');
	$query->execute(array(
		':field' => $sortField,
		':type' => $orderType
		));

	return $query->fetchAll(PDO::FETCH_ASSOC);

}
?>