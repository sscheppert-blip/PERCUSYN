// Parameter LED rings encode duration versus timbre/level, never label spelling.
// Numeric seven-segment displays keep the hardware reference's orange color.
export function parameterRingColor(parameter){
  return parameter.unit==='ms'||parameter.unit==='s'?'var(--blue)':'var(--orange)';
}
