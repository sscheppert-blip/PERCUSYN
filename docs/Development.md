# PERCUSYN – Development

## Releases

Builds run only on demand (pushes don't trigger them, to save GitHub Actions minutes). To publish a
new version, bump `VERSION` in `CMakeLists.txt`, commit and push, then:

```bash
gh workflow run build -f version=0.1.0
```

(or Actions → build → Run workflow). The workflow checks the version, scans for credentials and
local-machine data, then builds and tests on all three platforms:

| Platform | Formats | Notes |
| --- | --- | --- |
| macOS | AU, VST3, CLAP, Standalone | Apple Silicon (arm64) only, ad-hoc signed, `auval` run |
| Windows | VST3, CLAP, Standalone | x64 |
| Linux | VST3, CLAP, Standalone | x86_64, tests under `xvfb-run` |

and publishes release `v<version>` with one archive per platform and format. Nothing is signed with a
developer identity or notarized.

## Building

Needs CMake ≥ 3.22, a C++20 compiler and Ninja (optional). JUCE 9.0.3 and clap-juce-extensions
(pinned commit) are fetched by CMake into the build tree.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/PercusynTests_artefacts/Release/PercusynTests
```

`-DPERCUSYN_COPY_PLUGIN=ON` copies the plugins into the user plugin folders after building (off by
default). On Linux install the JUCE dependencies first (see `.github/workflows/build.yml`).

## Test runner

`PercusynTests` without arguments runs all checks (run it from the repository root, one check
imports a file from `examples/presets`):

- every voice renders, stays bounded with extreme settings (drive, resonance, comb feedback 95 %,
  resonator decay 8 s, +12 dB) at 96 kHz, velocity sensitivity, sustain hold and release;
- sequencer: internal clock, conditions, swing, host clock at several block sizes (no double or
  missing steps at block borders);
- processor: MIDI note map, ignored notes, internal sequencer start/stop, multi-out routing, host
  state round trip, all factory presets, INIT, preset JSON round trip, mockup preset import;
- step painting with simulated mouse events (left drag draws, right drag erases).

Other modes:

| Command | Result |
| --- | --- |
| `--render out.wav <voice 0-8> [preset]` | one-shot of a voice at velocity 115 (24-bit, 48 kHz) |
| `--kit out.wav [preset] [bars]` | the internal sequencer playing a preset |
| `--snapshot out.png <tab 0-3> [voice] [scale]` | editor screenshot (tab: SYNTH, SEQ, FX, MIX) |
| `--load <path to PERCUSYN.vst3>` | loads the built VST3 like a host: scan, instance, buses, MIDI audio, state, editor |

Manual images: `docs/images/synth.png`, `seq.png`, `fx.png`, `mix.png` are
`--snapshot docs/images/<name>.png <tab> 0 0.9115` (1400 px wide). Regenerate them when the panel
changes.

## Code map

| Path | Content |
| --- | --- |
| `Source/dsp/Types.h` | Parameter contract: keys, ranges, units, per-voice sound defaults, step data. Keys are permanent IDs (state, presets, automation). |
| `Source/dsp/Voice.*` | One instrument: two overlapping hits (monophonic retrigger with 4 ms fade), oscillator, noise, drive, SVF filter, envelopes, comb, resonator, level/pan. No JUCE. |
| `Source/dsp/Fx.*` | Stereo delay and an 8-line FDN reverb. |
| `Source/dsp/Engine.*` | Nine instruments, mute/solo, voice outputs, FX bus, master, Closed→Open Hat choke. |
| `Source/dsp/Sequencer.*` | Sample-accurate 16-step clock from the internal tempo or the host ppq position, swing, probability, conditions. |
| `Source/Params.*` | APVTS layout (317 host parameters), value formatting, LED colour rule. |
| `Source/Pattern.*` | Lock-free step store (plug-in state, not host parameters). |
| `Source/Presets.*` | Factory presets (embedded from `examples/presets`), user presets, JSON in the mockup's format (schema 1 and 2). |
| `Source/OneShot.*` | Offline render from a parameter snapshot, WAV writer, render folder. |
| `Source/PluginProcessor.*` | Buses (main + 9 voice outputs), MIDI, transport/host-sync logic, preview playback, state. |
| `Source/PluginEditor.*`, `Source/ui/` | Native editor: rack in 1536 × 1120 logical pixels scaled with the window; look, widgets, pages. |
| `mockup/` | The approved browser mockup (design reference, `cd mockup && npm start`). |

Realtime rules: the audio thread never allocates, locks (the preview uses a try-lock), logs or
touches files. Parameters are read through raw atomic pointers, the pattern through atomics copied
into a block snapshot.

## Sound design notes

- Base pitch at Tune 0: Closed/Open Hat 400 Hz, Snare 185, Tom 98, Shaker 520, Clap 880, Perc 330,
  Rim 520, Click 1400 Hz. FM ratio per voice between 1.41 and 3.1.
- Osc Decay shapes only the tonal layer; Amp Decay the whole voice. Sustain > 0 holds while the
  gate is open (MIDI note, or one sixteenth from the sequencer) and then releases with Decay.
- Pitch envelope Amount: up to +48 semitones at 100 %.
- Comb: Hermite fractional delay, one-pole damping and a soft limit in the loop, DC blocker.
- Resonator: six two-pole modes; ratio tables METAL 1, 2.756, 5.404, 8.933, 13.34, 18.64 · PLATE
  1, 2.08, 3.41, 3.89, 5.0, 5.95 · TUBE 1, 3, 5, 7, 9, 11; Damping shortens higher modes
  (`T60 · ratio^(-1.4 · damping)`), modes above 0.45 × sample rate are dropped.
- Voice headroom factor 0.6 (about -4.4 dB) so simultaneous hits stay below full scale.
- One-shot: one sixteenth gate at 120 BPM, tail until -90 dBFS for 50 ms (max 10 s), 2 ms end fade.

## Validation status

Done on Windows x64: build of VST3, CLAP and Standalone, all offline checks, `--load` of the VST3.
The release workflow adds macOS (incl. `auval`) and Linux builds and tests. Not yet done:
pluginval, clap-validator, and tests in real DAWs (Ableton Live, Bitwig Studio, FL Studio, REAPER,
Waveform Free).
