#pragma once

#include <cmath>
#include "GameObject.h"

inline float angle_between(float dirX1, float dirY1, float dirX2, float dirY2)
{
    auto dotProduct = dirX1 * dirX2 + dirY1 * dirY2;
    auto abs1Pow2 = dirX1 * dirX1 + dirY1 * dirY1;
    auto abs2Pow2 = dirX2 * dirX2 + dirY2 * dirY2;
    return acos(dotProduct / sqrt(abs1Pow2 * abs2Pow2));
}

inline void ai_check_visible(GameObject &ai, GameObject &player, TileMap &tiles)
{
    if (ai.get_counter(ai_check_visible_counter_id) == 0)
    {
        const auto distance = player.get_distance(ai) + 1e-6;
        const auto dx = (player.get_x() - ai.get_x()) / distance;
        const auto dy = (player.get_y() - ai.get_y()) / distance;

        float x = ai.get_x();
        float y = ai.get_y();
        bool clear_path = true;
        for (float a = 0; a < distance; a += 16)
        {
            x += dx * 16;
            y += dy * 16;
            if (tiles.check_point_props(x, y) != 0)
            {
                clear_path = false;
                break;
            }
        }
        ai.set_flag(ai_sees_player_flag, clear_path ? 1 : 0);

        ai.set_counter(ai_check_visible_counter_id, 5);
    }
}

inline void ship_ai(GameObject &ship, GameObject &player)
{
    const auto dx = player.get_x() - ship.get_x();
    const auto dy = player.get_y() - ship.get_y();
    const auto dir_x = -sin(ship.get_direction() + ALLEGRO_PI);
    const auto dir_y = cos(ship.get_direction() + ALLEGRO_PI);
    if (ship.get_counter(ai_change_direction_counter_id) <= 0)
    {
        const auto distance_squared = dx * dx + dy * dy;
        ship.set_flag(ai_distance_flag, distance_squared);
        if (distance_squared < 512 * 512 && ship.get_flag(ai_sees_player_flag))
        {
            ship.set_counter(ai_continue_trajectory_after_losing_sight_counter, 100);
            const auto a = dir_y / dir_x;
            const auto b = ship.get_y() - a * ship.get_x();
            const auto eq = a * player.get_x() + b;
            if (dir_x > 0)
            {
                if (eq < player.get_y())
                {
                    ship.set_flag(ai_preferred_direction_flag, 'R');
                }
                else
                {
                    ship.set_flag(ai_preferred_direction_flag, 'L');
                }
            }
            else
            {
                if (eq > player.get_y())
                {
                    ship.set_flag(ai_preferred_direction_flag, 'R');
                }
                else
                {
                    ship.set_flag(ai_preferred_direction_flag, 'L');
                }
            }
        }
        else
        {
            ship.set_flag(ai_preferred_direction_flag, 0);
        }
        ship.set_counter(ai_change_direction_counter_id, 3);
    }
    const auto preferred_dir = ship.get_flag(ai_preferred_direction_flag);

    if (preferred_dir != 0)
    {
        auto angle_difference_from_target = angle_between(dir_x, dir_y, dx, dy);
        if (angle_difference_from_target > 0.1)
        {
            if (preferred_dir == 'L')
                ship.increment_direction_angle(-ALLEGRO_PI / 32);
            else if (preferred_dir == 'R')
                ship.increment_direction_angle(ALLEGRO_PI / 32);
        }
        else
        {
            if (ship.get_flag(ai_distance_flag) > 64 * 64 && ship.get_speed_in_direction() < 8)
                //ship.add_speed_in_direction(0.5);
                ship.accelerate_in_direction(0.15);
        }
        if (angle_difference_from_target < 0.2 && ship.get_counter(reload_counter_id) == 0)
        {
            ship.set_flag(ai_wants_to_shoot_flag, 1);
        }
        else
        {
            ship.set_flag(ai_wants_to_shoot_flag, 0);
        }
    }
    else
    {
        ship.set_flag(ai_wants_to_shoot_flag, 0);
        if (ship.get_counter(ai_continue_trajectory_after_losing_sight_counter) > 0 &&
            ship.get_speed_in_direction() < 8)
            ship.accelerate_in_direction(0.15);
    }
}

