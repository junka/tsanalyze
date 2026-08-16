/*
 * HTML output template for the tsanalyze table viewer.
 *
 * The HTML report is emitted as a single self-contained file:
 *
 *   <!DOCTYPE html> ... <style> ... </style>
 *   <script type="application/json" id="tsdata">           <- html_head
 *   <the JSON array built by result.c's container stack>
 *   </script>
 *   <script> (renderer) ... </script></body></html>        <- html_foot
 *
 * Extracted from result.c so the markup and the renderer can be maintained
 * (and reviewed) independently from the C emission logic.
 */
#ifndef TSANALYZE_HTML_TEMPLATE_H
#define TSANALYZE_HTML_TEMPLATE_H

/*
 * Document head + the opening <script> that will carry the JSON array.
 * The trailing part (closing </script> + renderer) is html_foot.
 */
static const char html_head[] =
	"<!DOCTYPE html>\n"
	"<html lang=\"en\">\n"
	"<head>\n"
	"<meta charset=\"utf-8\">\n"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	"<title>TS Table Viewer</title>\n"
	"<style>\n"
	"body{font-family:ui-monospace,Menlo,monospace;margin:0;background:#0f1117;color:#e6e6e6;}\n"
	"header{position:sticky;top:0;background:#171a23;border-bottom:1px solid #2a2e3a;padding:10px 16px;z-index:5;}\n"
	"header h1{margin:0;font-size:16px;font-weight:600;}\n"
	"header .sub{color:#8a93a6;font-size:12px;margin-top:2px;}\n"
	"#app{padding:12px;max-width:1200px;margin:0 auto;}\n"
	".grp{border:1px solid #262b38;border-radius:8px;margin:10px 0;background:#14171f;overflow:hidden;}\n"
	".gh{padding:8px 12px;background:#1a1e29;display:flex;align-items:center;gap:10px;flex-wrap:wrap;}\n"
	".gname{font-weight:700;color:#7fd0ff;}\n"
	".gmeta{color:#8a93a6;font-size:12px;}\n"
	".pg{display:flex;align-items:center;gap:8px;padding:6px 12px;border-top:1px solid #262b38;background:#161a24;}\n"
	".pg button{background:#242a3a;border:1px solid #3a415a;color:#e6e6e6;border-radius:5px;padding:3px 10px;cursor:pointer;}\n"
	".pg button:hover{background:#303850;}\n"
	".pg .info{margin:0 6px;font-size:13px;color:#8a93a6;}\n"
	".pg .vchips{display:flex;gap:4px;flex-wrap:wrap;}\n"
	".pg .vchip{font-size:11px;padding:2px 6px;border-radius:10px;background:#242a3a;color:#9fb0c8;}\n"
	".tree,.tree ul{list-style:none;margin:0;padding-left:18px;}\n"
	".tree>ul{padding-left:6px;}\n"
	".node{margin:0;padding:1px 0;}\n"
	".node.c>.kids{display:none;}\n"
	".tw{display:inline-block;width:16px;height:16px;line-height:15px;text-align:center;cursor:pointer;\n"
	"     border:1px solid #3a415a;border-radius:3px;color:#9fb0c8;font-size:12px;user-select:none;margin-right:6px;}\n"
	".tw.leaf{visibility:hidden;}\n"
	".k{color:#d7b66f;}\n"
	".sep{color:#6b7280;}\n"
	".v{color:#8effa1;word-break:break-all;}\n"
	".meta{color:#5c6474;font-size:11px;}\n"
	"pre{margin:2px 0 2px 22px;background:#0b0d12;border:1px solid #2a2e3a;border-radius:4px;\n"
	"    padding:6px;overflow:auto;color:#c9d1e8;font-size:12px;}\n"
	"</style>\n"
	"</head>\n"
	"<body>\n"
	"<header><h1>TS Table Viewer</h1><div class=\"sub\" id=\"sub\"></div></header>\n"
	"<div id=\"app\"></div>\n"
	"<script type=\"application/json\" id=\"tsdata\">\n";

/*
 * Closing </script> + the renderer.  Reads the JSON from #tsdata, groups the
 * top-level array by table name, renders one expandable tree per table, and
 * pages across multiple versions of the same table.
 */
