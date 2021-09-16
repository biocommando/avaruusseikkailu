#pragma once
#include "Synth.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include "ConfigFile.h"

struct MidiEvent
{
    bool end_of_track;
    unsigned char data[3];
    unsigned time_delta;
};

class MidiTracker
{
    float tempo = 120;
    float sample_rate;
    float signature = 4;
    int ticks_per_quarter_note = 48;
    int samples_per_tick = 0;
    unsigned pos = 0;
    int tick_pos = 0;
    int tracks_at_end = 0;
    std::vector<std::vector<MidiEvent>> tracks;
    std::vector<unsigned> next_track_event_at;
    std::vector<unsigned> next_track_event_idx;
    std::vector<SynthParams> sound_effects;
    std::vector<MidiEvent> sfx_release;
    FILE *f = nullptr;
    Synth synth;

    void change_endianness(void *data, int length)
    {
        unsigned char temp[4];
        memcpy(temp, data, length);
        unsigned char *datap = (unsigned char *)data;
        for (int i = 0; i < length; i++)
        {
            datap[length - i - 1] = temp[i];
        }
    }

    void read_chunk_hdr(std::string &chunk_type, unsigned &length)
    {
        char hdr[4];
        fread(hdr, 1, 4, f);
        chunk_type.assign(hdr, 4);
        fread(&length, sizeof(unsigned), 1, f);
        change_endianness(&length, 4);
    }

    unsigned read_variable_length_quantity(int &length)
    {
        unsigned out = 0;
        //std::string debug;
        for (int i = 0; i < 4; i++)
        {
            unsigned char u;
            fread(&u, 1, 1, f);
            //debug = debug + std::to_string(u) + ';';
            unsigned char val = 0x7F & u;
            out = (out << 7) | val;
            length = i + 1;
            if (!(u & 0x80))
            {
                break;
            }
        }
        //std::cout << "var len: " << debug << " -> " << std::to_string(out) << "\n";
        return out;
    }

    void read_track(unsigned length)
    {
        std::vector<MidiEvent> events;
        unsigned read_amt;
        unsigned char last_status = 0;
        while (read_amt < length)
        {
            int length;
            auto time_delta = read_variable_length_quantity(length);
            read_amt += length;
            unsigned char event_data;
            unsigned char next_byte;
            fread(&next_byte, 1, 1, f);
            // Running status... status is omitted if consequent events have same status
            if ((next_byte & 0x80) == 0)
            {
                fseek(f, -1, SEEK_CUR);
                event_data = last_status;
            }
            else
            {
                event_data = next_byte;
                read_amt++;
            }
            last_status = event_data;
            //std::cout << "Event type " << std::to_string(event_data) << "@" << std::to_string(ftell(f)) << ", td=" << std::to_string(time_delta) << "\n";
            if (event_data == 0xF0 || event_data == 0xF7)
            {
                auto data_len = read_variable_length_quantity(length);
                //std::cout << "Skip bytes " << std::to_string(length) << '+' << std::to_string(data_len) << "\n";
                read_amt += length + data_len;
                fseek(f, data_len, SEEK_CUR);
            }
            // Meta event
            else if (event_data == 0xFF)
            {
                unsigned char type;
                fread(&type, 1, 1, f);
                auto data_len = read_variable_length_quantity(length);
                if (type == 0x2F)
                {
                    events.push_back(MidiEvent{true});
                }
                //std::cout << "Skip bytes 1+" << std::to_string(length) << '+' << std::to_string(data_len) << "\n";
                read_amt += length + data_len + 1;
                fseek(f, data_len, SEEK_CUR);
            }
            else if (event_data == 0b11110010)
            {
                // Song position pointer
                fseek(f, 3, SEEK_CUR);
                read_amt += 3;
            }
            else if (event_data == 0b11110011)
            {
                // song select
                fseek(f, 1, SEEK_CUR);
                read_amt++;
            }
            else if ((event_data & 0xF0) == 0xF0)
            {
                // single byte message; do nothing
            }
            else
            {
                auto evt_no_ch = event_data & 0xF0;
                if (evt_no_ch == 0b11000000 || evt_no_ch == 0b11010000)
                {
                    // Pgm change / after touch
                    fseek(f, 1, SEEK_CUR);
                    read_amt++;
                }
                else
                {
                    //std::cout << "Midi event add: " << std::to_string(event_data) << "\n";
                    MidiEvent e{false};
                    e.data[0] = event_data;
                    fread(e.data + 1, 1, 2, f);
                    e.time_delta = time_delta;
                    // Ignore other than note on / off
                    if ((event_data & 0xF0) == 0b10000000 || (event_data & 0xF0) == 0b10010000)
                        events.push_back(e);
                    read_amt += 2;
                }
            }
        }
        if (events.size() > 0)
        {
            /*for (auto &evt : events)
            {
                std::cout << ((evt.data[0] & 0xF0) == 0b10000000 ? "note off" : "note on") << ", key="
                          << std::to_string(evt.data[1]) << ", vel=" << std::to_string(evt.data[2])
                          << ". TD=" << std::to_string(evt.time_delta) << ", instrument:" << std::to_string(evt.data[0] & 0xF) << "\n";
            }*/
            tracks.push_back(events);
            next_track_event_at.push_back(events[0].time_delta);
            next_track_event_idx.push_back(0);
        }
    }

