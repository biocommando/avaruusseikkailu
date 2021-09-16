#pragma once

#include "ConfigFile.h"

class EnemyProfile
{
public:
    std::string sprite;
    std::string name;
    int weapon;
    int health;
    int hitbox_w;
    int hitbox_h;
    bool hitbox_from_sprite = true;
    float sprite_zoom = -1;
    std::string type;

    static std::map<int, EnemyProfile> read_from_file(const std::string &file)
    {
        std::map<int, EnemyProfile> m;
        int id = 0;
        ConfigFile cf([&m, &id](const auto &key, const auto &val)
                      {
                          if (key == "id")
                              id = std::stoi(val);
                          if (key == "type")
                              m[id].type = val;
                          if (key == "sprite")
                              m[id].sprite = val;
                          if (key == "name")
                              m[id].name = val;
                          if (key == "health")
                              m[id].health = std::stoi(val);
                          if (key == "weapon")
                              m[id].weapon = std::stoi(val);
                          if (key == "hitbox_w")
                          {
                              m[id].hitbox_w = std::stoi(val);
                              m[id].hitbox_from_sprite = false;
                          }
                          if (key == "hitbox_h")
                          {
                              m[id].hitbox_h = std::stoi(val);
                              m[id].hitbox_from_sprite = false;
                          }
                          if (key == "sprite_zoom")
                              m[id].sprite_zoom = std::stof(val);
                      },
                      [](const auto &s) {});
        cf.read_config_file(file);
        return m;
    }
};