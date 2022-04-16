function addLoadEvent(func) {
  var oldonload = window.onload;
  if (typeof window.onload != 'function') {
    window.onload = func;
  } else {
    window.onload = function(e) {
      if (oldonload) {
        oldonload(e);
      }
      func(e);
    }
  }
}

function shrinkFontsForSmallScreens(event)
{
	if( document.body.clientWidth <= 825 ){
		document.body.style.fontSize = '14px';
	}else{
		document.body.style.fontSize = '16px';
	}
}

window.onresize = shrinkFontsForSmallScreens;
addLoadEvent( shrinkFontsForSmallScreens );
