<?php
error_reporting(E_ERROR | E_WARNING | E_PARSE);

class DB { static $link; }
DB::$link = mysqli_connect("127.0.0.1", "samver_test", "testje", "samver_test");
if (!DB::$link) die("Error: Unable to connect to MySQL.");

$table = array(
    'cols' => array(
        array('label' => 'Tijd', 'type' => 'datetime'),
        array('label' => 'Max', 'type' => 'number'),
		array('label' => 'Gemiddelde', 'type' => 'number'),
        array('label' => 'Min', 'type' => 'number')
    ),
    'rows' => array()
);

$todaysDate = date("Y-m-d");
$datefilter = "DATE(time) = '$todaysDate'";
if(isset($_REQUEST['from'])) {
	$datefilter = "DATE(time) BETWEEN '" . $_REQUEST['from'] . "' AND '" . $_REQUEST['to'] . "'";
}
$query = "SELECT * FROM `sensors` WHERE $datefilter ORDER by time DESC";

if ($result = DB::$link->query($query)) {
	while($r = $result->fetch_assoc()) {
		$date = new DateTime($r['time']);
		$sdate = "Date(".$date->format('Y, m, d, H, i, s').")"; 
		$table['rows'][] = array('c' => array(
			array('v' => $sdate),
			array('v' => $r['max']),
			array('v' => $r['mean']),
			array('v' => $r['min'])
		));
	}
}
echo json_encode($table, JSON_NUMERIC_CHECK);

mysqli_close(DB::$link);
?>