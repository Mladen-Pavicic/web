// JavaScript Document
function bws()
{
	d=document;
	this.agt=navigator.userAgent.toLowerCase();
	this.major=parseInt(navigator.appVersion);
	this.dom=(d.getElementById);
	this.ns=(d.layers);
	this.ns4up=(this.ns && this.major>=4);
	this.ns6=(this.dom&&navigator.appName=="Netscape");
	this.op=(window.opera);
	if(d.all)this.ie=1;else this.ie=0;
	this.ie4=(d.all&&!this.dom);
	this.ie4up=(this.ie&&this.major>=4);
	this.ie5=(d.all&&this.dom);
	this.ie6=(d.nodeType);
	this.sf=(this.agt.indexOf("safari")!=-1);
	this.win=((this.agt.indexOf("win")!=-1)||(this.agt.indexOf("16bit")!=-1));
	this.winme=(this.agt.indexOf("win 9x 4.90")!=-1);
	this.xpsp2=(this.agt.indexOf("sv1")!=-1);
	this.mac=(this.agt.indexOf("mac")!=-1);
}
var oBw=new bws();

var ie4 = (document.all) ? true : false;
var ns4 = (document.layers) ? true : false;
var ns6 = (document.getElementById && !document.all) ? true : false;

function hidelayer(lay) 
{
	if (ie4) {document.all[lay].style.visibility = "hidden";}
	if (ns4) {document.layers[lay].visibility = "hide";}
	if (ns6) {document.getElementById([lay]).style.display = "none";}
}
function showlayer(lay) 
{
	if (ie4) {document.all[lay].style.visibility = "visible";}
	if (ns4) {document.layers[lay].visibility = "show";}
	if (ns6) {document.getElementById([lay]).style.display = "block";}
}

var bkIsEnter = true;
function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_showHideLayers() { //v6.0
  var i,p,v,obj,args=MM_showHideLayers.arguments;
  for (i=0; i<(args.length-2); i+=3) if ((obj=MM_findObj(args[i]))!=null) { v=args[i+2];
    if (obj.style) { obj=obj.style; v=(v=='show')?'visible':(v=='hide')?'hidden':v; }
    obj.visibility=v; }
}

function MM_goToURL() { //v3.0
  var i, args=MM_goToURL.arguments; document.MM_returnValue = false;
  for (i=0; i<(args.length-1); i+=2) eval(args[i]+".location='"+args[i+1]+"'");
}

function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}
	
function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}

function MM_openBrWindow(theURL,winName,features) { //v2.0
  window.open(theURL,winName,features);
}


// josip izmjenio 

function MM_validateForm() { //v4.0
  var i,p,q,nm,test,num,min,max,errors='',args=MM_validateForm.arguments;
  for (i=0; i<(args.length-2); i+=3) { test=args[i+2]; val=MM_findObj(args[i]);
    if (val) { nm=val.name; if ((val=val.value)!="") {
      if (test.indexOf('isEmail')!=-1) { p=val.indexOf('@');
        if (p<1 || p==(val.length-1)) errors+='- '+nm+' mora biti e-mail adresa.\n';
      } else if (test!='R') { num = parseFloat(val);
        if (isNaN(val)) errors+='- '+nm+' must contain a number.\n';
        if (test.indexOf('inRange') != -1) { p=test.indexOf(':');
          min=test.substring(8,p); max=test.substring(p+1);
          if (num<min || max<num) errors+='- '+nm+' must contain a number between '+min+' and '+max+'.\n';
    } } } else if (test.charAt(0) == 'R') errors += '- '+nm+' obavezno polje.\n'; }
  } if (errors) alert('Greška u formi:\n'+errors);
  document.MM_returnValue = (errors == '');
}

//////////////////////////
//	Table DOM functions
//////////////////////////
function AppendRow(tid)
{
	var tbl = document.getElementById(tid);
	var newRow = tbl.insertRow(tbl.rows.length);
	var newCell = newRow.insertCell(0);
	newCell.innerHTML = 'Hello World!';
}

function AppendRow(tid, cNo, t)
{
	var tbl = document.getElementById(tid);
	var newRow = tbl.insertRow(tbl.rows.length);
	for(i=0;i<cNo;i++)
	{
		var newCell = newRow.insertCell(0);
		switch(t)
		{
			case 1:
				newCell.innerHTML = '<INPUT type="text">';
			break;
			default:
			break;	
		}
		//newCell.innerHTML = 'Hello World!';
	}
}

function AppendRow(tid, cNo, Itype, Iname)
{
	var tbl = document.getElementById(tid);
	var newRow = tbl.insertRow(tbl.rows.length);
	var SplitISname = Iname.split(',');
	var L = SplitISname.length; 
	for(i=0 ; i < cNo && i < L ; i++)
	{
		var newCell = newRow.insertCell(0);
		var INPUTNAME = SplitISname[i]+(tbl.rows.length-1);
		newCell.innerHTML = '<INPUT type="'+Itype+'" name="'+INPUTNAME+'">';
		//newCell.innerHTML = 'Hello World!';
	}
}

