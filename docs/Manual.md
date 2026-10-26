# PERCUSYN – Manual

PERCUSYN is a percussion synthesizer with nine voices: **Closed Hat, Open Hat, Snare, Tom, Shaker, Clap, Perc, Rim and Click**. Every voice is synthesized (no samples) and has its own oscillator, envelopes, noise, drive, filter, comb and resonator. A 16-step sequencer, two send effects, a mixer with separate outputs and a one-shot renderer that drags samples straight into your DAW complete the instrument.

![SYNTH page](images/synth.png)

## Contents

1. [Installation](#installation)
2. [Quick start](#quick-start)
3. [The panel](#the-panel)
4. [SYNTH page](#synth-page)
5. [Sequencer](#sequencer)
6. [FX page](#fx-page)
7. [MIX page and outputs](#mix-page-and-outputs)
8. [MIDI](#midi)
9. [One-shot export](#one-shot-export)
10. [Presets](#presets)
11. [Tips and troubleshooting](#tips-and-troubleshooting)

## Installation

Download the package for your system from the [Releases](https://github.com/sscheppert-blip/PERCUSYN/releases) page and unpack it.

| System | Format | Folder |
| --- | --- | --- |
| Windows | VST3 | `C:\Program Files\Common Files\VST3` |
| Windows | CLAP | `C:\Program Files\Common Files\CLAP` |
| macOS | AU | `~/Library/Audio/Plug-Ins/Components` |
| macOS | VST3 | `~/Library/Audio/Plug-Ins/VST3` |
| macOS | CLAP | `~/Library/Audio/Plug-Ins/CLAP` |
| Linux | VST3 | `~/.vst3` |
| Linux | CLAP | `~/.clap` |

The **Standalone** app runs without a DAW (choose the audio and MIDI device under *Options*).

The plug-ins are not signed. On Windows, SmartScreen may warn about the Standalone app. On macOS, remove the download quarantine once after copying, for example:

```bash
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/PERCUSYN.component
```

Then rescan the plug-ins in your DAW. PERCUSYN appears as an instrument by *sscheppert-blip*.

## Quick start

1. Load PERCUSYN on an instrument track. The first factory kit, **Acid Hats**, is loaded.
2. Press **▶** in the instrument bar: the internal sequencer plays (while the DAW is stopped).
3. Click an instrument button, for example **SNARE**: it is selected for editing and plays once.
4. Turn knobs on the SYNTH page. Drag up/down; hold **Shift** for fine moves; double-click resets.
5. Draw steps in the lane at the bottom right: hold the **left** mouse button and drag to draw, hold the **right** button and drag to erase.
6. Play the voices from a MIDI keyboard or clip: **C1** (note 36) is Closed Hat, then upward in the order of the buttons.

## The panel

The window can be resized freely by dragging its lower right corner; the whole instrument scales and keeps its proportions.

- **Header:** preset display with **‹ ›** (previous/next) – click the name for the preset list – **INIT** (all voices to their defaults, all steps cleared), **SAVE** (user preset) and **•••** (export/import JSON, delete user preset, open the preset or render folder, help). **SYNTH / SEQ / FX / MIX** switch the main page.
- **Instrument bar** (on every page): the nine instrument buttons light up when selected and flash when they play. Click one to select and audition it. **M** and **S** below each button mute and solo that voice. Then **▶ / ■** (internal sequencer), **HOST** (sequencer follows the DAW, see [Sequencer](#sequencer)), **PATTERN** (opens the SEQ page), **RANDOM** (random steps and velocities for the selected voice), **LENGTH** (1–16 steps) and **SWING**.
- **Bottom left:** one-shot export. **Bottom right:** the step lane of the selected voice and the internal tempo (**BPM**).

All numeric displays can be clicked to type an exact value. Hover over any control for an explanation.

**LED ring colours:** blue rings show times (ms/s), orange rings show everything else (pitch, timbre, level, mix).

## SYNTH page

All controls on this page belong to the selected voice.

### Oscillator

| Control | Function |
| --- | --- |
| TUNE | Pitch in semitones (±24). |
| DECAY | How fast the tonal oscillator part fades (1 ms – 10 s). The noise layer is not affected; it follows the amp envelope. A short Osc Decay with a longer Amp Decay gives a click followed by a noise tail. |
| TONE | Timbre, depending on the waveform: **SINE/TRI** wavefolding (more harmonics), **SAW** brightness, **NOISE** blend from white noise to a metallic cluster of six detuned square waves (classic hi-hat source), **FM** modulation depth (with an inharmonic ratio per voice; the depth follows the oscillator decay). |
| SINE / TRI / SAW / NOISE / FM | Oscillator waveform. |

### Pitch envelope

**AMOUNT** sets how far above TUNE the hit starts (up to four octaves at 100 %). **ATTACK** is the rise time, **DECAY** the time back to TUNE.

### Amp envelope

**ATTACK** and **DECAY** shape the level of the whole voice. **SUSTAIN** is the level held while a MIDI note is held (or for one step when the sequencer plays); after the note ends, the level falls with the DECAY time. At **0 %** (default) every hit is a pure one-shot and note lengths do not matter.

### Filter, noise, drive, pitch, velocity

| Control | Function |
| --- | --- |
| CUTOFF / RESONANCE / TYPE | State-variable filter, LP (low-pass), HP (high-pass), BP (band-pass), BR (band-reject). |
| NOISE COLOR | Noise spectrum from dark (0 %) via white (50 %) to bright (100 %). |
| NOISE LEVEL | Level of the noise layer (-60 to +12 dB). |
| DRIVE AMOUNT | Saturation before the filter. |
| PITCH BEND | How far the MIDI pitch wheel bends this voice (±24 semitones; negative inverts, 0 ignores the wheel). |
| VELOCITY SENS | 0 % = every hit at full level; 100 % = level follows velocity fully. |

### Comb and resonator

Each voice has two metallic stages after the filter, each with its own **ON/OFF**. Settings are kept when a stage is off; the knobs then show dimmed rings.

- **COMB** – a tuned feedback comb for hollow, pitched or metallic ringing. **TUNE** (20 Hz – 12 kHz) sets the pitch of the ringing, **FEEDBACK** (up to 95 %) its length, **DAMPING** darkens it, **MIX** blends it with the dry sound.
- **RESONATOR** – six resonant modes excited by the voice. **MODEL**: **METAL** (free metal bar, strongly inharmonic), **PLATE** (round plate) or **TUBE** (closed pipe, odd harmonics). **TUNE** sets the lowest mode, **DECAY** how long it rings (time to -60 dB), **DAMPING** shortens the higher modes, **MIX** blends.

Signal path of a voice: oscillator + noise → drive → filter → comb → resonator → level/pan → main output or the voice's own output; the FX send feeds delay and reverb.

## Sequencer

![SEQ page](images/seq.png)

Each voice has 16 steps. The **SEQ** page shows all nine lanes; the lane at the bottom right shows the details of the selected voice.

- **Draw and erase:** hold the **left** mouse button and drag over steps to switch them on, hold the **right** button and drag to switch them off – in one stroke, on the SEQ page even across voices. The step you left-click is also the step used for one-shot renders (marked with a line below it).
- **VEL** (1–127): drag up/down or use the mouse wheel; double-click to type.
- **PROB** (0–100 %): chance that the step plays.
- **COND**: click to cycle **·** (every cycle), **1:2** (first of every two cycles), **2:2** (second of two), **1:4** (first of four). Shift-click goes backwards.
- **CLEAR** switches off all steps of the selected lane (velocity, probability and condition are kept).
- **LENGTH** sets the pattern length (steps beyond it are kept but not played), **SWING** delays every second sixteenth.

### Internal sequencer and the DAW

- **DAW stopped:** **▶** starts the internal sequencer at the **BPM** shown in the lane, **■** stops it and returns to step 1.
- **DAW playing, HOST off (default):** the internal sequencer is off. Play PERCUSYN with MIDI notes from your DAW.
- **DAW playing, HOST lit:** the sequencer follows the DAW transport and tempo, locked to the bar position (also when looping or jumping).

MIDI notes play the voices in every mode, so you can combine a pattern with notes from a clip.

## FX page

![FX page](images/fx.png)

Two global effects on the FX bus. Every voice feeds them with its **SEND FX** (MIX page).

- **DELAY:** stereo delay with **DELAY TIME** (20 ms – 1 s), **FEEDBACK** and **DELAY MIX** (return level).
- **REVERB:** **ROOM SIZE**, **REVERB DECAY** (100 ms – 10 s) and **REVERB MIX** (return level).

The page also summarises routing, MIDI notes and outputs.

## MIX page and outputs

![MIX page](images/mix.png)

Per voice: **PAN**, **LEVEL** fader (-60 to +12 dB), **SEND FX** and **M**/**S** (mute/solo; several voices can be soloed). **MASTER** sets the level of the main output.

**Outputs:** PERCUSYN has a main stereo output plus one stereo output per voice ("Closed Hat", "Open Hat", …). When you enable a voice output in your DAW, that voice leaves the main mix and plays only there (its FX send still goes to the main output's effects). Master does not affect the voice outputs.

## MIDI

| Note | Voice |
| --- | --- |
| C1 (36) | Closed Hat |
| C#1 (37) | Open Hat |
| D1 (38) | Snare |
| D#1 (39) | Tom |
| E1 (40) | Shaker |
| F1 (41) | Clap |
| F#1 (42) | Perc |
| G1 (43) | Rim |
| G#1 (44) | Click |

All MIDI channels are accepted. Velocity is used according to VELOCITY SENS. **Closed Hat chokes Open Hat.** Every voice is monophonic: a new hit fades out the previous hit of the same voice quickly, while its comb and resonator keep ringing naturally. The pitch wheel bends each voice by its PITCH BEND range. Some DAWs call note 36 "C2"; it is the same key.

## One-shot export

The panel at the bottom left turns the selected voice into a WAV sample.

1. Select the voice and left-click the step whose **velocity** you want (the badge shows **VEL** and **STEP**).
2. Choose **RATE** (44.1/48/96 kHz) and **DEPTH** (16 or 24 bit PCM, 32-bit float).
3. **RENDER** calculates the one-shot and shows its waveform. **PREVIEW** plays it on the main output. **WAV** saves a copy wherever you like.
4. **DRAG TO DAW:** drag this field into your arrangement or a sampler – the WAV file is created (if needed) and dropped there.

What is rendered: the selected voice including comb, resonator, level and pan, in stereo, until the tail falls below -90 dBFS (at most 10 s). Global effects and master are not included, and nothing is normalised. The live playback is not affected. Rendered files stay in **Documents/PERCUSYN/Renders**, so dropped files keep working after the plug-in closes.

## Presets

A preset contains **everything**: all nine voices, comb and resonator settings, the patterns, length, swing, tempo, FX and mixer.

- Six factory kits: Acid Hats, Warehouse 909, Dust & Metal, Broken Circuit, Minimal Pulse and Metal Controls Demo.
- **SAVE** stores a user preset (a JSON file in the user application data folder, `PERCUSYN/Presets`; *••• → Show preset folder* opens it).
- **••• → Export / Import preset JSON** exchanges presets as files. Imported presets become user presets only when you SAVE them.
- Your DAW project stores the complete state of each instance anyway.

## Tips and troubleshooting

- **Metallic hats:** NOISE waveform, raise TONE, HP filter, short decays; add the resonator (METAL, tune 3–6 kHz) with a little mix.
- **Toms and percussion:** SINE with pitch envelope; the comb tuned to the tom pitch adds a hollow body.
- **Claps** get their three fast bursts automatically; lengthen AMP DECAY for a bigger room.
- **No sound while the DAW plays:** with HOST off, the internal sequencer stops while the DAW plays – send MIDI notes or switch HOST on.
- **A voice is silent:** check its **M**/**S** buttons and whether its separate output is enabled in the DAW.
- **The plug-in does not appear:** check the folder from the installation table and rescan; on macOS remove the quarantine as shown above.
