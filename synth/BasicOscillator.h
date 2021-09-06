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
    float dcFilterState[2] = {0,0};
public:
    BasicOscillator(int sampleRate = 44100);
    ~BasicOscillator();
    void calculateNext();
    void setWavetable(float *wt);
    float getValue(enum OscType oscType);
    void setFrequency(float f_Hz);
    void setWaveTableParams(float pos, float window);
    void setSamplerate(int rate);
    void randomizePhase(float rndAmount = 1);
};
