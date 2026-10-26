// UI-independent parameter contract. IDs and version hints are permanent.
export const VOICES=['closedhat','openhat','snare','tom','shaker','clap','perc','rim','click'];
export const LABELS=['CLOSED HAT','OPEN HAT','SNARE','TOM','SHAKER','CLAP','PERC','RIM','CLICK'];
const param=(key,name,min,max,def,unit='',extra={})=>({key,name,min,max,default:def,unit,step:1,type:'float',versionHint:1,...extra});
export const VOICE_PARAMS=[
 param('osc_tune','Tune',-24,24,0,'st'),param('osc_decay','Decay',1,10000,124,'ms',{scale:'log'}),param('osc_tone','Tone',0,100,28,'%'),
 param('osc_wave','Wave',0,4,3,'',{type:'choice',choices:['SINE','TRI','SAW','NOISE','FM']}),
 param('pitch_amount','Amount',0,100,62,'%'),param('pitch_attack','Attack',.1,500,.5,'ms',{step:.1,scale:'log'}),param('pitch_decay','Decay',1,5000,86,'ms',{scale:'log'}),
 param('amp_attack','Attack',.1,500,.2,'ms',{step:.1,scale:'log'}),param('amp_decay','Decay',1,5000,210,'ms',{scale:'log'}),param('amp_sustain','Sustain',0,100,0,'%'),
 param('filter_cutoff','Cutoff',20,20000,8500,'Hz',{scale:'log'}),param('filter_resonance','Resonance',0,100,12,'%'),param('filter_type','Type',0,3,0,'',{type:'choice',choices:['LP','HP','BP','BR']}),
 param('noise_color','Color',0,100,38,'%'),param('noise_level','Level',-60,12,0,'dB',{step:.1}),param('drive_amount','Amount',0,100,24,'%'),param('pitch_bend','Bend',-24,24,0,'st'),param('velocity_sensitivity','Sens',0,100,72,'%'),
 param('comb_enabled','Comb enabled',0,1,0,'',{type:'bool'}),param('comb_tune','Tune',20,12000,1200,'Hz',{scale:'log'}),param('comb_feedback','Feedback',0,95,45,'%'),param('comb_damping','Damping',0,100,35,'%'),param('comb_mix','Mix',0,100,25,'%'),
 param('resonator_enabled','Resonator enabled',0,1,0,'',{type:'bool'}),param('resonator_model','Model',0,2,0,'',{type:'choice',choices:['METAL','PLATE','TUBE']}),param('resonator_tune','Tune',20,12000,800,'Hz',{scale:'log'}),param('resonator_decay','Decay',5,8000,350,'ms',{scale:'log'}),param('resonator_damping','Damping',0,100,40,'%'),param('resonator_mix','Mix',0,100,25,'%'),
 param('level','Level',-60,12,-6,'dB',{step:.1}),param('send_fx','Send FX',-60,0,-12,'dB',{step:.1}),param('pan','Pan',-100,100,0,'%'),param('mute','Mute',0,1,0,'',{type:'bool'}),param('solo','Solo',0,1,0,'',{type:'bool'})];
