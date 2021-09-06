#pragma once

#include <vector>
#include "random.h"
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"

struct explosion_circle
{
    // Relative position
    float x, y;
    // Intensity
    float i;
    // Radius
    float r;
};

typedef struct
{
    int exists;
    int x;
    int y;
    int phase;
    int circle_count;
    struct explosion_circle circles[10];
} Explosion;

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

inline int comp_expl_circle(const void *elem1, const void *elem2)
{
    if (((struct explosion_circle *)elem1)->i > ((struct explosion_circle *)elem2)->i)
        return 1;
    return -1;
}

inline Explosion create_explosion(int x, int y, float base_scale = 1)
{
    static int explosion_counter = 0;
    const float circle_max_radius = 36;

    Explosion ex;

    ex.x = x + randomint(-32, 32);
    ex.y = y + randomint(-32, 32);
    ex.phase = randomint(0, 4);

    ex.circle_count = 5 + rand() % 6;
    double scale = random() + base_scale;
    for (int i = 0; i < ex.circle_count; i++)
    {
        struct explosion_circle *c = &ex.circles[i];
        c->i = random() * 0.25 + 0.75;
        c->x = circle_max_radius / 2 + (1 - 2 * random()) * circle_max_radius * scale;
        c->y = circle_max_radius / 2 + (1 - 2 * random()) * circle_max_radius * scale;
        c->r = MAX(random() * circle_max_radius * scale, 5);
    }
    // Sort so that most intense are on top (last)
    qsort(ex.circles, ex.circle_count, sizeof(struct explosion_circle), comp_expl_circle);

    ex.exists = 1;
    return ex;
}

extern float camera_offset_x, camera_offset_y;

inline void draw_explosion_circle(float x, float y, float intensity, float radius)
{
    const float red = MIN((intensity * 0.5 + 0.5), 1);
    const float sqintens = intensity * intensity;
    const float green = MIN((sqintens * 0.8 + 0.2), 1);
    const float blue = MIN((sqintens * sqintens * sqintens * 0.8), 1);

    const auto col = al_map_rgb_f(red, green, blue);

    x -= camera_offset_x;
    y -= camera_offset_y;

    al_draw_filled_circle(x - 32, y - 32, radius, col);
}

inline void progress_and_draw_explosions(std::vector<Explosion> &explosions)
{
    const float circle_max_radius = 36;
    for (int i = explosions.size() - 1; i >= 0; i--)
    {
        Explosion *ex = &explosions[i];

        for (int j = 0; j < ex->circle_count; j++)
        {
            struct explosion_circle *c = &ex->circles[j];

            while (c->x + c->r > circle_max_radius * 2 - 1)
                c->x -= 1;
            while (c->x - c->r < 0)
                c->x += 1;
            while (c->y + c->r > circle_max_radius * 2 - 1)
                c->y -= 1;
            while (c->y - c->r < 0)
                c->y += 1;

            draw_explosion_circle(c->x + ex->x, c->y + ex->y, c->i * .9, c->r);
            draw_explosion_circle(c->x + ex->x, c->y + ex->y, c->i, c->r * .8);
            draw_explosion_circle(c->x + ex->x, c->y + ex->y, c->i * 1.1, c->r * .7);

            // Fade

            c->x = c->x > circle_max_radius / 2 ? c->x + random() * 3 : c->x - random() * 3;
            c->y = c->y > circle_max_radius / 2 ? c->y + random() * 3 : c->y - random() * 3;
            float multiplier_factor = log(sqrt(j) + 2) / 15;
            float intensity_multiplier = (1 - multiplier_factor) + random() * multiplier_factor;
            c->i *= intensity_multiplier;
            c->r *= intensity_multiplier;
        }

        if (++ex->phase >= 30)
        {
            ex->exists = 0;
            explosions.erase(explosions.begin() + i);
        }
    }
}