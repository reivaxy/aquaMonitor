<?
// ini_set("display_errors", 1);
require('../includes/utils.inc.php');

$json = file_get_contents('php://input');

//$json = $_REQUEST['data'];

$data = json_decode($json, true) ;
if(json_last_error()) {
  die("Json error");
}

$key = $data['key'];
$moduleStats = $data['stat'];

$mysqli = connect();

// Debugging help
// $stmt =  $mysqli->stmt_init();
// $stmt->prepare('insert into logs (log) values (?)') OR die("Invalid log statement " . $stmt->error);
// $stmt->bind_param("s", $json);
// $stmt->execute();
// $stmt->close();

$modules = array();

// Read modules and user for apikey
$stmt =  $mysqli->stmt_init();

// TODO: open join, and  create new module if necessary ?
$stmt->prepare('select u.userid, moduleid, m.name from aquanetuser u, aquanetmodule m where apikey = ? and enabled = 1' .
               ' and m.userid = u.userid' ) OR die("Invalid statement");
$stmt->bind_param("s", $key);
$stmt->execute();
$stmt->store_result();
$count = $stmt->num_rows;
if($count == 0) {
  die("Permission denied");
}
$stmt->bind_result($userId, $moduleId, $moduleName);
while($row = $stmt->fetch()) {
  $modules[$moduleName] = array("userId" => $userId, "moduleId" => $moduleId);
}
// print_r($modules);

$stmt->free_result();
$stmt->close();

$count = count($moduleStats);

for($i = 0; $i < $count; $i++) {
  $moduleName = $moduleStats[$i]["id"];
  // print_r($moduleName);
  $ids = $modules[$moduleName];
  // print_r($ids);
  if($ids) {
    $stmt =  $mysqli->stmt_init();
    $stmt->prepare('insert into aquanetstat (userid, moduleid, '
            . 'lightlevel, lightlevelalert, '
            . 'temperature, temperaturealert, '
            . 'waterlevel, waterlevelalert, '
            . 'power, poweralert, '
            . 'datadate'
            .' ) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)') OR die("Invalid statement " . $stmt->error);

    $date = DateTime::createFromFormat('Y/m/d H:i', $moduleStats[$i]["date"]);
    $timestamp = $date->format('Y-m-d H:i:s');
    $temp = $moduleStats[$i]["temp"] / 100;

    $waterLevel = !$moduleStats[$i]["waterLevelAlert"];
    $mainPower = !$moduleStats[$i]["powerAlert"];
    $stmt->bind_param("iiiidiiiiis",
            $ids["userId"], $ids["moduleId"],
            $moduleStats[$i]["light"], $moduleStats[$i]["lightAlert"],
            $temp, $moduleStats[$i]["tempAlert"],
            $waterLevel, $moduleStats[$i]["waterLevelAlert"],
            $mainPower, $moduleStats[$i]["powerAlert"],
            $timestamp);

    $stmt->execute();
    // print_r($stmt->error);
    // print_r($stmt->affected_rows);
    $stmt->close();
  } else {
    echo "module $moduleName not found";
  }
}
/**/
$mysqli->close();

echo "Done.";
?>