    void read_metadata(const std::string &file, bool load_sfx = false)
    {
        SynthParams currentParams;
        float delay_send = 0, delay_time = 500, delay_feed = 0.5;
        int instrument_count = 0;
        float total_volume = 1;
        ConfigFile cf(
            [this, &total_volume, &currentParams, &delay_send, &delay_time, &delay_feed, &load_sfx](const auto &name, const auto &strValue)
            {
                auto value = std::stof(strValue);
                if (name == "total_volume")
                    total_volume = value;

                if (name == "o1type")
                    currentParams.osc1_type = (int)(5 * value * 0.99);
                if (name == "o2type")
                    currentParams.osc2_type = (int)(5 * value * 0.99);

                if (name == "o1tune")
                    currentParams.osc1_semitones = -24 + 48 * value;
                if (name == "o2tune")
                    currentParams.osc2_semitones = -24 + 48 * value;

                if (name == "o1mix")
                    currentParams.osc1_mix = value;
                if (name == "o2mix")
                    currentParams.osc2_mix = value;

                if (name == "v_a")
                    currentParams.amp_attack = value;
                if (name == "v_d")
                    currentParams.amp_decay = value;
                if (name == "v_s")
                    currentParams.amp_sustain = value;
                if (name == "v_r")
                    currentParams.amp_release = value;

                if (name == "f_a")
                    currentParams.filter_attack = value;
                if (name == "f_d")
                    currentParams.filter_decay = value;
                if (name == "f_s")
                    currentParams.filter_sustain = value;
                if (name == "f_r")
                    currentParams.filter_release = value;

                if (name == "cut")
                    currentParams.filter_cutoff = value;
                if (name == "env2f")
                    currentParams.filter_mod_amount = value;
                if (name == "res")
                    currentParams.filter_resonance = value;

                if (name == "phrand")
                    currentParams.randomize_phase = value > 0.5 ? 1 : 0;

                if (name == "dist")
                    currentParams.distortion = value;

                if (name == "env2p")
                    currentParams.env_to_pitch = value > 0.5 ? 1 : 0;

                if (name == "noise")
                    currentParams.noise_amount = value;

                if (name == "volume")
                    currentParams.volume = value * total_volume;
                if (name == "pan")
                    currentParams.pan = value;

                if (name == "delsnd")
                    delay_send = value;
                if (!load_sfx)
                {
                    if (name == "deltm")
                    {
                        delay_time = 1000 * value;
                        this->synth.set_send_delay_params(delay_feed, delay_time);
                    }
                    if (name == "delfb")
                    {
                        delay_feed = value;
                        this->synth.set_send_delay_params(delay_feed, delay_time);
                    }

                    if (name == "tempo")
                        this->tempo = value;
                }
            },
            [this, &instrument_count, &currentParams, &delay_send, &load_sfx](const auto &str)
            {
                if (str == "SYNTH_DATA_END")
                {
                    if (load_sfx)
                    {
                        this->sound_effects.push_back(currentParams);
                    }
                    else if (instrument_count < 16)
                    {
                        this->synth.add_instrument(instrument_count, currentParams, delay_send);
                        instrument_count++;
                        // Skip FX track (drum track in GM) (ch num 10 = idx 9)
                        if (instrument_count == 9)
                            instrument_count++;
                    }
                }
            });
        cf.read_config_file(file);
    }

public:
    MidiTracker(float sample_rate) : sample_rate(sample_rate), synth(sample_rate)
    {
        synth.set_send_delay_params(0.5, 100);
    }

    std::vector<unsigned> get_track_pos_marker()
    {
        std::vector<unsigned> ret;
        for (int trk = 0; trk < tracks.size(); trk++)
        {
            ret.push_back(next_track_event_at[trk]);
            ret.push_back(next_track_event_idx[trk]);
        }
        ret.push_back(pos);
        return ret;
    }

    void set_track_pos_marker(const std::vector<unsigned> &marker)
    {
        for (int trk = 0; trk < tracks.size(); trk++)
        {
            next_track_event_at[trk] = marker[2 * trk];
            next_track_event_idx[trk] = marker[2 * trk + 1];
        }
        pos = marker.back();
    }

    void load_sound_effects(const std::string &file)
    {
        read_metadata(file, true);
    }

