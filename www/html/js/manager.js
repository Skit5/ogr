window.onload = function(){
	var entries = '';
	$.post("/api/controller.php",
		{'q':'list'},
		function(data){
			console.log(data)
		if(data.length > 0){
			$.each(data, function(i, item){
				entries += "<tr>\
					<td>"+item['nom']+"</td>\
					<td>"+item['details']+"</td>\
					<td>"+item['insertDate']+"</td>\
				</tr>";
			});
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