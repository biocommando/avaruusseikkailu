#include "synth.h"
#include "kick_wav.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static inline unsigned _random()
{
    static unsigned state = 123;
    state = (state >> 5) ^ state;
    state = (state << 7) ^ state;
    return state;
}

void init_SynthVoice(SynthVoice *sv, float sample_rate, int key, int channel)
{
    sv->osc1_type = 0;
    sv->osc2_type = 0;
    sv->osc1_mix = 1;
    sv->osc2_mix = 1;

    sv->volume = 1;
    sv->delay_send = 0;
    sv->allow_note_stealing = 1;

    sv->sample_rate = sample_rate;
    init_MicrotrackerMoog(&sv->filter, sample_rate);
    init_BasicOscillator(&sv->osc1, sample_rate);
    init_BasicOscillator(&sv->osc2, sample_rate);
    sv->key = key;
    sv->channel = channel;
    // This is kind of cheating but creating good sounding kicks
    // is just near impossible with this limited sub synth otherwise
    BasicOscillator_setWavetable(&sv->osc1, get_kick_wav());
    BasicOscillator_setWaveTableParams(&sv->osc1, 0, 1);
    BasicOscillator_setWavetable(&sv->osc2, get_kick_wav());
    BasicOscillator_setWaveTableParams(&sv->osc2, 0, 1);

    init_AdsrEnvelope(&sv->amp_envelope);
    init_AdsrEnvelope(&sv->filter_envelope);

    AdsrEnvelope_trigger(&sv->amp_envelope);
    AdsrEnvelope_trigger(&sv->filter_envelope);
}

void SynthVoice_process(SynthVoice *sv, float *delay_sample, float *left, float *right)
{
    float v = 0;
    if (sv->osc1_mix > 0)
    {
        BasicOscillator_calculateNext(&sv->osc1);
        v += BasicOscillator_getValue(&sv->osc1, (enum OscType)sv->osc1_type) * sv->osc1_mix;
    }
    if (sv->osc2_mix > 0)
    {
        BasicOscillator_calculateNext(&sv->osc2);
        v += BasicOscillator_getValue(&sv->osc2, (enum OscType)sv->osc2_type) * sv->osc2_mix;
    }
    AdsrEnvelope_calculateNext(&sv->filter_envelope);
    float e = AdsrEnvelope_getEnvelope(&sv->filter_envelope);
    if (sv->env_to_pitch)
    {
        BasicOscillator_setFrequency(&sv->osc1, sv->osc1_base_freq * e);
        BasicOscillator_setFrequency(&sv->osc2, sv->osc2_base_freq * e);
    }
    else
    {
        MicrotrackerMoog_setModulation(&sv->filter, e * sv->filter_mod_amount);
    }

    if (sv->noise_amount > 0)
    {
        v += sv->noise_amount * (1 - _random() / 2147483648.0f);
    }

    if (sv->distortion > 0)
    {
        v = v * (1 + sv->distortion);
        v = v > 1 ? 1 : (v < -1 ? -1 : v);
    }

    v = MicrotrackerMoog_calculate(&sv->filter, v);

    AdsrEnvelope_calculateNext(&sv->amp_envelope);
    v *= AdsrEnvelope_getEnvelope(&sv->amp_envelope);
    v *= sv->volume;
    *left += v * sv->left_volume;
    *right += v * sv->right_volume;
    *delay_sample += v * sv->delay_send;
}

void SynthVoice_release(SynthVoice *sv)
{
    AdsrEnvelope_release(&sv->amp_envelope);
    AdsrEnvelope_release(&sv->filter_envelope);
}

float note_to_hz(float note)
{
    return pow(2, note / 12) * 16.352;
}

void SynthVoice_set_params(SynthVoice *sv, const SynthParams *params)

{
    sv->osc1_base_freq = note_to_hz(sv->key + params->osc1_semitones);
    BasicOscillator_setFrequency(&sv->osc1, sv->osc1_base_freq);
    sv->osc1_type = params->osc1_type;
    sv->osc1_mix = params->osc1_mix;
    sv->osc2_base_freq = note_to_hz(sv->key + params->osc2_semitones);
    BasicOscillator_setFrequency(&sv->osc2, sv->osc2_base_freq);
    if (params->randomize_phase)
    {
        BasicOscillator_randomizePhase(&sv->osc1, 1);
        BasicOscillator_randomizePhase(&sv->osc2, 1);
    }
    sv->osc2_type = params->osc2_type;
    sv->osc2_mix = params->osc2_mix;

    AdsrEnvelope_setAttack(&sv->amp_envelope, sv->sample_rate * 4 * params->amp_attack);
    AdsrEnvelope_setDecay(&sv->amp_envelope, sv->sample_rate * 4 * params->amp_decay);
    AdsrEnvelope_setSustain(&sv->amp_envelope, params->amp_sustain);
    AdsrEnvelope_setRelease(&sv->amp_envelope, sv->sample_rate * 4 * params->amp_release);

    AdsrEnvelope_setAttack(&sv->filter_envelope, sv->sample_rate * 4 * params->filter_attack);
    AdsrEnvelope_setDecay(&sv->filter_envelope, sv->sample_rate * 4 * params->filter_decay);
    AdsrEnvelope_setSustain(&sv->filter_envelope, params->filter_sustain);
    AdsrEnvelope_setRelease(&sv->filter_envelope, sv->sample_rate * 4 * params->filter_release);

    MicrotrackerMoog_setCutoff(&sv->filter, params->filter_cutoff);
    MicrotrackerMoog_setResonance(&sv->filter, params->filter_resonance);
    sv->filter_mod_amount = params->filter_mod_amount;

    sv->distortion = params->distortion * 100;
    sv->env_to_pitch = params->env_to_pitch != 0;
    sv->noise_amount = params->noise_amount;

    sv->left_volume = params->pan < 0.5 ? 1 : 2 - 2 * params->pan;
    sv->right_volume = params->pan > 0.5 ? 1 : 2 * params->pan;

    sv->volume *= params->volume;
}

