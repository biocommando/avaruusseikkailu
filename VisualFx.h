#pragma once

#include <vector>
#include "random.h"
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"

extern float camera_offset_x, camera_offset_y;

struct VisualFx
{
    float x, y, rad;
    float x_delta, y_delta, rad_delta;
    float r, g, b;
    float r_delta, g_delta, b_delta;
    int alive_counter;
};

class VisualFxTool
{
    std::vector<VisualFx> vfx;

    void draw_one(const VisualFx &fx)
    {
        auto col = al_map_rgb_f(fx.r, fx.g, fx.b);
        al_draw_filled_circle(fx.x, fx.y, fx.rad, col);
    }

public:
    void progress_and_draw()
    {
        for (int i = vfx.size() - 1; i >= 0; i--)
        {
            auto &fx = vfx[i];
            if (--fx.alive_counter <= 0)
            {
                vfx.erase(vfx.begin() + i);
                continue;
            }
            draw_one(fx);
            fx.x += fx.x_delta;
            fx.y += fx.y_delta;
            fx.rad += fx.rad_delta;
            fx.r += fx.r_delta;
            fx.g += fx.g_delta;
            fx.b += fx.b_delta;
        }
    }

    void draw()
    {
        for (auto &fx : vfx)
        {
            draw_one(fx);
        }
    }

    VisualFx &add(float x, float y, float rad, float r, float g, float b, int alive = 10)
    {
        vfx.push_back(
            VisualFx{
                x, y, rad, 0, 0, 0, r, g, b, 0, 0, 0, alive});
        return vfx[vfx.size() - 1];
    }

    static void disappear(VisualFx &fx)
    {
        fx.rad_delta = -fx.rad / fx.alive_counter;
    }

    static void fade_to_color(VisualFx &fx, float r, float g, float b)
    {
        fx.r_delta = (r - fx.r) / fx.alive_counter;
        fx.g_delta = (g - fx.g) / fx.alive_counter;
        fx.b_delta = (b - fx.b) / fx.alive_counter;
    }
};