function jkAppRow(tid,max,it,v,n,css,a,msg)
{
	var tbl = document.getElementById(tid);
	if(tbl.rows.length < max)
	{
		var newRow = tbl.insertRow(tbl.rows.length);
		var cNo = tbl.rows[0].cells.length;
		for(i = 0; i < cNo; i++)
		{
			var newCell = newRow.insertCell(i);
			if(it.split(',')[i].length > 2)
			{
				if(it.split(',')[i]=='[count]')
					newCell.innerHTML = '3.9.'+(tbl.rows.length)+'.'
				else
					newCell.innerHTML = '<INPUT maxlength=50 type="'+it.split(',')[i]+'" value="'+v.split(',')[i]+'" name="'+n.split(',')[i]+'" class="'+css.split(',')[i]+'" title="'+a.split(',')[i]+'">';
			}
			else
			{
				newCell.innerHTML = '';
			}
		}
	}
	else
	{
		alert(msg);
	}
}

function AppendRowWithNo(tid, cNo, cnt, Itype, Iname, InnerHTM, NoV, css)
{
	var tbl = document.getElementById(tid);
	var newRow = tbl.insertRow(tbl.rows.length);
	var SplitISname = Iname.split(',');
	var L = SplitISname.length;
	var N = NoV.split(',');
	var IneerH = InnerHTM.split('||');
	var z=0;
	
	for(i=0;i<cNo&&i<L;i++)
	{
		var newCell = newRow.insertCell(i);
		var INPUTNAME = SplitISname[i]+(tbl.rows.length-1);
		if(i == 0)
		{
			newCell.innerHTML = cnt + (tbl.rows.length-2) + '.'
		}
		else
		{
			if(parseInt(N[z])==(i+1))
			{
				//alert(IneerH[z]);
				newCell.innerHTML = '<INPUT type="'+Itype+'" class="'+css+'" name="'+INPUTNAME+'" '+IneerH[z]+' >';
				z++;
			}
			else
			{
				newCell.innerHTML = '<INPUT type="'+Itype+'" class="'+css+'" name="'+INPUTNAME+'">';
			}
		}
	}
}

function EnableObject(o)
{
	var obj = document.getElementById(o);
	if(obj!=null)
	{
		alert("ima objekta!")
		if(obj.disabled)
			obj.disabled = false;
		else
			obj.disabled = true;
	}
	else
	{
		alert("nema objekta!")
	}
}
/*
function AddSelectOption(c,itm,val)
{
	var obj = document.getElementById(c);
	if(obj != null)
	{
		alert("len: " + obj.options.length)
		op = new Option(itm,val)
		obj.options.add(op)
	}
}
*/
function DeleteLastRow(tid)
{
	var tbl = document.getElementById(tid);
	if (tbl.rows.length > 0) tbl.deleteRow(tbl.rows.length - 1);
}

function InsertRow(tid, txtIndex, txtError)
{
	var tbl = document.getElementById(tid);
	var rowIndex = document.getElementById(txtIndex).value;
	try {
		var newRow = tbl.insertRow(rowIndex);
		var newCell = newRow.insertCell(0);
		newCell.innerHTML = 'Hello World! insert';
	} catch (ex) {
		document.getElementById(txtError).value = ex;
	}
}

function DeleteRow(tid, txtIndex, txtError)
{
	var tbl = document.getElementById(tid);
	var rowIndex = document.getElementById(txtIndex).value;
	try {
		tbl.deleteRow(rowIndex);
	} catch (ex) {
		document.getElementById(txtError).value = ex;
	}
}

//////////////////////////
//	/Table DOM functions
//////////////////////////

function NewWin(frm,name,width,height,scrollbars,resize,toolbar,status,location,menubar)
{
	//alert("par: " + "width="+width+",height="+height+",scrolbars="+scrollbars+",resizable="+resize+",toolbar="+toolbar+",status="+status+",location="+location+",menubar="+menubar)
	return window.open(frm,name,"width="+width+",height="+height+",scrollbars="+scrollbars+",resizable="+resize+",toolbar="+toolbar+",status="+status+",location="+location+",menubar="+menubar);
	
}

function getObj(obj) {
  var theObj;
  if(document.all)
  {
    if(typeof obj=="string")
    	return document.all(obj);
    else 
    	return obj.style;
  }
  if(document.getElementById)
  {
    if(typeof obj=="string")
		return document.getElementById(obj);
	else 
		return obj.style;
  }
  return null;
}

function CharCount(en,ex,txt,c)
{
  var enObj=getObj(en);
  var exObj=getObj(ex);
  var cnnt=c-enObj.value.length;
  if(cnnt <= 0)
  {
    cnnt=0;
    txt='<span class="disable">&nbsp;'+txt+'&nbsp;</span>';
    enObj.value=enObj.value.substr(0,c);
  }
  exObj.innerHTML = txt.replace("{CHAR}",cnnt);
}

