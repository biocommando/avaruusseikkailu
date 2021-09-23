#pragma once

#include "ConfigFile.h"

enum CollectableType
{
    Collectable_AMMO,
    Collectable_WEAPON,
    Collectable_HEALTH,
    Collectable_ARMOR,
    Collectable_COIN,
    Collectable_MISSION_ITEM,
    Collectable_MODIFY_MAP,
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
    int cost = -1;
    int buy_allow_flags = 0xFF;
    int buy_menu_order;

    int flip_props = 0;
    int hidden_sprite_x = -1;
    int hidden_sprite_y = -1;
    int shown_sprite_x = -1;
    int shown_sprite_y = -1;

    std::string name;

    CollectableProfile()
    {
    }

    static std::map<int, CollectableProfile> read_from_file(const std::string &file)
    {
        std::map<int, CollectableProfile> m;
        int id = 0;
        int order = 0;

        ConfigFile cf([&m, &id, &order](const auto &key, const auto &val)
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
                              else if (val == "armor")
                                  m[id].type = Collectable_ARMOR;
                              else if (val == "coin")
                                  m[id].type = Collectable_COIN;
                              else if (val == "mission")
                                  m[id].type = Collectable_MISSION_ITEM;
                              else if (val == "modify_map")
                                  m[id].type = Collectable_MODIFY_MAP;
                          }
                          if (key == "id")
                          {
                              id = std::stoi(val);
                              m[id].buy_menu_order = order;
                              order++;
                          }
                          if (key == "sprite")
                              m[id].sprite = val;
                          if (key == "name")
                              m[id].name = val;
                          if (key == "sound")
                              m[id].sound = std::stoi(val);
                          if (key == "sound_key")
                              m[id].sound_key = std::stoi(val);
                          if (key == "cost")
                              m[id].cost = std::stoi(val);
                          if (key == "buy_allow_flags")
                              m[id].buy_allow_flags = std::stoi(val);
                          if (key == "flip_props")
                              m[id].flip_props = std::stoi(val);
                          if (key == "hidden_sx")
                              m[id].hidden_sprite_x = std::stoi(val);
                          if (key == "hidden_sy")
                              m[id].hidden_sprite_y = std::stoi(val);
                          if (key == "shown_sx")
                              m[id].shown_sprite_x = std::stoi(val);
                          if (key == "shown_sy")
                              m[id].shown_sprite_y = std::stoi(val);
                      },
                      [](const auto &s) {});
        cf.read_config_file(file);
        return m;
    }
};