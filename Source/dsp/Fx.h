#pragma once

// Global send effects: stereo delay and an eight-line feedback delay network reverb.

#include <array>
#include <vector>

namespace percusyn::dsp
{
class Delay
{
public:
    void prepare (float sampleRate);
    void reset();
    void set (float timeMs, float feedback01);
    void process (float inL, float inR, float& outL, float& outR) noexcept;

private:
    std::vector<float> left, right;
    int mask = 0, write = 0;
    float sampleRate = 48000, time = 12000, targetTime = 12000, feedback = 0;
    float lpL = 0, lpR = 0;
};

class Reverb
{
public:
    static constexpr int numLines = 8;
    void prepare (float sampleRate);
    void reset();
    void set (float size01, float decayMs);
    void process (float inL, float inR, float& outL, float& outR) noexcept;

private:
    struct Line
    {
        std::vector<float> buffer;
        int mask = 0, write = 0, length = 1;
        float gain = 0, lp = 0;
    };
    std::array<Line, numLines> lines;
    struct Allpass
    {
        std::vector<float> buffer;
        int index = 0;
        float process (float x) noexcept;
    };
    std::array<Allpass, 4> diffusers;
    float sampleRate = 48000, size = -1, decay = -1;
};
} // namespace percusyn::dsp
