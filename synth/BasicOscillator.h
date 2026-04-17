#pragma once
enum OscType
{
    OSC_SINE,
    OSC_TRIANGLE,
    OSC_SAW,
    OSC_SQUARE,
    OSC_WT
};

class BasicOscillator
{
private:
    float phase, frequency, hzToF;
    int wtPos, wtWindow;
    float *wt = nullptr;
    int wtSize = 0;
    float dcFilterState[2] = {0,0};
public:
    bool wt_oneshot = false;
    BasicOscillator(int sampleRate = 44100);
    ~BasicOscillator();
    void calculateNext();
    void setWavetable(float *wt, int size);
    float getValue(enum OscType oscType);
    float getValue(enum OscType oscType, float fmAmount);
    void setFrequency(float f_Hz);
    void setWaveTableParams(float pos, float window);
    void setSamplerate(int rate);
    void randomizePhase(float rndAmount = 1);
};
