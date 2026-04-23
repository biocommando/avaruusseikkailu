#include "basic_delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_BasicDelay(BasicDelay *bd, unsigned long bufferLength, int sampleRate)
{
    bd->sampleRate = sampleRate;
    bd->bufferLength = bufferLength;
    bd->delaySamples = 0;
    bd->feed = 0;
    bd->buffer = (double *)malloc(sizeof(double) * bufferLength);
    memset(bd->buffer, 0, sizeof(double) * bufferLength);
    bd->index = 0;
}
void free_BasicDelay(BasicDelay *bd)
{
    free(bd->buffer);
    bd->buffer = NULL;
}
double BasicDelay_process(BasicDelay *bd, double input)
{
    const double temp = bd->buffer[bd->index];
    bd->buffer[bd->index] = bd->buffer[bd->index] * bd->feed + input;
    if (++bd->index >= bd->delaySamples)
        bd->index = 0;
    if (bd->index >= bd->bufferLength)
        bd->index = 0;
    return temp;
}
void BasicDelay_setTime(BasicDelay *bd, double milliseconds)
{
    unsigned long oldDelay = bd->delaySamples;
    bd->delaySamples = (unsigned long)(milliseconds * 0.001 * bd->sampleRate);
    if (bd->delaySamples > bd->bufferLength)
        bd->delaySamples = bd->bufferLength;
    if (bd->delaySamples < oldDelay)
        for (unsigned long ul = bd->delaySamples; ul < oldDelay; ul++)
            bd->buffer[ul] = 0;
}
void BasicDelay_setFeedback(BasicDelay *bd, double feedback)
{
    bd->feed = feedback;
    if (bd->feed < 0)
        bd->feed = 0;
    else if (bd->feed > 0.99)
        bd->feed = 0.99;
}