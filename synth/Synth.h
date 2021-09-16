#pragma once
#include "MoogFilter.h"
#include "AdsrEnvelope.h"
#include "BasicOscillator.h"
#include "BasicDelay.h"
#include <map>
#include <mutex>

struct SynthParams
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
};

class SynthVoice
{
    MicrotrackerMoog filter;
    BasicOscillator osc1;
    BasicOscillator osc2;
    float osc1_mix = 1;
    float osc2_mix = 1;
    int osc1_type = 0;
    int osc2_type = 0;
    AdsrEnvelope amp_envelope;
    AdsrEnvelope filter_envelope;
    float sample_rate;
    float distortion;
    bool env_to_pitch;
    float osc1_base_freq;
    float osc2_base_freq;
    float noise_amount;
    float filter_mod_amount;
    float left_volume;
    float right_volume;

public:
    int key;
    int channel;
    float volume = 1;
    bool allow_note_stealing = true;

    SynthVoice(float sample_rate, int key, int channel);

    float process();

    void pan_sample(const float orig, float &sample_left, float &sample_right);

    void release();

    void set_params(const SynthParams &params);

    bool ended();
};

class Synth
{
    std::mutex voice_lock;
    std::vector<SynthVoice> voices;
    float sample_rate;
    std::map<int, SynthParams> instruments;
    BasicDelay send_delay;
    float send_delay_amounts[16];

public:
    Synth(float sample_rate) : sample_rate(sample_rate), send_delay(sample_rate)
    {
    }

    void handle_midi_event(unsigned char *event_data, unsigned flags = 0);

    void add_instrument(int channel, SynthParams &params, float send_delay_amount);

    void set_send_delay_params(float feedback, float delay_ms);

    void process(float *buffer_left, float *buffer_right, int buffer_size);

    void kill_voices();
};
