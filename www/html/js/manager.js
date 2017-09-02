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
			<td>"+item['insertDate']+"</td>\
	      	<td><input type='checkbox' name='checks[]' value='"+item['nom']+"'></td>\
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
		cEntries.push("yololo 2");
		window.open('applet.html?v='+encodeURIComponent(cEntries), '_blank');
	}
});