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
    std::string name;

    WeaponProfile()
    {}

    WeaponProfile(const std::string &file)
    {
        ConfigFile cf([this](const auto &key, const auto &val)
                      {
                          if (key == "damage")
                              this->damage = std::stof(val);
                          if (key == "blast_radius")
                              this->blast_radius = std::stoi(val);
                          if (key == "shots")
                              this->shots = std::stoi(val);
                          if (key == "random_spread")
                              this->random_spread = std::stof(val) * ALLEGRO_PI / 180;
                          if (key == "spread")
                              this->spread = std::stof(val) * ALLEGRO_PI / 180;
                          if (key == "angle")
                              this->angle = std::stof(val) * ALLEGRO_PI / 180;
                          if (key == "speed")
                              this->speed = std::stof(val);
                          if (key == "reload")
                              this->reload = std::stoi(val);
                          if (key == "alive_timer")
                              this->alive_timer = std::stoi(val);
                          if (key == "alive_timer_variation")
                              this->alive_timer_variation = std::stoi(val);
                          if (key == "number_of_child_particles")
                              this->number_of_child_particles = std::stoi(val);
                          if (key == "child_particle_id")
                              this->child_particle_id = std::stoi(val);
                          if (key == "id")
                              this->id = std::stoi(val);
                          if (key == "sprite")
                              this->sprite_def_file = val;
                          if (key == "name")
                              this->name = val;
                          if (key == "affected_by_gravity")
                              this->affected_by_gravity = std::stoi(val) != 0;
                          if (key == "bouncy")
                              this->bouncy = std::stoi(val) != 0;
                      },
                      [](const auto &s) {});
        cf.read_config_file(file);
    }
};