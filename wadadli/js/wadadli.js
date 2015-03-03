$(document).ready(function(){
	setupInteractiveInputs();
});

function setupInteractiveInputs(){
	setupDates();
	setupColourPickers();
	setupTextEditors();
}

function setupDates(){
	for(var i = 0; i < dateInputs.length; i++){
		var dateInput = dateInputs[i];
		$('#'+dateInput.id).datetimepicker({defaultDate:dateInput.value,validateOnBlur:false,minDate:dateInput.minDate,maxDate:dateInput.maxDate,step:30,
			format:dateInput.format,timepicker:dateInput.timepicker,closeOnDateSelect:!dateInput.timepicker,datepicker:dateInput.datepicker});
	}
	
	dateInputs = Array();
}

function setupColourPickers(){
	for(var i = 0; i < colourInputs.length; i++){
		var colourInput = colourInputs[i];
		$('#'+colourInput.id).ColorPicker({
			color:colourInput.colour,
			onSubmit: function(hsb, hex, rgb, el) {
				$(el).val(hex);
				$(el).ColorPickerHide();
			},
			onBeforeShow: function () {
				$(this).ColorPickerSetColor(this.value);
			}
		})
		.bind('keyup', function(){
			$(this).ColorPickerSetColor(this.value);
		});
	}
	
	colourInputs = Array();
}

var configuredTextEditors = [];
function setupTextEditors(){
	for(var i = 0; i < textEditors.length; i++){
		var textEditor = textEditors[i];
		var editor = new TINY.editor.edit('editor',{
			id: textEditor.id,
			width:500,
			height:300,
			cssclass: 'tinyeditor',
			controlclass: 'tinyeditor-control',
			rowclass: 'tinyeditor-header',
			dividerclass: 'tinyeditor-divider',
			controls: ['bold', 'italic', 'underline','|',
			'orderedlist', 'unorderedlist', '|', 'outdent', 'indent', '|', 'leftalign',
			'centeralign', 'rightalign', 'blockjustify', '|', 'unformat', '|', 'undo', 'redo', 'n',
			'font', 'size', 'style', '|', 'image', 'hr', 'link', 'unlink'],
			footer: true,
			fonts: ['Verdana','Arial','Georgia','Trebuchet MS'],
			xhtml: true,
			cssfile: 'custom.css',
			bodyid: 'editor',
			footerclass: 'tinyeditor-footer',
			toggle: {text: 'source', activetext: 'wysiwyg', cssclass: 'toggle'},
			resize: {cssclass: 'resize'}
		});
		
		$('.tinyeditor-header').click(function(e){
			editor.post();
		});
		
		$('.tinyeditor-header select').change(function(e){
			editor.post();
		});
		
		$('body',$('#'+textEditor.id + '+ iframe').contents()).keyup(function(e){
			editor.post();
		});
	}
	
	textEditors = Array();
}

//Sends the user to the resource located at URL
function redirect(url){
	location.href = url;
}

//Loads HTML content using AJAX and inserts it into the current DOM dynamically
var loadedViews = Array();
function loadView(){
	var loadArguments = arguments;
	
	var actionUrl = loadArguments[0];
	var target = loadArguments[1];
	var isDynamic = loadArguments[2];
	var callback = loadArguments[3];
	var callbackArg = loadArguments[4];
	
	var actionArguments = Array();
	for(i = 5; i < loadArguments.length-1; i++){
		actionArguments.push(loadArguments[i]);
	}
	
	var actionElement = loadArguments[loadArguments.length-1];
	if(actionElement){
		var menu = actionElement.parentElement;
	}
	
	for(var i=0; i < actionArguments.length; i++){
		var nextArgument = actionArguments[i];
		if(isDynamic){
			var originalArgument = nextArgument;
			nextArgument = $('#'+originalArgument).val();
			if(!nextArgument){
				nextArgument = $('#'+originalArgument).html();
				
				if(!nextArgument){
					nextArgument = originalArgument;
				}
			}
		}
		
		actionUrl = actionUrl.replace('@',nextArgument);
	}
	
	var scripts= document.getElementsByTagName('script');
	var path= scripts[scripts.length-1].src.split('?')[0];
	var mydir= (path.split('/').slice(0, -1).join('/')+'/').replace('js','wadadli/img'); 
	$('#'+target).html("<div class='viewLoadingStatus'><span>Loading</span><img id='loadingImage' src='"+mydir+"loading.gif' /></div>");
	
	$.get(actionUrl, function(data){
		$('#'+target).html(data);
		$('#'+target).load();
		$('.selected').removeClass('selected');
		$(actionElement).addClass('selected');
		$(this).addClass('selected');
		loadedViews[target] = loadArguments;
		setupInteractiveInputs();
		
		if(callback){
			var func = window[callback];
			func(callbackArg,actionElement);
		}
	});
}

