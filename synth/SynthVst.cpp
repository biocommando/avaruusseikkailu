#include "audioeffectx.h"
#ifdef VST_GUI
#include "aeffguieditor.h"
#include <windows.h>
#include <fstream>
#endif
#include "Synth.h"
#include <cstring>

struct Parameter
{
    std::string name;
    float value;
};

#ifdef VST_GUI
constexpr const char *file_guid = "9ef9ad6d-26ab-47e6-9bc4-484edb92e144";

static inline void log(std::string s)
{
    FILE *f = fopen("C:\\Users\\space\\Desktop\\avaruusseikkailu\\synth\\log.txt", "a");
    fprintf(f, s.c_str());
    fclose(f);
}

static inline std::string getWorkDir()
{
    // work out the resource directory
    // first we get the DLL path from windows API
    extern void *hInstance;
    wchar_t workDirWc[1024];
    GetModuleFileName((HMODULE)hInstance, workDirWc, 1024);
    char workDirC[1024];
    wcstombs(workDirC, workDirWc, 1024);

    std::string d = workDirC;

    // let's get rid of the DLL file name
    auto posBslash = d.find_last_of('\\');
    if (posBslash != std::string::npos)
    {
        d = d.substr(0, posBslash);
    }
    return d;
}

static inline CBitmap *loadBitmap(const std::string &relativePath)
{
    auto workDir = getWorkDir();
    std::string s = workDir + "\\" + relativePath;
    std::wstring ws(s.size(), L'#');
    mbstowcs(&ws[0], s.c_str(), s.size());
    log("loading file...\n");
    auto bmp = Gdiplus::Bitmap::FromFile(ws.c_str(), false);
    log("creating bmp\n");
    auto cbmp = new CBitmap(bmp);
    log("deleting gdi+ bmp\n");
    delete bmp;
    return cbmp;
}

class SynthVstGui : public AEffGUIEditor, public CControlListener
{
    std::map<int, CTextLabel *> knob_labels;
    std::map<int, CKnob *> knobs;
    CTextEdit *current_param = nullptr;
    int last_tweaked = -1;

    std::vector<std::string> read_preset_names()
    {
        std::vector<std::string> list;
        std::ifstream ifs;
        ifs.open(getWorkDir() + "\\presets-" + file_guid + ".dat");
        std::string s;
        while (std::getline(ifs, s))
        {
            if (s != "" && s[0] == '#')
            {
                list.push_back(s.substr(1));
            }
        }
        return list;
    }

    std::map<int, float> read_preset(const std::string &name)
    {
        const auto find = '#' + name;
        std::map<int, float> preset;
        std::ifstream ifs;
        ifs.open(getWorkDir() + "\\presets-" + file_guid + ".dat");
        std::string s;
        while (std::getline(ifs, s))
        {
            if (s == find)
            {
                int i = 0;
                while (std::getline(ifs, s) && s != "")
                {
                    if (s[0] == 'p')
                        i = std::stoi(s.substr(1));
                    else
                        preset[i] = std::stof(s);
                }
            }
        }
        return preset;
    }

    void save_preset(const std::string &name, const std::map<int, float> &preset)
    {
        std::ofstream ofs;
        ofs.open(getWorkDir() + "\\presets-" + file_guid + ".dat", std::ios_base::app);
        ofs << '#' << name << std::endl;
        for (const auto &entry : preset)
        {
            ofs << 'p' << entry.first << std::endl;
            ofs << entry.second << std::endl;
        }
        ofs << std::endl;
    }

public:
    bool open(void *ptr)
    {
        log("loaded data\n");
        CRect frameSize(0, 0, 700, 600);
        CColor cBg = {127, 127, 127, 255}, cFg = kWhiteCColor;
        ERect *wSize;
        getRect(&wSize);

        wSize->top = wSize->left = 0;
        wSize->bottom = (VstInt16)frameSize.bottom;
        wSize->right = (VstInt16)frameSize.right;

        auto xframe = new CFrame(frameSize, ptr, this);

        xframe->setBackgroundColor(cBg);

        CBitmap *knobBg = loadBitmap(std::string("knob-") + file_guid + ".bmp");
        int i = 0, x = 10, y = 10;
        while (true)
        {
            char param_name[100], param_value[100];
            effect->getParameterName(i, param_name);
            if (!*param_name)
                break;
            std::string s_name = std::string(param_name);
            if (s_name == "o2type" || s_name == "v_a" || s_name == "f_a" ||
                s_name == "cut" || s_name == "env2p" || s_name == "volume")
            {
                y = 10;
                x += 90;
            }
            if (s_name == "my_id")
            {
                i++;
                continue;
            }
            effect->getParameterDisplay(i, param_value);
            CRect rect(x, y, x + 80, y + 20);
            auto label = new CTextLabel(rect, param_name);
            label->setBackColor(cBg);
            xframe->addView(label);
            CRect rect2(x, y + 60, x + 80, y + 80);
            label = new CTextLabel(rect2, param_value);
            label->setBackColor(cBg);
            knob_labels[i] = label;
            xframe->addView(label);
            CRect rect3(x + 20, y + 20, x + 60, y + 60);
            auto knob = new CKnob(rect3, this, i, knobBg, nullptr);
            knob->setValue(effect->getParameter(i));
            knobs[i] = knob;
            xframe->addView(knob);
            y += 120;
            i++;
        }
        CRect rect_options(0, 550, 120, 570);
        auto presets = new COptionMenu(rect_options, this, 1000);
        presets->addEntry(new CMenuItem("Select preset...", 1 << 1));
        presets->addEntry(new CMenuItem("Save as new"));
        const auto list = read_preset_names();
        for (const auto &name : list)
        {
            presets->addEntry(new CMenuItem(name.c_str()));
        }
        xframe->addView(presets);
        rect_options = CRect(0, 575, 120, 595);
        current_param = new CTextEdit(rect_options, this, 2000, "0");
        xframe->addView(current_param);

        //knobBg->forget();
        /*knobBackground->forget();
        backgroundImage->forget();*/

        frame = xframe;

        return true;
    }

