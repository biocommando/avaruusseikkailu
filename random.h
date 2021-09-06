#pragma once
#include <cstdlib>

inline float random(float min = 0, float max = 1)
{
    float r = (rand() % 20000) / 20000.0f;
    return min + (max - min) * r;
}

inline float randomint(int min, int max)
{
    return rand() % (max - min + 1) + min;
}