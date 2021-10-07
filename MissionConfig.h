#pragma once

#include "ConfigFile.h"

struct MissionGoal
{
public:
    // kill, retrieve, survive_until, time_limit
    std::string name;
    int value;
};

struct MissionTrigger
{
public:
    int id;
    std::string type;
    std::string text;
};

class MissionConfig
{
public:
    std::string name;
    int mission_number;
    std::vector<MissionGoal> mission_goals;
    std::string map_file;
    std::string music;
    int buy_allow_flags = 1;
    std::vector<MissionTrigger> triggers;

    static std::vector<MissionConfig> read_from_file(const std::string &file)
    {
        std::vector<MissionConfig> v;
        MissionConfig c;
        MissionGoal g;
        MissionTrigger t;
        ConfigFile cf([&c, &g, &t](const auto &key, const auto &val)
                      {
                          if (key == "num")
                              c.mission_number = std::stoi(val);
                          if (key == "goal_type")
                              g.name = val;
                          if (key == "goal_value")
                              g.value = std::stoi(val);
                          if (key == "map")
                              c.map_file = val;
                          if (key == "name")
                              c.name = val;
                          if (key == "music")
                              c.music = val;
                          if (key == "buy_allow_flags")
                              c.buy_allow_flags = std::stoi(val);

                          if (key == "trigger_type")
                              t.type = val;
                          if (key == "trigger_text")
                              t.text = val;
                          if (key == "trigger_id")
                              t.id = std::stoi(val);
                      },
                      [&c, &v, &g, &t](const auto &s)
                      {
                          if (s == "set goal")
                          {
                              c.mission_goals.push_back(g);
                              g = MissionGoal();
                          }
                          if (s == "set trigger")
                          {
                              c.triggers.push_back(t);
                              t = MissionTrigger();
                          }
                          if (s == "end")
                          {
                              v.push_back(c);
                              c = MissionConfig();
                          }
                      });
        cf.read_config_file(file);
        return v;
    }

    MissionTrigger *get_trigger(int id)
    {
        for(auto &t : triggers)
        {
            if (t.id == id)
                return &t;
        }
        return nullptr;
    }
};