    void read_midi_file(const std::string &file)
    {
        read_metadata(file + "_meta.ini");
        f = fopen(file.c_str(), "rb");
        unsigned short num_tracks;
        unsigned short division;
        unsigned short format;
        synth.kill_voices();
        tracks.clear();
        next_track_event_at.clear();
        next_track_event_idx.clear();
        tracks_at_end = 0;
        pos = 0;
        while (!feof(f))
        {
            std::string chunk_type;
            unsigned length;
            read_chunk_hdr(chunk_type, length);
            if (feof(f))
                break;
            const auto current_pos = ftell(f);
            if (chunk_type == "MThd")
            {
                fread(&format, sizeof(unsigned short), 1, f);
                change_endianness(&format, 2);
                fread(&num_tracks, sizeof(unsigned short), 1, f);
                change_endianness(&num_tracks, 2);
                fread(&division, sizeof(unsigned short), 1, f);
                change_endianness(&division, 2);
                ticks_per_quarter_note = division;
                auto ticks_per_second = tempo / 60.0f * ticks_per_quarter_note;
                samples_per_tick = sample_rate / ticks_per_second;
                /*std::cout << "Tempo: " << std::to_string(tempo) << "\n";
                std::cout << "Samples per tick: " << std::to_string(samples_per_tick) << "\n";*/
            }
            else if (chunk_type == "MTrk")
            {
                read_track(length);
            }
            fseek(f, current_pos + length, SEEK_SET);
        }
        fclose(f);
    }

    /**
     * trigger_mode flags:
     * 1 = trigger (note on)
     * 2 = release (note off)
     * 4 = don't allow note stealing
     * --
     * id can be omitted if only trigger_mode release flag is set
     * or if the last effect is re-used
     */
    void trigger_sfx(int key, int id = 1000, int volume = 100, int trigger_mode = 3)
    {
        unsigned char midi_data[3] = {0, (unsigned char)key, (unsigned char)volume};
        if (trigger_mode & 1)
        {
            if (id < sound_effects.size())
                synth.add_instrument(9, sound_effects[id], 0);
            midi_data[0] = 0b10010000 | 9;
            const auto flags = (trigger_mode & 4) ? 1 : 0;
            synth.handle_midi_event(midi_data, flags);
        }
        if (trigger_mode & 2)
        {
            midi_data[0] = 0b10000000 | 9;
            //synth.handle_midi_event(midi_data);
            // make all triggers at least one tick long
            MidiEvent e;
            memcpy(e.data, midi_data, 3);
            sfx_release.push_back(e);
        }
    }

    void process_buffer(float *buf, int size)
    {
        int process_at_idx = 0;
        size *= 2; // stereo
        for (int i = 0; i < size; i++)
        {
            if (++tick_pos == samples_per_tick)
            {
                synth.process(&buf[process_at_idx], nullptr, i - process_at_idx);
                if (sfx_release.size() > 0)
                {
                    for (auto &sfx_rel : sfx_release)
                    {
                        synth.handle_midi_event(sfx_rel.data);
                    }
                    sfx_release.clear();
                }

                process_at_idx = i;
                tick_pos = 0;
                for (int trk = 0; trk < tracks.size(); trk++)
                {
                    auto &events = tracks[trk];
                    while (pos == next_track_event_at[trk])
                    {
                        /*std::cout << "Handling midi event\n";
                        auto &evt = events[next_track_event_idx[trk]];
                        std::cout << ((evt.data[0] & 0xF0) == 0b10000000 ? "note off" : "note on") << ", key="
                            << std::to_string(evt.data[1]) << ", vel=" << std::to_string(evt.data[2])
                            << ". TD=" << std::to_string(evt.time_delta) << ", instrument:" << std::to_string(evt.data[0]&0xF) << "\n";*/
                        if (!events[next_track_event_idx[trk]].end_of_track)
                            synth.handle_midi_event(events[next_track_event_idx[trk]].data);
                        next_track_event_idx[trk]++;
                        if (next_track_event_idx[trk] < events.size())
                        {
                            next_track_event_at[trk] = pos + events[next_track_event_idx[trk]].time_delta;
                        }
                        else
                        {
                            next_track_event_at[trk] = ~0;
                            tracks_at_end++;
                        }
                    }
                }
                pos++;
                if (tracks_at_end == tracks.size())
                {
                    //std::cout << "all tracks ended\n";
                    for (int trk = 0; trk < tracks.size(); trk++)
                    {
                        next_track_event_at[trk] = tracks[trk][0].time_delta;
                        next_track_event_idx[trk] = 0;
                    }
                    pos = 0;
                    tracks_at_end = 0;
                }
            }
            // in stereo audio every other sample is for left and every other for right
            // so let's make sure that we run any logic only when index is at left channel
            // so we don't need to care about misalignment of the channels
            i++;
        }
        synth.process(&buf[process_at_idx], nullptr, size - process_at_idx);
    }
};