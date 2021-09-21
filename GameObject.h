#pragma once

#include "TileMap.h"
#include "Sprite.h"
#include <cmath>
#include <memory>
#include "Explosions.h"
#include <functional>
#include "flag_id.h"
#include "VisualFx.h"
#include "SingletonInjector.h"

enum GameObjectType
{
    GameObjectType_PLAYER,
    GameObjectType_ENEMY,
    GameObjectType_COLLECTABLE,
    GameObjectType_PLAYER_SHOT,
    GameObjectType_ENM_SHOT,
};

constexpr float gravity = 0.06f;
constexpr float acceleration_damping = 0.5f;
constexpr float acceleration_max = 0.3f;
constexpr float speed_limit_in_one_dir = 3;

class GameObject
{
    Sprite sprite;
    float x = 0;
    float y = 0;
    float dx = 0;
    float dy = 0;
    float hitbox_w = 0;
    float hitbox_h = 0;
    float direction_angle = 0;
    GameObjectType type;
    int sub_type = 0;
    TileMap *tiles;
    float health = 100;
    float armor = 1;
    std::map<int, int> counters;
    std::map<int, int> flags;
    bool affected_by_gravity = true;
    float acceleration = 0;
    bool is_shot;
    bool is_collectable;
    VisualFxTool *vfx_tool;
    int weapon = 0;

public:
    GameObject(Sprite &sprite, GameObjectType type, float hitbox_w, float hitbox_h)
        : sprite(sprite), hitbox_w(hitbox_w), hitbox_h(hitbox_h), type(type),
          is_shot(type == GameObjectType_PLAYER_SHOT || type == GameObjectType_ENM_SHOT),
          is_collectable(type == GameObjectType_COLLECTABLE)
    {
        vfx_tool = get_singleton<VisualFxTool>();
        tiles = get_singleton<TileMap>();
    }

    void set_position(float x, float y)
    {
        this->x = x;
        this->y = y;
    }

    void increment_position_in_direction(float amt)
    {
        this->x += dx * amt;
        this->y += dy * amt;
    }

    void set_affected_by_gravity(bool b)
    {
        affected_by_gravity = b;
    }

    void set_health(float h)
    {
        health = h;
    }

    void set_armor(float r)
    {
        armor = r;
    }

    void deal_damage(float damage)
    {
        health -= damage * armor;
    }

    float get_health() const
    {
        return health;
    }

    float get_armor() const
    {
        return armor;
    }

    int get_weapon() const
    {
        return weapon;
    }

    void set_weapon(int weapon)
    {
        this->weapon = weapon;
    }

    void set_sub_type(int st)
    {
        sub_type = st;
    }

    int get_sub_type() const
    {
        return sub_type;
    }

    void set_speed(float dx, float dy, int flags = 0)
    {
        if (!(flags & 1))
            this->dx = dx;
        if (!(flags & 2))
            this->dy = dy;
    }

    void set_counter(int id, int value)
    {
        counters[id] = value;
    }

    int get_counter(int id)
    {
        return counters[id];
    }

    void set_flag(int id, int value)
    {
        flags[id] = value;
    }

    void add_to_flag(int id, int value)
    {
        flags[id] += value;
    }

    int get_flag(int id)
    {
        return flags[id];
    }

    void add_speed(float dx, float dy)
    {
        this->dx += dx;
        this->dy += dy;
    }

    void add_speed_in_direction(float speed, float angle = 0)
    {
        dx += speed * sin(direction_angle + angle);
        dy -= speed * cos(direction_angle + angle);
    }

    void accelerate_in_direction(float acceleration, bool limit = true)
    {
        this->acceleration += acceleration;
        if (limit && this->acceleration > acceleration_max)
            this->acceleration = acceleration_max;
    }

    float get_speed_in_direction()
    {
        return sqrt(dx * dx + dy * dy);
    }

    void increment_direction_angle(float angle)
    {
        direction_angle += angle;
        sprite.set_angle(direction_angle);
    }

    void set_direction(float angle)
    {
        direction_angle = angle;
        sprite.set_angle(direction_angle);
    }

    float get_direction() const
    {
        return direction_angle;
    }

    float get_dx() const { return dx; }
    float get_dy() const { return dy; }

    bool get_is_shot() const { return is_shot; }

    GameObjectType get_type()
    {
        return type;
    }