static const char html_foot[] =
	"\n</script>\n"
	"<script>\n"
	"(function(){\n"
	"var raw=document.getElementById('tsdata').textContent;\n"
	"var data=JSON.parse(raw);\n"
	"var groups={},order=[];\n"
	"data.forEach(function(el){var k=Object.keys(el)[0];if(!groups[k]){groups[k]=[];order.push(k);}groups[k].push(el[k]);});\n"
	"function esc(s){return (''+s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');}\n"
	"function typeOf(v){return Array.isArray(v)?'Array':(v&&typeof v==='object'?'Object':'Scalar');}\n"
	"function sizeOf(v){return Array.isArray(v)?v.length:(v&&typeof v==='object'?Object.keys(v).length:0);}\n"
	"function node(key,val){\n"
	"  if(val&&typeof val==='object'){\n"
	"    var n=sizeOf(val)>0;\n"
	"    var html='<li class=\"node\"><span class=\"tw'+(n?'':' leaf')+'\">'+(n?'-':' ')+'</span>';\n"
	"    if(key)html+='<span class=\"k\">'+esc(key)+'</span>';\n"
	"    html+=' <span class=\"meta\">'+typeOf(val)+' '+sizeOf(val)+'</span>';\n"
	"    html+='<ul class=\"kids\">';\n"
	"    if(Array.isArray(val))val.forEach(function(v){html+=node('',v);});\n"
	"    else Object.keys(val).forEach(function(k2){html+=node(k2,val[k2]);});\n"
	"    html+='</ul></li>';return html;\n"
	"  }else{\n"
	"    var s=val===null?'null':(''+val);\n"
	"    var html='<li class=\"node\"><span class=\"tw leaf\">&nbsp;</span>';\n"
	"    if(key)html+='<span class=\"k\">'+esc(key)+'</span><span class=\"sep\">:&nbsp;</span>';\n"
	"    if(s.indexOf('\\n')>=0)html+='<pre>'+esc(s)+'</pre>';\n"
	"    else if(typeOf(val)==='Scalar')html+='<span class=\"v\">&quot;'+esc(s)+'&quot;</span>';\n"
	"    else html+='<span class=\"v\">'+esc(s)+'</span>';\n"
	"    html+='</li>';return html;\n"
	"  }\n"
	"}\n"
	"function setPage(tb,info,cv,b0,b1,list,cur){\n"
	"  tb.innerHTML=node(list[cur].name,list[cur].value);\n"
	"  info.textContent=(cur+1)+' / '+list.length;\n"
	"  cv.textContent=extver(list[cur].value);\n"
	"  b0.disabled=cur===0;\n"
	"  b1.disabled=cur===list.length-1;\n"
	"}\n"
	"function extver(o){if(!o||typeof o!=='object')return'';var k=Object.keys(o).filter(function(x){return /version/i.test(x);})[0];\n"
	"  return k&&o[k]!==undefined?(' v'+(o[k]==null?'?':(''+o[k]).trim())):'';}\n"
	"var app=document.getElementById('app');\n"
	"order.forEach(function(name){\n"
	"  var vs=groups[name];\n"
	"  var list=vs.map(function(v){return{name:name,value:v};});\n"
	"  var sv=document.createElement('section');sv.className='grp';\n"
	"  var gh=document.createElement('div');gh.className='gh';\n"
	"  gh.innerHTML='<span class=\"gname\">'+esc(name)+'</span><span class=\"gmeta\">'+vs.length+' version(s)</span>';\n"
	"  var pg=document.createElement('div');pg.className='pg';\n"
	"  var b0=document.createElement('button');b0.textContent='\u25C0';b0.className='btnprev';\n"
	"  var b1=document.createElement('button');b1.textContent='\u25B6';b1.className='btnnext';\n"
	"  var info=document.createElement('span');info.className='info';\n"
	"  var cv=document.createElement('span');cv.className='vchip curver';\n"
	"  pg.appendChild(b0);pg.appendChild(info);pg.appendChild(cv);pg.appendChild(b1);sv.appendChild(gh);sv.appendChild(pg);\n"
	"  var body=document.createElement('div');body.className='tree';\n"
	"  var tb=document.createElement('ul');tb.className='treebody';\n"
	"  body.appendChild(tb);\n"
	"  sv.appendChild(body);\n"
	"  var cur=0;\n"
	"  b0.onclick=function(){if(cur>0){cur--;setPage(tb,info,cv,b0,b1,list,cur);}};\n"
	"  b1.onclick=function(){if(cur<list.length-1){cur++;setPage(tb,info,cv,b0,b1,list,cur);}};\n"
	"  setPage(tb,info,cv,b0,b1,list,0);\n"
	"  app.appendChild(sv);\n"
	"});\n"
	"document.getElementById('sub').textContent=data.length+' top-level table(s) parsed';\n"
	"document.addEventListener('click',function(e){\n"
	"  var t=e.target.closest('.tw');if(!t||t.classList.contains('leaf'))return;\n"
	"  var li=t.closest('li.node');if(!li)return;\n"
	"  if(li.classList.contains('c')){li.classList.remove('c');t.textContent='-';}\n"
	"  else{li.classList.add('c');t.textContent='+';}\n"
	"});\n"
	"})();\n"
	"</script>\n"
	"</body>\n"
	"</html>\n";

#endif /* TSANALYZE_HTML_TEMPLATE_H */