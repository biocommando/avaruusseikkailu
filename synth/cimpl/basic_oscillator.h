#pragma once

enum OscType
{
    OSC_SINE,
    OSC_TRIANGLE,
    OSC_SAW,
    OSC_SQUARE,
    OSC_WT
};

typedef struct
{
    float phase, frequency, hzToF;
    int wtPos, wtWindow;
    float *wt;
    float dcFilterState[2];    
} BasicOscillator;

void init_BasicOscillator(BasicOscillator* bo, int sampleRate);
void BasicOscillator_calculateNext(BasicOscillator* bo);
void BasicOscillator_setWavetable(BasicOscillator* bo, float *wt);
float BasicOscillator_getValue(BasicOscillator* bo, enum OscType oscType);
void BasicOscillator_setFrequency(BasicOscillator* bo, float f_Hz);
void BasicOscillator_setWaveTableParams(BasicOscillator* bo, float pos, float window);
void BasicOscillator_setSamplerate(BasicOscillator* bo, int rate);
void BasicOscillator_randomizePhase(BasicOscillator* bo, float rndAmount);