<html>
  <head>
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript" src="//ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js"></script>
    <script type="text/javascript">

    google.charts.load('current', {'packages':['corechart', 'line']});
    google.charts.setOnLoadCallback(drawChart);
      
    function drawChart() {
		var jsonData = $.ajax({
			url: "getData.php",
			dataType: "json",
			async: false
			}).responseText;
          
		var data = new google.visualization.DataTable(jsonData);

        var options = {
			aggregationTarget: 'category',
			legend: 'none',
			series: {
				0: { lineWidth: 0, color: '#cccc00', areaOpacity: 0.1 },
				1: { type: 'line', color: '#7f0000' },
				2: { lineWidth: 0, color: '#eeee00', areaOpacity: 0.2 }
			},
			title: 'Vandaag',
			seriesType: 'area',
			vAxis: {
				minValue: 0
			},
			hAxis: {
				format: 'MM-dd HH:mm',
				slantedText: true,
				gridlines: {
					count: 24
				}
			},
			theme: 'material',
			interpolateNulls: false, 
			width: 1200,
			height: 400
        };

        var chart = new google.visualization.ComboChart(document.getElementById('chart_div'));
        chart.draw(data, options);
    }

    </script>
  </head>

  <body>
    <div id="chart_div"></div>
  </body>
</html>