export const GLOBAL_PARAMS=[param('tempo','Tempo',40,240,124,'BPM'),param('swing','Swing',0,75,12,'%'),param('length','Length',1,16,16,'',{type:'int'}),param('fx_delay_time','Delay time',20,1000,250,'ms',{scale:'log'}),param('fx_delay_feedback','Feedback',0,95,28,'%'),param('fx_delay_mix','Delay mix',0,100,15,'%'),param('fx_reverb_size','Room size',0,100,45,'%'),param('fx_reverb_decay','Reverb decay',100,10000,1800,'ms',{scale:'log'}),param('fx_reverb_mix','Reverb mix',0,100,20,'%'),param('master_level','Master',-60,6,-6,'dB',{step:.1})];
export const STEP_FIELDS=[param('active','Active',0,1,0,'',{type:'bool'}),param('velocity','Velocity',1,127,100,'',{type:'int'}),param('probability','Probability',0,100,100,'%'),param('condition','Condition',0,3,0,'',{type:'choice',choices:['ALL','1:2','2:2','1:4']}),param('octave','Octave',-2,2,0,'oct',{type:'int'})];
export const schema=[...GLOBAL_PARAMS.map(p=>({...p,id:`global_${p.key}`})),...VOICES.flatMap(v=>[...VOICE_PARAMS.map(p=>({...p,id:`${v}_${p.key}`})),...Array.from({length:16},(_,i)=>STEP_FIELDS.map(p=>({...p,id:`${v}_step_${String(i+1).padStart(2,'0')}_${p.key}`}))).flat()])];
export const byId=new Map(schema.map(p=>[p.id,p]));
export const stepId=(voice,i,key)=>`${voice}_step_${String(i+1).padStart(2,'0')}_${key}`;
export function constrain(p,value){const n=Number(value);if(!Number.isFinite(n))throw Error(`Invalid value: ${p.id}`);return Math.max(p.min,Math.min(p.max,Math.round((n-p.min)/p.step)*p.step+p.min));}
export function normalized(p,value){return p.scale==='log'?Math.log(value/p.min)/Math.log(p.max/p.min):(value-p.min)/(p.max-p.min);}
export function denormalized(p,n){n=Math.max(0,Math.min(1,n));return constrain(p,p.scale==='log'?p.min*Math.pow(p.max/p.min,n):p.min+n*(p.max-p.min));}
export function factory(index=0){
 const values=Object.fromEntries(schema.map(p=>[p.id,p.default]));
 const soundDefaults={
  closedhat:{osc_decay:60,amp_decay:75,osc_tone:35,pitch_amount:0,filter_type:1,filter_cutoff:6500,filter_resonance:5,noise_level:-9},
  openhat:{osc_decay:650,amp_decay:680,osc_tone:46,pitch_amount:0,filter_type:1,filter_cutoff:5000,filter_resonance:7,noise_level:-7},
  snare:{osc_wave:0,osc_decay:90,amp_decay:190,osc_tone:30,pitch_amount:22,filter_cutoff:8500,noise_level:-5},
  tom:{osc_wave:0,osc_tune:-7,osc_decay:320,amp_decay:370,noise_level:-38,filter_cutoff:2600,pitch_amount:35},
  shaker:{osc_decay:50,amp_decay:160,osc_tone:15,pitch_amount:0,filter_type:1,filter_cutoff:3500,noise_color:70,noise_level:-3},
  clap:{osc_wave:3,osc_decay:90,amp_decay:240,osc_tone:12,pitch_amount:0,filter_type:1,filter_cutoff:900,noise_level:-5},
  perc:{osc_wave:4,osc_decay:170,amp_decay:210,osc_tone:48,pitch_amount:32,noise_level:-23,filter_cutoff:6000},
  rim:{osc_wave:1,osc_decay:55,amp_decay:85,osc_tone:42,pitch_amount:10,noise_level:-35,filter_cutoff:8000},
  click:{osc_wave:2,osc_decay:12,amp_decay:23,osc_tone:23,pitch_amount:0,noise_level:-18,filter_type:1,filter_cutoff:1900}
 };
 for(const v of VOICES){
  for(const [key,value]of Object.entries(soundDefaults[v]))values[`${v}_${key}`]=value;
  values[`${v}_filter_cutoff`]=Math.min(20000,Math.round(values[`${v}_filter_cutoff`]*[1,.65,1.25,.8,.9][index]));
  const pattern=v==='closedhat'?[0,2,4,6,8,10,12,14]:v==='openhat'?[7,15]:v==='snare'||v==='clap'?[4,12]:v==='shaker'?[1,3,5,7,9,11,13,15]:[6,14];
  for(let i=0;i<16;i++){values[stepId(v,i,'active')]=Number(pattern.includes((i+index)%16));values[stepId(v,i,'velocity')]=i%4===0?115:82;}
 }
 return values;
}
// Schema v1 used one HiHat mode and a Kick. Keep old user presets readable.
export function migrateV1(parameters){
 if(!parameters||typeof parameters!=='object'||Array.isArray(parameters))throw Error('Missing parameter object');
 const next={};
 for(const [id,value]of Object.entries(parameters)){
  if(id.startsWith('kick_')||id.endsWith('_hat_open'))continue;
  if(id.startsWith('hihat_')){const key=id.slice(6);next['closedhat_'+key]=key.endsWith('_active')&&parameters.hihat_hat_open===1?0:value;next['openhat_'+key]=key.endsWith('_active')&&parameters.hihat_hat_open!==1?0:value;}
  else next[id]=value;
 }
 if(parameters.hihat_hat_open===1){next.closedhat_amp_decay=75;next.closedhat_osc_decay=60;}
 else {next.openhat_amp_decay=680;next.openhat_osc_decay=650;}
 return next;
}
export class ParameterStore{
 constructor(){this.values=factory();this.listeners=new Set();}
 get(id){if(!byId.has(id))throw Error(`Unknown ID: ${id}`);return this.values[id];}
 set(id,value){const p=byId.get(id);if(!p)throw Error(`Unknown ID: ${id}`);const next=constrain(p,value);if(this.values[id]===next)return;this.values[id]=next;this.emit(id);}
 subscribe(fn){this.listeners.add(fn);return()=>this.listeners.delete(fn);}
 emit(id=null){for(const fn of this.listeners)fn(id);}
 replace(values){if(!values||typeof values!=='object'||Array.isArray(values))throw Error('Missing parameter object');const next=Object.fromEntries(schema.map(p=>[p.id,p.default]));for(const [id,value]of Object.entries(values)){const p=byId.get(id);if(!p)throw Error(`Unknown parameter ID: ${id}`);if(typeof value!=='number'||!Number.isFinite(value))throw Error(`Invalid value: ${id}`);next[id]=constrain(p,value);}this.values=next;this.emit();}
 serialize(name){return {format:'percusyn-preset',schemaVersion:2,name,parameters:{...this.values}};}
 import(data){if(data?.format!=='percusyn-preset'||![1,2].includes(data.schemaVersion)||typeof data.name!=='string')throw Error('Unsupported PERCUSYN preset (schema v1/v2 required)');this.replace(data.schemaVersion===1?migrateV1(data.parameters):data.parameters);return data.name.slice(0,64);}
}
