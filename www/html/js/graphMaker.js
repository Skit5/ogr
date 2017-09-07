$(function() {
	var sin = [], cos = [];
	for (var i = 0; i < 14; i += 0.1) {
		sin.push([i, Math.sin(i)]);
		cos.push([i, Math.cos(i)]);
	}
	var curves = getLines();
	var fdata = [];
	console.log(curves);
	$(getLines()).each(function(i, item){
		console.log("yo");
		console.log(item);
		console.log(i);
		fdata.push({'data' : item[0], 'label': "P"});
		//fdata.push({data : item['C'], label: "C", yaxis:2});
	});
	console.log(curves[0]);

	$.plot("#placeholder", fdata, {
		series: {
			lines: {
				show: true
			}
		},
		xaxes: [ { mode: "time" } ],
		yaxes: [ { min: 0 }, {
			// align if we are to the right
			alignTicksWithAxis: 1,
			//position: right,
			//tickFormatter: euroFormatter
		} ],
		crosshair: {
			mode: "x"
		},
		grid: {
			hoverable: true,
			autoHighlight: false
		},
		legend: { position: "sw" }
	});

	/*plot = $.plot("#placeholder", [
		{ data: curves['C'], label: "sin(x) = -0.00"},
		{ data: curves['P'], label: "cos(x) = -0.00" }
	], {
		series: {
			lines: {
				show: true
			}
		},
		crosshair: {
			mode: "x"
		},
		grid: {
			hoverable: true,
			autoHighlight: false
		},
		yaxis: {
			min: 0,
			max: 120
		}
	});*/

	var legends = $("#placeholder .legendLabel");

	legends.each(function () {
		// fix the widths so they don't jump around
		$(this).css('width', $(this).width());
	});

	var updateLegendTimeout = null;
	var latestPosition = null;

	function updateLegend() {

		updateLegendTimeout = null;

		var pos = latestPosition;

		var axes = plot.getAxes();
		if (pos.x < axes.xaxis.min || pos.x > axes.xaxis.max ||
			pos.y < axes.yaxis.min || pos.y > axes.yaxis.max) {
			return;
		}

		var i, j, dataset = plot.getData();
		for (i = 0; i < dataset.length; ++i) {

			var series = dataset[i];

			// Find the nearest points, x-wise

			for (j = 0; j < series.data.length; ++j) {
				if (series.data[j][0] > pos.x) {
					break;
				}
			}

			// Now Interpolate

			var y,
				p1 = series.data[j - 1],
				p2 = series.data[j];

			if (p1 == null) {
				y = p2[1];
			} else if (p2 == null) {
				y = p1[1];
			} else {
				y = p1[1] + (p2[1] - p1[1]) * (pos.x - p1[0]) / (p2[0] - p1[0]);
			}

			legends.eq(i).text(series.label.replace(/=.*/, "= " + y.toFixed(2)));
		}
	}

	$("#placeholder").bind("plothover",  function (event, pos, item) {
		latestPosition = pos;
		if (!updateLegendTimeout) {
			updateLegendTimeout = setTimeout(updateLegend, 50);
		}
	});

	// Add the Flot version string to the footer

	$("#footer").prepend("Flot " + $.plot.version + " &ndash; ");
 	//$("#glob-container").css('height', "100%");
 	//updateGraphHeight(); 
});

$(window).on('resize',updateGraphHeight());

function updateGraphHeight(){
	var h =$(window).height();
	$("#graphcontainer").height(h - $('#header').height() - $('#footer').height());
}