<html>
<meta charset="utf-8">
<head>
<style> /* set the CSS */
.axis { font: 14px sans-serif; }
.line {
  fill: none;
  stroke: steelblue;
  stroke-width: 2px;
}
</style>
<script src="https://d3js.org/d3.v4.js"></script>
</head>
<body>

<script>
// set the dimensions and margins of the graph
var margin = {top: 20, right: 20, bottom: 100, left: 50},
    width = 960 - margin.left - margin.right,
    height = 500 - margin.top - margin.bottom;
// parse the date / time
var parseTime = d3.timeParse("%Y-%m-%d %H:%M:%S");
// set the ranges
var x = d3.scaleTime().range([0, width]);
var y = d3.scaleLinear().range([height, 0]);
// define the line
var lightLine = d3.line()
    .x(function(d) { return x(d.date); })
    .y(function(d) { return y(d.light); });
    
var lightAlertLine = d3.line()
    .x(function(d) { return x(d.date); })
    .y(function(d) { return y(d.lightAlert); });
// append the svg obgect to the body of the page
// appends a 'group' element to 'svg'
// moves the 'group' element to the top left margin
var svg = d3.select("body").append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
  .append("g")
    .attr("transform",
          "translate(" + margin.left + "," + margin.top + ")");
// Get the data
d3.json("getStat.php", function(error, data) {
  if (error) throw error;
  // format the data
  data.forEach(function(d) {
      d.date = parseTime(d.date);
      d.light = d.lightLevel;
//      d.lightAlert = d.lightLevelAlert*500;
  });
  // Scale the range of the data
  x.domain(d3.extent(data, function(d) { return d.date; }));
  y.domain([300, d3.max(data, function(d) { return d.light; })]);
  // Add the lightLine path.
  svg.append("path")
      .data([data])
      .attr("class", "line")
      .attr("d", lightLine);
//  svg.append("path")
//      .data([data])
//      .attr("class", "line")
//      .attr("d", lightAlertLine);
  // Add the X Axis
  svg.append("g")
      .attr("class", "axis")
      .attr("transform", "translate(0," + height + ")")
      .call(d3.axisBottom(x)
              .tickFormat(d3.timeFormat("%Y-%m-%d")))
      .selectAll("text")
        .style("text-anchor", "end")
        .attr("dx", "-.8em")
        .attr("dy", ".15em")
        .attr("transform", "rotate(-65)");
  // Add the Y Axis
  svg.append("g")
      .attr("class", "axis")
      .call(d3.axisLeft(y));
});
</script>

</body>
</html>