inline void soldier_ai(GameObject &soldier, GameObject &player, TileMap &tm)
{
    const auto shooting_dist = fabs(soldier.get_x() - player.get_x()) < 120;
    if (soldier.get_counter(reload_counter_id) == 0 && soldier.get_flag(ai_sees_player_flag))
    {
        if (soldier.get_y() > player.get_y() && shooting_dist)
        {
            soldier.set_flag(ai_wants_to_shoot_flag, 1);
            soldier.set_counter(ai_soldier_shoot_anim_counter_id, 5);
        }
        else
        {
            soldier.set_flag(ai_wants_to_shoot_flag, 0);
        }
    }
    else
    {
        soldier.set_flag(ai_wants_to_shoot_flag, 0);
    }
    const auto dir = soldier.get_flag(ai_preferred_direction_flag);
    const auto dy = soldier.get_dy();
    const auto dx = soldier.get_dx();
    if (dir == 0)
    {
        soldier.set_flag(ai_preferred_direction_flag, random() < 0.5 ? 'R' : 'L');
    }
    else
    {
        if (dir == 'L')
        {
            soldier.set_speed(-2, 0, 2);
            if (shooting_dist)
                soldier.set_animation(3);
            else
                soldier.set_animation(1);
        }
        else
        {
            soldier.set_speed(2, 0, 2);
            if (shooting_dist)
                soldier.set_animation(2);
            else
                soldier.set_animation(0);
        }
    }

    if (!tm.check_collision(soldier.get_x() + soldier.get_dx() * 2, soldier.get_y() + 1, soldier.get_hitbox_w(), soldier.get_hitbox_h()) ||
        tm.check_collision(soldier.get_x() + soldier.get_dx() * 2, soldier.get_y(), soldier.get_hitbox_w(), soldier.get_hitbox_h()))
    {
        soldier.set_speed(-soldier.get_dx(), soldier.get_dy());
        soldier.set_flag(ai_preferred_direction_flag, dir == 'R' ? 'L' : 'R');
    }
}

inline void tank_ai(GameObject &tank, GameObject &player)
{
    const auto distance = tank.get_distance(player);
    const auto shooting_range = distance < 512;
    bool target_in_sights = false;
    if (shooting_range && tank.get_flag(ai_sees_player_flag))
    {
        const auto dx = fabs(player.get_x() - tank.get_x());
        const auto current_dir = tank.get_direction() - (int)(tank.get_direction() / (2 * ALLEGRO_PI)) * 2 * ALLEGRO_PI;
        auto target_dir =
            tank.get_y() > player.get_y()
                ? asin(dx / tank.get_distance(player))
                : acos(dx / tank.get_distance(player)) + ALLEGRO_PI / 2;
        if (tank.get_x() > player.get_x())
            target_dir = -target_dir;
        const auto dir_diff = target_dir - current_dir;
        if (fabs(dir_diff) > 0.1)
            tank.set_direction(current_dir + (target_dir - current_dir) * 0.2);
        else
        {
            tank.set_direction(target_dir);
            target_in_sights = true;
        }
    }
    if (tank.get_counter(reload_counter_id) == 0)
    {
        if (target_in_sights)
        {
            tank.set_flag(ai_wants_to_shoot_flag, 1);
            tank.set_counter(ai_soldier_shoot_anim_counter_id, 5);
        }
        else
        {
            tank.set_flag(ai_wants_to_shoot_flag, 0);
        }
    }
    else
    {
        tank.set_flag(ai_wants_to_shoot_flag, 0);
    }
}