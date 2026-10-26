#include "dsp/Fx.h"

#include <algorithm>
#include <cmath>

namespace percusyn::dsp
{
namespace
{
int nextPowerOfTwo (int n)
{
    int size = 1;
    while (size < n)
        size <<= 1;
    return size;
}

// Prime-ish base lengths in milliseconds for the reverb lines and diffusers.
constexpr std::array<float, Reverb::numLines> lineMs { 29.7f, 37.1f, 41.1f, 43.7f, 53.3f, 59.9f, 67.1f, 73.3f };
constexpr std::array<float, 4> diffuserMs { 4.77f, 3.59f, 2.73f, 1.73f };
} // namespace

void Delay::prepare (float sr)
{
    sampleRate = sr;
    const int size = nextPowerOfTwo ((int) (sr * 1.05f) + 4);
    left.assign ((size_t) size, 0.0f);
    right.assign ((size_t) size, 0.0f);
    mask = size - 1;
    reset();
}

void Delay::reset()
{
    std::fill (left.begin(), left.end(), 0.0f);
    std::fill (right.begin(), right.end(), 0.0f);
    write = 0;
    lpL = lpR = 0;
    time = targetTime;
}

void Delay::set (float timeMs, float feedback01)
{
    targetTime = std::clamp (timeMs * 0.001f * sampleRate, 1.0f, (float) mask - 2.0f);
    feedback = std::clamp (feedback01, 0.0f, 0.95f);
}

void Delay::process (float inL, float inR, float& outL, float& outR) noexcept
{
    time += 0.0005f * (targetTime - time);   // glide instead of clicks when Time moves
    const float readPos = (float) write - time;
    const int i = (int) std::floor (readPos);
    const float f = readPos - (float) i;
    auto read = [&] (const std::vector<float>& b)
    {
        const float a = b[(size_t) (i & mask)], c = b[(size_t) ((i + 1) & mask)];
        return a + f * (c - a);
    };
    const float dl = read (left), dr = read (right);
    lpL = dl + 0.35f * (lpL - dl);
    lpR = dr + 0.35f * (lpR - dr);
    float wl = inL + feedback * lpL, wr = inR + feedback * lpR;
    if (std::abs (wl) < 1.0e-15f) wl = 0;
    if (std::abs (wr) < 1.0e-15f) wr = 0;
    left[(size_t) write] = std::tanh (wl);
    right[(size_t) write] = std::tanh (wr);
    write = (write + 1) & mask;
    outL = dl;
    outR = dr;
}

// ---------------------------------------------------------------------------------------------
float Reverb::Allpass::process (float x) noexcept
{
    const float delayed = buffer[(size_t) index];
    const float y = delayed - 0.6f * x;
    buffer[(size_t) index] = x + 0.6f * y;
    index = (index + 1) % (int) buffer.size();
    return y;
}

void Reverb::prepare (float sr)
{
    sampleRate = sr;
    for (size_t k = 0; k < lines.size(); ++k)
    {
        auto& line = lines[k];
        const int bufferSize = nextPowerOfTwo ((int) (lineMs[k] * 1.6f * 0.001f * sr) + 4);
        line.buffer.assign ((size_t) bufferSize, 0.0f);
        line.mask = bufferSize - 1;
    }
    for (size_t k = 0; k < diffusers.size(); ++k)
        diffusers[k].buffer.assign ((size_t) std::max (1, (int) (diffuserMs[k] * 0.001f * sr)), 0.0f);
    size = decay = -1;
    reset();
}

void Reverb::reset()
{
    for (auto& line : lines)
    {
        std::fill (line.buffer.begin(), line.buffer.end(), 0.0f);
        line.write = 0;
        line.lp = 0;
    }
    for (auto& d : diffusers)
    {
        std::fill (d.buffer.begin(), d.buffer.end(), 0.0f);
        d.index = 0;
    }
}

void Reverb::set (float size01, float decayMs)
{
    if (size01 == size && decayMs == decay)
        return;
    size = size01;
    decay = decayMs;
    const float scale = 0.3f + 1.3f * std::clamp (size01, 0.0f, 1.0f);
    for (size_t k = 0; k < lines.size(); ++k)
    {
        auto& line = lines[k];
        line.length = std::clamp ((int) (lineMs[k] * scale * 0.001f * sampleRate), 8, line.mask - 1);
        // Per-line gain for the requested T60.
        line.gain = std::pow (10.0f, -3.0f * (float) line.length / (std::max (100.0f, decayMs) * 0.001f * sampleRate));
    }
}

void Reverb::process (float inL, float inR, float& outL, float& outR) noexcept
{
    float in = (inL + inR) * 0.5f;
    for (auto& d : diffusers)
        in = d.process (in);

    std::array<float, numLines> x {};
    for (size_t k = 0; k < lines.size(); ++k)
    {
        auto& line = lines[k];
        const float y = line.buffer[(size_t) ((line.write - line.length) & line.mask)];
        line.lp = y + 0.25f * (line.lp - y);   // gentle high-frequency loss
        x[k] = line.lp * line.gain;
    }

    // Normalised Hadamard mixing (fast Walsh transform).
    for (int len = 1; len < numLines; len <<= 1)
        for (int i = 0; i < numLines; i += len << 1)
            for (int j = i; j < i + len; ++j)
            {
                const float a = x[(size_t) j], b = x[(size_t) (j + len)];
                x[(size_t) j] = a + b;
                x[(size_t) (j + len)] = a - b;
            }
    constexpr float norm = 0.35355339f;   // 1 / sqrt(8)

    const float side = (inL - inR) * 0.3f;
    for (size_t k = 0; k < lines.size(); ++k)
    {
        auto& line = lines[k];
        float w = x[k] * norm + (k % 2 == 0 ? in + side : in - side) * 0.5f;
        if (std::abs (w) < 1.0e-15f)
            w = 0;
        line.buffer[(size_t) line.write] = w;
        line.write = (line.write + 1) & line.mask;
    }

    outL = (lines[0].lp + lines[2].lp + lines[4].lp + lines[6].lp) * 0.5f;
    outR = (lines[1].lp + lines[3].lp + lines[5].lp + lines[7].lp) * 0.5f;
}
} // namespace percusyn::dsp
