#pragma once
#include "ConfigFile.h"
#include <algorithm>

class Playlist
{
    std::vector<int> order;
    std::vector<std::string> midi_files;
    std::vector<std::string> titles;
    int pos = -1;

public:
    void read_from_file(const std::string &file)
    {
        ConfigFile cf(
            [this](const auto &name, const auto &str_value)
            {
                if (name == "track")
                {
                    order.push_back(midi_files.size());
                    midi_files.push_back(str_value);
                    titles.push_back("unnamed");
                }
                if (name == "title")
                    titles[midi_files.size() - 1] = str_value;
            },
            [](const auto &str) {});
        cf.read_config_file(file);
    }

    // behavior_at_end:
    // 1 = start over, 0 = randomize
    void next_file(int behavior_at_end)
    {
        pos++;
        if (pos < order.size() || order.size() == 0)
        {
            return;
        }
        if (behavior_at_end == 1 || order.size() == 1)
            pos = -1;
        else
        {
            const auto latest_track = order.back();
            do
            {
                randomize_order();
            } while (latest_track == order[0]);
            pos = -1;
        }
        next_file(behavior_at_end);
    }

    std::string get_current_file()
    {
        if (pos > -1 && pos < order.size())
        {
            return midi_files[order[pos]];
        }
        return "";
    }

    std::string get_current_tilte()
    {
        if (pos > -1 && pos < order.size())
        {
            return titles[order[pos]];
        }
        return "";
    }

    void randomize_order()
    {
        pos = -1;
        std::sort(order.begin(), order.end(), [](int a, int b)
                  { return rand() % 2; });
    }
};