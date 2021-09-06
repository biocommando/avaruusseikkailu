// Based on
// https://github.com/ddiakopoulos/MoogLadders/blob/master/src/MicrotrackerModel.h
// which is based on implementation by Magnus Jonsson
// https://github.com/magnusjonsson/microtracker (unlicense)

#pragma once

#include "FilterBase.h"

class MicrotrackerMoog :  public FilterBase
{

public:
    MicrotrackerMoog(float sampleRate) : sampleRate(sampleRate)
    {
        p0 = p1 = p2 = p3 = p32 = p33 = p34 = 0.0;
        cutmod = 0;
        setCutoff(1.0f);
        setResonance(0.10f);
    }

    virtual ~MicrotrackerMoog() {}

    float calculate(float x)
    {
        const float k = 4 * resonance;
        // Coefficients optimized using differential evolution
        // to make feedback gain 4.0 correspond closely to the
        // border of instability, for all values of omega.
        const float out = p3 * 0.360891 + p32 * 0.417290 + p33 * 0.177896 + p34 * 0.0439725;

        p34 = p33;
        p33 = p32;
        p32 = p3;

        p0 += (fast_tanh(x - k * out) - fast_tanh(p0)) * coCalc;
        p1 += (fast_tanh(p0) - fast_tanh(p1)) * coCalc;
        p2 += (fast_tanh(p1) - fast_tanh(p2)) * coCalc;
        p3 += (fast_tanh(p2) - fast_tanh(p3)) * coCalc;

        return out;
    }

    void setResonance(float r)
    {
        resonance = r;
    }

    void setCutoff(float c)
    {
        cutoff = c;
        calculateCutoff();
    }

    void setModulation(float m)
    {
        if (cutmod != m)
        {
            cutmod = m;
            calculateCutoff();
        }
    }

    void reset()
    {
        p0 = p1 = p2 = p3 = p32 = p33 = p34 = 0.0;
    }

    void setSamplerate(int sr)
    {
        sampleRate = sr;
        calculateCutoff();
    }

private:
    inline static float fast_tanh(float x)
    {
        const auto x2 = x * x;
        return x * (27.0 + x2) / (27.0 + 9.0 * x2);
    }

    void calculateCutoff()
    {
        coCalc = cutoff + cutmod;
        coCalc = coCalc * 44100 / sampleRate;// 6.28318530717 * 1000 / sampleRate;
        coCalc = coCalc > 1 ? 1 : (coCalc < 0 ? 0 : coCalc);
    }

    float p0;
    float p1;
    float p2;
    float p3;
    float p32;
    float p33;
    float p34;
    float cutoff, cutmod, coCalc;
    float resonance;
    float sampleRate;
};