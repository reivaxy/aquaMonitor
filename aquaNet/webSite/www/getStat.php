<?
// returns json formatted data

// TODO: module parameter, date parameters

ini_set("display_errors", 1);
require('../includes/utils.inc.php');

$moduleId = 1;   // TODO hardcoded for now.
$stat = array();

$mysqli = connect();
$stmt =  $mysqli->stmt_init();
$stmt->prepare('select datadate, power, poweralert, waterlevel, waterlevelalert, '
     . ' lightlevel, lightlevelalert, temperature, temperaturealert '
     . ' from aquanetstat where moduleid = ? and datadate > DATE_SUB(NOW(), INTERVAL 24 HOUR) order by statid ') OR die("Invalid statement");

$stmt->bind_param("i", $moduleId);
$stmt->execute();

$stmt->bind_result($date, $power, $powerAlert, $waterLevel, $waterAlert,
                   $lightLevel, $lightLevelAlert, $temperature, $temperatureAlert);

while($row = $stmt->fetch()) {
  array_push($stat, array("date" => $date, "power" => $power, "powerAlert" => $powerAlert,
               "waterLevel" => $waterLevel, "waterLevelAlert" => $waterLevelAlert,
               "lightLevel" => $lightLevel, "lightLevelAlert" => $lightLevelAlert,
               "temperature" => $temperature, "temperatureAlert" => $temperatureAlert));

}
$stmt->free_result();
$stmt->close();

echo json_encode($stat);

?>
