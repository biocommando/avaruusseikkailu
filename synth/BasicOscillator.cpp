#include "BasicOscillator.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
constexpr int wtSize = 4537;

BasicOscillator::BasicOscillator(int sampleRate) : phase(0), hzToF(1.0f / (float)sampleRate),
                                                   frequency(0), wtPos(0), wtWindow(0)
{
}

void BasicOscillator::setWaveTableParams(float pos, float window)
{
    wtWindow = 2 + window * (wtSize - 2);
    wtPos = pos * (wtSize - wtWindow - 1);
}

void BasicOscillator::setWavetable(float *wt)
{
    this->wt = wt;
}

BasicOscillator::~BasicOscillator()
{
}

void BasicOscillator::setSamplerate(int rate)
{
    hzToF = 1.0f / (float)rate;
}

void BasicOscillator::randomizePhase(float rndAmount)
{
    phase = (float)(rand() % 100000) * 0.00001f;
    phase *= rndAmount;
}

inline float sin1(float phase)
{
    const float phase_rad = phase * 6.283185307179586476925286766559f;
    return sin(phase_rad);
}
inline float tri1(float phase)
{
    if (phase < 0.5)
        return 4 * phase - 1;
    else
        return -4 * phase + 3;
}
inline float saw1(float phase)
{
    return 2 * phase - 1;
}
inline float sqr1(float phase)
{
    return phase < 0.5 ? 1.0f : -1.0f;
}

inline float wt1(float *wt, float phase, int pos, int window, float dcFilterState[2])
{
    const float value = wt[(int)(pos + window * phase)];

    // Remove DC offset
    dcFilterState[0] = 0.9984 * (value + dcFilterState[0] - dcFilterState[1]);
    dcFilterState[1] = value;

    return dcFilterState[0];
}

void BasicOscillator::calculateNext()
{
    phase = phase + frequency;
    if (phase >= 1.0)
        phase = phase - 1.0f;
}
void BasicOscillator::setFrequency(float f_Hz)
{
    frequency = f_Hz * hzToF;
}
float BasicOscillator::getValue(enum OscType oscType)
{
    switch (oscType)
    {
    case OSC_SINE:
        return sin1(phase);
    case OSC_TRIANGLE:
        return tri1(phase);
    case OSC_SAW:
        return saw1(phase);
    case OSC_SQUARE:
        return sqr1(phase);
    case OSC_WT:
        return wt1(wt, phase, wtPos, wtWindow, dcFilterState);
    default:
        return 0;
    }
}