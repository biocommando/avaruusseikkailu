#pragma once
#include <vector>
#include "EnvelopeStage.h"
class AdsrEnvelope
{
    std::vector<EnvelopeStage> stages;
    int stage, releaseStage;
    bool endReached;
    bool cycleAttackDecay;
    float sustain, releaseLevel;
    float envelope;
    void triggerStage(int stage);

public:
    void setAttack(int samples);
    void setDecay(int samples);
    void setSustain(float level);
    float getSustain() { return sustain; }
    void setRelease(int samples);
    void setCycleOnOff(bool on) { cycleAttackDecay = on; }

    void calculateNext();
    void trigger();
    void release();

    int getStage();
    int getReleaseStage() { return releaseStage; }
    float getRatio(int stage = -1);
    float getEnvelope();
    bool ended();

    AdsrEnvelope();
    ~AdsrEnvelope();
};
