#pragma once

#include "moog_filter.h"
#include "adsr_envelope.h"
#include "basic_oscillator.h"
#include "basic_delay.h"

typedef struct
{
    int osc1_type;
    float osc1_semitones, osc1_mix;
    int osc2_type;
    float osc2_semitones, osc2_mix;
    float amp_attack, amp_decay, amp_sustain, amp_release;
    float filter_attack, filter_decay, filter_sustain, filter_release;
    float filter_cutoff, filter_resonance;
    int randomize_phase;
    float distortion;
    int env_to_pitch;
    float noise_amount;
    float filter_mod_amount;
    float volume;
    float pan;
} SynthParams;

typedef struct
{
    // "private"
    MicrotrackerMoog filter;
    BasicOscillator osc1;
    BasicOscillator osc2;
    float osc1_mix; // = 1;
    float osc2_mix; // = 1;
    int osc1_type;  // = 0;
    int osc2_type;  // = 0;
    AdsrEnvelope amp_envelope;
    AdsrEnvelope filter_envelope;
    float sample_rate;
    float distortion;
    int env_to_pitch;
    float osc1_base_freq;
    float osc2_base_freq;
    float noise_amount;
    float filter_mod_amount;
    float left_volume;
    float right_volume;
    // "public"

    int key;
    int channel;
    float volume;            // = 1;
    float delay_send;        // = 0;
    int allow_note_stealing; // = 1;
} SynthVoice;

void init_SynthVoice(SynthVoice *sv, float sample_rate, int key, int channel);

void SynthVoice_process(SynthVoice *sv, float *delay_sample, float *left, float *right);

void SynthVoice_release(SynthVoice *sv);

void SynthVoice_set_params(SynthVoice *sv, const SynthParams *params);

int SynthVoice_ended(SynthVoice *sv);

#define SYNTH_MAX_VOICES 64

typedef struct
{
    SynthVoice voices[SYNTH_MAX_VOICES];
    int active_voices[SYNTH_MAX_VOICES];
    float sample_rate;
    SynthParams instruments[16];
    BasicDelay send_delay;
    float send_delay_amounts[16];

    int diagnostics_max_n_voices;
} Synth;

void init_Synth(Synth *s, float sample_rate);

void free_Synth(Synth *s);

void Synth_handle_midi_event(Synth *s, unsigned char *event_data, unsigned flags);

void Synth_add_instrument(Synth *s, int channel, SynthParams params, float send_delay_amount);

void Synth_set_send_delay_params(Synth *s, float feedback, float delay_ms);

void Synth_process(Synth *s, float *buffer_left, float *buffer_right, int buffer_size);

void Synth_kill_all_voices(Synth *s);

void Synth_kill_voices(Synth *s, int channel);

void Synth_read_instruments(Synth *s, const char *file);