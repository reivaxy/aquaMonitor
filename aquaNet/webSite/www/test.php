<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="fr" lang="fr">
  <head>
    <title>Tests</title>
  </head>
  <body>
<?
require('../includes/utils.inc.php');

$dbh = connect();
$req_count = 'SELECT count(*) NB FROM aquanetuser';
$result = mysql_query($req_count) OR die ("Invalid request: " .mysql_error());
$liste_msg = array();
while($row = mysql_fetch_assoc($result)) {
   array_push($liste_msg, $row) ;
}
mysql_free_result($result);

?>
Il y a <?=$liste_msg[0]['NB']?> enregistrement
  </body>
</head>
