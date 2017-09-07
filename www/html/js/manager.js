window.onload = function(){
	var entries = '';
	$.post("/api/controller.php",
		{'q':'list'},
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
	});
};

function setListEntries(list2set){
	var entries = '';
	$.each(list2set, function(i, item){
		entries += "<tr>\
			<td>"+item['nom']+"</td>\
			<td>"+item['details']+"</td>\
			<td>"+item['constructor']+"</td>\
			<td>"+dateToMonthYear(item['constructionDate'])+"</td>\
			<td>"+item['insertDate']+"</td>\
	      	<td><input type='checkbox' name='checks[]' value='"+item['id']+"'></td>\
		</tr>";
	});
	return entries;
}
function getCheckedEntries(){
	var cEntries = [];
	$('input[name="checks[]"]').each(function(){
		if($(this).is(':checked'))
			cEntries.push($(this).val());
	});
	return cEntries;
}

$("#viewTool").click(function(){
	var cEntries = getCheckedEntries();
	console.log(cEntries);
	if(cEntries.length > 0){
		window.open('applet.html?v='+encodeURIComponent(cEntries), '_blank');
	}
});

function dateToMonthYear(date2conv){
	var sliced = date2conv.split('-');
	var sMonth = '';
	switch(sliced[1]){
		case "01":
			sMonth = "Jan";
			break;
		case "02":
			sMonth = "Fév";
			break;
		case "03":
			sMonth = "Mars";
			break;
		case "04":
			sMonth = "Avr";
			break;
		case "05":
			sMonth = "Mai";
			break;
		case "06":
			sMonth = "Juin";
			break;
		case "07":
			sMonth = "Juil";
			break;
		case "08":
			sMonth = "Août";
			break;
		case "09":
			sMonth = "Sept";
			break;
		case "10":
			sMonth = "Oct";
			break;
		case "11":
			sMonth = "Nov";
			break;
		case "12":
			sMonth = "Déc";
			break;
		default:
			sMonth = "";
			break;
	} 
	return sMonth+" "+sliced[0];
}

$('#searchbar input').on('input',function(){
	var customQuery = this.value;
	var entries = '';
	$.post("/api/controller.php",
		{'q':'list',
		'toSearch':customQuery},
		function(data){
		console.log(data);
		if(data.length > 0){
			entries = setListEntries(data);
		}else{
			entries = "<tr>\
				<td colspan=4>\
				Pas de résultat correspondant à votre recherche.\
				</td>\
			</tr>";
		}
		$('#list-displayer tbody').html(entries);
	});
});