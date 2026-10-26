#pragma once

// Parameter contract shared by the engine, the plug-in and the editor. Keys and order mirror
// mockup/src/model.js; the keys are permanent IDs (state, presets and host automation).

#include <array>
#include <cstdint>
#include <stddef.h>

namespace percusyn
{
constexpr int numVoices = 9;
constexpr int numSteps = 16;
constexpr int firstMidiNote = 36;   // C1 plays Closed Hat, upward in voice order

inline constexpr std::array<const char*, numVoices> voiceKeys { "closedhat", "openhat", "snare", "tom", "shaker",
                                                                "clap", "perc", "rim", "click" };
inline constexpr std::array<const char*, numVoices> voiceLabels { "CLOSED HAT", "OPEN HAT", "SNARE", "TOM", "SHAKER",
                                                                  "CLAP", "PERC", "RIM", "CLICK" };

enum class Kind : std::uint8_t { Float, Choice, Bool, Int };

struct ParamDef
{
    const char* key;
    const char* name;
    float min, max, def, step;
    const char* unit;
    bool log;
    Kind kind;
    const char* choices;   // '|' separated, for Kind::Choice
};

enum VoiceParam
{
    oscTune, oscDecay, oscTone, oscWave,
    pitchAmount, pitchAttack, pitchDecay,
    ampAttack, ampDecay, ampSustain,
    filterCutoff, filterResonance, filterType,
    noiseColor, noiseLevel, driveAmount, pitchBend, velocitySens,
    combEnabled, combTune, combFeedback, combDamping, combMix,
    resEnabled, resModel, resTune, resDecay, resDamping, resMix,
    level, sendFx, pan, mute, solo,
    numVoiceParams
};

inline constexpr std::array<ParamDef, numVoiceParams> voiceParamDefs { {
    { "osc_tune", "Tune", -24, 24, 0, 1, "st", false, Kind::Float, nullptr },
    { "osc_decay", "Decay", 1, 10000, 124, 1, "ms", true, Kind::Float, nullptr },
    { "osc_tone", "Tone", 0, 100, 28, 1, "%", false, Kind::Float, nullptr },
    { "osc_wave", "Wave", 0, 4, 3, 1, "", false, Kind::Choice, "SINE|TRI|SAW|NOISE|FM" },
    { "pitch_amount", "Amount", 0, 100, 62, 1, "%", false, Kind::Float, nullptr },
    { "pitch_attack", "Attack", 0.1f, 500, 0.5f, 0.1f, "ms", true, Kind::Float, nullptr },
    { "pitch_decay", "Decay", 1, 5000, 86, 1, "ms", true, Kind::Float, nullptr },
    { "amp_attack", "Attack", 0.1f, 500, 0.2f, 0.1f, "ms", true, Kind::Float, nullptr },
    { "amp_decay", "Decay", 1, 5000, 210, 1, "ms", true, Kind::Float, nullptr },
    { "amp_sustain", "Sustain", 0, 100, 0, 1, "%", false, Kind::Float, nullptr },
    { "filter_cutoff", "Cutoff", 20, 20000, 8500, 1, "Hz", true, Kind::Float, nullptr },
    { "filter_resonance", "Resonance", 0, 100, 12, 1, "%", false, Kind::Float, nullptr },
    { "filter_type", "Type", 0, 3, 0, 1, "", false, Kind::Choice, "LP|HP|BP|BR" },
    { "noise_color", "Color", 0, 100, 38, 1, "%", false, Kind::Float, nullptr },
    { "noise_level", "Level", -60, 12, 0, 0.1f, "dB", false, Kind::Float, nullptr },
    { "drive_amount", "Amount", 0, 100, 24, 1, "%", false, Kind::Float, nullptr },
    { "pitch_bend", "Bend", -24, 24, 0, 1, "st", false, Kind::Float, nullptr },
    { "velocity_sensitivity", "Sens", 0, 100, 72, 1, "%", false, Kind::Float, nullptr },
    { "comb_enabled", "Comb enabled", 0, 1, 0, 1, "", false, Kind::Bool, nullptr },
    { "comb_tune", "Tune", 20, 12000, 1200, 1, "Hz", true, Kind::Float, nullptr },
    { "comb_feedback", "Feedback", 0, 95, 45, 1, "%", false, Kind::Float, nullptr },
    { "comb_damping", "Damping", 0, 100, 35, 1, "%", false, Kind::Float, nullptr },
    { "comb_mix", "Mix", 0, 100, 25, 1, "%", false, Kind::Float, nullptr },
    { "resonator_enabled", "Resonator enabled", 0, 1, 0, 1, "", false, Kind::Bool, nullptr },
    { "resonator_model", "Model", 0, 2, 0, 1, "", false, Kind::Choice, "METAL|PLATE|TUBE" },
    { "resonator_tune", "Tune", 20, 12000, 800, 1, "Hz", true, Kind::Float, nullptr },
    { "resonator_decay", "Decay", 5, 8000, 350, 1, "ms", true, Kind::Float, nullptr },
    { "resonator_damping", "Damping", 0, 100, 40, 1, "%", false, Kind::Float, nullptr },
    { "resonator_mix", "Mix", 0, 100, 25, 1, "%", false, Kind::Float, nullptr },
    { "level", "Level", -60, 12, -6, 0.1f, "dB", false, Kind::Float, nullptr },
    { "send_fx", "Send FX", -60, 0, -12, 0.1f, "dB", false, Kind::Float, nullptr },
    { "pan", "Pan", -100, 100, 0, 1, "%", false, Kind::Float, nullptr },
    { "mute", "Mute", 0, 1, 0, 1, "", false, Kind::Bool, nullptr },
    { "solo", "Solo", 0, 1, 0, 1, "", false, Kind::Bool, nullptr },
} };

enum GlobalParam
{
    tempo, swing, length, delayTime, delayFeedback, delayMix, reverbSize, reverbDecay, reverbMix, masterLevel,
    hostSync,
    numGlobalParams
};

// global_host_sync is new in the native plug-in: 0 = the internal sequencer stops while the
// host plays (MIDI only), 1 = the internal sequencer follows the host transport.
inline constexpr std::array<ParamDef, numGlobalParams> globalParamDefs { {
    { "tempo", "Tempo", 40, 240, 124, 1, "BPM", false, Kind::Float, nullptr },
    { "swing", "Swing", 0, 75, 12, 1, "%", false, Kind::Float, nullptr },
    { "length", "Length", 1, 16, 16, 1, "", false, Kind::Int, nullptr },
    { "fx_delay_time", "Delay time", 20, 1000, 250, 1, "ms", true, Kind::Float, nullptr },
    { "fx_delay_feedback", "Feedback", 0, 95, 28, 1, "%", false, Kind::Float, nullptr },
    { "fx_delay_mix", "Delay mix", 0, 100, 15, 1, "%", false, Kind::Float, nullptr },
    { "fx_reverb_size", "Room size", 0, 100, 45, 1, "%", false, Kind::Float, nullptr },
    { "fx_reverb_decay", "Reverb decay", 100, 10000, 1800, 1, "ms", true, Kind::Float, nullptr },
    { "fx_reverb_mix", "Reverb mix", 0, 100, 20, 1, "%", false, Kind::Float, nullptr },
    { "master_level", "Master", -60, 6, -6, 0.1f, "dB", false, Kind::Float, nullptr },
    { "host_sync", "Follow host", 0, 1, 0, 1, "", false, Kind::Bool, nullptr },
} };

// Per-voice sound defaults (model.js factory()), applied on top of voiceParamDefs.
struct SoundDefault { VoiceParam param; float value; };
inline constexpr std::array<std::array<SoundDefault, 8>, numVoices> soundDefaults { {
    { { { oscDecay, 60 }, { ampDecay, 75 }, { oscTone, 35 }, { pitchAmount, 0 }, { filterType, 1 }, { filterCutoff, 6500 }, { filterResonance, 5 }, { noiseLevel, -9 } } },
    { { { oscDecay, 650 }, { ampDecay, 680 }, { oscTone, 46 }, { pitchAmount, 0 }, { filterType, 1 }, { filterCutoff, 5000 }, { filterResonance, 7 }, { noiseLevel, -7 } } },
    { { { oscWave, 0 }, { oscDecay, 90 }, { ampDecay, 190 }, { oscTone, 30 }, { pitchAmount, 22 }, { filterCutoff, 8500 }, { noiseLevel, -5 }, { noiseLevel, -5 } } },
    { { { oscWave, 0 }, { oscTune, -7 }, { oscDecay, 320 }, { ampDecay, 370 }, { noiseLevel, -38 }, { filterCutoff, 2600 }, { pitchAmount, 35 }, { pitchAmount, 35 } } },
    { { { oscDecay, 50 }, { ampDecay, 160 }, { oscTone, 15 }, { pitchAmount, 0 }, { filterType, 1 }, { filterCutoff, 3500 }, { noiseColor, 70 }, { noiseLevel, -3 } } },
    { { { oscWave, 3 }, { oscDecay, 90 }, { ampDecay, 240 }, { oscTone, 12 }, { pitchAmount, 0 }, { filterType, 1 }, { filterCutoff, 900 }, { noiseLevel, -5 } } },
    { { { oscWave, 4 }, { oscDecay, 170 }, { ampDecay, 210 }, { oscTone, 48 }, { pitchAmount, 32 }, { noiseLevel, -23 }, { filterCutoff, 6000 }, { filterCutoff, 6000 } } },
    { { { oscWave, 1 }, { oscDecay, 55 }, { ampDecay, 85 }, { oscTone, 42 }, { pitchAmount, 10 }, { noiseLevel, -35 }, { filterCutoff, 8000 }, { filterCutoff, 8000 } } },
    { { { oscWave, 2 }, { oscDecay, 12 }, { ampDecay, 23 }, { oscTone, 23 }, { pitchAmount, 0 }, { noiseLevel, -18 }, { filterType, 1 }, { filterCutoff, 1900 } } },
} };

inline float voiceDefault (int voice, int param)
{
    float value = voiceParamDefs[(size_t) param].def;
    for (const auto& d : soundDefaults[(size_t) voice])
        if (d.param == param)
            value = d.value;
    return value;
}

struct VoiceParams
{
    std::array<float, numVoiceParams> v {};
    float operator[] (int i) const noexcept { return v[(size_t) i]; }
    float& operator[] (int i) noexcept { return v[(size_t) i]; }
};

struct GlobalParams
{
    std::array<float, numGlobalParams> v {};
    float operator[] (int i) const noexcept { return v[(size_t) i]; }
    float& operator[] (int i) noexcept { return v[(size_t) i]; }
};

struct Step
{
    bool active = false;
    int velocity = 100;      // 1..127
    float probability = 100; // %
    int condition = 0;       // ALL, 1:2, 2:2, 1:4
};

using Lane = std::array<Step, numSteps>;
using PatternData = std::array<Lane, numVoices>;

inline bool conditionAllows (int condition, long long cycle) noexcept
{
    switch (condition)
    {
        case 1: return cycle % 2 == 0;
        case 2: return cycle % 2 == 1;
        case 3: return cycle % 4 == 0;
        default: return true;
    }
}
} // namespace percusyn
