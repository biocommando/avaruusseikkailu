#pragma once
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

extern float camera_offset_x, camera_offset_y;

inline ALLEGRO_BITMAP *load_bitmap(const std::string &file = "unload")
{
    static std::map<std::string, ALLEGRO_BITMAP *> bitmaps;
    if (file == "unload")
    {
        for (const auto &kv : bitmaps)
        {
            al_destroy_bitmap(kv.second);
        }
        return nullptr;
    }
    if (bitmaps.find(file) == bitmaps.end())
    {
        bitmaps[file] = al_load_bitmap(file.c_str());
    }
    return bitmaps[file];
}

inline void unload_bitmaps()
{
    load_bitmap();
}

struct AnimationFrame
{
    int x;
    int y;
    int w;
    int h;
    int animation_id;
    int length;
    int first_of_animation;
    bool has_next;
    int flip;
};

class Sprite
{
    ALLEGRO_BITMAP *bitmap = nullptr;

    std::vector<AnimationFrame> frames;

    int frame = -1;
    int timer = 0;
    int sx;
    int sy;
    int flip;
    float zoom = 1;
    float angle = 0;
    bool simple = true;
    float w;
    float h;

    void sync()
    {
        sx = frames[frame].x;
        sy = frames[frame].y;
        w = frames[frame].w;
        h = frames[frame].h;
        flip = frames[frame].flip;
        timer = frames[frame].length;
    }

    void add_animation_frame(int x, int y, int w, int h, int animation_id, int length, int flip = 0)
    {
        auto f = AnimationFrame{x, y, w, h, animation_id, length, (int)frames.size(), false, flip};
        if (frames.size() > 0 && frames[frames.size() - 1].animation_id == animation_id)
        {
            f.first_of_animation = frames[frames.size() - 1].first_of_animation;
            frames[frames.size() - 1].has_next = true;
        }
        frames.push_back(f);
        frame = 0;
    }
public:
    Sprite()
    {
    }

    Sprite(ALLEGRO_BITMAP *bitmap, float w, float h) : bitmap(bitmap), w(w), h(h)
    {
    }

    Sprite(const std::string &init_file)
    {
        std::ifstream ifs;
        ifs.open(init_file);
        std::string s;
        int x, y, id, length, flip;
        while (std::getline(ifs, s))
        {
            if (s == "end")
            {
                add_animation_frame(x, y, w, h, id, length, flip);
                continue;
            }
            const auto pos = s.find('=');
            if (pos != std::string::npos)
            {
                const auto key = s.substr(0, pos);
                const auto val = s.substr(pos + 1);
                if (key == "file")
                    bitmap = load_bitmap(val);
                if (key == "w")
                    w = std::stof(val);
                if (key == "h")
                    h = std::stof(val);
                if (key == "x")
                    x = std::stoi(val);
                if (key == "y")
                    y = std::stoi(val);
                if (key == "length")
                    length = std::stoi(val);
                if (key == "flip")
                {
                    flip = 0;
                    if (val.find("vertical") != std::string::npos)
                        flip |= ALLEGRO_FLIP_VERTICAL;
                    if (val.find("horizontal") != std::string::npos)
                        flip |= ALLEGRO_FLIP_HORIZONTAL;
                }
                if (key == "animation_id")
                    id = std::stoi(val);
            }
        }
    }

    void set_zoom(float factor)
    {
        zoom = factor;
        simple = false;
    }

    void set_angle(float rad)
    {
        angle = rad;
        simple = false;
    }

    void debug()
    {
        std::cout << "Sprite:" << std::endl;
        std::cout << "sx=" << std::to_string(sx) << ",sy=" << std::to_string(sy)
                  << ",frame=" << std::to_string(frame) << ",zoom=" << std::to_string(zoom)
                  << ",angle=" << std::to_string(angle) << std::endl;
        for (const auto &f : frames)
        {
            std::cout << " *FRAME: id=" << std::to_string(f.animation_id)
                      << ", first=" << std::to_string(f.first_of_animation)
                      << ", has_next=" << std::to_string(f.has_next)
                      << ", length=" << std::to_string(f.length)
                      << ", x=" << std::to_string(f.x)
                      << ", y=" << std::to_string(f.y) << std::endl;
        }
    }

    void set_animation(int id)
    {
        if (frames[frame].animation_id == id)
            return;
        for (frame = 0; frame < frames.size(); frame++)
        {
            if (frames[frame].animation_id == id)
                break;
        }
        sync();
    }

    void progress()
    {
        if (frame >= 0)
        {
            timer--;
            if (timer <= 0)
            {
                if (frames[frame].has_next)
                    frame++;
                else
                    frame = frames[frame].first_of_animation;
                sync();
            }
        }
    }

    void draw(float x, float y)
    {
        if (simple)
            al_draw_bitmap_region(bitmap, sx, sy, w, h, x - w / 2, y - h / 2, flip);
        else
            al_draw_tinted_scaled_rotated_bitmap_region(bitmap, sx, sy, w, h, al_map_rgb_f(1, 1, 1), w / 2, h / 2, x, y, zoom, zoom, angle, flip);
    }

    float get_w() const { return w; }
    float get_h() const { return h; }
};

class TileSprite
{
    ALLEGRO_BITMAP *bitmap = nullptr;
    int w;
    int h;
    int sx = 0;
    int sy = 0;

public:
    TileSprite(const std::string &file, int w, int h, int sx, int sy) : w(w), h(h), sx(sx), sy(sy)
    {
        bitmap = load_bitmap(file);
    }

    void draw(float x, float y)
    {
        al_draw_bitmap_region(bitmap, sx, sy, w, h, x, y, 0);
    }

    void debug()
    {
        std::cout << "TileSprite w=" << std::to_string(w) << ",h=" << std::to_string(h)
                  << ",sx=" << std::to_string(sx) << ",sy=" << std::to_string(sy) << std::endl;
    }
};