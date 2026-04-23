#pragma once

#define MAX_WT_SAMPLE_SLOTS 8

struct wt_sample
{
    float *data;
    int size;
};

/** Reads all wt_sample_slot_X.wav files from the directory.
 * Returns number of files read successfully. */
int wt_sample_read_all(const char *directory);

void wt_sample_free();

struct wt_sample *get_wt_sample(unsigned slot);