#include "AdsrEnvelope.h"
#include <iostream>
AdsrEnvelope::AdsrEnvelope() : cycleAttackDecay(false),
                               endReached(true), sustain(0), stage(0),
                               stages({EnvelopeStage(true), EnvelopeStage(true), EnvelopeStage(false), EnvelopeStage(true)})
{
}

AdsrEnvelope::~AdsrEnvelope()
{
}

void AdsrEnvelope::setAttack(int samples)
{
    stages[0].setLength(samples);
}

void AdsrEnvelope::setDecay(int samples)
{
    stages[1].setLength(samples);
}

void AdsrEnvelope::setSustain(float level)
{
    sustain = level;
}

void AdsrEnvelope::setRelease(int samples)
{
    if (samples < 40)
        samples = 40; // to smoothen the clicking sound when using minimal release
    stages[3].setLength(samples);
}

void AdsrEnvelope::calculateNext()
{
    if (endReached)
    {
        return;
    }
    EnvelopeStage *current = &stages[stage];
    if (current->hasNext())
    {
        current->calcuateNext();
        float ratio = current->getRatio();
        switch (stage)
        {
        case 0:
            envelope = ratio;
            break;
        case 1:
            envelope = 1 - (1 - sustain) * ratio;
            break;
        case 2:
            envelope = sustain;
            break;
        case 3:
            envelope = releaseLevel * (1 - ratio);
            break;
        default:
            break;
        }
    }
    else if (stage < 3)
    {
        // If cycling A-D-S-A-D-S... at the end of decay stage, trigger attack again
        if (cycleAttackDecay && stage == 1)
            triggerStage(0);
        else
            triggerStage(stage + 1);
        calculateNext();
    }
    else
    {
        // release ended
        envelope = 0;
        endReached = true;
    }
}

bool AdsrEnvelope::ended()
{
    return endReached;
}

void AdsrEnvelope::trigger()
{
    endReached = false;
    triggerStage(0);
}
void AdsrEnvelope::triggerStage(int stage)
{
    this->stage = stage;
    stages[stage].reset();
}

void AdsrEnvelope::release()
{
    if (stage < 3)
    {
        releaseLevel = envelope;
        releaseStage = stage;
        triggerStage(3);
    }
}

int AdsrEnvelope::getStage()
{
    return stage;
}

float AdsrEnvelope::getRatio(int xStage)
{
    xStage = xStage == -1 ? stage : xStage;
    return stages[xStage].getRatio();
}

float AdsrEnvelope::getEnvelope()
{
    return envelope;
}
