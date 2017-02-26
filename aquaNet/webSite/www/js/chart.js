

function AquaNetChart(domContainer, margin, width, height) {
  var that = this;
  // set the dimensions and margins of the graph
  this.margin = margin;
  this.width = width - margin.left - margin.right;
  this.height = height - margin.top - margin.bottom ;
  
  // set the axis
  this.x = d3.scaleTime().range([0, this.width]);
  // left  y axis for light
  this.y0 = d3.scaleLinear().range([this.height, 0]);
  this.yAxisLeft = d3.axisLeft(this.y0).ticks(5);
  // right y axis for temperature
  this.y1 = d3.scaleLinear().range([this.height, 0]);
  this.yAxisRight = d3.axisRight(this.y1).ticks(5);

  // define the data line for the light level
  this.lightLine = d3.line()
    .x(function (d) {
      return that.x(d.date);
    })
    .y(function (d) {
      return that.y0(d.lightLevel);
    });

  // define the data line for the temperature
  this.temperatureLine = d3.line()
    .x(function (d) {
      return that.x(d.date);
    })
    .y(function (d) {
      return that.y1(d.temperature);
    });

  // append the svg object to the body of the page
  // appends a 'group' element to 'svg'
  // moves the 'group' element to the top left margin
  this.svg = domContainer.append("svg")
    .attr("width", this.width + this.margin.left + this.margin.right)
    .attr("height", this.height + 50 + this.margin.top + this.margin.bottom)
    .append("g")
    .attr("transform",
      "translate(" + this.margin.left + "," + this.margin.top + ")");
};

AquaNetChart.prototype.parseTime = d3.timeParse("%Y-%m-%d %H:%M:%S");
  
// Get the data
AquaNetChart.prototype.getData = function getData() {
  var that = this;
  var now = new Date();
  var today = dateFormat(now, "yyyy-mm-dd");
  var startDateElt = document.querySelector('#startDate input');
  if (!startDateElt.value) {
    startDateElt.value = '2016-10-04'; // today;
  }
  var startDate = startDateElt.value;
   var endDateElt = document.querySelector('#endDate input');
  if (!endDateElt.value) {
    endDateElt.value = '2016-10-06'; // today;
  }
  var endDate = endDateElt.value;
  
  d3.json("getStat.php?h=3370&sd=" + encodeURIComponent(startDate) + "&ed=" + encodeURIComponent(endDate), function (error, data) {
    if (error) throw error;
    // format the data
    data.forEach(function (d) {
      d.date = that.parseTime(d.date);
      d.temperature = parseFloat(d.temperature);
    });
    // Scale the range of the data
    that.x.domain(d3.extent(data, function (d) {
      return d.date;
    }));
    that.y0.domain([0, 1024]);
    that.y1.domain([0, 30]);
    // Clear previous lines
    var lightLine = document.body.querySelector('svg path.lightLine');
    if (lightLine) lightLine.remove();
    var tempLine = document.body.querySelector('svg path.tempLine');
    if (tempLine) tempLine.remove();
    var lightAxis = document.body.querySelector('svg g.lightAxis');
    if (lightAxis) lightAxis.remove();
    var tempAxis = document.body.querySelector('svg g.tempAxis');
    if (tempAxis) tempAxis.remove();
    var xAxis = document.body.querySelector('svg g.xAxis');
    if (xAxis) xAxis.remove();
    
    // Add the lightLine path.
    that.svg.append("path")
      .data([data])
      .attr("class", "lightLine")
      .attr("d", that.lightLine);
    that.svg.append("path")
      .data([data])
      .attr("class", "tempLine")
      .attr("d", that.temperatureLine);
    // Add the X Axis
    that.svg.append("g")
      .attr("class", "xaxis")
      .attr("transform", "translate(0," + that.height + ")")
      .call(d3.axisBottom(that.x)
        .tickFormat(d3.timeFormat("%Y-%m-%d %H:%M")))
      .selectAll("text")
      .style("text-anchor", "end")
      .attr("dx", "-.8em")
      .attr("dy", ".15em")
      .attr("transform", "rotate(-65)");
    // Add the Y Axis
    that.svg.append("g")
      .attr("class", "lightAxis")
      .call(that.yAxisLeft);

    that.svg.append("g")
      .attr("class", "tempAxis")
      .attr("transform", "translate(" + that.width + " ,0)")
      .call(that.yAxisRight);
  });
};

function initGraph() {
  var chart = new AquaNetChart(d3.select("body"), {top: 20, right: 50, bottom: 100, left: 50}, 960, 550);
  chart.getData();
  return chart;
}
