#pragma once
#include <string>
#include <vector>
#include "allegro5/allegro_font.h"
extern float camera_offset_x, camera_offset_y;

struct TimedPermanentText
{
    int time;
    ALLEGRO_COLOR color;
    std::string text;
    int flags;
    float x;
    float y;
    float fade_delta;
    bool use_camera_offset;
};

class TextDrawer
{
private:
    ALLEGRO_FONT *font = nullptr;
    ALLEGRO_COLOR color;
    int flags = 0;
    bool use_camera_offset = true;
    std::vector<TimedPermanentText> timed_texts;

public:
    TextDrawer() : font(al_create_builtin_font()), color(al_map_rgb_f(1, 1, 1))
    {
    }

    ~TextDrawer()
    {
        al_destroy_font(font);
    }

    void draw_text(float x, float y, const std::string &text)
    {
        if (!use_camera_offset)
        {
            x += camera_offset_x;
            y += camera_offset_y;
        }
        al_draw_text(font, color, x, y, flags, text.c_str());
    }

    void add_timed_permanent_text(float x, float y, const std::string &text, int time = 30)
    {
        TimedPermanentText t{time, color, text, flags, x, y, 1.0f / time, use_camera_offset};
        timed_texts.push_back(t);
    }

    void draw_timed_permanent_texts()
    {
        for (int i = timed_texts.size() - 1; i >= 0; i--)
        {
            const auto t = &timed_texts[i];

            auto x = t->x;
            auto y = t->y;
            if (!t->use_camera_offset)
            {
                x += camera_offset_x;
                y += camera_offset_y;
            }

            al_draw_text(font, t->color, x, y, t->flags, t->text.c_str());

            t->color.r -= t->fade_delta;
            if (t->color.r < 0)
                t->color.r = 0;
            t->color.g -= t->fade_delta;
            if (t->color.g < 0)
                t->color.g = 0;
            t->color.b -= t->fade_delta;
            if (t->color.b < 0)
                t->color.b = 0;
            t->time--;
            if (t->time <= 0)
            {
                timed_texts.erase(timed_texts.begin() + i);
            }
        }
    }

    void set_color(float r, float g, float b)
    {
        color = al_map_rgb_f(r, g, b);
    }

    void set_centered(bool centered)
    {
        flags = centered ? ALLEGRO_ALIGN_CENTER : 0;
    }

    void set_use_camera_offset(bool yes)
    {
        use_camera_offset = yes;
    }
};