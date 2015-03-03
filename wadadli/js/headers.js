//Load Date Time Pickers
var dateInputs = [];
function addDateInput(id,value,minDate,maxDate,format,timepicker,datepicker){
	dateInputs.push({id:id,value:value,minDate:minDate,maxDate:maxDate,format:format,timepicker:timepicker,datepicker:datepicker});
}

//Load Colour Pickers
var colourInputs = [];
function addColourInput(id,colour){
	colourInputs.push({id:id,colour:colour});
}

//Load Text Editors
var textEditors = [];
function addTextEditor(id){
	textEditors.push({id:id});
}