<?php

function getList($pdo, $sortBy=0, $orderAsc = true, $toSearch=""){
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
		case 3:
			$sortField = 'constructor';
			break;
		case 4:
			$sortField = 'constructionDate';
			break;
		default:
			$sortField = 'nom';
			break;
	}
	$orderType = (!$orderAsc)?'DESC':'ASC';
	if(empty($toSearch)){
		$query = $pdo->prepare('SELECT vehicules.idVehicule as id, vehicules.nom as nom, vehicules.details as details, vehicules.insertDate as insertDate, constructors.name as constructor, vehicules.constructionDate as constructionDate FROM vehicules INNER JOIN constructors ON vehicules.constructor = constructors.idConstructor ORDER BY :field :type');
		$query->execute(array(
			':field' => $sortField,
			':type' => $orderType
			));
	}else{
		$query = $pdo->prepare('SELECT vehicules.idVehicule as id, vehicules.nom as nom, vehicules.details as details, vehicules.insertDate as insertDate, constructors.name as constructor, vehicules.constructionDate as constructionDate FROM vehicules INNER JOIN constructors ON vehicules.constructor = constructors.idConstructor WHERE vehicules.nom LIKE :toSearch OR vehicules.details LIKE :toSearch OR constructors.name LIKE :toSearch ORDER BY :field :type');
		$query->execute(array(
			':field' => $sortField,
			':type' => $orderType,
			':toSearch' => '%'.$toSearch.'%'
			));
	}

	return $query->fetchAll(PDO::FETCH_ASSOC);

}
?>