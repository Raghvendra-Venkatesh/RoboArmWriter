#pragma once
// Dashboard page, served from flash.
const char PAGE[] PROGMEM = R"rawpage(<!doctype html>
<html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>RoboArm Writer</title>
<style>
:root{--bg:#efeae0;--grid:#e3dccd;--card:#fbf9f4;--ink:#1e2a33;--mut:#68727b;--line:#d6cfbf;--faint:#b9b2a3;
--acc:#d4471c;--acc2:#1f5f8b;--ok:#2f7d5b;--warn:#b7791f;--bad:#c0392b;--shadow:0 1px 0 rgba(0,0,0,.04),0 6px 18px -10px rgba(40,30,10,.25)}
@media (prefers-color-scheme:dark){:root{--bg:#14181b;--grid:#1b2024;--card:#1d2226;--ink:#e8e3d7;--mut:#99a1a8;--line:#2e353a;--faint:#4b545a;
--acc:#f0673a;--acc2:#5aa6d6;--ok:#4cb488;--warn:#e0a644;--bad:#ef6b5c;--shadow:0 6px 18px -10px rgba(0,0,0,.6)}}
*{box-sizing:border-box}
body{margin:0;font:15px/1.45 "Segoe UI",system-ui,-apple-system,Roboto,sans-serif;color:var(--ink);background:var(--bg);
background-image:linear-gradient(var(--grid) 1px,transparent 1px),linear-gradient(90deg,var(--grid) 1px,transparent 1px);background-size:24px 24px}
.mono,input[type=number],.us,.ang,.pos b{font-family:Consolas,"SF Mono",Menlo,monospace}
header{position:sticky;top:0;z-index:5;display:flex;gap:12px;align-items:center;flex-wrap:wrap;padding:10px 16px;background:var(--card);border-bottom:2px solid var(--ink)}
.brand{display:flex;gap:10px;align-items:center;margin-right:auto}
.brand b{font-size:17px;letter-spacing:.3px}.brand small{display:block;color:var(--mut);font-size:12px}
.pill{display:inline-block;padding:3px 10px;border-radius:99px;font-size:13px;font-weight:600;background:var(--line)}
.pill.run{background:var(--acc2);color:#fff}.pill.warn{background:var(--warn);color:#fff}
.bar{width:120px;height:6px;background:var(--line);border-radius:3px;overflow:hidden;display:inline-block;vertical-align:middle;margin-left:8px}
.bar i{display:block;height:100%;width:0;background:var(--acc2);transition:width .3s}
nav{display:flex;gap:4px;padding:10px 16px 0;max-width:1200px;margin:0 auto;flex-wrap:wrap}
nav button{border:0;border-bottom:3px solid transparent;background:none;padding:8px 12px;font-weight:600;color:var(--mut);border-radius:0}
nav button.on{color:var(--ink);border-bottom-color:var(--acc)}
main{max-width:1200px;margin:0 auto;padding:12px 16px 60px}
section{display:none}section.on{display:block}
.card{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:16px;box-shadow:var(--shadow);margin-bottom:14px}
h2{margin:0 0 12px;font-size:17px}h2 small{font-weight:400;color:var(--mut);font-size:13px}
h3{margin:0;font-size:16px}
.g2{display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1.3fr);gap:14px}
.cals{display:grid;grid-template-columns:repeat(auto-fill,minmax(340px,1fr));gap:14px}
@media(max-width:820px){.g2{grid-template-columns:1fr}.cals{grid-template-columns:1fr}}
label{display:block;font-size:13px;color:var(--mut);margin-bottom:8px}
input,select{font:inherit;color:var(--ink);background:var(--bg);border:1px solid var(--line);border-radius:6px;padding:7px 9px;width:100%}
input[type=checkbox]{width:auto}input[type=range]{padding:0;accent-color:var(--acc)}
#txt{font-size:22px;letter-spacing:2px;text-transform:uppercase;font-weight:600}
.row{display:grid;grid-template-columns:repeat(auto-fit,minmax(110px,1fr));gap:10px;align-items:end}
.chk{display:flex;gap:8px;align-items:center;color:var(--ink);padding-bottom:10px}
button{font:inherit;font-weight:600;cursor:pointer;border:1px solid var(--ink);background:var(--card);color:var(--ink);border-radius:7px;padding:8px 12px}
button:hover{background:var(--line)}button:active{transform:translateY(1px)}
button.pri{background:var(--ink);color:var(--card)}button.pri:hover{opacity:.88}
button.ghost{border-color:var(--line)}
button.stop{background:var(--bad);border-color:var(--bad);color:#fff;padding:8px 18px;letter-spacing:1px}
.btns{display:flex;gap:8px;flex-wrap:wrap;margin:10px 0}
.hint{color:var(--mut);font-size:13px;margin:6px 0}
.msg{font-size:14px;min-height:20px}.msg.bad{color:var(--bad)}.msg.ok{color:var(--ok)}
.banner{border-left:4px solid var(--warn);background:var(--card);padding:10px 14px;border-radius:6px;margin-bottom:14px;display:none}
#pv{width:100%;height:auto;max-height:60vh;display:block;background:var(--bg);border-radius:8px;border:1px solid var(--line)}
.legend{display:flex;gap:14px;flex-wrap:wrap;font-size:12px;color:var(--mut);margin-top:8px}
.legend i{display:inline-block;width:18px;height:3px;vertical-align:middle;margin-right:5px}
/* calibration */
.calhead{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;gap:8px}
.badges b{display:inline-block;width:24px;height:24px;line-height:22px;text-align:center;border-radius:50%;border:1px solid var(--faint);color:var(--mut);font-size:12px;margin-left:4px}
.badges b.ok{background:var(--ok);border-color:var(--ok);color:#fff}
.refs{display:grid;grid-template-columns:1fr 1fr;gap:10px}
figure{margin:0;border:1px dashed var(--line);border-radius:8px;padding:8px;font-size:12.5px;color:var(--mut)}
figure b{color:var(--ink)}figure input{width:64px;padding:2px 5px;display:inline-block}
.pic svg{width:100%;height:120px;display:block}
.live{display:flex;gap:14px;align-items:baseline;margin:12px 0 6px}
.us{font-size:22px;font-weight:700}.ang{color:var(--acc2);font-weight:600}.state{margin-left:auto;font-size:12px;color:var(--mut)}
.nudge{display:grid;grid-template-columns:repeat(6,1fr);gap:5px}.nudge button{padding:7px 0}
.cal input[type=range]{margin:10px 0 2px}
details{margin-top:8px;border-top:1px solid var(--line);padding-top:8px}summary{cursor:pointer;font-weight:600;font-size:14px}
.steps{margin:0;padding-left:20px}.steps li{margin:3px 0}
/* jog */
.pos{display:flex;gap:18px;font-size:15px;margin-bottom:10px;flex-wrap:wrap}.pos b{font-size:20px}
.pad{display:grid;grid-template-columns:repeat(3,64px);grid-template-rows:repeat(3,52px);gap:6px}
.pad button{padding:0}.zcol{display:grid;grid-template-rows:repeat(3,52px);gap:6px;width:72px}
.jogwrap{display:flex;gap:18px;align-items:center;flex-wrap:wrap}
#toast{position:fixed;left:50%;bottom:18px;transform:translateX(-50%);background:var(--ink);color:var(--card);padding:10px 16px;border-radius:8px;
font-size:14px;max-width:92vw;opacity:0;pointer-events:none;transition:opacity .25s}
#toast.show{opacity:1}#toast.bad{background:var(--bad);color:#fff}
.tp{width:100%;border-collapse:collapse;margin:8px 0;font-size:14px}.tp td{padding:5px 4px;border-top:1px solid var(--line)}.tp td.z{font-family:Consolas,monospace}.tp button{padding:4px 10px}
.diag{width:100%;max-width:520px;height:auto;display:block;margin:0 auto 10px}
.diag text{font:12px Consolas,monospace;fill:var(--ink)}
</style></head><body>
<header>
 <div class="brand">
  <svg width="34" height="34" viewBox="0 0 34 34"><rect x="5" y="27" width="14" height="4" rx="1" fill="currentColor"/><path d="M12 27 L12 17 L22 9 L28 16" stroke="currentColor" stroke-width="3" fill="none" stroke-linecap="round" stroke-linejoin="round"/><path d="M28 16 L28 25" stroke="#d4471c" stroke-width="3" stroke-linecap="round"/></svg>
  <div><b>RoboArm Writer</b><small id="net">connecting...</small></div>
 </div>
 <div><span id="pill" class="pill">...</span><span class="bar"><i id="barFill"></i></span></div>
 <button id="bRelax" class="ghost" title="Cut power to all servos (arm goes limp)">Relax</button>
 <button id="bStop" class="stop">STOP</button>
</header>
<nav>
 <button data-t="draw" class="on">1. Draw</button>
 <button data-t="cal">2. Servo calibration</button>
 <button data-t="arm">3. Arm size</button>
 <button data-t="jog">4. Writing point</button>
</nav>
<main>
<div id="calWarn" class="banner"><b>Not calibrated yet.</b> Do tabs 2, 3 and 4 in order. Drawing is locked until every servo has Ref A and Ref B saved.</div>

<section id="t-draw" class="on">
 <div class="g2">
  <div class="card">
   <h2>Write text</h2>
   <label>Text (max 40)<input id="txt" maxlength="40" value="HELLO WORLD" autocomplete="off"></label>
   <div class="row">
    <label>Letter height (mm)<input id="tH" type="number" step="1" min="3"></label>
    <label>Letter gap (mm)<input id="tG" type="number" step="0.5"></label>
    <label class="chk"><input id="tR" type="checkbox"> Rotate 180&deg;</label>
   </div>
   <p class="hint">A to Z, 0 to 9, space and - . ! ? are supported. Long text wraps by words into more lines, centered on the writing area.</p>
   <div class="btns"><button id="bPrev">Preview</button><button id="bDraw" class="pri">Draw it</button></div>
   <div id="prevMsg" class="msg"></div>
   <details><summary>Tests</summary>
    <p class="hint">Test square: 40 mm square at the center. Corner check: pen touches each corner of the writing area for 1.5 s, so you can see if the paper height is right everywhere.</p>
    <div class="btns"><button id="bSquare" class="ghost">Test square</button><button id="bCorners" class="ghost">Corner check</button><button id="bPark" class="ghost">Park</button></div>
   </details>
  </div>
  <div class="card">
   <h2>Paper preview <small>top view, base at the bottom, you stand at the bottom</small></h2>
   <svg id="pv" viewBox="0 0 100 100"></svg>
   <div class="legend"><span><i style="background:var(--ink)"></i>pen down</span><span><i style="background:var(--faint)"></i>pen up</span><span><i style="background:var(--acc2);opacity:.3;height:8px"></i>arm can reach</span><span><i style="background:var(--bad)"></i>cannot reach</span></div>
  </div>
 </div>
</section>

<section id="t-cal">
 <div class="card">
  <h2>How to calibrate a servo</h2>
  <ol class="steps">
   <li>Keep one hand on the arm and one hand near the servo power switch. Do <b>one card at a time</b>.</li>
   <li>Press <b>Hold</b>. The servo wakes up at the pulse shown. It may jump once.</li>
   <li>Use the <b>-50 ... +50</b> buttons until the real arm matches <b>picture A</b>. Check with a set square. Press <b>Save as Ref A</b>.</li>
   <li>Do the same for <b>picture B</b>. Press <b>Save as Ref B</b>. Both circles turn green.</li>
   <li>Open <b>Limits</b>. Nudge slowly toward each end until the arm is about 5&deg; before it touches the frame or a cable pulls. Press <b>Set min</b> / <b>Set max</b>.</li>
  </ol>
  <p class="hint">Ref angles can be changed if your arm cannot reach a picture pose (for example use 45 instead of 0). The picture follows the number.</p>
 </div>
 <div id="calCards" class="cals"></div>
</section>

<section id="t-arm">
 <div class="g2">
  <div class="card">
   <h2>Arm size <small>measure pivot center to pivot center, in mm</small></h2>
   <svg class="diag" viewBox="0 0 420 250">
    <line x1="10" y1="225" x2="410" y2="225" stroke="var(--faint)" stroke-width="2"/>
    <rect x="60" y="185" width="50" height="40" rx="4" fill="var(--line)"/>
    <line x1="85" y1="160" x2="85" y2="185" stroke="var(--line)" stroke-width="10"/>
    <line x1="85" y1="160" x2="170" y2="70" stroke="var(--ink)" stroke-width="7" stroke-linecap="round"/>
    <line x1="170" y1="70" x2="285" y2="100" stroke="var(--ink)" stroke-width="7" stroke-linecap="round"/>
    <line x1="285" y1="100" x2="285" y2="222" stroke="var(--acc)" stroke-width="6" stroke-linecap="round"/>
    <circle cx="85" cy="160" r="6" fill="var(--card)" stroke="var(--ink)" stroke-width="2"/>
    <circle cx="170" cy="70" r="6" fill="var(--card)" stroke="var(--ink)" stroke-width="2"/>
    <circle cx="285" cy="100" r="6" fill="var(--card)" stroke="var(--ink)" stroke-width="2"/>
    <line x1="35" y1="225" x2="35" y2="160" stroke="var(--acc2)" stroke-dasharray="3 3"/><text x="14" y="196">H</text>
    <text x="108" y="105">L1</text><text x="225" y="74">L2</text><text x="295" y="165">L3</text>
    <line x1="85" y1="240" x2="85" y2="245" stroke="var(--acc2)"/><text x="64" y="246" font-size="10">base axis</text>
    <text x="300" y="218" font-size="10">pen tip</text>
   </svg>
   <div class="row">
    <label>H: table to shoulder pivot<input id="g_H" type="number" step="0.5"></label>
    <label>L1: shoulder to elbow<input id="g_L1" type="number" step="0.5"></label>
    <label>L2: elbow to wrist pitch<input id="g_L2" type="number" step="0.5"></label>
    <label>L3: wrist pitch to pen tip<input id="g_L3" type="number" step="0.5"></label>
    <label>d0: shoulder ahead of base axis<input id="g_d0" type="number" step="0.5"></label>
    <label>Max pen tilt (&deg; from straight down)<input id="g_maxTilt" type="number" step="1" min="0" max="70"></label>
    <label>Pen side offset (mm, + = right)<input id="g_sideOff" type="number" step="0.5"></label>
    <label>Wrist rotate while drawing (&deg;)<input id="g_rollDraw" type="number" step="1"></label>
    <label class="chk"><input id="g_clawOn" type="checkbox"> Gripper (claw) fitted</label>
   </div>
   <p class="hint"><b>L3</b> is wrist pitch pivot to marker tip, with the marker taped on. <b>Side offset</b>: how far the marker tip sits to the right of the arm center line (left = minus). <b>Max tilt</b>: the pen may lean this much either way. Leaning back toward the base lets the shoulder sit much lower. Leaning away adds reach at the far edge. The firmware picks the lowest arm pose for each point. 30 is good for a felt marker. 0 = always straight down.</p>
  </div>
  <div class="card">
   <h2>Drawing settings</h2>
   <div class="row">
    <label>Writing area center X (mm)<input id="d_cx" type="number" step="1"></label>
    <label>Writing area center Y (mm)<input id="d_cy" type="number" step="1"></label>
    <label>Area width (mm)<input id="d_areaW" type="number" step="1"></label>
    <label>Area depth (mm)<input id="d_areaH" type="number" step="1"></label>
    <label>Paper height Z, no touch points (mm)<input id="d_zPaper" type="number" step="0.1"></label>
    <label>Pen up lift (mm)<input id="d_lift" type="number" step="1"></label>
    <label>Pen press (mm below paper)<input id="d_press" type="number" step="0.1"></label>
    <label>Draw speed (mm/s)<input id="d_vDraw" type="number" step="1"></label>
    <label>Travel speed (mm/s)<input id="d_vTravel" type="number" step="1"></label>
    <label>Up/down speed (mm/s)<input id="d_vZ" type="number" step="1"></label>
    <label>Joint move speed (&deg;/s)<input id="d_slew" type="number" step="1"></label>
   </div>
   <p class="hint">X is to your right, Y is forward away from the base. Paper height comes from the 5 touch points in tab 4. The Z field is only used when no touch points are saved.</p>
   <div class="btns"><button id="bSaveArm" class="pri">Save arm and drawing settings</button></div>
   <details><summary>Factory reset</summary>
    <p class="hint">Clears all calibration and sizes. Servos go limp.</p>
    <button id="bReset" class="ghost">Reset everything</button>
   </details>
  </div>
 </div>
</section>

<section id="t-jog">
 <div class="g2">
  <div class="card">
   <h2>Move the pen <small>arrow keys and PgUp/PgDn also work on this tab</small></h2>
   <div class="pos">Pen at <span>X <b id="px">-</b></span><span>Y <b id="py">-</b></span><span>Z <b id="pz">-</b></span> mm</div>
   <label style="max-width:180px">Step<select id="jstep"><option>0.5</option><option selected>1</option><option>2</option><option>5</option><option>10</option><option>20</option></select></label>
   <div class="jogwrap">
    <div class="pad">
     <span></span><button data-j="0,1,0">Y+<br><small>away</small></button><span></span>
     <button data-j="-1,0,0">X-</button><button id="bCenter" title="Go to writing area center, pen up">&#8982;</button><button data-j="1,0,0">X+</button>
     <span></span><button data-j="0,-1,0">Y-<br><small>closer</small></button><span></span>
    </div>
    <div class="zcol"><button data-j="0,0,1">Z+ up</button><span></span><button data-j="0,0,-1">Z- down</button></div>
   </div>
   <div class="btns"><button id="bPark2" class="ghost">Park</button><button id="bSavePark" class="ghost">Save this pose as park</button></div>
  </div>
  <div class="card">
   <h2>Writing point <small>one point, that is all</small></h2>
   <ol class="steps">
    <li>Press <b>Go to center</b>. The pen goes above the middle of the paper.</li>
    <li>Want the text somewhere else? Move it with the <b>X / Y</b> buttons on the left.</li>
    <li>Press <b>Arm down</b> until the tip <b>just touches</b> the paper. Use a small step at the end.</li>
    <li>Press <b>Save writing point</b>. Text is written centered here.</li>
    <li>Press <b>Go to bottom point</b> (25 mm closer to the base). Press <b>Arm down</b> until it just touches. Press <b>Save bottom point</b>. Now the bottom of the letters touches too.</li>
   </ol>
   <div class="btns"><button id="bGoC">Go to center</button></div>
   <div class="btns" style="align-items:end">
    <label style="max-width:120px;margin:0">Step (mm)<select id="tstep"><option>5</option><option>2</option><option selected>1</option><option>0.5</option><option>0.2</option></select></label>
    <button id="bDown" class="pri">&#9660; Arm down</button><button id="bUp">&#9650; Arm up</button>
   </div>
   <div class="btns"><button id="bSaveLvl" class="pri">1. Save writing point</button></div>
   <div class="btns"><button id="bGoB">Go to bottom point</button><button id="bSaveBot" class="pri">2. Save bottom point</button></div>
   <p class="hint" id="planeInfo"></p>
   <div id="clawBox">
   <h2 style="margin-top:18px">Claw</h2>
   <div class="btns"><button id="bOpen">Open</button><button id="bClose">Close on marker</button></div>
   <input id="claw" type="range" min="0" max="100" step="1" value="0">
   </div>
  </div>
 </div>
</section>
</main>
<div id="toast"></div>

<script>
const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
let CFG=null, ST=null, lastErr=0, built=false;
const D='°';
const JD=[
 {u:D,a:'Arm points straight FORWARD, at the middle of the paper.',b:'Arm points to your RIGHT. Stand behind the base, facing the paper.'},
 {u:D,a:'Upper arm straight UP. Check with a set square on the table.',b:'Upper arm level, pointing FORWARD.'},
 {u:D,a:'Forearm at a RIGHT ANGLE to the upper arm.',b:'Forearm in one STRAIGHT LINE with the upper arm.'},
 {u:D,a:'Claw in its normal position. Jaws open left and right.',b:'Claw turned a quarter turn CLOCKWISE, looking from behind.'},
 {u:D,a:'Claw in one STRAIGHT LINE with the forearm.',b:'Claw bent DOWN at a right angle to the forearm.'},
 {u:'%',a:'CLOSED on the marker. Firm grip, servo must not buzz.',b:'Fully OPEN.'}];

function toast(m,bad){const t=$('#toast');t.textContent=m;t.className='show'+(bad?' bad':'');clearTimeout(t._h);t._h=setTimeout(()=>t.className='',bad?7000:2200)}
async function api(cmd,x={}){
 try{const r=await fetch('/api/cmd',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({cmd,...x})});
  const j=await r.json(); if(!j.ok) toast(j.err||'Error',true); else if(j.warn){toast(j.warn,true);j.warned=1} return j;}catch(e){toast('No connection',true);return {ok:false}}}
const num=id=>{const v=parseFloat($(id).value);return isNaN(v)?undefined:v};

// ---------- tabs ----------
$$('nav button').forEach(b=>b.onclick=()=>{$$('nav button').forEach(x=>x.classList.toggle('on',x===b));
 $$('section').forEach(s=>s.classList.toggle('on',s.id==='t-'+b.dataset.t));});
const tabOn=t=>$('#t-'+t).classList.contains('on');

// ---------- pictures ----------
function arcPath(c,r,a0,a1){const p0=[c[0]+r*Math.cos(a0),c[1]-r*Math.sin(a0)],p1=[c[0]+r*Math.cos(a1),c[1]-r*Math.sin(a1)];
 return `M${p0[0]},${p0[1]} A${r},${r} 0 ${Math.abs(a1-a0)>Math.PI?1:0} ${a1>a0?0:1} ${p1[0]},${p1[1]}`}
function sideSVG(s,e,w,hi,lab){
 const R=Math.PI/180,L=[46,42,30],a=[s*R,(s-e)*R,(s-e-w)*R],P=[[0,0]];
 for(let i=0;i<3;i++){const p=P[i];P.push([p[0]+L[i]*Math.cos(a[i]),p[1]-L[i]*Math.sin(a[i])])}
 const ref=hi===0?0:a[hi-1],J=P[hi],rl=36,RL=[J[0]+rl*Math.cos(ref),J[1]-rl*Math.sin(ref)];
 let xs=P.map(p=>p[0]).concat([RL[0],-16,16]),ys=P.map(p=>p[1]).concat([RL[1],34]);
 const x0=Math.min(...xs)-14,x1=Math.max(...xs)+14,y0=Math.min(...ys)-12,y1=Math.max(...ys)+4;
 let g=`<svg viewBox="${x0} ${y0} ${x1-x0} ${y1-y0}"><line x1="${x0}" y1="34" x2="${x1}" y2="34" stroke="var(--faint)" stroke-width="1.5"/>`+
  `<rect x="-10" y="4" width="20" height="30" rx="2" fill="var(--line)"/>`;
 for(let i=0;i<3;i++)g+=`<line x1="${P[i][0]}" y1="${P[i][1]}" x2="${P[i+1][0]}" y2="${P[i+1][1]}" stroke="${i===hi?'var(--acc)':(i===2?'var(--faint)':'var(--mut)')}" stroke-width="${i===hi?5:3.5}" stroke-linecap="round"/>`;
 for(let i=0;i<3;i++)g+=`<circle cx="${P[i][0]}" cy="${P[i][1]}" r="3" fill="var(--card)" stroke="var(--ink)"/>`;
 g+=`<line x1="${J[0]}" y1="${J[1]}" x2="${RL[0]}" y2="${RL[1]}" stroke="var(--acc2)" stroke-dasharray="3 3"/>`;
 if(Math.abs(a[hi]-ref)>0.01)g+=`<path d="${arcPath(J,15,ref,a[hi])}" fill="none" stroke="var(--acc2)" stroke-width="1.5"/>`;
 g+=`<text x="${x0+3}" y="${y0+10}" font-size="10" fill="var(--acc2)" font-family="Consolas">${lab}</text></svg>`;return g}
function topSVG(b,lab){const R=b*Math.PI/180,e=[48*Math.sin(R),-48*Math.cos(R)];
 return `<svg viewBox="-62 -66 124 96"><rect x="-40" y="-64" width="80" height="20" fill="none" stroke="var(--faint)"/><text x="-12" y="-51" font-size="8" fill="var(--mut)">paper</text>`+
 `<line x1="0" y1="0" x2="0" y2="-50" stroke="var(--acc2)" stroke-dasharray="3 3"/>`+
 (Math.abs(b)>0.5?`<path d="${arcPath([0,0],16,Math.PI/2,Math.PI/2-R)}" fill="none" stroke="var(--acc2)" stroke-width="1.5"/>`:'')+
 `<line x1="0" y1="0" x2="${e[0]}" y2="${e[1]}" stroke="var(--acc)" stroke-width="5" stroke-linecap="round"/><circle r="9" fill="var(--line)" stroke="var(--ink)"/>`+
 `<text x="-26" y="25" font-size="8" fill="var(--mut)">YOU STAND HERE</text><text x="-58" y="-34" font-size="10" fill="var(--acc2)" font-family="Consolas">${lab}</text></svg>`}
function rollSVG(r,lab){return `<svg viewBox="-50 -42 100 84"><line x1="-40" y1="0" x2="40" y2="0" stroke="var(--acc2)" stroke-dasharray="3 3"/>`+
 `<circle r="24" fill="none" stroke="var(--faint)" stroke-width="2"/><g transform="rotate(${r})"><rect x="-30" y="-5" width="18" height="10" rx="2" fill="var(--acc)"/><rect x="12" y="-5" width="18" height="10" rx="2" fill="var(--acc)"/><circle r="4" fill="var(--ink)"/></g>`+
 `<text x="-48" y="-30" font-size="10" fill="var(--acc2)" font-family="Consolas">${lab}</text><text x="-34" y="38" font-size="8" fill="var(--mut)">seen from behind</text></svg>`}
function clawSVG(p,lab){const g=5+Math.max(0,Math.min(100,p))*0.22;
 return `<svg viewBox="-50 -42 100 84"><rect x="-4" y="-34" width="8" height="70" rx="3" fill="var(--faint)"/><text x="7" y="32" font-size="8" fill="var(--mut)">marker</text>`+
 `<rect x="${-g-8}" y="-16" width="8" height="34" rx="2" fill="var(--acc)"/><rect x="${g}" y="-16" width="8" height="34" rx="2" fill="var(--acc)"/>`+
 `<text x="-48" y="-30" font-size="10" fill="var(--acc2)" font-family="Consolas">${lab}</text></svg>`}
function pic(j,v,lab){v=+v;switch(j){case 0:return topSVG(v,lab);case 1:return sideSVG(v,v,90,0,lab);case 2:return sideSVG(90,v,0,1,lab);
 case 3:return rollSVG(v,lab);case 4:return sideSVG(90,90,v,2,lab);default:return clawSVG(v,lab)}}

// ---------- calibration cards ----------
function buildCal(){
 const box=$('#calCards');box.innerHTML='';
 CFG.servos.forEach((s,j)=>{const d=JD[j],c=document.createElement('div');c.className='card cal';c.id='cal'+j;
  c.innerHTML=`<div class="calhead"><h3>CH<span class="mono chv">${s.ch}</span> &middot; ${s.name}</h3><span class="badges"><b class="bA">A</b><b class="bB">B</b></span></div>
  <div class="refs"><figure><div class="pic pA"></div><b>Ref A</b> at <input class="aA" type="number" step="1"> ${d.u}<br>${d.a}</figure>
  <figure><div class="pic pB"></div><b>Ref B</b> at <input class="aB" type="number" step="1"> ${d.u}<br>${d.b}</figure></div>
  <div class="live"><span class="us">-</span><span class="ang">-</span><span class="state">-</span></div>
  <div class="nudge">${[-50,-10,-1,1,10,50].map(v=>`<button class="ghost" data-n="${v}">${v>0?'+':''}${v}</button>`).join('')}</div>
  <input class="sl" type="range" min="500" max="2500" step="1">
  <div class="btns"><button class="hold ghost">Hold</button><button class="sA pri">Save as Ref A</button><button class="sB pri">Save as Ref B</button></div>
  <details><summary>Limits, channel and test</summary>
   <div class="row" style="margin-top:8px"><label>Min ${d.u}<input class="mn" type="number" step="1"></label><label>Max ${d.u}<input class="mx" type="number" step="1"></label><label>PCA channel<input class="ch" type="number" min="0" max="15"></label></div>
   <div class="btns"><button class="lmn ghost">Set min = here</button><button class="lmx ghost">Set max = here</button></div>
   <div class="row"><label>Test angle ${d.u}<input class="ta" type="number" step="1" value="0"></label><button class="go">Go to angle</button></div>
   <div class="btns"><button class="sv ghost">Save ref angles, limits, channel</button></div>
  </details>`;
  box.appendChild(c);
  const q=s=>c.querySelector(s),redraw=()=>{q('.pA').innerHTML=pic(j,q('.aA').value,'A '+q('.aA').value+d.u);q('.pB').innerHTML=pic(j,q('.aB').value,'B '+q('.aB').value+d.u)};
  q('.aA').oninput=redraw;q('.aB').oninput=redraw;
  c.querySelectorAll('.nudge button').forEach(b=>b.onclick=()=>api('nudge',{j,d:+b.dataset.n}));
  q('.sl').onchange=e=>api('raw',{j,us:+e.target.value});
  q('.hold').onclick=()=>api('raw',{j});
  q('.sA').onclick=async()=>{{const r=await api('ref',{j,which:'A'});if(r.ok){if(!r.warned)toast(s.name+' Ref A saved');loadCfg()}}};
  q('.sB').onclick=async()=>{{const r=await api('ref',{j,which:'B'});if(r.ok){if(!r.warned)toast(s.name+' Ref B saved');loadCfg()}}};
  q('.lmn').onclick=async()=>{if((await api('limit',{j,which:'min'})).ok){toast('Min saved');loadCfg()}};
  q('.lmx').onclick=async()=>{if((await api('limit',{j,which:'max'})).ok){toast('Max saved');loadCfg()}};
  q('.go').onclick=()=>api('joint',{j,a:+q('.ta').value});
  q('.sv').onclick=async()=>{const o={j,aA:+q('.aA').value,aB:+q('.aB').value,minA:+q('.mn').value,maxA:+q('.mx').value,ch:+q('.ch').value};
   if(o.aA===o.aB){toast('Ref A and Ref B angles must be different',true);return}
   if((await api('set',{servo:o})).ok){toast('Saved');loadCfg()}};
 });built=true}
function fillCal(){CFG.servos.forEach((s,j)=>{const c=$('#cal'+j),q=x=>c.querySelector(x);
 q('.bA').classList.toggle('ok',s.doneA);q('.bB').classList.toggle('ok',s.doneB);q('.chv').textContent=s.ch;
 q('.aA').value=s.aA;q('.aB').value=s.aB;q('.mn').value=s.minA;q('.mx').value=s.maxA;q('.ch').value=s.ch;q('.aA').oninput()})}

// ---------- config ----------
const GEO=['H','L1','L2','L3','d0','maxTilt','sideOff','rollDraw'],DRW=['cx','cy','areaW','areaH','zPaper','lift','press','vDraw','vTravel','vZ','slew'];
async function loadCfg(){try{CFG=await (await fetch('/api/config')).json()}catch(e){setTimeout(loadCfg,2000);return}
 if(!built)buildCal();fillCal();fillTouch();
 $('#g_clawOn').checked=CFG.geo.clawOn;$('#cal5').style.display=CFG.geo.clawOn?'':'none';$('#clawBox').style.display=CFG.geo.clawOn?'':'none';
 GEO.forEach(k=>$('#g_'+k).value=CFG.geo[k]);DRW.forEach(k=>$('#d_'+k).value=CFG.draw[k]);
 if(!$('#tH').dataset.set){$('#tH').value=CFG.draw.textH;$('#tG').value=CFG.draw.gap;$('#tR').checked=CFG.draw.rot180;$('#tH').dataset.set=1}
}
$('#bSaveArm').onclick=async()=>{const geo={clawOn:$('#g_clawOn').checked},draw={};GEO.forEach(k=>geo[k]=num('#g_'+k));DRW.forEach(k=>draw[k]=num('#d_'+k));
 if((await api('set',{geo,draw})).ok){toast('Settings saved');loadCfg();preview()}};
$('#bReset').onclick=async()=>{if(confirm('Erase all calibration and settings?')&&(await api('defaults')).ok){toast('Reset done');loadCfg()}};

// ---------- state polling ----------
async function poll(){
 try{ST=await (await fetch('/api/state')).json();
  $('#net').textContent=(ST.ap?'Hotspot ':'Home WiFi ')+ST.ip;
  const p=$('#pill');
  if(ST.busy){p.textContent=ST.job?ST.job+' '+Math.round(ST.prog*100)+'%':'Moving';p.className='pill run'}
  else if(!ST.cal){p.textContent='Not calibrated';p.className='pill warn'}else{p.textContent='Idle';p.className='pill'}
  $('#barFill').style.width=(ST.job?ST.prog*100:0)+'%';
  $('#calWarn').style.display=ST.cal?'none':'block';
  if(ST.errId!==lastErr){if(lastErr&&ST.err)toast(ST.err,true);lastErr=ST.errId}
  $('#px').textContent=ST.x.toFixed(1);$('#py').textContent=ST.y.toFixed(1);$('#pz').textContent=ST.z.toFixed(1);
  if(built)for(let j=0;j<6;j++){const c=$('#cal'+j),sl=c.querySelector('.sl');
   c.querySelector('.us').textContent=ST.us[j]+' µs';c.querySelector('.ang').textContent='= '+ST.an[j]+JD[j].u;
   c.querySelector('.state').textContent=ST.live[j]?'holding':'relaxed';if(document.activeElement!==sl)sl.value=ST.us[j]}
 }catch(e){$('#net').textContent='connection lost...'}
 setTimeout(poll,ST&&ST.busy?300:600)}

// ---------- preview ----------
async function preview(){
 const t=$('#txt').value.toUpperCase(),u=`/api/preview?text=${encodeURIComponent(t)}&h=${num('#tH')||25}&g=${num('#tG')??6}&r=${$('#tR').checked?1:0}`;
 let d;try{d=await (await fetch(u)).json()}catch(e){return}
 if(!CFG)return;const w=CFG.draw,Y=v=>-v;
 let xs=[w.cx-w.areaW/2,w.cx+w.areaW/2,-30,30],ys=[w.cy-w.areaH/2,w.cy+w.areaH/2,-25];
 d.strokes.forEach(s=>s.forEach(p=>{xs.push(p[0]);ys.push(p[1])}));
 const x0=Math.min(...xs)-15,x1=Math.max(...xs)+15,y0=Math.min(...ys)-10,y1=Math.max(...ys)+15,sw=(x1-x0)/300;
 let g='';
 if(d.rMax>0){g+=`<path d="M${-d.rMax},0 A${d.rMax},${d.rMax} 0 1 1 ${d.rMax},0 A${d.rMax},${d.rMax} 0 1 1 ${-d.rMax},0 Z M${-d.rMin},0 A${d.rMin},${d.rMin} 0 1 0 ${d.rMin},0 A${d.rMin},${d.rMin} 0 1 0 ${-d.rMin},0 Z" fill="var(--acc2)" fill-opacity=".12" fill-rule="evenodd"/>`}
 g+=`<rect x="${w.cx-w.areaW/2}" y="${Y(w.cy+w.areaH/2)}" width="${w.areaW}" height="${w.areaH}" fill="var(--card)" stroke="var(--faint)" stroke-width="${sw}" stroke-dasharray="${sw*4} ${sw*3}"/>`;
 g+=`<circle r="14" fill="var(--line)" stroke="var(--ink)" stroke-width="${sw}"/><text x="0" y="${14+sw*14}" font-size="${sw*12}" text-anchor="middle" fill="var(--mut)">base</text>`;
 let prev=null;
 d.strokes.forEach(s=>{if(prev)g+=`<line x1="${prev[0]}" y1="${Y(prev[1])}" x2="${s[0][0]}" y2="${Y(s[0][1])}" stroke="var(--faint)" stroke-width="${sw}" stroke-dasharray="${sw*3} ${sw*3}"/>`;
  g+=`<polyline points="${s.map(p=>p[0]+','+Y(p[1])).join(' ')}" fill="none" stroke="var(--ink)" stroke-width="${sw*2.4}" stroke-linecap="round" stroke-linejoin="round"/>`;prev=s[s.length-1]});
 d.bad.forEach(p=>g+=`<circle cx="${p[0]}" cy="${Y(p[1])}" r="${sw*5}" fill="var(--bad)"/>`);
 const pv=$('#pv');pv.setAttribute('viewBox',`${x0} ${Y(y1)} ${x1-x0} ${y1-y0}`);pv.innerHTML=g;
 const m=$('#prevMsg');
 if(!d.strokes.length){m.textContent='Nothing to draw.';m.className='msg bad'}
 else if(d.nBad){m.textContent=d.nBad+' points are out of reach. Make letters smaller, move the area, or check arm sizes.';m.className='msg bad'}
 else{m.textContent=d.lines+(d.lines>1?' lines. ':' line. ')+'All points reachable. Reach at paper height: '+d.rMin+' to '+d.rMax+' mm from the base.';m.className='msg ok'}}
let pvT;['#txt','#tH','#tG','#tR'].forEach(s=>$(s).addEventListener('input',()=>{clearTimeout(pvT);pvT=setTimeout(preview,350)}));
$('#bPrev').onclick=preview;
$('#bDraw').onclick=async()=>{const r=await api('draw',{text:$('#txt').value.toUpperCase(),h:num('#tH'),g:num('#tG'),r:$('#tR').checked});if(r.ok)toast('Drawing started')};
$('#bSquare').onclick=()=>api('square');
function fillTouch(){const d=CFG.draw,t=CFG.touch||[],b=t[1]||[];
 $('#planeInfo').textContent='Writing point: X '+d.cx+'  Y '+d.cy+'  Z '+d.zPaper+' mm.  '+(b[3]?'Bottom point: Y '+b[1]+'  Z '+b[2]+' mm (height change '+(b[2]-d.zPaper).toFixed(1)+' mm).':'Bottom point: not set.')}
$('#bGoB').onclick=()=>api('goBottom');
$('#bSaveBot').onclick=async()=>{if((await api('saveBottom')).ok){toast('Bottom point saved');loadCfg()}};
$('#bGoC').onclick=()=>api('center');
$('#bDown').onclick=()=>api('jog',{dz:-(+$('#tstep').value)});
$('#bUp').onclick=()=>api('jog',{dz:+$('#tstep').value});
$('#bSaveLvl').onclick=async()=>{if((await api('saveLevel')).ok){toast('Writing point saved');await loadCfg();preview()}};
$('#bCorners').onclick=()=>api('corners');

// ---------- header + jog ----------
$('#bStop').onclick=()=>api('stop').then(()=>toast('Stopped'));
$('#bRelax').onclick=()=>api('relax').then(()=>toast('Servos relaxed. Hold the arm!'));
$('#bPark').onclick=$('#bPark2').onclick=()=>api('park');
$('#bSavePark').onclick=async()=>{if((await api('savePark')).ok){toast('Park pose saved');loadCfg()}};
$('#bCenter').onclick=()=>api('center');
$('#bOpen').onclick=()=>api('claw',{pct:100});$('#bClose').onclick=()=>api('claw',{pct:0});
$('#claw').onchange=e=>api('claw',{pct:+e.target.value});
function jog(v){const s=+$('#jstep').value;api('jog',{dx:v[0]*s,dy:v[1]*s,dz:v[2]*s})}
$$('[data-j]').forEach(b=>b.onclick=()=>jog(b.dataset.j.split(',').map(Number)));
document.addEventListener('keydown',e=>{if(!tabOn('jog')||e.target.tagName==='INPUT')return;
 const k={ArrowUp:[0,1,0],ArrowDown:[0,-1,0],ArrowLeft:[-1,0,0],ArrowRight:[1,0,0],PageUp:[0,0,1],PageDown:[0,0,-1]}[e.key];
 if(k){e.preventDefault();jog(k)}});

loadCfg().then(preview);poll();
</script></body></html>)rawpage";
