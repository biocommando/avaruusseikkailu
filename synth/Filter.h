#pragma once

class Filter
{
    float factor = 0;
    float sampleRate;
    float state[2] = {0, 0};

public:
    Filter(float sampleRate) : sampleRate(sampleRate)
    {
    }

    void updateLowpass(float cutFreqHz)
    {
        factor = 1.0f / (1.0f / (2.0f * 3.14159265f * 1.0f / sampleRate * cutFreqHz) + 1.0f);
    }

    void updateHighpass(float cutFreqHz)
    {
        factor = 1.0f / (2.0f * 3.14159265f * 1.0f / sampleRate * cutFreqHz + 1.0f);
    }

    float processHighpass(float input)
    {
        state[0] = factor * (input + state[0] - state[1]);
        state[1] = input;

        return state[0];
    }

    float processLowpass(float input)
    {
        state[0] = factor * input + (1.0f - factor) * state[1];
        state[1] = state[0];

        return state[0];
    }
};

class LowpassFilter : Filter
{
public:
    LowpassFilter(float sampleRate) : Filter(sampleRate) {}

    inline float process(float input) { return processLowpass(input); }

    inline void update(float hz) { updateLowpass(hz); }
};

class DcFilter : Filter
{
public:
    DcFilter(float sampleRate) : Filter(sampleRate)
    {
        updateHighpass(10);
    }

    inline float process(float input) { return processHighpass(input); }
};