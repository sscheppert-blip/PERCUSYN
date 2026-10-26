# Parametervertrag für JUCE / APVTS

`src/model.js` ist die einzige Quelle für **1036 Parameter**. `npm run generate:juce` erzeugt `parameters.json` und `ParameterLayout.h` ohne Browser oder weitere Pakete. Einträge enthalten ID, Namen, Typ, Min/Max, Intervall, Standardwert, Einheit, ggf. logarithmische Skalierung/Choices und `versionHint: 1`.

## Feste IDs

- Globale Werte: `global_tempo`, `global_swing`, `global_length`, `global_fx_delay_time`, `global_master_level` usw.
- Stimmen: `closedhat_osc_tune`, `snare_amp_decay`, `tom_filter_cutoff`, `clap_level` usw.
- Steps: `closedhat_step_01_active`, `closedhat_step_01_velocity`, `closedhat_step_01_probability`, `closedhat_step_01_condition`, `closedhat_step_01_octave` bis `click_step_16_octave`.
- Stimmen-IDs: `closedhat`, `openhat`, `snare`, `tom`, `shaker`, `clap`, `perc`, `rim`, `click`.
- Open und Closed HiHat sind eigenständige Stimmen. Kein `hat_open`-Umschalter und keine Kick in Schema v2. Der Browser migriert v1-Presets; siehe `migrateV1()` in `src/model.js`.
- IDs und VersionHints nach einer Veröffentlichung nicht ändern. Neue Parameter ergänzen; semantische Änderungen verlangen eine State-Migration.
- Klangregler erscheinen ausschließlich im Synth-Panel. Der Mixer enthält Level, Pan, Send FX, Mute/Solo. Keine zusätzlichen Makro-Duplikate.

## Typen und Werte

`float` → AudioParameterFloat; `int` → AudioParameterInt; `bool` → AudioParameterBool; `choice` → AudioParameterChoice. Choice-Werte sind nullbasierte Indizes. Persistiert werden reale Werte (Hz, ms, dB usw.), keine normalisierten Positionen. `noise_level = -60 dB` ist ein endlicher Wert, kein −∞-Sentinel.

Lineare Normalisierung: `(value - min) / (max - min)`.
Logarithmische Normalisierung: `log(value / min) / log(max / min)`.
Inverse: `min * pow(max / min, normalized)`. Intervalle werden relativ zum Minimum gerundet und danach begrenzt. Der Header erzeugt dafür die entsprechenden NormalisableRange-Konvertierungen inklusive Snap-Funktion. Diese Abbildung darf nicht durch einen beliebigen Skew-Faktor ersetzt werden, wenn Browser und JUCE identische Stellungen zeigen sollen.

Die 16 Steps je Stimme sind hier für transparente Automatisierbarkeit ebenfalls Parameter. Das erzeugt viele Host-Parameter. Soll ein späteres Produkt Pattern stattdessen als nicht automatisierbaren ValueTree-State speichern, ist diese Entscheidung vor der ersten veröffentlichten Plug-in-Version zu treffen; dann die JSON-IDs in die neue State-Struktur übernehmen und Versionierung/Migration festlegen.

## APVTS anbinden

Im AudioProcessor: `apvts (*this, nullptr, "PERCUSYN", percusyn::createParameterLayout())`.
SliderAttachment für Float/Int, ComboBoxAttachment für Choices, ButtonAttachment für Bools verwenden. Der Editor zeichnet Metall, Segmentringe und digitale Anzeigen in einem eigenen LookAndFeel. SVG-Pfade und CSS-Geometrie dienen als übertragbare Designvorlage, CSS selbst ist kein JUCE-LookAndFeel.

Falls zunächst eine WebView-Oberfläche beibehalten wird, einen Adapter mit folgenden Nachrichten implementieren:

```text
beginGesture { id }
setParameter { id, normalizedValue }
endGesture { id }
parameterChanged { id, realValue }   // Host → UI, Rückkopplung vermeiden
replaceState { schemaVersion, parameters }
```

Host-Änderungen zurück an den Store melden; Gesten müssen später beginChangeGesture/endChangeGesture kapseln. Das Browser-Mockup emittiert aktuell Store-Änderungen, aber keine Host-Gesten; der Adapter ist bewusst noch nicht implementiert.

APVTS copyState/replaceState für Plug-in-State auf einem geeigneten Thread verwenden. Im Audiothread ausschließlich atomare Parameterwerte lesen und unveränderliche Pattern-Snapshots verwenden. Die Browser-Strukturen (DOM, Map, Events, setTimeout, JSON, localStorage) gehören nicht in eine Audio-Callback-Funktion.

