#pragma once

#include "ConfigFile.h"

class KeyConfig
{
public:
    int up;
    int down;
    int left;
    int right;
    int strafe;
    int shoot;

    KeyConfig(const std::string &file)
    {
        ConfigFile cf([this](const auto &key, const auto &val)
                      {
                          if (key == "up")
                            up = std::stoi(val);
                          if (key == "down")
                            down = std::stoi(val);
                          if (key == "left")
                            left = std::stoi(val);
                          if (key == "right")
                            right = std::stoi(val);
                          if (key == "strafe")
                            strafe = std::stoi(val);
                          if (key == "shoot")
                            shoot = std::stoi(val);
                      },
                      [](const auto &s) {});
        cf.read_config_file(file);
    }
};