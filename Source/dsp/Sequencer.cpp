#include "dsp/Sequencer.h"

#include <algorithm>
#include <cmath>

namespace percusyn::dsp
{
void Sequencer::prepare (double sr)
{
    sampleRate = sr;
}

void Sequencer::startInternal() noexcept
{
    internalPpq = 0;
    internalRunning = true;
    lastK = -(1LL << 62);
    lastEnd = -1.0e9;
}

void Sequencer::stopInternal() noexcept
{
    internalRunning = false;
    internalPpq = 0;
    step = -1;
}

float Sequencer::random01() noexcept
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return (float) (rng >> 8) * (1.0f / 16777216.0f);
}

int Sequencer::process (int numSamples, bool useHost, double hostPpq, double hostBpm, double internalBpm,
                        const PatternData& pattern, float swingPercent, int length, Events& out) noexcept
{
    if (! useHost && ! internalRunning)
        return 0;

    const double bpm = std::clamp (useHost ? hostBpm : internalBpm, 20.0, 999.0);
    const double ppqPerSample = bpm / (60.0 * sampleRate);
    const double start = useHost ? hostPpq : internalPpq;
    const double end = start + ppqPerSample * numSamples;
    if (! useHost)
        internalPpq = end;
    if (start < lastEnd - 0.01 || start > lastEnd + 0.25)
        lastK = -(1LL << 62);   // loop, seek or restart
    lastEnd = end;

    length = std::clamp (length, 1, numSteps);
    const double swing = std::clamp (swingPercent, 0.0f, 75.0f) / 100.0;
    int count = 0;

    for (long long k = (long long) std::floor (start * 4.0) - 1; k <= (long long) std::floor (end * 4.0); ++k)
    {
        const double t = ((double) k + ((k & 1) != 0 ? swing : 0.0)) * 0.25;
        if (t < start || t >= end || k <= lastK)
            continue;
        lastK = k;

        const long long cycle = (k >= 0 ? k : k - length + 1) / length;
        const int s = (int) (((k % length) + length) % length);
        step = s;
        const int offset = std::clamp ((int) std::floor ((t - start) / ppqPerSample + 0.5), 0, numSamples - 1);

        for (int v = 0; v < numVoices; ++v)
        {
            const auto& st = pattern[(size_t) v][(size_t) s];
            if (! st.active || ! conditionAllows (st.condition, cycle))
                continue;
            if (st.probability < 100.0f && random01() * 100.0f >= st.probability)
                continue;
            if (count < maxEvents)
                out[(size_t) count++] = { offset, v, std::clamp (st.velocity, 1, 127) };
        }
    }
    return count;
}
} // namespace percusyn::dsp
