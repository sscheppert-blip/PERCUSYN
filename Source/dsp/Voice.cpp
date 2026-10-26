#include "dsp/Voice.h"

namespace percusyn::dsp
{
namespace
{
// Base pitch at Tune 0 st, per voice.
constexpr std::array<float, numVoices> basePitch { 400, 400, 185, 98, 520, 880, 330, 520, 1400 };
// FM modulator ratio per voice: inharmonic ratios give metallic spectra.
constexpr std::array<float, numVoices> fmRatio { 1.47f, 1.47f, 1.6f, 1.5f, 2.3f, 1.9f, 1.41f, 2.76f, 3.1f };
// Classic six-square metallic cluster (relative to 400 Hz).
constexpr std::array<float, 6> clusterRatio { 205.3f / 400, 304.4f / 400, 369.6f / 400,
                                              522.7f / 400, 540.0f / 400, 800.0f / 400 };

constexpr float modeRatios[3][Resonator::numModes] {
    { 1.0f, 2.756f, 5.404f, 8.933f, 13.34f, 18.64f },   // METAL: free bar
    { 1.0f, 2.08f, 3.41f, 3.89f, 5.0f, 5.95f },         // PLATE: clamped circular plate
    { 1.0f, 3.0f, 5.0f, 7.0f, 9.0f, 11.0f },            // TUBE: closed tube, odd harmonics
};
constexpr float modeAmps[Resonator::numModes] { 1.0f, 0.75f, 0.6f, 0.45f, 0.35f, 0.28f };

constexpr int clapVoice = 5;

inline float softLimit (float x) noexcept
{
    return std::abs (x) < 1.0f ? x : std::tanh (x);
}
} // namespace

// ---------------------------------------------------------------------------------------------
void Comb::prepare (float sr)
{
    sampleRate = sr;
    int size = 1;
    while (size < (int) (sr / 20.0f) + 8)
        size <<= 1;
    buffer.assign ((size_t) size, 0.0f);
    mask = size - 1;
    reset();
}

void Comb::reset()
{
    std::fill (buffer.begin(), buffer.end(), 0.0f);
    write = 0;
    lp = 0;
    dc.reset();
}

void Comb::set (float tuneHz, float feedback01, float damping01, float mix01)
{
    delay = std::clamp (sampleRate / std::max (20.0f, tuneHz), 2.0f, (float) mask - 4.0f);
    feedback = std::clamp (feedback01, 0.0f, 0.95f);
    damping = std::clamp (damping01, 0.0f, 1.0f) * 0.92f;
    mix = std::clamp (mix01, 0.0f, 1.0f);
}

float Comb::process (float x) noexcept
{
    const float readPos = (float) write - delay;
    const int i1 = (int) std::floor (readPos);
    const float f = readPos - (float) i1;
    const float y0 = buffer[(size_t) ((i1 - 1) & mask)], y1 = buffer[(size_t) (i1 & mask)];
    const float y2 = buffer[(size_t) ((i1 + 1) & mask)], y3 = buffer[(size_t) ((i1 + 2) & mask)];
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    const float delayed = ((c3 * f + c2) * f + c1) * f + y1;

    lp = delayed + damping * (lp - delayed);
    float y = softLimit (x + feedback * lp);
    if (std::abs (y) < 1.0e-15f)
        y = 0;
    buffer[(size_t) write] = y;
    write = (write + 1) & mask;

    const float wet = dc.process (y) * (1.0f - 0.55f * feedback);
    return x + mix * (wet - x);
}

// ---------------------------------------------------------------------------------------------
void Resonator::prepare (float sr)
{
    sampleRate = sr;
    reset();
}

void Resonator::reset()
{
    for (auto& m : modes)
        m.y1 = m.y2 = 0;
    dc.reset();
}

void Resonator::set (int model, float tuneHz, float decayMs, float damping01, float mix01)
{
    model = std::clamp (model, 0, 2);
    mix = std::clamp (mix01, 0.0f, 1.0f);
    const float damping = std::clamp (damping01, 0.0f, 1.0f);
    for (int k = 0; k < numModes; ++k)
    {
        auto& m = modes[(size_t) k];
        const float ratio = modeRatios[model][k];
        const float freq = tuneHz * ratio;
        m.on = freq < sampleRate * 0.45f;
        if (! m.on)
        {
            m.y1 = m.y2 = 0;
            continue;
        }
        // Damping shortens the higher modes progressively; the lowest mode keeps the Decay time.
        const float t60 = std::max (2.0f, decayMs * std::pow (ratio, -1.4f * damping));
        const float r = t60Coefficient (t60, sampleRate);
        const float w = twoPi * freq / sampleRate;
        m.c1 = 2.0f * r * std::cos (w);
        m.c2 = -r * r;
        m.b0 = modeAmps[k] * std::sin (w) * 0.18f;
    }
}

float Resonator::process (float x) noexcept
{
    float sum = 0;
    for (auto& m : modes)
    {
        if (! m.on)
            continue;
        float y = m.b0 * x + m.c1 * m.y1 + m.c2 * m.y2;
        if (std::abs (y) < 1.0e-15f)
            y = 0;
        m.y2 = m.y1;
        m.y1 = y;
        sum += y;
    }
    const float wet = softLimit (dc.process (sum));
    return x + mix * (wet - x);
}

// ---------------------------------------------------------------------------------------------
void Hit::prepare (float sr, int voiceIndex)
{
    sampleRate = sr;
    invSampleRate = 1.0f / sr;
    voice = voiceIndex;
    active = false;
}

void Hit::start (const VoiceParams& p, float velocity01, int gateSamples, std::uint32_t seed)
{
    active = true;
    fading = false;
    fadeGain = 1;
    gated = true;
    gateRemaining = gateSamples;
    age = 0;
    rng.seed (seed);

    const float sens = p[velocitySens] * 0.01f;
    velocityGain = 1.0f - sens + sens * std::pow (std::clamp (velocity01, 0.0f, 1.0f), 1.5f);

    ampLevel = 0;
    ampStage = 0;
    ampAttackStep = 1.0f / std::max (1.0f, p[ampAttack] * 0.001f * sampleRate);
    sustain = p[ampSustain] * 0.01f;
    ampCoeff = t60Coefficient (p[ampDecay], sampleRate);

    oscEnv = 1;
    oscCoeff = t60Coefficient (p[oscDecay], sampleRate);

    pitchEnv = 0;
    pitchRising = true;
    pitchAttackStep = 1.0f / std::max (1.0f, p[pitchAttack] * 0.001f * sampleRate);
    pitchCoeff = t60Coefficient (p[pitchDecay], sampleRate);

    phase = 0;
    modPhase = 0;
    sawLp = 0;
    for (size_t i = 0; i < clusterPhase.size(); ++i)
        clusterPhase[i] = (float) i * 0.17f;
    filter.reset();
    noiseLp.z = noiseHp.z = sawFilter.z = 0;
    updateBlock (p);
}

void Hit::release() noexcept
{
    gated = false;
}

void Hit::fadeOut (float ms) noexcept
{
    if (! active)
        return;
    fading = true;
    fadeCoeff = t60Coefficient (ms, sampleRate);
}

void Hit::updateBlock (const VoiceParams& p)
{
    const float res = p[filterResonance] * 0.01f;
    filter.set (p[filterCutoff], res, (int) p[filterType], sampleRate);
    noiseGain = dbToGain (p[noiseLevel]);

    colour = p[VoiceParam::noiseColor] * 0.01f;
    if (colour < 0.5f)
    {
        noiseLp.setCutoff (300.0f * std::exp2 (colour * 2.0f * 6.0f), sampleRate);
        noiseHp.setCutoff (5.0f, sampleRate);
    }
    else
    {
        noiseLp.setCutoff (sampleRate * 0.45f, sampleRate);
        noiseHp.setCutoff (20.0f * std::exp2 ((colour - 0.5f) * 2.0f * 8.6f), sampleRate);
    }
}

float Hit::polyBlep (float t, float dt) const noexcept
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

float Hit::oscillator (float freq, float tone, int wave) noexcept
{
    const float dt = std::min (0.49f, freq * invSampleRate);
    float y = 0;
    switch (wave)
    {
        case 0:   // SINE: Tone folds the wave for extra harmonics
        {
            const float s = std::sin (twoPi * phase);
            y = tone < 0.005f ? s : std::sin (s * (0.5f * pi + tone * 2.5f * pi));
            break;
        }
        case 1:   // TRI, folded the same way
        {
            const float t = 4.0f * std::abs (phase - 0.5f) - 1.0f;
            y = tone < 0.005f ? t : std::sin (t * (0.5f * pi + tone * 2.5f * pi));
            break;
        }
        case 2:   // SAW: Tone opens a lowpass from dark to bright
        {
            const float s = 2.0f * phase - 1.0f - polyBlep (phase, dt);
            sawFilter.setCutoff (freq * (1.5f + 40.0f * tone * tone), sampleRate);
            y = sawFilter.lowpass (s);
            break;
        }
        case 3:   // NOISE: Tone blends white noise into a six-square metallic cluster
        {
            float cluster = 0;
            const float scale = freq / 400.0f;
            for (size_t i = 0; i < clusterPhase.size(); ++i)
            {
                const float cdt = std::min (0.49f, 400.0f * clusterRatio[i] * scale * invSampleRate);
                float& cp = clusterPhase[i];
                float sq = cp < 0.5f ? 1.0f : -1.0f;
                sq += polyBlep (cp, cdt);
                float shifted = cp + 0.5f;
                if (shifted >= 1.0f)
                    shifted -= 1.0f;
                sq -= polyBlep (shifted, cdt);
                cluster += sq;
                cp += cdt;
                if (cp >= 1.0f)
                    cp -= 1.0f;
            }
            y = rng.bipolar() * (1.0f - tone) + cluster * (tone / 4.0f);
            break;
        }
        default:   // FM: Tone sets the modulation index, which follows the oscillator envelope
        {
            const float index = tone * 8.0f * oscEnv;
            y = std::sin (twoPi * phase + index * std::sin (twoPi * modPhase));
            modPhase += std::min (0.49f, freq * fmRatio[(size_t) voice] * invSampleRate);
            if (modPhase >= 1.0f)
                modPhase -= 1.0f;
            break;
        }
    }
    phase += dt;
    if (phase >= 1.0f)
        phase -= 1.0f;
    return y;
}

float Hit::process (const VoiceParams& p, float wheel) noexcept
{
    if (! active)
        return 0.0f;

    if (gateRemaining > 0 && --gateRemaining == 0)
        gated = false;

    // Pitch envelope: linear rise, exponential fall back to Tune.
    if (pitchRising)
    {
        pitchEnv += pitchAttackStep;
        if (pitchEnv >= 1.0f)
        {
            pitchEnv = 1.0f;
            pitchRising = false;
        }
    }
    else
        pitchEnv *= pitchCoeff;

    const float semis = p[oscTune] + p[pitchBend] * wheel + pitchEnv * p[pitchAmount] * 0.48f;
    const float freq = basePitch[(size_t) voice] * std::exp2 (semis / 12.0f);

    const float osc = oscillator (freq, p[oscTone] * 0.01f, (int) p[oscWave]) * oscEnv * 0.8f;
    oscEnv *= oscCoeff;

    float noise = rng.bipolar();
    noise = noiseHp.highpass (noiseLp.lowpass (noise));
    float x = osc + noise * noiseGain * 0.7f;

    // Drive: bounded saturation with peak compensation.
    const float drive = p[driveAmount] * 0.01f;
    if (drive > 0.001f)
    {
        const float g = 1.0f + drive * 9.0f;
        x = x + std::min (1.0f, drive * 4.0f) * (std::tanh (g * x) / std::tanh (g) - x);
    }
    x = filter.process (x);

    // Amp envelope: Attack -> Decay -> Sustain (held while gated) -> Release with Decay time.
    switch (ampStage)
    {
        case 0:
            ampLevel += ampAttackStep;
            if (ampLevel >= 1.0f)
            {
                ampLevel = 1.0f;
                ampStage = 1;
            }
            break;
        case 1:
            if (sustain > 0.0f && gated)
                ampLevel = sustain + (ampLevel - sustain) * ampCoeff;
            else if (sustain > 0.0f)
                ampStage = 2;
            else
                ampLevel *= ampCoeff;
            break;
        default:
            ampLevel *= ampCoeff;
            break;
    }

    float env = ampLevel;
    if (voice == clapVoice)
    {
        // Clap: three fast bursts in the first 33 ms before the body.
        const float t = (float) age * invSampleRate * 1000.0f;
        if (t < 33.0f)
            env *= 0.35f + 0.9f * std::exp (-std::fmod (t, 11.0f) / 2.5f);
    }
    ++age;

    if (fading)
    {
        fadeGain *= fadeCoeff;
        if (fadeGain < 1.0e-4f)
            active = false;
    }

    if (ampStage > 0 && ampLevel < 1.0e-4f && (sustain <= 0.0f || ! gated))
        active = false;

    return x * env * velocityGain * fadeGain;
}

// ---------------------------------------------------------------------------------------------
void Instrument::prepare (float sr, int voiceIndex)
{
    sampleRate = sr;
    voice = voiceIndex;
    for (auto& h : hits)
        h.prepare (sr, voiceIndex);
    comb.prepare (sr);
    resonator.prepare (sr);
    reset();
}

void Instrument::reset()
{
    for (auto& h : hits)
        h.prepare (sampleRate, voice);
    comb.reset();
    resonator.reset();
    chokeGain = chokeTarget = 1;
    chokeStep = 0;
    idle = true;
    quietSamples = 0;
}

void Instrument::trigger (const VoiceParams& p, int velocity, int gateSamples)
{
    if (idle)
    {
        // Nothing is sounding, so jump straight to the current level/pan/send.
        beginBlock (p);
        smoothL = gainL;
        smoothR = gainR;
        smoothSend = send;
    }
    hits[(size_t) current].fadeOut (4.0f);
    current ^= 1;
    hits[(size_t) current].start (p, (float) velocity / 127.0f, gateSamples,
                                  0x1234567u * (std::uint32_t) (voice + 1) + 7919u * hitCounter++);
    chokeGain = chokeTarget = 1;
    idle = false;
    quietSamples = 0;
}

void Instrument::noteOff()
{
    hits[(size_t) current].release();
}

void Instrument::choke()
{
    for (auto& h : hits)
        h.fadeOut (6.0f);
    chokeTarget = 0;
    chokeStep = 1.0f / std::max (1.0f, 0.008f * sampleRate);
}

void Instrument::beginBlock (const VoiceParams& p)
{
    for (auto& h : hits)
        if (h.isActive())
            h.updateBlock (p);

    const bool combOn = p[combEnabled] > 0.5f, resOn = p[resEnabled] > 0.5f;
    if (combOn && ! combWasOn)
        comb.reset();
    if (resOn && ! resWasOn)
        resonator.reset();
    combWasOn = combOn;
    resWasOn = resOn;
    if (combOn)
        comb.set (p[combTune], p[combFeedback] * 0.01f, p[combDamping] * 0.01f, p[combMix] * 0.01f);
    if (resOn)
        resonator.set ((int) p[resModel], p[resTune], p[resDecay], p[resDamping] * 0.01f, p[resMix] * 0.01f);

    const float gain = dbToGain (p[level]);
    const float angle = (p[pan] * 0.01f + 1.0f) * 0.25f * pi;
    gainL = gain * std::cos (angle) * 1.41421356f;
    gainR = gain * std::sin (angle) * 1.41421356f;
    send = dbToGain (p[sendFx]);
}

void Instrument::process (const VoiceParams& p, float wheel, float& outL, float& outR, float& sendL, float& sendR,
                          bool audible) noexcept
{
    // Smooth level/pan/send changes over ~5 ms.
    constexpr float smooth = 0.995f;
    smoothL = gainL + smooth * (smoothL - gainL);
    smoothR = gainR + smooth * (smoothR - gainR);
    smoothSend = send + smooth * (smoothSend - send);

    if (idle)
        return;

    // 0.6 = about 4.4 dB of headroom so several simultaneous hits stay below full scale.
    float x = (hits[0].process (p, wheel) + hits[1].process (p, wheel)) * 0.6f;
    if (combWasOn)
        x = comb.process (x);
    if (resWasOn)
        x = resonator.process (x);

    if (chokeGain != chokeTarget)
    {
        chokeGain = std::max (chokeTarget, chokeGain - chokeStep);
        if (chokeGain <= 0.0f)
        {
            comb.reset();
            resonator.reset();
        }
    }
    x *= chokeGain;

    if (! hits[0].isActive() && ! hits[1].isActive())
    {
        quietSamples = std::abs (x) < 1.0e-5f ? quietSamples + 1 : 0;
        if (quietSamples > (int) (sampleRate * 0.05f) || chokeGain <= 0.0f)
        {
            idle = true;
            comb.reset();
            resonator.reset();
        }
    }

    if (! audible)
        return;
    const float l = x * smoothL, r = x * smoothR;
    outL += l;
    outR += r;
    sendL += l * smoothSend;
    sendR += r * smoothSend;
}
} // namespace percusyn::dsp
