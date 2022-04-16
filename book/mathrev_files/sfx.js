var	version = 1;
var	appname = "";

if (navigator.appName.indexOf("Netscape") >= 0) {
	appname = "ns"; }
else if (navigator.appName.indexOf("Explorer") >= 0) {
	appname = "ie"; }

if (navigator.userAgent.indexOf("Mozilla/2") >= 0) {
	version = 2;
} else if (navigator.userAgent.indexOf("Mozilla/3") >= 0) {
	version = 3;
} else if (navigator.userAgent.indexOf("Mozilla/4") >= 0) {
	version = 4;
}

function openWin(BASEURL, sid, object_info) {
var url = BASEURL + "?" + "sid=" + sid + "&" + object_info;
window2  =    window.open(url , "SFXmenu" ,
			"location=no,"		+
			"status=yes,"		+
			"menubar=no,"		+
			"scrollbars=yes,"	+
			"resizable=yes,"	+
			"width=460,"		+
			"height=520");
if ((version>2 && appname=="ns")||(version>3 && appname== "ie")) {
		window2.focus();
	}
}

function SFXButton(BASEURL, sid, button, object_info) {
if (BASEURL.length < 4 && BASEURL.indexOf("http:") < 0) return;
var url = "javascript:openWin('"+ BASEURL +"', '" + sid + "', '" + object_info + "')";
var anchorStr  = "<img src=" + button + " alt=\"More links\" BORDER=\"0\">";
document.write( '<a href="' + url + '">' + anchorStr + '</a>' );
return true;
}