//Submits a form asynchronously returning the results to the target window
function submitAsyncForm(actionUrl,target,callback,callbackArg,form){
	if(actionUrl){
		var data = new FormData($(form)[0]);
		var scripts= document.getElementsByTagName('script');
		var path = scripts[scripts.length-1].src.split('?')[0];
		var mydir= (path.split('/').slice(0, -1).join('/')+'/').replace('js','wadadli/img'); 
		$('#'+target).html("<div class='viewLoadingStatus'><span>Loading</span><img id='loadingImage' src='"+mydir+"loading.gif' /></div>");

		$.ajax({url: actionUrl, data: data, 
			type: 'POST',
			contentType: false,
			processData: false,
			success: function(data){
				$('#'+target).html(data);
				$('#'+target).load();
				setupInteractiveInputs();
				
				if(callback){
					var func = window[callback];
					func(callbackArg);
				}
			}
		});
	}else if(callback){
		var func = window[callback];
		func(callbackArg);
	}
	
	return false;
}

//Manages the controls for the Wadadli Image Selector
var defaultImage = "";
function toggleImageMenu(e){
	if(!defaultImage){
		defaultImage = $(e).parent().next().find('img').attr('src').replace(/img.*/g,"wadadli/img/default.png");
	}
	
	$(e).next().toggle();
}

function uploadImage(e){
	$(e.parentElement.parentElement).next().next().find('input').val($(e).find('input').val());
	toggleImageMenu($(e.parentElement).prev());
}

function deleteImage(e){
	$(e.parentElement.parentElement).next().next().find('input').val('No Image');
	$('#'+e.id.replace('Link','TextField')).val('true');
	$(e.parentElement.parentElement).next().attr('src',defaultImage);
	toggleImageMenu($(e.parentElement).prev());
}

//Manages the controls for the Wadadli Form List Entry Component
function addObjectFormCollectionElement(template,link){
	var collectionContainer = link.parentElement;
	var elementTemplate = document.getElementById(template+'Template');
	
	var newElement = document.createElement('div');
	newElement.className = 'objectFormCollectionElement';
	
	var entryComponent = elementTemplate.cloneNode(true);
	entryComponent.id = template+'[]';
	entryComponent.className = entryComponent.className.replace("hidden","");
	newElement.appendChild(entryComponent);
	
	var removeLink = document.createElement('a');
	removeLink.innerHTML = 'Remove';
	removeLink.className = "wadadliAnchor removeObjectCollectionElementLink";
	removeLink.onclick = function(){
		removeObjectFormCollectionElement(removeLink);
	};
	newElement.appendChild(removeLink);
	
	collectionContainer.insertBefore(newElement,link);
}

function removeObjectFormCollectionElement(e){
	var collectionElement = e.parentElement;
	collectionElement.parentElement.removeChild(collectionElement);
}

//Handles WadadliPasswordConfirmField Component toggle
function showPasswordFields(button){
	var passwordFieldContainer = button.parentElement;
	$('.wadadliPasswordField').show();
	$('.wadadliPasswordField input').prop('disabled', false);
	
	$(button).html('- Hide Password');
	button.onclick = null;
	$(button).unbind();
	$(button).click(function(){
		hidePasswordFields(button);
	});
}

function hidePasswordFields(button){
	var passwordFieldContainer = button.parentElement;
	$('.wadadliPasswordField').hide();
	$('.wadadliPasswordField input').prop('disabled', true);
	
	$(button).html('+ Change Password');
	button.onclick = null;
	$(button).unbind();
	$(button).click(function(){
		showPasswordFields(button);
	});
}

//Rating Star Scripts
function updateStarRating(rating,star){
	var ratingComponent = star.parentElement;
	
	$(ratingComponent).find('img').each(function(index){
		$(this).prop('src',$(this).prop('src').replace('img/rated','img/unrated'));
		
		if(index < rating){
			$(this).prop('src',$(this).prop('src').replace('img/unrated','img/rated'));
		}
	});
	
	$(ratingComponent).find('input').val(rating);
}