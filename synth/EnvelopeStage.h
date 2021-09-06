#pragma once
class EnvelopeStage
{
    int length;
    int phase;
    float ratio;

public:
    void setLength(int samples);
    void calcuateNext();
    bool hasNext();
    float getRatio();
    void reset();
    EnvelopeStage(bool hasLength);
};
