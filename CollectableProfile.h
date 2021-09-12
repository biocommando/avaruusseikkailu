#pragma once

#include "ConfigFile.h"

enum CollectableType
{
    Collectable_AMMO,
    Collectable_WEAPON,
    Collectable_HEALTH,
    Collectable_COIN,
    Collectable_MISSION_ITEM,
};

class CollectableProfile
{
public:
    std::string sprite;
    CollectableType type;
    int weapon_id;
    float bonus_amount;
    int sound = -1;
    int sound_key = 48;
    std::string name;

    CollectableProfile()
    {
    }

    static std::map<int, CollectableProfile> read_from_file(const std::string &file)
    {
        std::map<int, CollectableProfile> m;
        int id = 0;

        ConfigFile cf([&m, &id](const auto &key, const auto &val)
                      {
                          if (key == "amount")
                              m[id].bonus_amount = std::stof(val);
                          if (key == "type")
                          {
                              if (val.find("ammo_") == 0)
                              {
                                  m[id].weapon_id = std::stoi(val.substr(5));
                                  m[id].type = Collectable_AMMO;
                              }
                              else if (val.find("weapon_") == 0)
                              {
                                  m[id].weapon_id = std::stoi(val.substr(7));
                                  m[id].type = Collectable_WEAPON;
                              }
                              else if (val == "health")
                                  m[id].type = Collectable_HEALTH;
                              else if (val == "coin")
                                  m[id].type = Collectable_COIN;
                              else if (val == "mission")
                                  m[id].type = Collectable_MISSION_ITEM;
                          }
                          if (key == "id")
                              id = std::stoi(val);
                          if (key == "sprite")
                              m[id].sprite = val;
                          if (key == "name")
                              m[id].name = val;
                          if (key == "sound")
                              m[id].sound = std::stoi(val);
                          if (key == "sound_key")
                              m[id].sound_key = std::stoi(val);
                      },
                      [](const auto &s) {});
        cf.read_config_file(file);
        return m;
    }
};