int SynthVoice_ended(SynthVoice *sv)
{
    return AdsrEnvelope_ended(&sv->amp_envelope);
}

void init_Synth(Synth *s, float sample_rate)
{
    s->sample_rate = sample_rate;
    init_BasicDelay(&s->send_delay, sample_rate, sample_rate);

    memset(s->active_voices, -1, sizeof(s->active_voices));
    memset(s->instruments, 0, sizeof(s->instruments));
    memset(s->send_delay_amounts, 0, sizeof(s->send_delay_amounts));

    s->diagnostics_max_n_voices = 0;
}

void free_Synth(Synth *s)
{
    free_BasicDelay(&s->send_delay);
}

void Synth_handle_midi_event(Synth *s, unsigned char *event_data, unsigned flags)
{
    if ((event_data[0] & 0xF0) == 0b10000000)
    {
        int channel = event_data[0] & 0xF;
        for (int i = 0; i < SYNTH_MAX_VOICES && s->active_voices[i] >= 0; i++)
        {
            SynthVoice *v = &s->voices[s->active_voices[i]];
            if (v->channel == channel && v->key == event_data[1])
                SynthVoice_release(v);
        }
    }
    else if ((event_data[0] & 0xF0) == 0b10010000)
    {
        int n_active_voices = 0;
        int channel = event_data[0] & 0xF;
        int inactive_voices[SYNTH_MAX_VOICES];
        memset(inactive_voices, 1, sizeof(inactive_voices));
        for (int i = 0; i < SYNTH_MAX_VOICES && s->active_voices[i] >= 0; i++)
        {
            inactive_voices[s->active_voices[i]] = 0;
            n_active_voices++;
        }
        int new_voice_idx = -1;
        for (int i = 0; i < SYNTH_MAX_VOICES; i++)
        {
            if (inactive_voices[i])
            {
                new_voice_idx = i;
                break;
            }
        }
        if (new_voice_idx == SYNTH_MAX_VOICES)
            return;
        if (s->diagnostics_max_n_voices < n_active_voices + 1)
        {
            s->diagnostics_max_n_voices = n_active_voices;
        }

        SynthVoice *new_voice = &s->voices[new_voice_idx];
        init_SynthVoice(new_voice, s->sample_rate, event_data[1], channel);
        new_voice->volume = event_data[2] / 127.0f;
        new_voice->delay_send = s->send_delay_amounts[channel];
        SynthVoice_set_params(new_voice, &s->instruments[channel]);
        // Note: note stealing not implemented
        if (flags & 1)
            new_voice->allow_note_stealing = 0;

        for (int i = 0; i < SYNTH_MAX_VOICES; i++)
        {
            if (s->active_voices[i] < 0)
            {
                s->active_voices[i] = new_voice_idx;
                break;
            }
        }
    }
}

void Synth_add_instrument(Synth *s, int channel, SynthParams params, float send_delay_amount)
{
    if (channel < 0 || channel >= 16)
        return;
    s->instruments[channel] = params;
    s->send_delay_amounts[channel] = send_delay_amount;
}

void Synth_set_send_delay_params(Synth *s, float feedback, float delay_ms)
{
    BasicDelay_setFeedback(&s->send_delay, feedback);
    BasicDelay_setTime(&s->send_delay, delay_ms);
}

static inline float soft_clip(const float f)
{
    const float f2 = f * f;
    return f * (27 + f2) / (27 + 9 * f2);
}

int Synth_active_voice_sort_function(const void *a, const void *b)
{
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;

    if (arg1 < arg2)
        return 1;
    if (arg1 > arg2)
        return -1;
    return 0;
}

void Synth_rearrange_active_voices(Synth *s)
{
    qsort(s->active_voices, SYNTH_MAX_VOICES, sizeof(int), Synth_active_voice_sort_function);
}

