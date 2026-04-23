#pragma once

typedef struct
{
    int length;
    int phase;
    float ratio;
} EnvelopeStage;

void EnvelopeStage_setLength(EnvelopeStage *es, int samples);
void EnvelopeStage_calcuateNext(EnvelopeStage *es);
int EnvelopeStage_hasNext(EnvelopeStage *es);
float EnvelopeStage_getRatio(EnvelopeStage *es);
void EnvelopeStage_reset(EnvelopeStage *es);
void init_EnvelopeStage(EnvelopeStage *es, int hasLength);