<?

// No longer used. Later: replace with start/end date initialisation ?
if(isset($_REQUEST['d'])) {
  $hours = 24 * intval($_REQUEST['d']); 
} else if(isset($_REQUEST['h'])) {
  $hours = intval($_REQUEST['h']);
} else {
  $hours = 12;
}

?>
<html>
<meta charset="utf-8">
<head>
  <link href='css/page.css' rel='stylesheet'/>
  <link href='css/chart.css' rel='stylesheet'/>
  <link href='css/localization.css' rel='stylesheet'/>

  <script src="https://d3js.org/d3.v4.js"></script>
  <script src='js/date.format.js'></script>
  <script src='js/chart.js'>
    var chart = null;
  </script>
  
</head>
<body>
<?php include ('header.php'); ?>  

<div id="form">
  <span id="startDate"><input type="date" name="startDate" onChange="chart.getData();"/></span>
  <span id="endDate"><input type="date" name="endDate" onChange="chart.getData();"/></span>
</div>

<script>
  chart = initGraph();
</script>
</body>
</html>
