#pragma once

#include "envelope_stage.h"

typedef struct
{
    EnvelopeStage stages[4];
    int stage, releaseStage;
    int endReached;
    int cycleAttackDecay;
    float sustain, releaseLevel;
    float envelope;
} AdsrEnvelope;

void AdsrEnvelope_triggerStage(AdsrEnvelope *env, int stage);

void AdsrEnvelope_setAttack(AdsrEnvelope *env, int samples);
void AdsrEnvelope_setDecay(AdsrEnvelope *env, int samples);
void AdsrEnvelope_setSustain(AdsrEnvelope *env, float level);
float AdsrEnvelope_getSustain(AdsrEnvelope *env);
void AdsrEnvelope_setRelease(AdsrEnvelope *env, int samples);
void AdsrEnvelope_setCycleOnOff(AdsrEnvelope *env, int on);

void AdsrEnvelope_calculateNext(AdsrEnvelope *env);
void AdsrEnvelope_trigger(AdsrEnvelope *env);
void AdsrEnvelope_release(AdsrEnvelope *env);

int AdsrEnvelope_getStage(AdsrEnvelope *env);
int AdsrEnvelope_getReleaseStage(AdsrEnvelope *env);
float AdsrEnvelope_getRatio(AdsrEnvelope *env, int stage);
float AdsrEnvelope_getEnvelope(AdsrEnvelope *env);
int AdsrEnvelope_ended(AdsrEnvelope *env);

void init_AdsrEnvelope(AdsrEnvelope *env);