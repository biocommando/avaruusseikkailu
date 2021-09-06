#pragma once
#include "Sprite.h"

constexpr bool check_is_inside_box(float point_x, float point_y, float x0, float y0, float x1, float y1)
{
    return point_x > x0 && point_x < x1 && point_y > y0 && point_y < y1;
}

constexpr int TILE_WALL = 1;
constexpr int TILE_HAZARD = 2;

class Tile
{
    int x;
    int y;
    int x2;
    int y2;
    TileSprite s;
    int properties;

public:
    Tile(int x, int y, int w, int h, TileSprite &sprite, int properties)
        : x(x), y(y), x2(x + w + 1), y2(y + h + 1), s(sprite), properties(properties)
    {
    }

    void draw()
    {
        s.draw(x, y);
    }

    int check_collision(float cx, float cy, float hb_w, float hb_h)
    {
        if (properties == 0)
            return 0;
        if (check_is_inside_box(cx + hb_w / 2, cy + hb_h / 2, x, y, x2, y2)    //
            || check_is_inside_box(cx + hb_w / 2, cy - hb_h / 2, x, y, x2, y2) //
            || check_is_inside_box(cx - hb_w / 2, cy + hb_h / 2, x, y, x2, y2) //
            || check_is_inside_box(cx - hb_w / 2, cy - hb_h / 2, x, y, x2, y2))
            return properties;
        return 0;
    }

    void debug()
    {
        std::cout << "TILE@" << std::to_string(x) << ',' << std::to_string(y)
                  << "->" << std::to_string(x2) << ',' << std::to_string(y2)
                  << ", props=" << std::to_string(properties) << std::endl;
        s.debug();
    }
};

struct InitialObjectPlacement
{
public:
    int type;
    int x;
    int y;
};

class TileMap
{
    std::vector<Tile> tiles;

    int w = 0;
    int h = 0;

public:
    std::vector<InitialObjectPlacement> initial_object_placement;
    void read_from_file(const std::string &file)
    {
        std::ifstream ifs;
        ifs.open(file);
        std::string s;

        std::map<std::string, std::string> m;

        while (std::getline(ifs, s))
        {
            if (s == "set tile")
            {
                auto w = std::stoi(m["w"]);
                auto h = std::stoi(m["h"]);
                TileSprite ts(m["sprite_sheet"], w, h, std::stoi(m["sx"]), std::stoi(m["sy"]));
                auto x = std::stoi(m["x"]);
                auto y = std::stoi(m["y"]);
                if (x + w > this->w)
                    this->w = x + w;
                if (y + h > this->h)
                    this->h = y + h;
                Tile t(x, y, w, h, ts, std::stoi(m["props"]));
                tiles.push_back(t);
            }
            else if (s == "set object")
            {
                InitialObjectPlacement iop { std::stoi(m["obj_type"]), std::stoi(m["obj_x"]), std::stoi(m["obj_y"]) };
                initial_object_placement.push_back(iop);
            }
            else
            {
                const auto pos = s.find('=');
                if (pos != std::string::npos)
                {
                    const auto key = s.substr(0, pos);
                    const auto val = s.substr(pos + 1);
                    m[key] = val;
                }
            }
        }
    }

    void draw()
    {
        for (auto &t : tiles)
        {
            t.draw();
        }
    }

    int check_collision(float cx, float cy, float hb_w, float hb_h)
    {
        for (auto &t : tiles)
        {
            const auto p = t.check_collision(cx, cy, hb_w, hb_h);
            if (p != 0)
                return p;
        }
        return 0;
    }

    void debug()
    {
        std::cout << "TileMap" << std::endl;
        for (auto &t : tiles)
            t.debug();
    }

    int get_w() { return w; }

    int get_h() { return h; }
};