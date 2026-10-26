// UI only. No sound generation, AudioContext, sample buffers or file rendering.
// Export preferences belong to the future render workflow, not APVTS sound parameters.
import {VOICES,LABELS} from './model.js';
const api=window.percusyn;
const canvas=document.querySelector('#envelope-preview');
const ctx=canvas.getContext('2d');
function draw(){
 const voice=api.selectedVoice;
 const attack=api.store.get(voice+'_amp_attack');
 const decay=api.store.get(voice+'_amp_decay');
 const sustain=api.store.get(voice+'_amp_sustain')/100;
 const duration=attack+decay+100;
 const x=t=>30+t/duration*940;
 const y=v=>112-v*80;
 ctx.clearRect(0,0,1000,150);
 ctx.fillStyle='#080808';ctx.fillRect(0,0,1000,150);
 ctx.strokeStyle='#262626';ctx.lineWidth=1;
 for(let i=0;i<11;i++){ctx.beginPath();ctx.moveTo(30+i*94,30);ctx.lineTo(30+i*94,115);ctx.stroke();}
 for(const height of [32,72,112]){ctx.beginPath();ctx.moveTo(30,height);ctx.lineTo(970,height);ctx.stroke();}
 ctx.beginPath();ctx.moveTo(x(0),y(0));ctx.lineTo(x(attack),y(1));
 for(let i=1;i<=100;i++){const n=i/100;ctx.lineTo(x(attack+n*decay),y(sustain+(1-sustain)*Math.exp(-n*6.9)));}
 ctx.lineTo(x(duration),y(sustain));ctx.strokeStyle='#f7a641';ctx.lineWidth=2.5;ctx.stroke();
 ctx.lineTo(x(duration),y(0));ctx.lineTo(x(0),y(0));ctx.closePath();const fill=ctx.createLinearGradient(0,32,0,112);fill.addColorStop(0,'#f7a64140');fill.addColorStop(1,'#f7a64100');ctx.fillStyle=fill;ctx.fill();
 ctx.font='18px Arial';ctx.fillStyle='#b8b8b8';ctx.fillText('AMP ENVELOPE / UI PREVIEW',30,21);
 ctx.fillStyle='#f7a641';ctx.textAlign='right';ctx.fillText(`A ${attack.toFixed(1)} ms  ·  D ${decay.toFixed(0)} ms  ·  S ${(sustain*100).toFixed(0)} %`,970,139);ctx.textAlign='left';
 document.querySelector('#sample-info').textContent=LABELS[VOICES.indexOf(voice)]+' · VST export planned · audio engine not implemented';
}
for(const key of ['rate','depth']){
 const el=document.querySelector('#export-'+key);
 try{const saved=localStorage.getItem('percusyn-export-'+key);if(saved&&[...el.options].some(o=>o.value===saved))el.value=saved;}catch{}
 el.addEventListener('change',()=>{try{localStorage.setItem('percusyn-export-'+key,el.value);}catch{}});
}
api.store.subscribe(id=>{if(!id||id.startsWith(api.selectedVoice+'_amp_'))draw();});
window.addEventListener('percusyn-voice',draw);
draw();