void Synth_process(Synth *s, float *buffer_left, float *buffer_right, int buffer_size)
{
    for (int i = 0; i < buffer_size; i++)
    {
        float sample_left = 0;
        float sample_right = 0;
        float delay_send_sample = 0;
        int active_voices_modified = 0;
        for (int i = 0; i < SYNTH_MAX_VOICES && s->active_voices[i] >= 0; i++)
        {
            const int vi = s->active_voices[i];
            SynthVoice *voice = &s->voices[vi];
            SynthVoice_process(voice, &delay_send_sample, &sample_left, &sample_right);
            if (SynthVoice_ended(voice))
            {
                s->active_voices[i] = -1;
                active_voices_modified = 1;
            }
        }
        if (active_voices_modified)
            Synth_rearrange_active_voices(s);
        const double delay_output = BasicDelay_process(&s->send_delay, delay_send_sample);
        buffer_left[i] = soft_clip(sample_left + delay_output);
        i++;
        buffer_left[i] = soft_clip(sample_right + delay_output);
    }
}

void Synth_kill_all_voices(Synth *s)
{
    memset(s->active_voices, -1, sizeof(s->active_voices));
}

void Synth_kill_voices(Synth *s, int channel)
{
    int voices_killed = 0;
    for (int i = 0; i < SYNTH_MAX_VOICES && s->active_voices[i] >= 0; i++)
    {
        if (s->voices[s->active_voices[i]].channel == channel)
        {
            s->active_voices[i] = -1;
            voices_killed = 1;
        }
    }
    if (voices_killed)
        Synth_rearrange_active_voices(s);
}

void Synth_read_instruments(Synth *s, const char *file)
{
    FILE *f = fopen(file, "r");
    if (!f)
        return;
    SynthParams currentParams;
    float delay_send = 0, delay_time = 500, delay_feed = 0.5;
    int instrument_count = 0;
    float total_volume = 1;

    while (instrument_count < 16)
    {
        char line[32];
        fgets(line, sizeof(line), f);
        if (feof(f))
            break;
        char name[32];
        float value;
        if (sscanf(line, "%[^=]=%f", name, &value) == 2)
        {
            if (!strcmp(name, "total_volume"))
                total_volume = value;

            if (!strcmp(name, "o1type"))
                currentParams.osc1_type = (int)(5 * value * 0.99);
            if (!strcmp(name, "o2type"))
                currentParams.osc2_type = (int)(5 * value * 0.99);

            if (!strcmp(name, "o1tune"))
                currentParams.osc1_semitones = -24 + 48 * value;
            if (!strcmp(name, "o2tune"))
                currentParams.osc2_semitones = -24 + 48 * value;

            if (!strcmp(name, "o1mix"))
                currentParams.osc1_mix = value;
            if (!strcmp(name, "o2mix"))
                currentParams.osc2_mix = value;

            if (!strcmp(name, "v_a"))
                currentParams.amp_attack = value;
            if (!strcmp(name, "v_d"))
                currentParams.amp_decay = value;
            if (!strcmp(name, "v_s"))
                currentParams.amp_sustain = value;
            if (!strcmp(name, "v_r"))
                currentParams.amp_release = value;

            if (!strcmp(name, "f_a"))
                currentParams.filter_attack = value;
            if (!strcmp(name, "f_d"))
                currentParams.filter_decay = value;
            if (!strcmp(name, "f_s"))
                currentParams.filter_sustain = value;
            if (!strcmp(name, "f_r"))
                currentParams.filter_release = value;

            if (!strcmp(name, "cut"))
                currentParams.filter_cutoff = value;
            if (!strcmp(name, "env2f"))
                currentParams.filter_mod_amount = value;
            if (!strcmp(name, "res"))
                currentParams.filter_resonance = value;

            if (!strcmp(name, "phrand"))
                currentParams.randomize_phase = value > 0.5 ? 1 : 0;

            if (!strcmp(name, "dist"))
                currentParams.distortion = value;

            if (!strcmp(name, "env2p"))
                currentParams.env_to_pitch = value > 0.5 ? 1 : 0;

            if (!strcmp(name, "noise"))
                currentParams.noise_amount = value;

            if (!strcmp(name, "volume"))
                currentParams.volume = value * total_volume;
            if (!strcmp(name, "pan"))
                currentParams.pan = value;

            if (!strcmp(name, "delsnd"))
                delay_send = value;
            if (!strcmp(name, "deltm"))
            {
                delay_time = 1000 * value;
                Synth_set_send_delay_params(s, delay_feed, delay_time);
            }
            if (!strcmp(name, "delfb"))
            {
                delay_feed = value;
                Synth_set_send_delay_params(s, delay_feed, delay_time);
            }
        }
        else
        {
            sscanf(line, "%s", name);
            if (!strcmp(name, "SYNTH_DATA_END"))
            {
                Synth_add_instrument(s, instrument_count, currentParams, delay_send);
                instrument_count++;
                // Skip drum track in GM (ch num 10 = idx 9)
                if (instrument_count == 9)
                    instrument_count++;
            }
        }
    }
}