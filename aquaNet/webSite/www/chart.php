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
  <link href='css/chart.css' rel='stylesheet'/>
  <link href='css/localization.css' rel='stylesheet'/>

  <script src="https://d3js.org/d3.v4.js"></script>
  <script src='js/date.format.js'></script>
  <script src='js/chart.js'>
    var chart = null;
  </script>
  
</head>
<body>
<div id="form">
  <input id="startDate" type="date" name="startDate" onChange="chart.getData();"/>
  <input id="endDate" type="date" name="endDate" onChange="chart.getData();"/>
</div>

<script>
  chart = initGraph();
</script>
</body>
</html>
