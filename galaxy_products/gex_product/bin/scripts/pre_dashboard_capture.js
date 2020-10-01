// Script launched just before capturing a report/dashboard from the BI server if any

/*
var divs = document.getElementsByTagName('div');
for(var i=0; i<divs.length; i++)
    if(divs[i].className == 'userBar')
        divs[i].style.display = 'none';

var divs = document.getElementsByTagName('div');
    for(var i=0; i<divs.length; i++)
        if(divs[i].className == 'prismBar')
            divs[i].style.display = 'none';

var divs = document.getElementsByTagName('iframe');
    for(var i=0; i<divs.length; i++)
        divs[i].scrolling = 'no';
*/

document.bgColor='#456287';

var buttons = document.getElementsByTagName('button');
for(var i=0; i<buttons.length; i++)
    buttons[i].style.display = 'none';

var rects = document.getElementsByTagName('rect');
for(var i=0; i<rects.length; i++)
	if ( rects[i].style.width=='1327')
		rects[i].style.display='none'; //rects[i].style.fill="#FFFFFF"

var as = document.getElementsByTagName('a');
for(var i=0; i<as.length; i++)
    if(as[i].className == 'rangepickerapplybutton')
		as[i].style.display='none';

// hide main-loading-inner ?

var pwts = document.getElementsByTagClass('prism_widget_toolbar');
for(var i=0; i<pwts.length; i++)
	pwts[i].style.display='none';
