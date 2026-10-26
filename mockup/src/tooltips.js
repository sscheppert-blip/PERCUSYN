// English UI help stays separate from parameter IDs and sound state.
const parameterDescriptions={
 comb_enabled:'Enables or bypasses the comb stage for this instrument. Settings are retained when bypassed.',
 comb_tune:'Sets the comb resonance spacing in Hertz. Lower tuning gives a larger, hollow body; higher tuning gives tighter metallic color.',
 comb_feedback:'Feeds part of the comb output back into its input. Higher settings produce stronger, longer ringing.',
 comb_damping:'Controls high-frequency loss in the comb feedback loop. Higher settings give darker ringing.',
 comb_mix:'Blends the original sound with the comb-filtered sound. Zero keeps the original signal.',
 resonator_enabled:'Enables or bypasses the resonator for this instrument. Settings are retained when bypassed.',
 resonator_model:'Selects a resonance character: METAL for an inharmonic metallic body, PLATE for sheet-like ringing, or TUBE for a hollow pipe-like body.',
 resonator_tune:'Sets the resonator’s reference pitch in Hertz. The selected model determines how its resonances are spaced.',
 resonator_decay:'Sets how long the resonating body rings after a hit.',
 resonator_damping:'Controls how quickly bright resonances fade. Higher settings give a softer, darker tail.',
 resonator_mix:'Blends the original sound with the resonating body. Zero keeps the original signal.',
 osc_tune:'Transposes this instrument in semitones. 12 semitones equals one octave.',
 osc_decay:'Sets how quickly the oscillator component fades after a hit.',
 osc_tone:'Shapes the oscillator timbre, from softer to brighter.',
 osc_wave:'Selects the oscillator source: sine, triangle, saw, noise or FM.',
 pitch_amount:'Sets the depth of the pitch envelope. Higher values produce a stronger pitch sweep.',
 pitch_attack:'Sets how quickly the pitch envelope reaches its maximum.',
 pitch_decay:'Sets how quickly the pitch envelope returns to the base tuning.',
 amp_attack:'Sets how quickly a hit reaches full level. Short settings give a sharper transient.',
 amp_decay:'Sets how quickly the hit falls from its peak to the sustain level.',
 amp_sustain:'Sets the level held after the decay stage while the voice is gated.',
 filter_cutoff:'Sets the filter frequency. Its effect depends on the selected filter type.',
 filter_resonance:'Emphasizes the filter frequency. Higher settings give a more ringing character.',
 filter_type:'Selects low-pass, high-pass, band-pass or band-reject filtering.',
 noise_color:'Shapes the noise spectrum, from darker to brighter.',
 noise_level:'Sets the level of the noise component in decibels.',
 drive_amount:'Sets the amount of saturation for extra harmonics and edge.',
 pitch_bend:'Applies an additional pitch offset in semitones.',
 velocity_sensitivity:'Sets how strongly the instrument responds to step velocity.',
 level:'Sets this instrument’s output level in decibels.',
 send_fx:'Sets how much of this instrument is sent to the effects bus.',
 pan:'Places this instrument between the left and right channels.',
 mute:'Mutes this instrument’s sequencer triggers.',
 solo:'Plays only soloed instruments. Multiple instruments can be soloed together.',
 tempo:'Sets the sequencer tempo in beats per minute.',
 swing:'Offsets alternating sixteenth notes to create a swung groove. Zero gives straight timing.',
 length:'Sets the number of steps before the pattern repeats. Steps beyond this length are retained.',
 fx_delay_time:'Sets the time between delay repeats in milliseconds.',
 fx_delay_feedback:'Sets how much delay output feeds back into the delay, controlling repeat length.',
 fx_delay_mix:'Balances the dry signal and delayed signal.',
 fx_reverb_size:'Sets the apparent size of the reverberant space.',
 fx_reverb_decay:'Sets how long the reverb tail lasts.',
 fx_reverb_mix:'Balances the dry signal and reverberated signal.',
 master_level:'Sets the overall output level after the mixer.'
};
const stepDescriptions={
 active:'Enables or disables this step. Its velocity, probability and condition values are retained.',
 velocity:'Sets the strength of this hit from 1 to 127. The Velocity Sensitivity control sets the response.',
 probability:'Sets the chance of this step triggering on each eligible pass. 100% always triggers; 0% never triggers.',
 condition:'Sets when the step may trigger: ALL = every cycle, 1:2 = first of two, 2:2 = second of two, 1:4 = first of four.'
};
export function parameterHelp(id){
 const step=id.match(/_step_\d+_(.+)$/);
 const key=id.startsWith('global_')?id.slice(7):id.slice(id.indexOf('_')+1);
 return step?stepDescriptions[step[1]]||'':parameterDescriptions[key]||'';
}
export function setTooltip(el,text){if(!el||!text)return;el.dataset.tooltip=text;el.removeAttribute('title');el.setAttribute('aria-description',text);}
const staticDescriptions={
 prev:'Loads the previous preset. Save any edits you want to keep first.',
 next:'Loads the next preset. Save any edits you want to keep first.',
 preset:'Loads a factory or user preset, replacing the current sound and pattern settings.',
 init:'Resets every instrument to default settings and clears all steps.',
 save:'Saves the current sounds and patterns as a new user preset in this browser.',
 more:'Opens preset import, export, delete and help options.',
 export:'Downloads the current preset as a portable JSON file. This does not export audio.',
 import:'Loads sounds and patterns from a PERCUSYN preset JSON file.',
 delete:'Deletes the selected user preset from this browser. Factory presets are read-only.',
 help:'Opens the control and workflow reference.',
 play:'Starts or pauses the visual sequencer. Audio playback is not implemented in this mockup.',
 stop:'Stops the sequencer and returns the play position to step 1.',
 pattern:'Opens the full pattern view. Lights only while the SEQ view is selected.',
 random:'Randomizes step on/off states and velocities for the selected instrument only.',
 clear:'Disables all steps in the selected lane. Their detail values are retained.',
 length:parameterDescriptions.length,swing:parameterDescriptions.swing,tempo:parameterDescriptions.tempo,
 'export-format':'Planned sample file format. WAV output will be available in the future VST.',
 'export-rate':'Selects the planned sample export rate. This setting does not change sequencer speed.',
 'export-depth':'Selects the planned WAV bit depth: 16-bit PCM, 24-bit PCM or 32-bit floating point.',
 'resonator-model':parameterDescriptions.resonator_model,
 'envelope-preview':'Visualizes the selected instrument’s amplitude envelope. This is not a rendered audio waveform.'
};
const waves=['Sine: a smooth fundamental with no extra harmonics.','Triangle: a softer waveform with odd harmonics.','Saw: a bright waveform with a full harmonic spectrum.','Noise: an unpitched, broadband source for noisy percussion.','FM: frequency modulation for complex, bright or metallic timbres.'];
const filters=['Low-pass: passes low frequencies and reduces frequencies above cutoff.','High-pass: passes high frequencies and reduces frequencies below cutoff.','Band-pass: isolates a frequency band around cutoff.','Band-reject (notch): removes a frequency band around cutoff.'];
export function applyTooltips(){
 for(const [id,text]of Object.entries(staticDescriptions))setTooltip(document.getElementById(id),text);
 document.querySelectorAll('[data-wave]').forEach(el=>setTooltip(el,waves[Number(el.dataset.wave)]));
 document.querySelectorAll('[name="filter-type"]').forEach(el=>{setTooltip(el,filters[Number(el.value)]);setTooltip(el.closest('label'),filters[Number(el.value)]);});
 document.querySelectorAll('[data-bool]').forEach(el=>setTooltip(el,parameterHelp(el.dataset.bool)));
 document.querySelectorAll('[data-voice],[data-select]').forEach(el=>setTooltip(el,'Selects '+el.textContent.trim()+' and its independent sound controls and pattern.'));
 document.querySelectorAll('[data-overview]').forEach(el=>setTooltip(el,'Click to enable or disable this step. Select the instrument to edit its velocity, probability and condition.'));
 document.querySelectorAll('[data-tab]').forEach(el=>setTooltip(el,{synth:'Edits the selected instrument’s synthesis parameters.',seq:'Shows and edits all nine instrument patterns.',fx:'Edits global delay and reverb settings. Audio effects are not implemented yet.',mix:'Adjusts instrument levels, pan, FX sends, mute, solo and master level.'}[el.dataset.tab]));
 document.querySelectorAll('.row-label[title]').forEach(el=>setTooltip(el,stepDescriptions[el.getAttribute('title')]));
 document.querySelectorAll('.sample-actions button:disabled').forEach(el=>setTooltip(el,{RENDER:'Planned: renders a one-shot from the current instrument. Requires the future VST audio engine.', '▶ PREVIEW':'Planned: auditions the rendered one-shot. No audio engine is included yet.','↓ WAV':'Planned: saves the rendered one-shot as a WAV file.'}[el.textContent.trim()]));
 setTooltip(document.querySelector('.sample-drag'),'Planned: drag a rendered WAV directly from the VST into your DAW. Native file drag is not implemented in this browser mockup.');
}
export function installTooltipLayer(){
 const tooltip=document.createElement('div');tooltip.id='control-tooltip';tooltip.role='tooltip';tooltip.hidden=true;document.body.append(tooltip);
 let active=null,timer=0,pressed=false;
 const hide=()=>{clearTimeout(timer);active?.removeAttribute('aria-describedby');active=null;tooltip.hidden=true;};
 const show=el=>{if(!el?.isConnected)return;hide();active=el;tooltip.textContent=el.dataset.tooltip;tooltip.hidden=false;el.setAttribute('aria-describedby',tooltip.id);const r=el.getBoundingClientRect();const box=tooltip.getBoundingClientRect();const width=document.documentElement.clientWidth,height=document.documentElement.clientHeight;tooltip.style.left=Math.max(10,Math.min(r.left+r.width/2-box.width/2,width-box.width-10))+'px';tooltip.style.top=Math.max(10,r.bottom+box.height+18>height?r.top-box.height-8:r.bottom+8)+'px';};
 document.addEventListener('pointerover',event=>{const el=event.target.closest?.('[data-tooltip]');if(el===active)return;hide();if(el&&!pressed)timer=setTimeout(()=>show(el),450);});
 document.addEventListener('pointerout',event=>{if(!event.target.closest?.('[data-tooltip]')?.contains(event.relatedTarget))hide();});
 document.addEventListener('focusin',event=>{if(!pressed){const el=event.target.closest?.('[data-tooltip]');if(el)show(el);}});
 document.addEventListener('focusout',hide);
 document.addEventListener('pointerdown',()=>{pressed=true;hide();},true);
 document.addEventListener('pointerup',()=>pressed=false,true);
 document.addEventListener('pointercancel',()=>pressed=false,true);
 document.addEventListener('keydown',event=>{if(event.key==='Escape')hide();});
 window.addEventListener('resize',hide);
}
