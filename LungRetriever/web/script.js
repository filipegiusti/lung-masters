function toggleButton(e) {
	var array = e.className.split(' ');
	var newClasses = new String('');
	
	var first = new Boolean(true);
	
	for( var classe in array ) {
		if (!first) {
			newClasses += ' ';
		}
		if (array[classe] == 'on') {
			newClasses += 'off';
		} else if (array[classe] == 'off') {
			newClasses += 'on';
		} else {
			newClasses += array[classe];
		}
		first = false;
	}
	e.className = newClasses;
}

function isButtonOn(e) {
	var array = e.className.split(' ');
	for( var classe in array ) {
		if(array[classe] == 'on') {
			return true;
		}
	}
	
	return false;
}

function buttonClick(e) {
	if (!isButtonOn(e)) {
		var divs = e.parentNode.getElementsByClassName('button');
		var i = 0;
		for (i = 0; i < divs['length']; i++) {
			if(divs[i] != e) {
				if(isButtonOn(divs[i])) {
					toggleButton(divs[i]);
				}
			}
		}
	}
	toggleButton(e);
}

var evaluationMode = false;
function toggleEvaluationMode() {
	evaluationMode = !evaluationMode;
	
	var e = document.getElementsByClassName('evaluation-mode');
	var i = 0;
	for (i = 0; i < e['length']; i++) {
		e[i].className = e[i].className.replace(/off/g, "")
		if (!evaluationMode) {
			e[i].className += ' off' ;
		}
	}	
}

function in_array(agulha, palheiro) {
	 for (key in palheiro) {
        if (palheiro[key] == agulha) {
            return true;
        }
    }
 
    return false;
}

function aval(e) {
	var div = document.getElementById('retrieved-images');
	var buttonsDiv = div.getElementsByClassName('RWbuttons');
	for (i = 0; i < buttonsDiv.length; i++) {
		 var buttons = buttonsDiv[i].getElementsByClassName('button');
		 var imageId = buttonsDiv[i].childNodes[1].nodeValue;
		 var hidden = document.getElementById('similar'+i);
		 var value = 0;
		 var classesYes = buttons[0].className.split(' ');
		 var classesNo = buttons[1].className.split(' ');
		 if (in_array('on', classesYes)){
		 	value += 1;
		 }
		 if (in_array('on', classesNo)){
		 	value -= 1;
		 }
		 hidden.value = imageId+','+value;
	}
}