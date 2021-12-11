#pragma once
#include <string>
#include <map>

inline std::map<std::string, std::string> parse_arguments(int argc, char **argv)
{
    std::map<std::string, std::string> args;
    std::string arg = "";
    for (int i = 0; i < argc; i++)
    {
        if (arg != "")
        {
            args[arg] = argv[i];
            arg = "";
        }
        if (argv[i][0] == '-')
            arg = argv[i];
    }
    return args;
}