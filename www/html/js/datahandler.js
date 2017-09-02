window.onload = function(){
	var vehicules = getParams();
	var dataPoints = [];
	$(vehicules).each(function(i, item){
		$.post("/api/controller.php",
		{'q':'read', 'vhc': item},
		function(data){
			addCurve(item, data);
		});
		console.log(item);
	});
	/*var entries = '';
	$.post("/api/controller.php",
		{'q':'read'},
		function(data){
		if(data.length > 0){
			entries = setListEntries(data);
		}else{
			entries = "<tr>\
				<td colspan=4>\
				Pas d\'entrée dans la liste\
				</td>\
			</tr>";
		}
		$('#list-displayer tbody').html(entries);
	});*/
};

function addCurve(vhc, entries){
	var dataPoints = [],
		w = [],
		P = [],
		C = [];
	$(entries).each(function(i, item){
		/*dataPoints.push({
			w : item['w'],
			P : item['P'],
			C : 715.95*item['P']/item['w']
		});*/
		w.push(item['w']);
		C.push(item['C']);
		P.push(item['C']*item['w']/702.35);
	});
	console.log(dataPoints);
	/*var ctx = new Chart(document.getElementById("myChart").getContext("2d"), {
		type: "line",
		data: dataPoints
	});*/

	var canvas = document.getElementById('myChart');
	new Chart(canvas, {
	  type: 'line',
	  data: {
	    labels: w,
	    datasets: [{
	      label: vhc+' P',
	      yAxisID: 'P',
	      data: P
	    }, {
	      label: vhc+' C',
	      yAxisID: 'C',
	      data: C
	    }]
	  },
	  options: {
	    scales: {
	      yAxes: [{
	        id: 'P',
	        type: 'linear',
	        position: 'left',
	      }, {
	        id: 'C',
	        type: 'linear',
	        position: 'right',
	      }]
	    }
	  }
	});

	/*new Morris.Line({
	  // ID of the element in which to draw the chart.
	  element: 'myfirstchart',
	  // Chart data records -- each entry in this array corresponds to a point on
	  // the chart.
	  data: dataPoints,
	  // The name of the data record attribute that contains x-values.
	  xkey: 'w',
	  // A list of names of data record attributes that contain y-values.
	  ykeys: ['P','C'],
	  // Labels for the ykeys -- will be displayed when you hover over the
	  // chart.
	  labels: ['P','C']
	});*/
}

function getParams(){
	var vList = decodeURIComponent(window.location.search.substring(3));
	return vList.split(',');
}