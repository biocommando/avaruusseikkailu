#pragma once

class FilterBase
{
public:
    virtual float calculate(float input) = 0;
    virtual void setCutoff(float v) = 0;
    virtual void setResonance(float v) = 0;
    virtual void setModulation(float v) = 0;
    virtual void setSamplerate(int rate) = 0;
    virtual void reset() = 0;
};