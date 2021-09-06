#include "EnvelopeStage.h"

EnvelopeStage::EnvelopeStage(bool hasLength)
{
    length = hasLength ? 0 : -1;
    phase = 0;
}

void EnvelopeStage::setLength(int samples)
{
    if (length >= 0 && samples >= 0)
    {
        length = samples;
    }
}

void EnvelopeStage::calcuateNext()
{
    if (++phase >= length)
    {
        phase = length;
        ratio = 1;
    }
    else
    {
        ratio = phase / (float)length;
    }
}

bool EnvelopeStage::hasNext()
{
    return length == -1 || phase < length;
}

float EnvelopeStage::getRatio()
{
    return ratio;
}

void EnvelopeStage::reset()
{
    phase = 0;
    ratio = 0;
}