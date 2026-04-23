#pragma once
// Based on
// https://github.com/ddiakopoulos/MoogLadders/blob/master/src/MicrotrackerModel.h
// which is based on implementation by Magnus Jonsson
// https://github.com/magnusjonsson/microtracker (unlicense)

typedef struct
{
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
} MicrotrackerMoog;

void init_MicrotrackerMoog(MicrotrackerMoog *mm, float sampleRate);

float MicrotrackerMoog_calculate(MicrotrackerMoog *mm, float x);

void MicrotrackerMoog_setResonance(MicrotrackerMoog *mm, float r);

void MicrotrackerMoog_setCutoff(MicrotrackerMoog *mm, float c);

void MicrotrackerMoog_setModulation(MicrotrackerMoog *mm, float m);

void MicrotrackerMoog_reset(MicrotrackerMoog *mm);

void MicrotrackerMoog_setSamplerate(MicrotrackerMoog *mm, int sr);