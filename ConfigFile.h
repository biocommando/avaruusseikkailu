#pragma once
#include <string>
#include <fstream>
#include <map>
#include <functional>

class ConfigFile
{
    std::function<void(const std::string &, const std::string &)> keyValue;
    std::function<void(const std::string &)> noKeyValue;

public:
    ConfigFile(std::function<void(const std::string &, const std::string &)> keyValue, std::function<void(const std::string &)> noKeyValue)
        : keyValue(keyValue), noKeyValue(noKeyValue)
    {
    }

    void read_config_file(const std::string &file_name)
    {
        std::ifstream ifs;
        ifs.open(file_name);
        std::string s;
        while (std::getline(ifs, s))
        {
            if (s == "" || s[0] == '#')
                continue;

            const auto pos = s.find('=');

            if (pos != std::string::npos)
            {
                const auto key = s.substr(0, pos);
                const auto val = s.substr(pos + 1);
                keyValue(key, val);
            }
            else
            {
                noKeyValue(s);
            }
        }
    }
};