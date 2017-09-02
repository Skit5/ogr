<?php

function getDataPoints($pdo, $vehicule){

	$query = $pdo->prepare('SELECT * FROM releves WHERE vehicule = :vhc ORDER BY w ASC');
	$query->execute(array(
		':vhc' => $vehicule
		));

	return $query->fetchAll(PDO::FETCH_ASSOC);

}
?>