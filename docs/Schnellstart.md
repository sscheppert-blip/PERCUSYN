# PERCUSYN – Schnellstart

Ausführlich: [Handbuch](Handbuch.md).

PERCUSYN ist ein Percussion-Synthesizer mit neun Stimmen – **Closed Hat, Open Hat, Snare, Tom, Shaker,
Clap, Perc, Rim, Click** –, einem 16-Step-Sequencer, zwei Effekten und einem Mixer mit Einzelausgängen.
Alles synthetisch, keine Samples.

![SYNTH-Seite](images/synth.png)

---

## Loslegen

1. PERCUSYN auf eine **Instrument-/MIDI-Spur** laden. Geladen ist das Kit **Acid Hats**.
2. In der Instrumentenleiste **▶** drücken – der interne Sequencer spielt (solange die DAW steht).
3. Eine Instrumenten-Taste anklicken, z. B. **SNARE**: Sie wird ausgewählt und angespielt.
4. Auf der **SYNTH**-Seite an den Knöpfen drehen. **Shift** = fein, **Doppelklick** = zurücksetzen.
5. Unten rechts Steps zeichnen: **linke** Maustaste ziehen zeichnet, **rechte** löscht.

Mit der DAW im Takt: **HOST** einschalten – dann folgt der Sequencer Transport und Tempo der DAW.

## Oben und unten

| Element | Funktion |
|---|---|
| **‹ Preset ›** | Kits blättern, Klick auf den Namen = Liste. **INIT**, **SAVE**, **•••** (Import/Export) |
| **SYNTH / SEQ / FX / MIX** | Hauptseite umschalten |
| **Instrumenten-Tasten** | Stimme wählen und anspielen; **M**/**S** darunter = Mute/Solo |
| **▶ ■ HOST PATTERN RANDOM LENGTH SWING** | Sequencer-Steuerung |
| **Unten links** | One-Shot-Export |
| **Unten rechts** | Step-Spur der gewählten Stimme und **BPM** |

Blaue LED-Ringe = Zeiten, orange Ringe = alles andere.

## SYNTH – eine Stimme formen

| Gruppe | Regler |
|---|---|
| **Oszillator** | **TUNE**, **DECAY**, **TONE**, Wellenform SINE/TRI/SAW/NOISE/FM |
| **Pitch-Hüllkurve** | **AMOUNT**, ATTACK, DECAY – der „Pew“ von Toms und Kicks |
| **Amp-Hüllkurve** | ATTACK, DECAY, SUSTAIN |
| **Filter / Noise / Drive** | CUTOFF, RESONANCE, TYPE, NOISE COLOR/LEVEL, DRIVE |
| **COMB / RESONATOR** | Metallisches Klingeln, je mit eigenem ON/OFF |

## SEQ – alle Spuren

![SEQ-Seite](images/seq.png)

Pro Step: **VEL**, **PROB** (Wahrscheinlichkeit) und **COND** (· / 1:2 / 2:2 / 1:4). **CLEAR** löscht die
Spur.

## FX und MIX

<img src="images/mix.png" alt="MIX-Seite" width="70%">

**FX:** Delay und Reverb, gespeist über **SEND FX** jeder Stimme. **MIX:** PAN, LEVEL, SEND FX, Mute/Solo
und MASTER. Jede Stimme hat zusätzlich einen **eigenen Stereo-Ausgang** in der DAW.

## Per MIDI spielen

C1 (Note 36) = Closed Hat, dann aufwärts: C#1 Open Hat, D1 Snare, D#1 Tom, E1 Shaker, F1 Clap, F#1 Perc,
G1 Rim, G#1 Click. Die Closed Hat würgt die Open Hat ab.

## Sound als WAV

Feld unten links: **RENDER** berechnet die gewählte Stimme, **PREVIEW** hört rein, **DRAG TO DAW** zieht
das WAV direkt ins Arrangement oder in einen Sampler.
