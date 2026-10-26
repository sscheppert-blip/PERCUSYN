# PERCUSYN · interaktives UI-Mockup

Englischsprachiges Browser-Instrument mit normalbreiter Arial-Schrift und neutralem Schwarz/Graphit ohne Olivstich, nach der gelieferten Hardware-Referenz: dunkles Gehäuse mit Metallrahmen, gebürstete Knobs, segmentierte LED-Ringe, schlanke Metallfader, beschriftete Skalen und echte, als SVG gezeichnete orange 7-Segment-Anzeigen. **Auf Wunsch ohne Klangerzeugung.** Es wird kein AudioContext erstellt, kein Sample geladen und keine Mikrofonberechtigung angefordert.

## Start

Voraussetzung: Node.js 20 oder neuer. Keine Installation von Paketen und kein Build nötig.

1. ZIP entpacken und im Ordner `mockup` ein Terminal öffnen.
2. `npm start` ausführen.
3. **http://127.0.0.1:5173** im Browser öffnen.

Auf macOS alternativ `Start-PERCUSYN.command` doppelklicken. Das Terminal bleibt für den lokalen Server offen; Ctrl+C beendet ihn. Falls macOS das Startskript sperrt, die Terminalvariante verwenden. Einen anderen Port wählt `PORT=5180 npm start` (macOS/Linux). Der Server bindet nur an den eigenen Rechner. Keine externen Fonts, CDN-Dateien, Konten oder Netzwerkdienste erforderlich. Direktes Öffnen von `index.html` über `file://` funktioniert wegen ES-Modulen nicht zuverlässig.

Das vollständige Instrument passt sich automatisch an Breite **und Höhe** des Browserfensters an, auch nach einem Wechsel zwischen SYNTH, SEQ, FX und MIX. Alle vier Ansichten verwenden dieselbe feste Gehäusegröße. Nur beim Verändern des Browserfensters wird das Instrument angepasst. Die Hardware-Anordnung bleibt erhalten, ohne Scrollbalken und ohne den Browser-Zoom zu verändern. In kleinen Fenstern werden entsprechend auch die Beschriftungen kleiner. Ein größeres Fenster bietet mehr Platz zum Bedienen. Dialoge öffnen sich separat in lesbarer Größe.

## Bedienung

- **Instrumente:** Closed HiHat, Open HiHat, Snare, Tom, Shaker, Clap, Perc, Rim und Click haben jeweils eigene Parameter und 16 eigene Steps. Es gibt keine Kick. Open/Closed sind eigenständige Stimmen mit getrennten Patterns und Parametern.
- **LED-Farben:** Zeitdauern in ms/s haben blaue Ringe; Klang, Tonhöhe, Pegel und Mischanteile orange. Resonance, Noise Color, Pitch Bend, Pan, Feedback und Damping sind orange. Digitale Wertanzeigen bleiben orange.
- **Tooltips:** englische Erklärungen für Klangparameter, Steps, Transport, Presets und Export. Nach kurzem Verweilen mit der Maus oder bei Tastaturfokus erscheinen sie außerhalb der skalierten Oberfläche in lesbarer Größe. Escape blendet sie aus.
- **Knobs:** nach oben/rechts ziehen; Shift für feine Bewegungen. Pfeiltasten, Page Up/Down und Home/End werden unterstützt. Doppelklick setzt den jeweiligen Standardwert.
- **Keine doppelten Klangregler:** Tune, Decay, Filter und Drive gibt es nur im Synth-Panel. Im Mixer gibt es pro Instrument genau einen Level-Fader sowie Pan, Send FX, Mute und Solo.
- **Geplanter Sample-Export im VST:** Der Bereich ONE-SHOT / EXPORT zeigt die Amp-Hüllkurve der gewählten Stimme als UI-Vorschau und die wählbaren Export-Zielwerte (WAV, Rate, Bittiefe). Diese Optionen sind Browser-Einstellungen, keine APVTS-Klangparameter. Render, Preview, WAV-Ausgabe und Drag ins Arrangement sind bewusst deaktiviert. Noch keine Audio-Engine, keine Audiodatei, kein funktionierender Datei-Drag. Die spätere JUCE-Version soll einen One-Shot im Hintergrund rendern, als WAV speichern und diese Datei nativ an die DAW ziehen lassen.
- **Wertanzeige:** klicken, exakten Wert eingeben, übernehmen. Die angezeigten kHz/s werden im Dialog in Hz/ms eingegeben.
- **SYNTH:** Oszillator-Wellenform und drei Regler, Pitch- und Amp-Hüllkurve, Filtertypen LP/HP/BP/BR, Noise, Drive, Pitch Bend und Velocity-Sensitivität.
- **COMB:** pro Instrument schaltbar, mit Tune (20 Hz–12 kHz), Feedback (0–95 %), Damping und Mix. Die Werte sind reine UI-/Preset-Parameter; noch keine Audioverarbeitung.
- **RESONATOR:** pro Instrument schaltbar, mit den geplanten Modellen METAL, PLATE und TUBE sowie Tune, Decay, Damping und Mix. OFF überbrückt den Bereich im geplanten Signalfluss, behält seine Einstellungen aber bei.
- **SEQ:** alle neun Patterns gleichzeitig bearbeiten. Instrumentname wählt die Detailspur unten.
- **Step-Reihen:** STEP an/aus; VEL 1–127; PROB 0–100 %; COND ALL, erster/jeder zweite Takt, zweiter/jeder zweite Takt oder erster/jeder vierte Takt. VEL/PROB öffnen die Eingabe, COND wechselt per Klick; Rechtsklick öffnet immer eine exakte Werteingabe. Mausrad ändert die Detailwerte.
- **Transport:** Play/Pause oder Leertaste außerhalb eines Eingabefelds bzw. Buttons. Stop setzt den Ablauf zurück. BPM 40–240, Länge 1–16, Swing 0–75 %. Ungerade Steps werden mit Swing verzögert, gerade entsprechend verkürzt. Verborgene Browserfenster pausieren den Transport.
- **Trigger-Anzeige:** Instrumenttasten blitzen bei ausgelösten Events auf. Mute, Solo, Wahrscheinlichkeit und Taktbedingungen werden berücksichtigt. Ein laufender Cursor markiert auch Steps, die wegen einer Bedingung nicht triggern. Noten-/Oktavwerte bleiben in diesem visuellen Prototyp gespeichert.
- **PATTERN:** zwischen SYNTH und SEQ wechseln. RANDOM belegt nur Steps und Velocity der gewählten Stimme zufällig. CLEAR schaltet nur deren Steps aus; Detailwerte bleiben erhalten.
- **FX:** globale Delay- und Reverb-Parameter, vollständig editierbar und im Preset enthalten; ohne Signalverarbeitung.
- **MIX:** neun Kanäle mit Pan, Level, Send FX, Mute und Solo plus Master-Level. Mute/Solo wirken auf sichtbare Trigger, Pegel/Pan auf gespeicherte Werte.
- **Presets:** fünf Factory-Zustände, Vor/Zurück und Auswahlmenü. SAVE erstellt eine benannte Kopie im lokalen Browser-Speicher. Der kleine orange Punkt hinter PRESET zeigt Änderungen an. Wechsel zu einem anderen Preset oder INIT verwirft ungespeicherte Änderungen.
- **•••:** JSON exportieren/importieren, ein eigenes Preset löschen oder die Bedienhilfe öffnen. Factory-Presets sind fest eingebaut. JSON-Dateien sind portable Sicherungen; Browser-Speicher hängt von Browserprofil und Server-Port ab. Import verändert zunächst nur den aktuellen Zustand; SAVE legt eine lokale Kopie an.
- **INIT:** alle Parameter auf Schema-Standard, alle neun Patterns leer. Das ist nicht der erste Factory-Zustand.

