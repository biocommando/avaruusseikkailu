#pragma once

#include "Sprite.h"
#include "ConfigFile.h"

class WeaponProfile
{
public:
    Sprite sprite;
    std::string sprite_def_file;
    float damage = 1;
    int shots = 1;
    float random_spread = 0;
    float spread = 0.1;
    float speed = 10;
    float acceleration = 0;
    float angle = 0;
    int blast_radius = 32;
    int reload = 5;
    int alive_timer = 999;
    int alive_timer_variation = 0;
    int number_of_child_particles = 0;
    int child_particle_id = 0;
    int id;
    bool affected_by_gravity = true;
    bool bouncy = false;
    int sound = -1;
    int sound_key = 48;
    std::string name;

    WeaponProfile()
    {}

    static void read_from_file(const std::string &file, std::map<int, WeaponProfile> &output)
    {
        int id = 0;
        ConfigFile cf([&id, &output](const auto &key, const auto &val)
                      {
                          if (key == "damage")
                              output[id].damage = std::stof(val);
                          if (key == "blast_radius")
                              output[id].blast_radius = std::stoi(val);
                          if (key == "shots")
                              output[id].shots = std::stoi(val);
                          if (key == "random_spread")
                              output[id].random_spread = std::stof(val) * ALLEGRO_PI / 180;
                          if (key == "spread")
                              output[id].spread = std::stof(val) * ALLEGRO_PI / 180;
                          if (key == "angle")
                              output[id].angle = std::stof(val) * ALLEGRO_PI / 180;
                          if (key == "speed")
                              output[id].speed = std::stof(val);
                          if (key == "acceleration")
                              output[id].acceleration = std::stof(val);
                          if (key == "reload")
                              output[id].reload = std::stoi(val);
                          if (key == "alive_timer")
                              output[id].alive_timer = std::stoi(val);
                          if (key == "alive_timer_variation")
                              output[id].alive_timer_variation = std::stoi(val);
                          if (key == "number_of_child_particles")
                              output[id].number_of_child_particles = std::stoi(val);
                          if (key == "child_particle_id")
                              output[id].child_particle_id = std::stoi(val);
                          if (key == "id")
                          {
                              id = std::stoi(val);
                              output[id].id = id;
                          }
                          if (key == "sprite")
                              output[id].sprite_def_file = val;
                          if (key == "name")
                              output[id].name = val;
                          if (key == "affected_by_gravity")
                              output[id].affected_by_gravity = std::stoi(val) != 0;
                          if (key == "sound")
                              output[id].sound = std::stoi(val);
                          if (key == "sound_key")
                              output[id].sound_key = std::stoi(val);
                          if (key == "bouncy")
                              output[id].bouncy = std::stoi(val) != 0;
                      },
                      [](const auto &s) {});
        cf.read_config_file(file);
    }
};