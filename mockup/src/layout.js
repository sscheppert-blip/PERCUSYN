// Fixed design coordinates match a resizable JUCE editor's logical bounds.
// Only the rack is fitted; native dialogs retain readable viewport-sized text.
const viewport=document.querySelector('.viewport');
const frame=document.querySelector('.rack-frame');
const rack=document.querySelector('.rack');
let pending=0;
function fit(){
  pending=0;
  const style=getComputedStyle(viewport);
  const availableWidth=viewport.clientWidth-parseFloat(style.paddingLeft)-parseFloat(style.paddingRight);
  const availableHeight=viewport.clientHeight-parseFloat(style.paddingTop)-parseFloat(style.paddingBottom);
  const width=rack.offsetWidth,height=rack.offsetHeight;
  if(!width||!height||availableWidth<=0||availableHeight<=0)return;
  const scale=Math.min(availableWidth/width,availableHeight/height);
  rack.style.setProperty('--rack-scale',scale);
  frame.style.width=`${width*scale}px`;
  frame.style.height=`${height*scale}px`;
  frame.classList.add('ready');
}
function schedule(){if(!pending)pending=requestAnimationFrame(fit);}
const observer=new ResizeObserver(schedule);
observer.observe(viewport);
observer.observe(rack);
window.addEventListener('resize',schedule);
window.visualViewport?.addEventListener('resize',schedule);
document.fonts?.ready.then(schedule);
fit();