    void valueChanged(CControl *control)
    {
        if (!frame)
            return;
        auto tag = control->getTag();

        if (tag == 1000)
        {
            auto menu = (COptionMenu *)control;
            auto idx = menu->getCurrentIndex();
            menu->setCurrent(0);
            if (idx == 1)
            {
                std::map<int, float> preset;
                for (auto &knob_entry : knobs)
                {
                    preset[knob_entry.first] = knob_entry.second->getValue();
                }
                const auto name = "preset " + std::to_string(menu->getNbEntries() - 1);
                save_preset(name, preset);
                menu->addEntry(new CMenuItem(name.c_str()));
            }
            else
            {
                auto preset = read_preset(menu->getEntry(idx)->getTitle());
                for (auto &entry : preset)
                {
                    effect->setParameterAutomated(entry.first, entry.second);
                }
                syncParameters();
            }
            return;
        }
        if (tag == 2000 && last_tweaked != -1)
        {
            char data[256];
            current_param->getText(data);
            float val;
            if (sscanf(data, "%f", &val))
            {
                if (val >= 0 && val <= 1)
                {
                    effect->setParameterAutomated(last_tweaked, val);
                    syncParameters();
                }
            }
            return;
        }

        auto knob = (CKnob *)control;
        auto value = knob->getValue();
        effect->setParameterAutomated(tag, value);
        setParameter(tag, value);
        current_param->setText(std::to_string(value).c_str());
        last_tweaked = tag;
    }

    void setParameter(int id, float value)
    {
        if (!frame)
            return;
        char param_value[100];
        knobs[id]->setValue(value);
        effect->getParameterDisplay(id, param_value);
        knob_labels[id]->setText(param_value);
    }

    void syncParameters()
    {
        for (auto &x : knobs)
        {
            auto val = effect->getParameter(x.first);
            setParameter(x.first, val);
        }
    }

    SynthVstGui(void *ptr) : AEffGUIEditor(ptr)
    {
    }

    void close()
    {
        auto xframe = frame;
        frame = 0;
        delete xframe;
    }
};
#endif

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
            currentParams.volume = value;

        if (name == "delsnd")
            delay_send = value;
        if (name == "deltm")
            delay_time = 1000 * value;
        if (name == "delfb")
            delay_feed = value;
        if (name == "pan")
            currentParams.pan = value;

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
    SynthVst(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, 27), synth(sampleRate)
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
        add_param("pan", 0.5);
        // This can be used to identify correct instrument instances from the binary chunk
        add_param("my_id", 0);
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
        int instance_id = 16 * parameters[parameters.size() - 1].value * .99;
        s += "SYNTH_INSTANCE_ID " + std::to_string(instance_id) + '\n';
        for (auto &param : parameters)
        {
            s += param.name + "=" + std::to_string(param.value) + '\n';
        }
        // To make it possible to parse settings directly from DAW project file
        s += "tempo=" + std::to_string(getTimeInfo(kVstTempoValid)->tempo) + '\n';
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
#ifdef VST_GUI
        if (editor)
        {
            ((SynthVstGui *)editor)->syncParameters();
        }
#endif
        return 0;
    }

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
    {
        synth.process(outputs[0], outputs[1], sampleFrames);
        //memcpy(outputs[1], outputs[0], sizeof(float) * sampleFrames);
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
#ifdef VST_GUI
            if (editor)
            {
                ((SynthVstGui *)editor)->setParameter(index, value);
            }
#endif
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
            {
                const char waves[][5] = {
                    "sin", "tri", "saw", "sqr", "kick"};
                int sel = 5 * param.value * 0.99;
                strcpy(text, waves[sel]);
            }
            else if (param.name == "o1tune" || param.name == "o2tune")
                float2string(-24 + 48 * param.value, text, kVstMaxParamStrLen);
            else if (param.name == "pan")
            {
                if (param.value == 0.5)
                {
                    strcpy(text, "center");
                }
                else if (param.value < 0.5)
                {
                    int percent = 200 * (0.5 - param.value);
                    strcpy(text, ("L" + std::to_string(percent) + "%").c_str());
                }
                else
                {
                    int percent = 200 * (param.value - 0.5);
                    strcpy(text, ("R" + std::to_string(percent) + "%").c_str());
                }
            }
            else if (param.name == "my_id")
                strcpy(text, std::to_string((int)(16 * param.value * .99)).c_str());
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
#ifdef VST_GUI
        setEditor(new SynthVstGui(this));
#endif
    }
};

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
    return new SynthVst(audioMaster);
}