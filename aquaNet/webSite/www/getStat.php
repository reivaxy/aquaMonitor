<?
// returns json formatted data

// TODO: module parameter, date parameters

//ini_set("display_errors", 1);
require('../includes/utils.inc.php');

// Start date
if(isset($_REQUEST['sd'])) {
  $startDate = $_REQUEST['sd'];
} else {
  $startDate = '2016-10-04'; //date("y-m-d");
}

// End date
if(isset($_REQUEST['ed'])) {
  $endDate = $_REQUEST['ed'];
} else {
  $endDate = '2016-10-06'; // date("y-m-d");
}

$moduleId = 1;   // TODO hardcoded for now.
$stat = array();

$mysqli = connect();
$stmt =  $mysqli->stmt_init();
$stmt->prepare('select datadate, power, poweralert, waterlevel, waterlevelalert, '
     . ' lightlevel, lightlevelalert, temperature, temperaturealert '
     . ' from aquanetstat where moduleid = ? and datadate >= STR_TO_DATE(?,\'%Y-%m-%d\') and datadate <= DATE_ADD(STR_TO_DATE(?,\'%Y-%m-%d\'), INTERVAL 1 DAY) order by statid ') OR die("Invalid statement");

//$stmt->bind_param("i", $moduleId);
$stmt->bind_param("iss", $moduleId, $startDate, $endDate);
$stmt->execute();

$stmt->bind_result($date, $power, $powerAlert, $waterLevel, $waterAlert,
                   $lightLevel, $lightLevelAlert, $temperature, $temperatureAlert);

while($row = $stmt->fetch()) {
  array_push($stat, array("date" => $date, "power" => $power, "powerAlert" => $powerAlert,
               "waterLevel" => $waterLevel, "waterLevelAlert" => $waterLevelAlert,
               "lightLevel" => $lightLevel, "lightLevelAlert" => $lightLevelAlert,
               "temperature" => $temperature, "temperatureAlert" => $temperatureAlert, "sd" => $startDate));

}
$stmt->free_result();
$stmt->close();

echo json_encode($stat);

?>