`src/transport.js` dient nur der UI-Vorschau und ist nicht samplegenau. Für ein Plug-in muss der Audioprozessor Steps anhand Sampleposition, Host-Tempo/PPQ und Transport berechnen. Die Bedingungsindizes sind ALL, 1:2, 2:2, 1:4; eine Zählung beginnt bei Cycle 0. Wahrscheinlichkeit wird pro Trigger ausgewertet. Stumm/Solo werden vor der Triggerausgabe gefiltert.

Der JSON-Import ist atomar: Schema-Version/Format/Name und sämtliche Werte werden geprüft, unbekannte IDs und nicht numerische Werte abgelehnt, gültige Zahlen auf den Bereich begrenzt. Fehlende Werte werden auf den Schema-Standard ergänzt. Im JUCE-State-Adapter dieselbe Policy oder eine ausdrücklich versionierte Migration umsetzen.

## Referenzen

- [JUCE AudioProcessorValueTreeState](https://docs.juce.com/develop/classjuce_1_1AudioProcessorValueTreeState.html)
- [JUCE NormalisableRange](https://docs.juce.com/develop/classjuce_1_1NormalisableRange.html)
- [JUCE ParameterID](https://docs.juce.com/develop/classjuce_1_1ParameterID.html)

Das generierte Layout zielt auf die aktuelle API mit ParameterID und Parameter-Attributes. Es wurde hier nicht gegen einen JUCE-Checkout kompiliert; beim Einbau im Zielprojekt verifizieren.

## Geplanter Sample-Export im VST (noch nicht implementiert)

Die Browser-Oberfläche zeigt nur Exportoptionen und eine aus Parametern gezeichnete Amp-Hüllkurve. Sie generiert weder Audiodaten noch Test-WAVs. Render, Preview und Datei-Drag sind deaktiviert.

Spätere JUCE-Implementierung: Parameter-Snapshot der aktuellen Stimme erfassen; einen vollständigen One-Shot außerhalb des Audiothreads offline durch die zukünftige Synth-Engine rendern; mit WAV-Writer (wählbare Rate/Bittiefe) als echte temporäre Audiodatei speichern. Die Datei vor dem Ziehen vollständig schreiben und schließen. Über [`juce::DragAndDropContainer::performExternalDragDropOfFiles`](https://docs.juce.com/master/classjuce_1_1DragAndDropContainer.html) den nativen Datei-Drag auslösen. Datei-Lebensdauer bis nach dem Drop und länger sicherstellen, keine sofortige Löschung. Als Alternative einen expliziten WAV-Speicherdialog anbieten.

Dieser native Drag-Ansatz ist das Ziel für das VST, keine Browser-DownloadURL-Lösung. Die Annahme der Datei muss später in den Ziel-DAWs und auf den jeweiligen Betriebssystemen verifiziert werden; universelle Unterstützung wird nicht allein durch den Mockup zugesichert. Exportoptionen gehören in separaten UI-/Export-State und müssen keine automatisierbaren Klangparameter sein.

Die OCT-Zeile wurde aus dem Sequencer entfernt. Die alten `_octave`-State-Felder bleiben nur zur Kompatibilität mit bereits exportierten Mockup-Presets erhalten; sie werden in der Oberfläche nicht mehr angeboten und sollen bei einer späteren Engine nicht als versteckte Transposition angewendet werden. Tune am Instrument ist weiterhin verfügbar.

## Comb und Resonator

Elf neue Parameter je Stimme: `comb_enabled`, `comb_tune`, `comb_feedback`, `comb_damping`, `comb_mix`, `resonator_enabled`, `resonator_model`, `resonator_tune`, `resonator_decay`, `resonator_damping`, `resonator_mix`, jeweils mit Stimmenpräfix. Tune ist logarithmisch in Hz, Resonator-Decay logarithmisch in ms. Model-Choices sind METAL (0), PLATE (1), TUBE (2). Beide Bereiche starten im Bypass. Fehlende Felder älterer v1-/v2-Presets erhalten automatisch die neuen Schema-Standards; vorhandene IDs bleiben unverändert.

Die Darstellung und Parameterpersistenz sind implementiert. Der gezeigte zukünftige Signalfluss lautet Instrument → Drive → Filter → Comb → Resonator → Send FX → Master. Kein Comb-Filter, Resonator oder anderer DSP wurde implementiert. Die Modellnamen definieren zunächst die geplante Bedienung; konkrete Resonanzverhältnisse und Algorithmen sind bei der späteren Engine-Entwicklung festzulegen.