## Aufbau und Portierung nach JUCE

```text
index.html                   semantische Oberfläche / Panels
src/style.css                Gehäuse, Metall, Ringe, Fader, responsive Layouts
src/app.js                   UI-Bindings, Gesten, Preset-Verwaltung
src/model.js                 unabhängiges Schema, Store, Validierung, Normalisierung
src/control-colors.js        zentrale semantische Zuordnung der LED-Ringfarben
src/tooltips.js              englische Control-Erklärungen und skalierungsfreie Tooltips
src/sample-ui.js             reine Hüllkurvenvorschau und geplante VST-Exportbedienung
src/layout.js                automatische Anpassung an beide Fensterdimensionen
src/transport.js             eigenständiger visueller Step-Taktgeber / Triggerbedingungen
juce/parameters.json          maschinenlesbares vollständiges Parameterverzeichnis
juce/ParameterLayout.h        daraus erzeugtes APVTS-ParameterLayout
juce/PORTING.md               Konventionen und Integrationshinweise
server.mjs                   kleiner lokaler HTTP-Server
```

Schema, Store und Transport sind DOM-unabhängig. Oberfläche und Fader greifen ausschließlich über die IDs auf denselben Store zu. Die Integration lässt sich im Browser über `window.percusyn` untersuchen. Es gibt absichtlich noch keine Audio-Engine, keinen DSP- oder Host-Adapter und kein fertiges JUCE-Plug-in. Das Preset-Schema ist jetzt v2. Alte v1-Presets werden beim Laden migriert: die einzelne HiHat wird in die getrennten Hat-Stimmen übernommen, die Kick entfernt. Neue Exporte verwenden v2.

## Prüfen und Parameterdateien neu erzeugen

```sh
npm test
npm run generate:juce
```

Die zwölf automatisierten Tests prüfen ID-Eindeutigkeit, alle 9×16 Steps, Parameterbereiche und Normalisierung, Trennung der Stimmen, Preset-Rundlauf und atomare Importvalidierung, Factory-Zustände, Triggerbedingungen und Transport-Reset. Das C++-Layout ist erzeugter Integrationscode; es wurde in diesem Projekt nicht gegen JUCE kompiliert.

Browser-Prüfung: Werteingabe, getrennte Instrumente, Step an/aus, Presetspeicherung, visueller Transport, neunspurige SEQ-Ansicht, FX und Mixer/Mute wurden im eingebauten Browser geprüft. Die komplette Hardware-Ansicht passt sich an Breite und Höhe des Fensters an; der untere Bereich enthält keine wiederholten Klangregler mehr. Kein Audio- oder DAW-Test, da diese Version bewusst nur die Oberfläche und Datenlogik enthält.
