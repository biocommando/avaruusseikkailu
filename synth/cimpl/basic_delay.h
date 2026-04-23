#pragma once

typedef struct
{
    unsigned long delaySamples, bufferLength, index;
    int sampleRate;
    double *buffer;
    double feed;
} BasicDelay;

void init_BasicDelay(BasicDelay *bd, unsigned long bufferLength, int sampleRate);
void free_BasicDelay(BasicDelay *bd);
double BasicDelay_process(BasicDelay *bd, double input);
void BasicDelay_setTime(BasicDelay *bd, double milliseconds);
void BasicDelay_setFeedback(BasicDelay *bd, double feedback);