#pragma once
#include "Sprite.h"

constexpr bool check_is_inside_box(float point_x, float point_y, float x0, float y0, float x1, float y1)
{
    return point_x > x0 && point_x < x1 && point_y > y0 && point_y < y1;
}

constexpr int TILE_WALL = 1;
constexpr int TILE_HAZARD = 2;

class TileBase
{
protected:
    int x;
    int y;
    int x2;
    int y2;
    int properties;

public:
    TileBase(int x, int y, int x2, int y2, int properties)
        : x(x), y(y), x2(x2), y2(y2), properties(properties)
    {
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

    int check_point_inside(float xx, float yy)
    {
        if (properties == 0)
            return 0;
        if (check_is_inside_box(xx, yy, x, y, x2, y2))
            return properties;
        return 0;
    }

    int get_x() const { return x; }
    int get_y() const { return y; }
    int get_x2() const { return x2; }
    int get_y2() const { return y2; }
    int get_properties() const { return properties; }

    void debug()
    {
        std::cout << "TILE@" << std::to_string(x) << ',' << std::to_string(y)
                  << "->" << std::to_string(x2) << ',' << std::to_string(y2)
                  << ", props=" << std::to_string(properties) << std::endl;
    }
};

class Tile : public TileBase
{
    TileSprite s;

public:
    Tile(int x, int y, int w, int h, TileSprite &sprite, int properties)
        : TileBase(x, y, x + w + 1, y + h + 1, properties), s(sprite)
    {
    }

    void draw()
    {
        s.draw(x, y);
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
    // These are the actual tiles against which the collisions are checked.
    // Maybe a more intelligent algorithm could be used but now just consequent tiles
    // in a row are merged as one logical tile.
    // The combining works only for tiles that are exactly aligned to the grid.
    // Other tiles will be used just as is.
    std::vector<TileBase> logical_tiles;

    int w = 0;
    int h = 0;

    void combine_tiles_to_logical_tiles()
    {
        std::map<int, bool> combined_tiles;
        for (int y = 0; y < h; y += 32)
        {
            int x1 = -1, x2 = -1, properties = -1;
            for (int x = 0; x < w; x += 32)
            {
                for (int i = 0; i < tiles.size(); i++)
                {
                    const auto &tile = tiles[i];
                    if (tile.get_y() == y && tile.get_y2() == y + 32 + 1 &&
                        tile.get_x() == x && tile.get_x2() == x + 32 + 1 &&
                        !combined_tiles[i])
                    {
                        combined_tiles[i] = true;
                        if (x2 != x + 1 || properties != tile.get_properties())
                        {
                            if (x1 != -1)
                            {
                                logical_tiles.push_back(TileBase(x1, y, x2, y + 32 + 1, properties));
                            }
                            x1 = tile.get_x();
                            x2 = tile.get_x2();
                            properties = tile.get_properties();
                        }
                        else
                        {
                            x2 = tile.get_x2();
                        }
                    }
                }
            }
            logical_tiles.push_back(TileBase(x1, y, x2, y + 32 + 1, properties));
        }
        for (int i = 0; i < tiles.size(); i++)
        {
            if (!combined_tiles[i])
            {
                const auto &tile = tiles[i];
                logical_tiles.push_back(TileBase(tile.get_x(), tile.get_y(), tile.get_x2(),
                                                 tile.get_y2(), tile.get_properties()));
            }
        }
        std::cout << "Tile count: " << std::to_string(tiles.size())
             << ", Combined count: " << std::to_string(logical_tiles.size())
              << ", Compression %: " << std::to_string(100 - 100 * logical_tiles.size() / tiles.size()) <<'\n';
    }

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
                InitialObjectPlacement iop{std::stoi(m["obj_type"]), std::stoi(m["obj_x"]), std::stoi(m["obj_y"])};
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
        combine_tiles_to_logical_tiles();
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
        for (auto &t : logical_tiles)
        {
            const auto p = t.check_collision(cx, cy, hb_w, hb_h);
            if (p != 0)
                return p;
        }
        return 0;
    }

    int check_point_props(float x, float y)
    {
        for (auto &t : logical_tiles)
        {
            const auto p = t.check_point_inside(x, y);
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
        std::cout << "TileMap, logical" << std::endl;
        for (auto &t : logical_tiles)
            t.debug();
    }

    int get_w() { return w; }

    int get_h() { return h; }
};