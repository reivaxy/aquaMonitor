<?php
$inc_path = ini_get('include_path');
$inc_path .= ':/home/reeftanklp/www/lib' ;
ini_set('include_path', $inc_path);

require_once('myconfig.inc.php');

function connect() {
  $config = set_config();
  $host = $config['host'] ;
  $only_db = $config['only_db'] ;

  $mysqli = new mysqli($config['host'], $config['user'],$config['password'],  $config['only_db']);
  if (mysqli_connect_errno()) {
    die("connection failed : ". mysqli_connect_error());
  }
  return $mysqli;
}

?>
