#include "dsp/Engine.h"

namespace percusyn::dsp
{
namespace
{
constexpr int closedHat = 0, openHat = 1;
}

void Engine::prepare (double sr)
{
    sampleRate = sr;
    for (int v = 0; v < numVoices; ++v)
        instruments[(size_t) v].prepare ((float) sr, v);
    delay.prepare ((float) sr);
    reverb.prepare ((float) sr);
    for (int v = 0; v < numVoices; ++v)
        for (int i = 0; i < numVoiceParams; ++i)
            params[(size_t) v][i] = voiceDefault (v, i);
    for (int i = 0; i < numGlobalParams; ++i)
        global[i] = globalParamDefs[(size_t) i].def;
}

void Engine::reset()
{
    for (auto& i : instruments)
        i.reset();
    delay.reset();
    reverb.reset();
}

void Engine::setParameters (const std::array<VoiceParams, numVoices>& voices, const GlobalParams& globals)
{
    params = voices;
    global = globals;

    bool anySolo = false;
    for (const auto& p : params)
        anySolo = anySolo || p[solo] > 0.5f;
    for (int v = 0; v < numVoices; ++v)
    {
        const auto& p = params[(size_t) v];
        audible[(size_t) v] = p[mute] < 0.5f && (! anySolo || p[solo] > 0.5f);
        instruments[(size_t) v].beginBlock (p);
    }

    delay.set (global[delayTime], global[delayFeedback] * 0.01f);
    reverb.set (global[reverbSize] * 0.01f, global[reverbDecay]);
    delayReturn = global[GlobalParam::delayMix] * 0.01f;
    reverbReturn = global[GlobalParam::reverbMix] * 0.01f * 0.8f;
    master = dbToGain (global[masterLevel]);
}

void Engine::noteOn (int voice, int velocity, int gateSamples)
{
    if (voice < 0 || voice >= numVoices)
        return;
    if (voice == closedHat)
        instruments[(size_t) openHat].choke();
    instruments[(size_t) voice].trigger (params[(size_t) voice], velocity, gateSamples);
}

void Engine::noteOff (int voice)
{
    if (voice >= 0 && voice < numVoices)
        instruments[(size_t) voice].noteOff();
}

void Engine::render (float* mainL, float* mainR, const std::array<float*, numVoices>& auxL,
                     const std::array<float*, numVoices>& auxR, int start, int n) noexcept
{
    for (int i = start; i < start + n; ++i)
    {
        float l = 0, r = 0, sendL = 0, sendR = 0;
        for (int v = 0; v < numVoices; ++v)
        {
            auto& inst = instruments[(size_t) v];
            if (inst.isIdle())
                continue;
            if (auxL[(size_t) v] != nullptr)
            {
                float vl = 0, vr = 0;
                inst.process (params[(size_t) v], wheel, vl, vr, sendL, sendR, audible[(size_t) v]);
                auxL[(size_t) v][i] += vl;
                auxR[(size_t) v][i] += vr;
            }
            else
                inst.process (params[(size_t) v], wheel, l, r, sendL, sendR, audible[(size_t) v]);
        }

        float dl, dr, rl, rr;
        delay.process (sendL, sendR, dl, dr);
        reverb.process (sendL, sendR, rl, rr);
        l += dl * delayReturn + rl * reverbReturn;
        r += dr * delayReturn + rr * reverbReturn;

        smoothMaster = master + 0.995f * (smoothMaster - master);
        mainL[i] += l * smoothMaster;
        mainR[i] += r * smoothMaster;
    }
}
} // namespace percusyn::dsp
