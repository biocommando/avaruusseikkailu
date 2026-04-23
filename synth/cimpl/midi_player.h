#pragma once

#include "midi_reader.h"
#include "synth.h"

typedef struct
{
    float tempo;// = 120;
    float sample_rate;
    float signature;// = 4;
    int ticks_per_quarter_note;// = 48;
    int samples_per_tick;// = 0;
    unsigned pos;// = 0;
    int tick_pos;// = 0;
    int tracks_at_end;// = 0;

    MidiFile *midi_file;
    
    unsigned *next_track_event_at;
    unsigned *next_track_event_idx;

    Synth synth;
} MidiPlayer;

void init_midi_player(MidiPlayer *p, float sample_rate);
void free_midi_player(MidiPlayer *p);
void midi_player_set_midi_file(MidiPlayer *p, MidiFile *midi_file);
void midi_player_process_buffer(MidiPlayer *p, float *buf, int size);