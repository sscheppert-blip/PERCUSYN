// Visual-only clock, deliberately separate from UI and any future audio engine.
export class Transport {
 constructor(store,onTick,onStop){this.store=store;this.onTick=onTick;this.onStop=onStop;this.running=false;this.step=0;this.cycle=0;this.timer=null;}
 start(){if(this.running)return;this.running=true;this.tick();}
 tick(){if(!this.running)return;const length=this.store.get('global_length');if(this.step>=length){this.step=0;this.cycle++;}this.onTick(this.step,this.cycle);const interval=60000/this.store.get('global_tempo')/4;const swing=this.store.get('global_swing')/100;const wait=interval*(this.step%2===0?1+swing:1-swing);this.step++;this.timer=setTimeout(()=>this.tick(),wait);}
 pause(){this.running=false;clearTimeout(this.timer);this.onStop(false);}
 stop(){this.pause();this.step=0;this.cycle=0;this.onStop(true);}
}
export function shouldTrigger(store,voice,step,cycle,random=Math.random){const id=`${voice}_step_${String(step+1).padStart(2,'0')}_`;if(!store.get(id+'active'))return false;const c=store.get(id+'condition');return(c===0||c===1&&cycle%2===0||c===2&&cycle%2===1||c===3&&cycle%4===0)&&random()*100<store.get(id+'probability');}
