#pragma once

#include <stdlib.h>

typedef struct
{
    int end_of_track;
    unsigned char data[3];
    unsigned time_delta;
} MidiEvent;

typedef struct
{
    unsigned count;
    MidiEvent *events;
    size_t byte_size;
} Track;

typedef struct
{
    Track *tracks;
    int track_count;
    float tempo;
    float signature;
    int ticks_per_quarter_note;
    int samples_per_tick;
    float sample_rate;
    size_t file_length_samples;

    unsigned *next_track_event_at;
    unsigned *next_track_event_idx;
} MidiFile;

void init_midi_file(MidiFile *mf);

void read_midi_file(const char *file, MidiFile *midi);

void free_midi_file(MidiFile *t);