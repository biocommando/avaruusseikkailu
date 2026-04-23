#include "basic_oscillator.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define wtSize 4537

static float sine_table[256];

static void init_sine_table()
{
    static int sine_table_initialized = 0;
    if (!sine_table_initialized)
    {
        sine_table_initialized = 1;
        for (int i = 0; i < 256; i++)
            sine_table[i] = sin(6.283185307179586476925286766559 / 256 * i);
    }
}

void init_BasicOscillator(BasicOscillator* bo, int sampleRate)
{
    memset(bo, 0, sizeof(BasicOscillator));
    bo->hzToF = 1.0f / (float)sampleRate;
    init_sine_table();
}

void BasicOscillator_setWaveTableParams(BasicOscillator* bo, float pos, float window)
{
    bo->wtWindow = 2 + window * (wtSize - 2);
    bo->wtPos = pos * (wtSize - bo->wtWindow - 1);
}

void BasicOscillator_setWavetable(BasicOscillator* this, float *wt)
{
    this->wt = wt;
}


void BasicOscillator_setSamplerate(BasicOscillator* bo, int rate)
{
    bo->hzToF = 1.0f / (float)rate;
}

void BasicOscillator_randomizePhase(BasicOscillator* bo, float rndAmount)
{
    bo->phase = (float)(rand() % 100000) * 0.00001f;
    bo->phase *= rndAmount;
}

inline static float sin1(float phase)
{
    return sine_table[(int)(phase * 256)];
}
inline static float tri1(float phase)
{
    if (phase < 0.5)
        return 4 * phase - 1;
    else
        return -4 * phase + 3;
}
inline static float saw1(float phase)
{
    return 2 * phase - 1;
}
inline static float sqr1(float phase)
{
    return phase < 0.5 ? 1.0f : -1.0f;
}

inline static float wt1(float *wt, float phase, int pos, int window, float *dcFilterState)
{
    const float value = wt[(int)(pos + window * phase)];

    // Remove DC offset
    dcFilterState[0] = 0.9984 * (value + dcFilterState[0] - dcFilterState[1]);
    dcFilterState[1] = value;

    return dcFilterState[0];
}

void BasicOscillator_calculateNext(BasicOscillator* bo)
{
    bo->phase = bo->phase + bo->frequency;
    if (bo->phase >= 1.0)
        bo->phase = bo->phase - 1.0f;
}
void BasicOscillator_setFrequency(BasicOscillator* bo, float f_Hz)
{
    bo->frequency = f_Hz * bo->hzToF;
}
float BasicOscillator_getValue(BasicOscillator* bo, enum OscType oscType)
{
    switch (oscType)
    {
    case OSC_SINE:
        return sin1(bo->phase);
    case OSC_TRIANGLE:
        return tri1(bo->phase);
    case OSC_SAW:
        return saw1(bo->phase);
    case OSC_SQUARE:
        return sqr1(bo->phase);
    case OSC_WT:
        return wt1(bo->wt, bo->phase, bo->wtPos, bo->wtWindow, bo->dcFilterState);
    default:
        return 0;
    }
}