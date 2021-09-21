#include "Synth.h"
#include <cmath>
#include <string>
#include <iostream>
#include "kick_wav.h"

SynthVoice::SynthVoice(float sample_rate, int key, int channel)
    : sample_rate(sample_rate), filter(sample_rate), osc1(sample_rate),
      osc2(sample_rate), key(key), channel(channel)
{
    // This is kind of cheating but creating good sounding kicks
    // is just near impossible with this limited sub synth otherwise
    osc1.setWavetable(get_kick_wav());
    osc1.setWaveTableParams(0, 1);
    osc2.setWavetable(get_kick_wav());
    osc2.setWaveTableParams(0, 1);
    amp_envelope.trigger();
    filter_envelope.trigger();
}

float SynthVoice::process()
{
    osc1.calculateNext();
    osc2.calculateNext();
    float v = 0;
    if (osc1_mix > 0)
        v += osc1.getValue((OscType)osc1_type) * osc1_mix;
    if (osc2_mix > 0)
        v += osc2.getValue((OscType)osc2_type) * osc2_mix;
    filter_envelope.calculateNext();
    if (env_to_pitch)
    {
        osc1.setFrequency(osc1_base_freq * filter_envelope.getEnvelope());
        osc2.setFrequency(osc2_base_freq * filter_envelope.getEnvelope());
    }
    else
    {
        filter.setModulation(filter_envelope.getEnvelope() * filter_mod_amount);
    }

    if (noise_amount > 0)
    {
        v += noise_amount * (1 - rand() % 20000 / 10000.0f);
    }

    if (distortion > 0)
    {
        v = v * (1 + distortion * 100);
        v = v > 1 ? 1 : (v < -1 ? -1 : v);
    }

    v = filter.calculate(v);

    amp_envelope.calculateNext();
    v *= amp_envelope.getEnvelope();
    v *= volume;
    return v;
}

void SynthVoice::release()
{
    amp_envelope.release();
    filter_envelope.release();
}

void SynthVoice::pan_sample(const float orig, float &sample_left, float &sample_right)
{
    sample_left = orig * left_volume;
    sample_right = orig * right_volume;
}

inline float note_to_hz(float note)
{
    return pow(2, note / 12) * 16.352;
}

void SynthVoice::set_params(const SynthParams &params)
{
    osc1_base_freq = note_to_hz(key + params.osc1_semitones);
    osc1.setFrequency(osc1_base_freq);
    osc1_type = params.osc1_type;
    osc1_mix = params.osc1_mix;
    osc2_base_freq = note_to_hz(key + params.osc2_semitones);
    osc2.setFrequency(osc2_base_freq);
    if (params.randomize_phase)
    {
        osc1.randomizePhase();
        osc2.randomizePhase();
    }
    osc2_type = params.osc2_type;
    osc2_mix = params.osc2_mix;
    amp_envelope.setAttack(sample_rate * 4 * params.amp_attack);
    amp_envelope.setDecay(sample_rate * 4 * params.amp_decay);
    amp_envelope.setSustain(params.amp_sustain);
    amp_envelope.setRelease(sample_rate * 4 * params.amp_release);

    filter_envelope.setAttack(sample_rate * 4 * params.filter_attack);
    filter_envelope.setDecay(sample_rate * 4 * params.filter_decay);
    filter_envelope.setSustain(params.filter_sustain);
    filter_envelope.setRelease(sample_rate * 4 * params.filter_release);

    filter.setCutoff(params.filter_cutoff);
    filter.setResonance(params.filter_resonance);
    filter_mod_amount = params.filter_mod_amount;

    distortion = params.distortion;
    env_to_pitch = params.env_to_pitch != 0;
    noise_amount = params.noise_amount;

    left_volume = params.pan < 0.5 ? 1 : 2 - 2 * params.pan;
    right_volume = params.pan > 0.5 ? 1 : 2 * params.pan;

    volume *= params.volume;
}

bool SynthVoice::ended()
{
    return amp_envelope.ended();
}

void Synth::handle_midi_event(unsigned char *event_data, unsigned flags)
{
    if ((event_data[0] & 0xF0) == 0b10000000)
    {
        int channel = event_data[0] & 0xF;
        voice_lock.lock();
        for (auto &v : voices)
        {
            if (v.channel == channel && v.key == event_data[1])
                v.release();
        }
        voice_lock.unlock();
    }
    else if ((event_data[0] & 0xF0) == 0b10010000)
    {
        int channel = event_data[0] & 0xF;
        SynthVoice new_voice(sample_rate, event_data[1], channel);
        new_voice.volume = event_data[2] / 127.0f;
        new_voice.set_params(instruments[channel]);
        if (flags & 1)
            new_voice.allow_note_stealing = false;
        bool voice_attached = false;
        voice_lock.lock();
        // Don't apply this logic to the FX channel unless we have very many voices active already
        if (channel != 9 || voices.size() > 16)
        {
            for (int i = 0; i < voices.size(); i++)
            {
                if (voices[i].allow_note_stealing && voices[i].channel == channel && voices[i].key == event_data[1])
                {
                    voices[i] = new_voice;
                    voice_attached = true;
                    break;
                }
            }
        }
        if (!voice_attached)
        {
            voices.push_back(new_voice);
        }
        voice_lock.unlock();
    }
}

void Synth::add_instrument(int channel, SynthParams &params, float send_delay_amount)
{
    instruments[channel] = params;
    send_delay_amounts[channel] = send_delay_amount;
}

void Synth::set_send_delay_params(float feedback, float delay_ms)
{
    send_delay.setFeedback(feedback);
    send_delay.setTime(delay_ms);
}

void Synth::kill_voices()
{
    voice_lock.lock();
    voices.clear();
    voice_lock.unlock();
}

void Synth::kill_voices(int channel)
{
    voice_lock.lock();
    for (int i = voices.size() - 1; i >= 0; i--)
    {
        if (voices[i].channel == channel)
            voices.erase(voices.begin() + i);
    }
    voice_lock.unlock();
}

static inline float soft_clip(const float f)
{
#ifdef NO_SOFTCLIP
    return f;
#else
    const auto f2 = f * f;
    return f * (27 + f2) / (27 + 9 * f2);
#endif
}

void Synth::process(float *buffer_left, float *buffer_right, int buffer_size)
{
    voice_lock.lock();
    for (int i = 0; i < buffer_size; i++)
    {
        float sample_left = 0;
        float sample_right = 0;
        float delay_send_sample = 0;
        for (int vi = voices.size() - 1; vi >= 0; vi--)
        {
            auto &voice = voices[vi];
            const auto voice_sample = voice.process();
            float panned_left, panned_right;
            voice.pan_sample(voice_sample, panned_left, panned_right);
            sample_left += panned_left;
            sample_right += panned_right;
            delay_send_sample += voice_sample * send_delay_amounts[voice.channel];
            if (voice.ended())
            {
                voices.erase(voices.begin() + vi);
            }
        }
        const auto delay_output = send_delay.process(delay_send_sample);
        if (buffer_right)
        {
            buffer_left[i] = soft_clip(sample_left + delay_output);
            buffer_right[i] = soft_clip(sample_right + delay_output);
        }
        else
        {
            buffer_left[i] = soft_clip(sample_left + delay_output);
            i++;
            buffer_left[i] = soft_clip(sample_right + delay_output);
        }
    }
    voice_lock.unlock();
}