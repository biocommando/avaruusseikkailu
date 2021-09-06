#include "audioeffectx.h"
#include "Synth.h"
#include <cstring>

struct Parameter
{
    std::string name;
    float value;
};

class SynthVst : public AudioEffectX
{
    char *chunk = nullptr;
    std::vector<Parameter> parameters;
    SynthParams currentParams;
    Synth synth;
    float delay_send, delay_time, delay_feed;
    int my_id;

    static std::vector<std::string> splitString(const std::string &s, char c)
    {
        std::vector<std::string> ret;
        int pos = 0;
        do
        {
            auto pos0 = pos;
            pos = s.find(c, pos);
            if (pos != std::string::npos)
            {
                ret.push_back(s.substr(pos0, pos - pos0));
                pos++;
            }
            else
            {
                ret.push_back(s.substr(pos0));
            }
        } while (pos != std::string::npos);
        return ret;
    }
    void add_param(const std::string &name, float value = 0)
    {
        Parameter p{name, value};
        parameters.push_back(p);
    }

    void sync_param(const std::string &name, float value, bool set_instrument = true)
    {

        if (name == "o1type")
            currentParams.osc1_type = (int)(4 * value * 0.99);
        if (name == "o2type")
            currentParams.osc2_type = (int)(4 * value * 0.99);

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
            currentParams.volume = value;

        if (name == "delsnd")
            delay_send = value;
        if (name == "deltm")
            delay_time = 1000 * value;
        if (name == "delfb")
            delay_feed = value;

        if (set_instrument)
        {
            if (name == "deltm" || name == "delfb")
                synth.set_send_delay_params(delay_feed, delay_time);
            else
                synth.add_instrument(0, currentParams, delay_send);
        }
    }

    void sync_params()
    {
        for (auto &param : parameters)
        {
            sync_param(param.name, param.value, false);
        }
        synth.add_instrument(0, currentParams, delay_send);
        synth.set_send_delay_params(delay_feed, delay_time);
    }

public:
    SynthVst(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, 25), synth(sampleRate)
    {
        setNumInputs(2);          // stereo in
        setNumOutputs(2);         // stereo out
        setUniqueID(-1336835329); // identify
        isSynth(true);
        programsAreChunks();
        add_param("o1type", 0);
        add_param("o1tune", 0.5);
        add_param("o1mix", 1);
        add_param("o2type", 0);
        add_param("o2tune", 0.5);
        add_param("o2mix", 1);
        add_param("v_a", 0);
        add_param("v_d", 0.2);
        add_param("v_s", 0.5);
        add_param("v_r", 0.2);
        add_param("f_a", 0);
        add_param("f_d", 0.2);
        add_param("f_s", 0.5);
        add_param("f_r", 0.2);
        add_param("cut", 1);
        add_param("res", 0);
        add_param("env2f", 1);
        add_param("phrand", 0);
        add_param("dist", 0);
        add_param("env2p", 0);
        add_param("noise", 0);
        add_param("delsnd", 0.5);
        add_param("deltm", 1);
        add_param("delfb", 0.5);
        add_param("volume", 0.5);
        sync_params();
    }

    ~SynthVst()
    {
        if (chunk)
        {
            free(chunk);
            chunk = nullptr;
        }
    }
    VstInt32 getChunk(void **data, bool isPreset)
    {
        if (chunk)
        {
            free(chunk);
            chunk = nullptr;
        }
        std::string s = "SYNTH_DATA_START\n";
        for (auto &param : parameters)
        {
            s += param.name + "=" + std::to_string(param.value) + '\n';
        }
        s += "SYNTH_DATA_END\n";
        chunk = (char *)malloc(s.size() + 1);
        if (chunk)
        {
            memcpy(chunk, s.c_str(), s.size() + 1);
            *data = chunk;
            return s.size() + 1;
        }
        return 0;
    }

    VstInt32 setChunk(void *data, VstInt32 byteSize, bool isPreset)
    {
        std::string s((char *)data, byteSize);
        const auto lines = splitString(s, '\n');
        for (const auto &line : lines)
        {
            const auto keyval = splitString(line, '=');
            if (keyval.size() == 2)
            {
                for (auto &param : parameters)
                {
                    if (param.name == keyval[0])
                    {
                        param.value = std::stof(keyval[1]);
                        break;
                    }
                }
            }
        }
        sync_params();
        return 0;
    }

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
    {
        synth.process(outputs[0], sampleFrames);
        memcpy(outputs[1], outputs[0], sizeof(float) * sampleFrames);
    }

    float getParameter(VstInt32 index)
    {
        if (index < parameters.size())
            return parameters[index].value;
        return 0;
    }
    void setParameter(VstInt32 index, float value)
    {
        if (index < parameters.size())
        {
            parameters[index].value = value;
            sync_param(parameters[index].name, value);
        }
    }
    void getParameterName(VstInt32 index, char *label)
    {
        if (index < parameters.size())
            strcpy(label, parameters[index].name.c_str());
        else
            *label = 0;
    }

    void getParameterDisplay(VstInt32 index, char *text)
    {
        if (index < parameters.size())
        {
            const auto &param = parameters[index];
            if (param.name == "phrand" || param.name == "env2p")
                strcpy(text, param.value > 0.5 ? "yes" : "no");
            else if (param.name == "deltm")
                strcpy(text, std::to_string((int)(param.value * 1000)).c_str());
            else if (param.name == "o1type" || param.name == "o2type")
                strcpy(text, std::to_string((int)(4 * param.value * 0.99)).c_str());
            else if (param.name == "o1tune" || param.name == "o2tune")
                float2string(-24 + 48 * param.value, text, kVstMaxParamStrLen);
            else
                float2string(param.value, text, kVstMaxParamStrLen);
        }
        else
            *text = 0;
    }
    bool getEffectName(char *name)
    {
        strcpy_s(name, 32, "MidiTrackerSynth");
        return true;
    }
    bool getProductString(char *text)
    {
        strcpy_s(text, 64, "MidiTrackerSynth");
        return true;
    }
    bool getVendorString(char *text)
    {
        strcpy_s(text, 64, "(c) 2021 Joonas Salonpaa");
        return true;
    }

    VstInt32 processEvents(VstEvents *events)
    {
        for (int i = 0; i < events->numEvents; i++)
        {
            if (!(events->events[i]->type & kVstMidiType))
            {
                continue;
            }
            VstMidiEvent *midievent = (VstMidiEvent *)(events->events[i]);
            unsigned char midiMessage[3];
            memcpy(midiMessage, midievent->midiData, 3);
            midiMessage[0] = midiMessage[0] & 0xF0;
            synth.handle_midi_event(midiMessage);
        }
        return 0;
    }
    void open()
    {
    }
};

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
    return new SynthVst(audioMaster);
}