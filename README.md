# PERCUSYN

Nine-voice percussion synthesizer: Closed Hat, Open Hat, Snare, Tom, Shaker, Clap, Perc, Rim and Click, each with its own Comb and Resonator stage, a 16-step sequencer, send FX, a mixer with multi-out and one-shot rendering with drag into the DAW. Built with JUCE as VST3, CLAP and Standalone for macOS, Windows and Linux, plus AU on macOS.

## Features

- **Nine synthesized voices**, no samples: oscillator (SINE, TRI, SAW, NOISE, FM) with its own decay, pitch envelope, amp envelope (Attack / Decay / Sustain), noise layer with color, drive, LP/HP/BP/BR filter, pitch-wheel range and velocity sensitivity per voice.
- **Metallic stages per voice:** a tuned feedback comb and a six-mode resonator (METAL, PLATE, TUBE), each with its own bypass.
- **Sequencer:** 16 steps per voice with velocity, probability and conditions (ALL, 1:2, 2:2, 1:4), length, swing, internal tempo. HOST off: while the host plays, the internal sequencer is off and MIDI plays the voices. HOST lit: the sequencer follows the host transport.
- **MIDI:** C1 (note 36) upward plays the voices in order, on any channel. Closed Hat chokes Open Hat; every voice retriggers monophonically.
- **Mixer and outputs:** level, pan, FX send, mute and solo per voice, stereo delay and reverb on the FX bus, master. Main stereo plus one stereo output per voice.
- **One-shot export:** renders the selected voice at the velocity of the selected step (comb, resonator, level and pan included; no global FX or master, no normalization), previews it, saves 16/24-bit PCM or 32-bit float WAV, and drags the file straight into the arrangement. Renders stay in `Documents/PERCUSYN/Renders`.
- **Presets contain everything:** sounds, patterns, sequencer settings, FX and mixer. Six factory kits, user presets, JSON import/export (compatible with the browser mockup's format).
- Freely scalable editor in the approved hardware design: graphite and brushed metal, orange/blue LED rings (blue for times), orange seven-segment values, English tooltips.

**Manual:** [English](docs/Manual.md) · [Deutsch](docs/Handbuch.md)

![PERCUSYN SYNTH page](docs/images/synth.png)

## Download

Ready-built plug-ins for macOS (AU, VST3, CLAP, Standalone; universal), Windows and Linux (VST3, CLAP, Standalone) are on the [Releases](https://github.com/sscheppert-blip/PERCUSYN/releases) page. Installation folders are listed in the manual. The plug-ins are not signed; on macOS remove the download quarantine once (see the manual).

## Building

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/PercusynTests_artefacts/Release/PercusynTests
```

CMake 3.22+, a C++20 compiler and internet access for the first configure (JUCE 9.0.3 and clap-juce-extensions are fetched into the build tree). The test tool also renders kits (`--kit`), single voices (`--render`), editor snapshots (`--snapshot`) and loads the built VST3 like a host (`--load`).

Details, test runner and code map: [docs/Development.md](docs/Development.md). The browser mockup that defined the design is in `mockup/` (`cd mockup && npm start`).

## License

PERCUSYN's own code is released into the public domain under the [Unlicense](LICENSE). The plug-ins are built with [JUCE](https://juce.com), which has its own license terms, and use [clap-juce-extensions](https://github.com/free-audio/clap-juce-extensions) for CLAP.