    void progress()
    {
        sprite.progress();
        for (auto &counter : counters)
        {
            if (counter.second > 0)
                counter.second--;
        }
        if (is_shot)
        {
            if (counters[alive_counter_id] == 0)
            {
                health = -1;
                return;
            }
        }
        else
        {
            if (dx > speed_limit_in_one_dir)
                dx = speed_limit_in_one_dir;
            else if (dx < -speed_limit_in_one_dir)
                dx = -speed_limit_in_one_dir;
            if (dy > speed_limit_in_one_dir)
                dy = speed_limit_in_one_dir;
            else if (dy < -speed_limit_in_one_dir)
                dy = -speed_limit_in_one_dir;
        }
        if (is_collectable && y > flags[collectable_original_pos_flag])
        {
            dy = -1 - flags[collectable_float_bounce_amount_flag] / 10.0f;
        }

        if (acceleration > 1e-3)
        {
            add_speed_in_direction(acceleration);
            if (!is_shot)
                acceleration *= acceleration_damping;
        }
        float new_x, new_y;
        int i = 1;
        float ddx = dx;
        float ddy = dy;
        do
        {
            ddx = dx / i;
            ddy = dy / i;
            i++;
        } while (ddx > 5 || ddy > 5);
        for (; i > 0; i--)
        {
            new_y = y + ddy;
            bool did_bounce = false;
            int y_coll = tiles->check_collision(x, new_y, hitbox_w, hitbox_h);
            if (y_coll != 0)
            {
                if (is_shot)
                {
                    if (flags[bouncy_flag])
                    {
                        dy = -0.8 * dy;
                        did_bounce = true;
                    }
                    else
                        health = -1;
                }
                else
                {
                    dy = 0;
                    dx *= 0.9; // friction
                }
                new_y = y;
            }
            new_x = x + ddx;
            int x_coll = tiles->check_collision(new_x, new_y, hitbox_w, hitbox_h);
            if (x_coll != 0)
            {
                if (is_shot)
                {
                    if (flags[bouncy_flag])
                    {
                        dx = -0.8 * dx;
                        did_bounce = true;
                    }
                    else
                        health = -1;
                }
                else
                {
                    dx = 0;
                }
                new_x = x;
            }
            // Hazardous terrain
            if (type == GameObjectType_PLAYER && (x_coll == 2 || y_coll == 2))
            {
                deal_damage(1);
            }
            if ((new_x == x && new_y == y) || did_bounce)
            {
                break;
            }
            x = new_x;
            y = new_y;
        }
        if (affected_by_gravity)
            dy += gravity;
    }

    bool collides(const GameObject &other)
    {
        return check_is_inside_box(other.x, other.y, x - (hitbox_w + other.hitbox_w) / 2,
                                   y - (hitbox_h + other.hitbox_h) / 2, x + (hitbox_w + other.hitbox_w) / 2,
                                   y + (hitbox_h + other.hitbox_h) / 2);
    }

    float get_distance(const GameObject &other)
    {
        return sqrt(get_distance_sqr(other));
    }

    float get_distance_sqr(const GameObject &other)
    {
        return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
    }

    void set_animation(int id)
    {
        sprite.set_animation(id);
    }

    void draw()
    {
        extern float camera_offset_x, camera_offset_y;
        if (!is_shot && !is_collectable)
        {
            const auto sprite_top = y - sprite.get_h() / 2 - camera_offset_y;
            for (int i = health / 20; i > 0; i--)
            {
                const auto col = al_map_rgb_f((5 - i) / 5.0f, i / 5.0f, 0);
                const auto x0 = x - 15 + i * 5 - camera_offset_x;
                al_draw_filled_rectangle(x0, sprite_top - 8, x0 + 3, sprite_top - 3, col);
            }
            if (acceleration > 0.1)
            {
                const auto ddx = sin(direction_angle + ALLEGRO_PI);
                const auto ddy = -cos(direction_angle + ALLEGRO_PI);
                auto &fx = vfx_tool->add(x + ddx * sprite.get_w() / 2,
                                         y + ddy * sprite.get_h() / 2,
                                         6, .42, .89, 1, randomint(15, 20));
                VisualFxTool::disappear(fx);
                VisualFxTool::fade_to_color(fx, 0, 0, 0);
            }
        }
        else if (is_shot)
        {
            auto &fx = vfx_tool->add(x + random(-1, 1), y + random(-1, 1),
                                     3, random(0.35, 0.8), random(0.15, 0.6),
                                     0, randomint(4, 6));
            VisualFxTool::disappear(fx);
            VisualFxTool::fade_to_color(fx, 0, 0, 0);
        }
        sprite.draw(x, y);
    }

    float get_x() const { return x; }

    float get_y() const { return y; }

    float get_hitbox_w() const { return hitbox_w; }

    float get_hitbox_h() const { return hitbox_h; }

    float get_hitbox_max_dim() const { return hitbox_h > hitbox_w ? hitbox_h : hitbox_w; }
};

class GameObjectHolder
{
    std::map<GameObjectType, std::vector<GameObject *>> objects;

public:
    GameObject &add_object(GameObject *obj)
    {
        auto &ct = get_category(obj->get_type());
        ct.push_back(obj);
        return *ct.back();
    }

    std::vector<GameObject *> &get_category(GameObjectType category)
    {
        return objects[category];
    }

    void draw()
    {
        for (auto &objv : objects)
        {
            for (auto &obj : objv.second)
            {
                obj->draw();
            }
        }
    }

    void progress()
    {
        for (auto &objv : objects)
        {
            for (auto &obj : objv.second)
            {
                obj->progress();
            }
        }
    }

    void clean_up(GameObjectType category, const std::function<void(GameObject *)> &onDestroy)
    {
        auto &objv = get_category(category);
        for (int i = objv.size() - 1; i >= 0; i--)
        {
            auto obj = objv[i];
            if (obj->get_health() < 0)
            {
                objv.erase(objv.begin() + i);
                onDestroy(obj);
                delete obj;
            }
        }
    }

    void clean_up(const std::vector<GameObjectType> &categories, const std::function<void(GameObject *)> &onDestroy)
    {
        for (auto &c : categories)
        {
            clean_up(c, onDestroy);
        }
    }

    ~GameObjectHolder()
    {
        for (auto &objv : objects)
        {
            for (auto &obj : objv.second)
            {
                delete obj;
            }
        }
    }
};