function DisableOnEnterSubmit() 
{ 
	if (event.keyCode == 13) 
	{ 
		event.cancelBubble = true; 
		event.returnValue = false; 
	} 
} 

function SetDefaultButton(btn,event)
{
	if (document.all)
	{
		if (event.keyCode == 13)
		{
			event.returnValue=false;
			event.cancel = true;
			btn.click();
		}
	}
	else if (document.getElementById)
	{
		if (event.which == 13)
		{
			event.returnValue=false;
			event.cancel = true;
			btn.click();
		}
	}
	else if(document.layers)
	{
		if(event.which == 13)
		{
			event.returnValue=false;
			event.cancel = true;
			btn.click();
		}
	}
}

function HideErrMessage(oDiv)
{
	var obj = document.getElementById(oDiv);
	obj.style.display='none';
}

function AutoSave(o)
{
	var objBtn = document.getElementById(o)
	if(objBtn != null)
		objBtn.click();
}

function GetFileExt(oid)
{	
	var ret = "[unknown]";
	var o = document.getElementById(oid);
	if(o != null)
	{	
		if(o.value.length > 2)
		{
			if(o.value.lastIndexOf(".") > 0)
			{
				//var t = o.value.lastIndexOf(".");
				ret = o.value.substr(o.value.lastIndexOf(".")+1,o.value.length);
			}
		}
	}
	
	return ret;
}


//Skripta: Sortiranje DataGrid preko JavaScripta
//Autor: Saša Mandiæ
//Datum: 01.06.2006
 
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 
//var GridID="<%=dgProjekti.ClientID %>"; 
var LastFilter; // Koji je zadnji filter korišten.
 
//+++++++++++++++++++++++++++++++++++++++++++++++++ŠMINKA++++++++++++++++++
function MouseIn(object)
{
    object.style.cursor="Hand"; 
    object.bgColor="Gray";
}
 
function MouseOut(object)
{
    object.style.cursor="Default"; 
    object.bgColor="Silver";
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
/////////////////////////////////////////////ODREÐIVANJE FILTERA/////////////
function Filter(Column)
{
    if(!LastFilter)
    {
        LastFilter="ASC"+","+Column;
    }
    else
    {
        var Filter=LastFilter.split(",");
        
        if(Filter[0]!="ASC" && Filter[1]==Column)
        {
             return false;
        }
        else
        {
            return true;
        }
    }
    
    return true;
}
//////////////////////////////////////////////////////////////////////////////
 
//***************************************************SRCE*********************
function Sort(Column)
{
    var ASC=Filter(Column);//Postavi filter
  
    var GridTable=document.getElementById(GridID);//Stavi referencu na tablicu
 
    var NbRows=GridTable.rows.length;
    var NbCols=GridTable.rows.item(1).cells.length;
    
    var ArrRows=new Array();
    var ArrCells;
   
    for(var r=1; r<NbRows; r++)//TD potrpaj u ArrCells i taj array stavi u ArrRows (Array u Array-u)
    {//-------------------------------------------------------------------------------------------------
        ArrCells=new Array(NbCols);  
        for(var c=0; c<ArrCells.length; c++) ArrCells[c]=GridTable.rows.item(r).cells.item(c).innerHTML;
        ArrRows[r-1]=ArrCells; 
    }//-------------------------------------------------------------------------------------------------
    
    var Swaped=false;
 do//Klasièni bouble sort
 {//------------------------------------------------------
  Swaped=false;
  for(var i=0;i<ArrRows.length-1;i++)
  {
      var value1=ArrRows[i];
      var value2=ArrRows[i+1];
      
      var a=value1[Column];
      var b=value2[Column];
      
      if(ASC)
      {
       if (a>b)
       {
        Swaped=true;
        Swap(i, i+1);
       }
   }
   else
   {
       if (a<b)
             {
              Swaped=true;
              Swap(i, i+1);
             }
   }
  }
 }while(Swaped);//------------------------------------------
    
    function Swap(value1, value2)//Mjenjanje pozicija (fizièko sortiranje)
    {
        var TempHolder=ArrRows[value2];
        ArrRows[value2]=ArrRows[value1];
        ArrRows[value1]=TempHolder;   
    }
 
    do//Ubij tablicu
    {//------------------------------------------
        GridTable.deleteRow(1);
        
    }while(GridTable.rows.length>1);//-----------
 
    for(var r=0; r<ArrRows.length; r++)//Priljepi sortiranje redove
    {//-------------------------------------------------
        ArrCells=ArrRows[r];
        var row = document.createElement("TR"); 
        
        
        for(var c=0; c<ArrCells.length; c++)
        {
            var cell = document.createElement("TD"); 
            cell.innerHTML=ArrCells[c]; 
            row.appendChild(cell);
        }
        
      GridTable.tBodies.item(0).appendChild(row);
    }//--------------------------------------------------
 
    if(ASC)LastFilter="DSC"+","+Column;//Zapamti zadnji filter
    else LastFilter="ASC"+","+Column;
}//****************************************************************
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX