#pragma once

#include "Sprite.h"
#include "TileMap.h"
#include "GameObject.h"
#include "random.h"
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "Explosions.h"
#include "WeaponProfile.h"
#include "flag_id.h"
#include "EnemyAi.h"
#include "EnemyProfile.h"
#include "KeyConfig.h"
#include "MissionConfig.h"
#include "TextDrawer.h"
#include "MidiTracker.h"
#include "VisualFx.h"
#include "SingletonInjector.h"
#include "CollectableProfile.h"
#include "Playlist.h"
#include <sstream>
#include "log.h"

extern int screen_w, screen_h;

struct PlrWeaponStatus
{
    bool owned;
    int ammo;
};

class World
{
    std::map<std::string, Sprite> sprites;
    int time_limit = -1;
    int survive_limit = -1;
    int kill_target = -1;
    int retrieve_target = -1;
    int mission_time = 0;
    int number_of_goals = 0;
    bool last_thrust_key_status = false;

    std::map<int, EnemyProfile> enemy_profiles;
    std::map<int, CollectableProfile> collectable_profiles;
    std::vector<int> collectable_profiles_in_buy_order;
    bool show_inventory = false;
    int inventory_cursor = 1;
    CollectableType inventory_category = Collectable_AMMO;
    int inventory_cursor_max = 9;
    int inventory_counter = 0;
    bool inventory_show_status = true;

    // Lower priority = more important
    std::map<int, float> target_priorities = {
        {enemy_type_ship, 1},
        {enemy_type_tank, 5},
        {enemy_type_soldier, 7},
        {enemy_type_building, 100},
    };
    GameObject *auto_aim_target = nullptr;

    std::map<int, PlrWeaponStatus> player_weapon_status;

public:
    std::map<int, WeaponProfile> weapon_profiles;
    TileMap tile_map;
    GameObjectHolder game_object_holder;
    std::vector<Explosion> explosions;
    GameObject *player = nullptr;
    KeyConfig key_config;
    MissionConfig mission_config;
    TextDrawer text_drawer;
    MidiTracker midi_tracker;
    VisualFxTool vfx_tool;
    // 0 = succeeded, -1 = failed, > 0 = goals unmet
    int goal_status = 0;

    World() : key_config("config/key_config.ini"), midi_tracker(44100)
    {
        text_drawer.set_centered(true);
        set_singleton<TileMap>(&tile_map);
        set_singleton<MidiTracker>(&midi_tracker);
        set_singleton<GameObjectHolder>(&game_object_holder);
        set_singleton<std::map<int, WeaponProfile>>(&weapon_profiles);
        set_singleton<std::vector<Explosion>>(&explosions);
        set_singleton<KeyConfig>(&key_config);
        set_singleton<MissionConfig>(&mission_config);
        set_singleton<TextDrawer>(&text_drawer);
        set_singleton<VisualFxTool>(&vfx_tool);
    }

    Sprite load_sprite(const std::string &file)
    {
        if (sprites.find(file) == sprites.end())
        {
            sprites[file] = Sprite(file);
        }

        return sprites[file];
    }

    void load_weapon_profiles()
    {
        WeaponProfile::read_from_file("config/profiles/weapon_profiles.ini", weapon_profiles);
        for (auto &wp : weapon_profiles)
        {
            wp.second.sprite = load_sprite(wp.second.sprite_def_file);
        }
    }

    void init_game()
    {
        camera_offset_x = 0;
        camera_offset_y = 0;
        const auto playlist = get_singleton<Playlist>();
        midi_tracker.read_midi_file(playlist->get_current_file());
        text_drawer.add_timed_permanent_text(screen_w / 2, 10, "Now playing: " + playlist->get_current_tilte(), 500);
        midi_tracker.load_sound_effects("sounds/soundfx.mid_meta.ini");
        for (const auto goal : mission_config.mission_goals)
        {
            if (goal.name == "kill")
                kill_target = goal.value;
            if (goal.name == "retrieve")
                retrieve_target = goal.value;
            if (goal.name == "survive_until")
                survive_limit = goal.value * 30; // scale to secs
            if (goal.name == "time_limit")
            {
                time_limit = goal.value * 30; // scale to secs
                number_of_goals--;            // This is a restriction rather than a goal per se
            }
            number_of_goals++;
        }
        goal_status = number_of_goals;
        load_weapon_profiles();
        tile_map.read_from_file(mission_config.map_file);
        spawn_enemies();
    }

    void create_shot(GameObject &parent, int weapon_profile = -1)
    {
        if (weapon_profile == -1)
            weapon_profile = parent.get_weapon();
        WeaponProfile &wp = weapon_profiles[weapon_profile];
        if (wp.sound > -1)
            midi_tracker.trigger_sfx(wp.sound_key, wp.sound);
        auto parentActive = !parent.get_is_shot();
        if (parentActive)
            parent.set_counter(reload_counter_id, wp.reload);

        float angle = parent.get_direction() + wp.angle - wp.spread / 2;
        float angle_inc = wp.shots > 1 ? wp.spread / (wp.shots - 1) : 0;
        for (int i = 0; i < wp.shots; i++)
        {
            auto type = !parentActive ? parent.get_type() : (parent.get_type() == GameObjectType_PLAYER ? GameObjectType_PLAYER_SHOT : GameObjectType_ENM_SHOT);
            auto &shot = game_object_holder.add_object(new GameObject(wp.sprite, type, 6, 6));
            shot.increment_direction_angle(angle + random(-wp.random_spread, wp.random_spread));
            shot.set_position(parent.get_x(), parent.get_y());
            if (parentActive)
            {
                shot.add_speed_in_direction(1);
                shot.increment_position_in_direction(parent.get_hitbox_max_dim() / 2);
                shot.set_speed(parent.get_dx(), parent.get_dy());
            }
            shot.add_speed_in_direction(wp.speed);
            shot.accelerate_in_direction(wp.acceleration, false);
            shot.set_affected_by_gravity(wp.affected_by_gravity);
            shot.set_counter(alive_counter_id, wp.alive_timer + randomint(0, wp.alive_timer_variation));
            shot.set_flag(number_of_child_particles_flag, wp.number_of_child_particles);
            shot.set_flag(child_particle_id_flag, wp.child_particle_id);
            shot.set_flag(blast_radius_flag, wp.blast_radius);
            shot.set_flag(bouncy_flag, wp.bouncy);
            shot.set_flag(damage_flag, wp.damage);
            angle += angle_inc;
        }
    }

    CollectableProfile *get_buy_menu_item(int cursor, CollectableType category)
    {
        for (auto &collectable_id : collectable_profiles_in_buy_order)
        {
            auto &c = collectable_profiles[collectable_id];
            if (!check_buyable(c) || c.type != category)
                continue;
            cursor--;
            if (cursor == 0)
            {
                return &c;
            }
        }
        return nullptr;
    }

    GameObject *spawn_collectable(int x, int y, int type, bool initial_setup = false)
    {
        if (collectable_profiles.find(type) != collectable_profiles.end())
        {
            const auto &profile = collectable_profiles[type];
            auto &sprite = sprites[profile.sprite];
            auto &collectable = game_object_holder.add_object(new GameObject(sprite, GameObjectType_COLLECTABLE, sprite.get_w(), sprite.get_h()));
            if (initial_setup)
                collectable.set_position(x + 32 - sprite.get_w() / 2, y + 32 - sprite.get_h() / 2);
            else
                collectable.set_position(x, y);
            collectable.set_sub_type((int)profile.type);
            collectable.set_flag(collectable_bonus_amount_flag, profile.bonus_amount);
            collectable.set_weapon(profile.weapon_id);
            collectable.set_flag(collect_sound_id_flag, profile.sound);
            collectable.set_flag(collect_sound_key_flag, profile.sound_key);
            collectable.set_flag(collectable_original_pos_flag, collectable.get_y());
            collectable.set_flag(collectable_float_bounce_amount_flag, randomint(0, 6));
            collectable.set_flag(collectable_orig_x_flag, x);
            collectable.set_flag(collectable_orig_y_flag, y);
            if (profile.type == Collectable_MODIFY_MAP)
            {
                collectable.set_flag(collectable_flip_props_flag, profile.flip_props);
                collectable.set_flag(collectable_hide_sx_flag, profile.hidden_sprite_x);
                collectable.set_flag(collectable_hide_sy_flag, profile.hidden_sprite_y);
                collectable.set_flag(collectable_show_sx_flag, profile.shown_sprite_x);
                collectable.set_flag(collectable_show_sy_flag, profile.shown_sprite_y);
            }
            if (profile.type == Collectable_MISSION_TRIGGER)
            {
                collectable.set_flag(collectable_mission_trigger_id_flag, profile.mission_trigger_id);
            }
            return &collectable;
        }
        return nullptr;
    }

    GameObject *spawn_enemy(int x, int y, int type, bool initial_setup = false)
    {
        if (enemy_profiles.find(type) != enemy_profiles.end())
        {
            const auto &e = enemy_profiles[type];
            int type = enemy_type_ship;
            if (e.type == "soldier")
                type = enemy_type_soldier;
            else if (e.type == "tank")
                type = enemy_type_tank;
            else if (e.type == "building")
                type = enemy_type_building;

            auto hitbox_w = e.hitbox_w;
            auto hitbox_h = e.hitbox_h;
            if (e.hitbox_from_sprite)
            {
                hitbox_w = sprites[e.sprite].get_w();
                hitbox_h = sprites[e.sprite].get_h();
            }
            auto &enemy = game_object_holder.add_object(new GameObject(sprites[e.sprite], GameObjectType_ENEMY, hitbox_w, hitbox_h));
            if (initial_setup)
                enemy.set_position(x + 32 - hitbox_w / 2, y + 32 - hitbox_h / 2);
            else
                enemy.set_position(x, y);
            enemy.set_armor(100.0f / e.health);
            enemy.set_weapon(e.weapon);
            enemy.set_sub_type(type);
            if (e.spawn_enemy != -1)
            {
                enemy.set_flag(enemy_spawn_enemy_id_flag, e.spawn_enemy);
                enemy.set_flag(enemy_spawn_enemy_count_flag, e.spawn_enemy_count);
                enemy.set_flag(enemy_spawn_enemy_is_random_flag, e.spawn_enemy_is_random);
            }
            if (e.drop_collectable != -1)
            {
                enemy.set_flag(enemy_drop_collectable_id_flag, e.drop_collectable);
                enemy.set_flag(enemy_drop_collectable_count_flag, e.drop_collectable_count);
                enemy.set_flag(enemy_drop_collectable_is_random_flag, e.drop_collectable_is_random);
            }
            enemy.set_counter(enemy_immune_to_damage_counter_id, 10);
            // Avoid spawning inside a wall... Not a very sophisticated algorithm, let's hope it works at least :D
            int origx = enemy.get_x();
            int origy = enemy.get_y();
            while (tile_map.check_collision(enemy.get_x(), enemy.get_y(), hitbox_w, hitbox_h) > 0)
            {
                enemy.set_position(origx + randomint(-16, 16), origy + randomint(-16, 16));
            }
            return &enemy;
        }
        return nullptr;
    }

    void spawn_enemies()
    {
        enemy_profiles = EnemyProfile::read_from_file("config/profiles/enemy_profiles.ini");
        collectable_profiles = CollectableProfile::read_from_file("config/profiles/collectable_profiles.ini");
        collectable_profiles_in_buy_order.resize(collectable_profiles.size());

        for (auto &e : enemy_profiles)
        {
            load_sprite(e.second.sprite);
            if (e.second.sprite_zoom > 0)
                sprites[e.second.sprite].set_zoom(e.second.sprite_zoom);
        }

        for (auto &c : collectable_profiles)
        {
            load_sprite(c.second.sprite);
            collectable_profiles_in_buy_order[c.second.buy_menu_order] = c.first;
        }

        int plr_x = 0, plr_y = 0;

        for (auto &iop : tile_map.initial_object_placement)
        {
            if (iop.type == 0)
            {
                plr_x = iop.x;
                plr_y = iop.y;
            }
            else
            {
                if (!spawn_enemy(iop.x, iop.y, iop.type, true))
                    spawn_collectable(iop.x, iop.y, iop.type, true);
            }
        }

        // Add player last so player will always be drawn on top of other sprites
        auto plr_ship_sprite = load_sprite("config/sprite/ship/player.ini");
        plr_ship_sprite.set_animation(1);
        auto &plr = game_object_holder.add_object(new GameObject(plr_ship_sprite, GameObjectType_PLAYER, 24, 24));
        player = &plr;
        player->set_position(plr_x + 20, plr_y + 20);
    }

    int get_player_ammo(int weapon_id)
    {
        return player_weapon_status[weapon_id].ammo;
    }

    bool check_player_owns_weapon(int weapon_id)
    {
        return player_weapon_status[weapon_id].owned;
    }

    void progress_game()
    {
        mission_time++;
        if (time_limit > -1 && mission_time > time_limit && goal_status != 0)
            goal_status = -1;
        else if (mission_time == survive_limit)
            goal_status--;
        if (player)
        {
            auto &enemies = game_object_holder.get_category(GameObjectType_ENEMY);
            for (auto &enm : enemies)
            {
                ai_check_visible(*enm, *player, tile_map);
                const auto type = enm->get_sub_type();
                if (type == enemy_type_ship)
                    ship_ai(*enm, *player, tile_map);
                else if (type == enemy_type_soldier)
                    soldier_ai(*enm, *player, tile_map);
                else if (type == enemy_type_tank)
                    tank_ai(*enm, *player);
                if (enm->get_flag(ai_wants_to_shoot_flag))
                {
                    create_shot(*enm);
                }
            }
        }
        game_object_holder.progress();
        auto &plr_shots = game_object_holder.get_category(GameObjectType_PLAYER_SHOT);
        for (auto &plr_shot : plr_shots)
        {
            auto &enemies = game_object_holder.get_category(GameObjectType_ENEMY);
            for (auto &enm : enemies)
            {
                if (enm->get_counter(enemy_immune_to_damage_counter_id) == 0 && plr_shot->collides(*enm))
                {
                    plr_shot->set_health(-1);
                    const auto orig_health = enm->get_health();
                    enm->deal_damage(plr_shot->get_flag(damage_flag));
                    break;
                }
            }
        }
        if (player)
        {
            auto &enm_shots = game_object_holder.get_category(GameObjectType_ENM_SHOT);
            for (auto &enm_shot : enm_shots)
            {
                if (player->collides(*enm_shot))
                {
                    enm_shot->set_health(-1);
                    player->deal_damage(enm_shot->get_flag(damage_flag));
                    break;
                }
            }
            auto &collectables = game_object_holder.get_category(GameObjectType_COLLECTABLE);
            for (auto &collectable : collectables)
            {
                const auto not_collectable_cntr = collectable->get_counter(collectable_not_collectable_counter);
                if (not_collectable_cntr == 0)
                {
                    if (player->collides(*collectable))
                    {
                        const auto type = (CollectableType)collectable->get_sub_type();
                        if (type != Collectable_HEALTH || player->get_health() < 100)
                        {
                            collectable->set_health(-1);
                        }
                    }
                    else if (collectable->get_flag(collectable_getting_sucked_in_flag) == 2)
                    {
                        const auto orig_x = collectable->get_flag(collectable_orig_x_flag);
                        const auto orig_y = collectable->get_flag(collectable_orig_y_flag);
                        if (fabs(collectable->get_x() - orig_x) < 20 &&
                            fabs(collectable->get_y() - orig_y) < 20)
                        {
                            collectable->set_speed(0, 0);
                            collectable->set_flag(collectable_getting_sucked_in_flag, 0);
                            collectable->set_flag(collectable_original_pos_flag, collectable->get_y() + 10);
                        }
                        else
                        {
                            collectable->set_speed(
                                orig_x < collectable->get_x() - 10 ? -1.5 : (orig_x > collectable->get_x() + 10 ? 1.5 : 0),
                                orig_y < collectable->get_y() - 10 ? -1.5 : (orig_y > collectable->get_y() + 10 ? 1.5 : 0));
                            collectable->set_flag(collectable_original_pos_flag, collectable->get_y() + 10);
                        }
                    }
                    else if (fabs(collectable->get_x() - collectable->get_flag(collectable_orig_x_flag)) > 64 ||
                             fabs(collectable->get_y() - collectable->get_flag(collectable_orig_y_flag)) > 64)
                    {
                        collectable->set_flag(collectable_getting_sucked_in_flag, 2);
                    }
                    else if (player->get_distance_sqr(*collectable) < 64 * 64)
                    {
                        collectable->set_speed(
                            player->get_x() < collectable->get_x() - 10 ? -1.5 : (player->get_x() > collectable->get_x() + 10 ? 1.5 : 0),
                            player->get_y() < collectable->get_y() - 10 ? -1.5 : (player->get_y() > collectable->get_y() + 10 ? 1.5 : 0));
                        collectable->set_flag(collectable_original_pos_flag, collectable->get_y() + 10);
                        collectable->set_flag(collectable_getting_sucked_in_flag, 1);
                    }
                    else if (collectable->get_flag(collectable_getting_sucked_in_flag) == 1)
                    {
                        collectable->set_speed(0, 0);
                        collectable->set_flag(collectable_getting_sucked_in_flag, 0);
                    }
                }
                else if (not_collectable_cntr == 1)
                {
                    collectable->set_speed(0, 0);
                    collectable->set_flag(collectable_orig_x_flag, collectable->get_x());
                    collectable->set_flag(collectable_orig_y_flag, collectable->get_y());
                }
            }

            const auto coll = player->get_collision_props();
            if (coll == 2) // hazard
            {
                player->deal_damage(1);
            }
        }
    }

    bool check_buyable(const CollectableProfile &c)
    {
        const bool not_buyable = c.cost == -1 ||
                                 // Can't buy weapons already owned
                                 (c.type == Collectable_WEAPON && check_player_owns_weapon(c.weapon_id));
        return !not_buyable && (c.buy_allow_flags & mission_config.buy_allow_flags);
    }

    void draw_player_ammo_bar()
    {
        if (!player)
            return;
        const auto sprite_btm = player->get_y() + 16;
        const auto col = player->get_counter(reload_counter_id) == 0
                             ? al_map_rgb_f(.8, .8, 0)
                             : al_map_rgb_f(.8, 0, 0);
        const auto ammo = get_player_ammo(player->get_weapon());
        int max = ammo / 5;
        if (max > 8)
            max = 8;
        else if (max == 0 && ammo > 0)
            max = 1;
        for (int i = max; i > 0; i--)
        {
            const auto x0 = player->get_x() - 15 + i * 3;
            al_draw_filled_rectangle(x0, sprite_btm, x0 + 2, sprite_btm + 5, col);
        }
    }

    void progress_and_draw()
    {
        const auto game_paused = show_inventory;

        ALLEGRO_TRANSFORM transform;
        al_identity_transform(&transform);
        al_translate_transform(&transform, (int)-camera_offset_x, (int)-camera_offset_y);
        al_use_transform(&transform);

        al_clear_to_color(al_map_rgb_f(0, 0, 0));

        tile_map.draw();
        if (game_paused)
        {
            vfx_tool.draw();
            draw_explosions(explosions);
            al_hold_bitmap_drawing(true);
            game_object_holder.draw();
            al_hold_bitmap_drawing(false);
            draw_player_ammo_bar();
        }
        else
        {
            progress_game();
            vfx_tool.progress_and_draw();
            progress_and_draw_explosions(explosions);
            al_hold_bitmap_drawing(true);
            game_object_holder.draw();
            al_hold_bitmap_drawing(false);
            draw_player_ammo_bar();
        }

        if (player && show_inventory)
        {
            const int w = 440, h = 300;
            const auto x0 = screen_w / 2 - w / 2 + camera_offset_x;
            const auto y0 = screen_h / 2 - h / 2 + camera_offset_y;
            const auto x_center = x0 + w / 2;
            al_draw_filled_rectangle(x0 - 15, y0 - 15, x0 + w + 15, y0 + h + 15,
                                     al_map_rgb_f(0.2, 0.2, 0.2));
            al_draw_filled_rectangle(x0, y0, x0 + w, y0 + h,
                                     al_map_rgb_f(0.1, 0.1, 0.1));
            int y = y0 + 15;
            text_drawer.draw_text(x_center, y, "INVENTORY");
            y += 15;
            if (inventory_show_status)
            {
                text_drawer.draw_text(x_center, y, "Health: " + std::to_string((int)player->get_health()) + '%');
                y += 15;
                text_drawer.draw_text(x_center, y, "Armor: " + std::to_string((int)(100.0f / player->get_armor())) + '%');
                y += 15;
                text_drawer.draw_text(x_center, y, "Coins: " + std::to_string(player->get_flag(player_coins_flag)));
                y += 15;
                text_drawer.draw_text(x_center, y, "Weapons / ammo:");
                y += 15;
                for (const auto &entry : player_weapon_status)
                {
                    if (entry.second.owned)
                    {
                        text_drawer.draw_text(x_center, y, weapon_profiles[entry.first].name + " / " + std::to_string(entry.second.ammo));
                        y += 15;
                    }
                }
                text_drawer.draw_text(x_center, y0 + h - 10, "right = go to buy menu");
            }
            else
            {
                text_drawer.draw_text(x_center, y, "Coins: " + std::to_string(player->get_flag(player_coins_flag)));
                y += 15;
                text_drawer.draw_text(x_center, y,
                                      (inventory_category > Collectable_AMMO ? collectable_type_to_string((CollectableType)((int)inventory_category - 1)) + " <- " : "Status <- ") +
                                          collectable_type_to_string(inventory_category) +
                                          (inventory_category < Collectable_ARMOR ? " -> " + collectable_type_to_string((CollectableType)((int)inventory_category + 1)) : ""));
                int i = 1;
                for (auto c = get_buy_menu_item(i, inventory_category); c != nullptr; c = get_buy_menu_item(i, inventory_category))
                {
                    y += 15;
                    if (inventory_cursor == i)
                        al_draw_filled_rectangle(x0 + 15, y - 2, x0 + w - 15, y + 10, al_map_rgb_f(0.5, 0.1, 0.1));
                    if (c->type == Collectable_WEAPON)
                        text_drawer.draw_text(x_center, y, weapon_profiles[c->weapon_id].name + " [" + std::to_string(c->cost) + "]");
                    else if (c->type == Collectable_AMMO)
                    {
                        std::stringstream ss;
                        ss << weapon_profiles[c->weapon_id].name << " +" << (int)c->bonus_amount << " [" << c->cost << "], current: " << get_player_ammo(c->weapon_id);
                        text_drawer.draw_text(x_center, y, ss.str());
                    }
                    else if (c->type == Collectable_HEALTH)
                        text_drawer.draw_text(x_center, y, "+" + std::to_string((int)c->bonus_amount) + " [" + std::to_string(c->cost) + "]");
                    else if (c->type == Collectable_ARMOR)
                    {
                        std::stringstream ss;
                        ss << (int)c->bonus_amount << "% [" << c->cost << "], current: " << (int)(100.0f / player->get_armor()) << '%';
                        text_drawer.draw_text(x_center, y, ss.str());
                    }
                    else
                        text_drawer.draw_text(x_center, y, "Dick in a box [priceless]"); // Misconfiguration
                    i++;
                }
                inventory_cursor_max = i - 1;
                if (inventory_cursor > inventory_cursor_max)
                    inventory_cursor = inventory_cursor_max;
                text_drawer.draw_text(x_center, y0 + h - 10, "left/right = page, up/down = select, enter = buy");
            }
        }

        if (goal_status <= 0)
        {
            const int w = 200, h = 50;
            const auto x0 = screen_w / 2 - w / 2 + camera_offset_x;
            const auto y0 = screen_h / 2 - h / 2 + camera_offset_y;
            al_draw_filled_rectangle(x0 - 15, y0 - 15, x0 + w + 15, y0 + h + 15,
                                     al_map_rgb_f(0.2, 0.2, 0.2));
            al_draw_filled_rectangle(x0, y0, x0 + w, y0 + h,
                                     al_map_rgb_f(0.1, 0.1, 0.1));
            if (goal_status == 0)
            {
                text_drawer.draw_text(screen_w / 2 + camera_offset_x, y0 + 15, "MISSION COMPLETED");
                text_drawer.draw_text(screen_w / 2 + camera_offset_x, y0 + 30, "Press enter");
            }
            else
            {
                text_drawer.draw_text(screen_w / 2 + camera_offset_x, y0 + 15, "MISSION FAILED");
                text_drawer.draw_text(screen_w / 2 + camera_offset_x, y0 + 30, "Retry? Y/N");
            }
        }

        text_drawer.draw_timed_permanent_texts();

        game_object_holder.clean_up({GameObjectType_PLAYER_SHOT, GameObjectType_ENM_SHOT},
                                    [this](GameObject *obj)
                                    {
                                        auto explosion_intensity = (float)obj->get_flag(blast_radius_flag) / 32;
                                        if (explosion_intensity > 1)
                                            this->midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_medium);
                                        else
                                            this->midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_small);
                                        this->explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), explosion_intensity));
                                        if (obj->get_flag(number_of_child_particles_flag) > 0)
                                        {
                                            this->create_shot(*obj, obj->get_flag(child_particle_id_flag));
                                        }

                                        float blast_radius_sqr = obj->get_flag(blast_radius_flag);
                                        blast_radius_sqr *= blast_radius_sqr;
                                        if (obj->get_type() == GameObjectType_ENM_SHOT)
                                        {
                                            if (this->player && this->player->get_distance_sqr(*obj) < blast_radius_sqr)
                                                this->player->deal_damage(obj->get_flag(damage_flag));
                                        }
                                        else
                                        {
                                            auto &enemies = this->game_object_holder.get_category(GameObjectType_ENEMY);
                                            for (auto enm : enemies)
                                            {
                                                if (enm->get_distance_sqr(*obj) - enm->get_hitbox_max_dim() / 2 < blast_radius_sqr)
                                                {
                                                    enm->deal_damage(obj->get_flag(damage_flag));
                                                }
                                            }
                                        }
                                    });
        game_object_holder.clean_up(GameObjectType_ENEMY,
                                    [this](GameObject *obj)
                                    {
                                        if (obj == this->auto_aim_target)
                                            this->auto_aim_target = nullptr;
                                        int sfx_id = sfx_explosion_large;
                                        float expl_rad = 1;
                                        auto type = obj->get_sub_type();
                                        if (type == enemy_type_soldier)
                                        {
                                            sfx_id = sfx_explosion_medium;
                                            expl_rad = 0.5;
                                        }
                                        else if (type == enemy_type_tank)
                                        {
                                            sfx_id = sfx_explosion_var;
                                            expl_rad = 3;
                                        }
                                        else if (type == enemy_type_building)
                                        {
                                            sfx_id = sfx_explosion_var;
                                            expl_rad = 7;
                                        }
                                        this->midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_large);
                                        this->explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), 1));
                                        if (--this->kill_target == 0)
                                            this->goal_status--;

                                        if (obj->get_flag(enemy_drop_collectable_id_flag))
                                        {
                                            const auto rand_ratio = obj->get_flag(enemy_drop_collectable_is_random_flag);
                                            for (int i = 0; i < obj->get_flag(enemy_drop_collectable_count_flag); i++)
                                            {
                                                if (!rand_ratio || random(0, 100) < rand_ratio)
                                                {
                                                    auto new_collectable = spawn_collectable(obj->get_x(), obj->get_y(), obj->get_flag(enemy_drop_collectable_id_flag));
                                                    new_collectable->set_speed(random(-1, 1), random(-1, 1));
                                                    new_collectable->set_counter(collectable_not_collectable_counter, 10);
                                                }
                                            }
                                        }
                                        if (obj->get_flag(enemy_spawn_enemy_id_flag))
                                        {
                                            const auto rand_ratio = obj->get_flag(enemy_spawn_enemy_is_random_flag);
                                            for (int i = 0; i < obj->get_flag(enemy_spawn_enemy_count_flag); i++)
                                            {
                                                if (!rand_ratio || random(0, 100) < rand_ratio)
                                                {
                                                    auto enm = spawn_enemy(obj->get_x(), obj->get_y(), obj->get_flag(enemy_spawn_enemy_id_flag));
                                                    enm->set_flag(ai_preferred_direction_flag, random() > 0.5 ? 'L' : 'R');
                                                }
                                            }
                                        }
                                    });
        game_object_holder.clean_up(GameObjectType_COLLECTABLE, [this](GameObject *obj)
                                    {
                                        if (obj->get_flag(collect_sound_id_flag) >= 0)
                                            this->midi_tracker.trigger_sfx(obj->get_flag(collect_sound_key_flag),
                                                                           obj->get_flag(collect_sound_id_flag));
                                        const auto type = (CollectableType)obj->get_sub_type();
                                        const auto bonus_amt = obj->get_flag(collectable_bonus_amount_flag);
                                        const auto buyval = obj->get_flag(collectable_buy_value_flag);
                                        auto msg = std::to_string(bonus_amt);
                                        int msg_time = 40;
                                        if (type == Collectable_HEALTH)
                                        {
                                            auto health_bonus = player->get_health() + bonus_amt;
                                            if (health_bonus > 100)
                                                health_bonus = 100;
                                            player->set_health(health_bonus);
                                            msg = "Health +" + msg;
                                        }
                                        else if (type == Collectable_ARMOR)
                                        {
                                            if (player->get_armor() > 100.0f / bonus_amt)
                                            {
                                                player->set_armor(100.0f / bonus_amt);
                                                msg = "Armor " + std::to_string(bonus_amt) + '%';
                                            }
                                            else if (buyval > 0)
                                            {
                                                msg = std::to_string(buyval) + " coins";
                                                player->add_to_flag(player_coins_flag, buyval);
                                            }
                                            else
                                            {
                                                msg = "You already have better armor";
                                            }
                                        }
                                        else if (type == Collectable_AMMO)
                                        {
                                            this->player_weapon_status[obj->get_weapon()].ammo += bonus_amt;
                                            msg = "+" + msg + " ammo for " + weapon_profiles[obj->get_weapon()].name;
                                        }
                                        else if (type == Collectable_WEAPON)
                                        {
                                            const auto wp_type = obj->get_weapon();
                                            if (check_player_owns_weapon(wp_type) && buyval > 0)
                                            {
                                                msg = std::to_string(buyval) + " coins";
                                                player->add_to_flag(player_coins_flag, buyval);
                                            }
                                            else
                                            {
                                                player_weapon_status[wp_type].owned = true;
                                                if (player->get_weapon() == 0)
                                                    player->set_weapon(wp_type);
                                                msg = weapon_profiles[obj->get_weapon()].name;
                                            }
                                        }
                                        else if (type == Collectable_COIN)
                                        {
                                            player->add_to_flag(player_coins_flag, bonus_amt);
                                            msg = msg + " coins";
                                        }
                                        else if (type == Collectable_MISSION_ITEM)
                                        {
                                            msg = "Mission item";
                                            if (--this->retrieve_target == 0)
                                                this->goal_status--;
                                        }
                                        else if (type == Collectable_MODIFY_MAP)
                                        {
                                            msg = "Key found, map modified";
                                            const auto props = obj->get_flag(collectable_flip_props_flag);
                                            const auto ssx = obj->get_flag(collectable_show_sx_flag);
                                            const auto ssy = obj->get_flag(collectable_show_sy_flag);
                                            const auto hsx = obj->get_flag(collectable_hide_sx_flag);
                                            const auto hsy = obj->get_flag(collectable_hide_sy_flag);

                                            tile_map.modify_map([this, &props, &ssx, &ssy, &hsx, &hsy](TileBase *t, bool is_logical_tile)
                                                                {
                                                                    if (t->get_properties() == props || t->get_properties() == -props)
                                                                    {
                                                                        const auto hide = t->get_properties() == props;
                                                                        t->set_properties(-t->get_properties());
                                                                        if (!is_logical_tile)
                                                                        {
                                                                            ((Tile *)t)->get_sprite()->set_s_xy(hide ? hsx : ssx, hide ? hsy : ssy);
                                                                            if (!hide)
                                                                            {
                                                                                for (auto enemy : this->game_object_holder.get_category(GameObjectType_ENEMY))
                                                                                {
                                                                                    if (t->check_point_inside(enemy->get_x(), enemy->get_y()))
                                                                                    {
                                                                                        enemy->set_health(-1);
                                                                                        // No point in spawning enemies or collectables inside the wall
                                                                                        enemy->set_flag(enemy_spawn_enemy_id_flag, 0);
                                                                                        enemy->set_flag(enemy_drop_collectable_id_flag, 0);
                                                                                    }
                                                                                }
                                                                                if (t->check_point_inside(this->player->get_x(), this->player->get_y()))
                                                                                {
                                                                                    player->set_health(-1);
                                                                                }
                                                                            }

                                                                            auto &vfx = vfx_tool.add((t->get_x() + t->get_x2()) / 2,
                                                                                                     (t->get_y() + t->get_y2()) / 2,
                                                                                                     (t->get_y2() - t->get_y()) / 2, 1, 1, 1, 20);
                                                                            VisualFxTool::disappear(vfx);
                                                                        }
                                                                    }
                                                                });
                                        }
                                        else if (type == Collectable_MISSION_TRIGGER)
                                        {
                                            msg_time = 200;
                                            const auto t = this->mission_config.get_trigger(obj->get_flag(collectable_mission_trigger_id_flag));
                                            if (t && t->type == "text")
                                                msg = t->text;
                                        }
                                        this->text_drawer.add_timed_permanent_text(obj->get_x(), obj->get_y(), msg, msg_time);
                                    });
        game_object_holder.clean_up(GameObjectType_PLAYER, [this](GameObject *obj)
                                    {
                                        midi_tracker.kill_all_sfx();
                                        if (goal_status != 0)
                                        {
                                            this->goal_status = -1;
                                            this->midi_tracker.read_midi_file("sounds/end_screen.mid");
                                        }
                                        this->midi_tracker.trigger_sfx(sfx_explosion_key_min, sfx_explosion_large);
                                        for (int x_offs = -1; x_offs <= 1; x_offs++)
                                            for (int y_offs = -1; y_offs <= 1; y_offs++)
                                                this->explosions.push_back(create_explosion(obj->get_x() + x_offs * 40, obj->get_y() + y_offs * 40, 2));
                                        this->player = nullptr;
                                    });
    }

    void handle_keys(std::map<int, bool> &key_status)
    {
        bool thrust_key = key_status[key_config.up];
        if (!show_inventory && thrust_key != last_thrust_key_status)
        {
            // Use a note that is not used by anything else
            if (thrust_key)
                midi_tracker.trigger_sfx(255, sfx_thrust, 100, 1 | 4);
            else
                midi_tracker.trigger_sfx(255, sfx_thrust, 100, 2);
            last_thrust_key_status = thrust_key;
        }

        if (!show_inventory && key_status[key_config.auto_aim])
        {
            auto &enemies = game_object_holder.get_category(GameObjectType_ENEMY);
            if (!auto_aim_target)
            {
                float nearest_dist = 1e200;
                for (auto &enm : enemies)
                {
                    if (!enm->get_flag(ai_sees_player_flag))
                        continue;
                    const auto priority = target_priorities[enm->get_sub_type()];
                    const auto dist = player->get_distance(*enm) * priority;
                    if (dist < nearest_dist)
                    {
                        nearest_dist = dist;
                        auto_aim_target = enm;
                    }
                }
                if (auto_aim_target)
                {
                    auto &vfx = vfx_tool.add(auto_aim_target->get_x(), auto_aim_target->get_y(), 5, 0, 0.5, 0, 20);
                    vfx.rad_delta = 3;
                    VisualFxTool::fade_to_color(vfx, 0.5, 1, 0.5);
                    midi_tracker.trigger_sfx(71, 10, 40);
                }
            }
            if (auto_aim_target)
            {
                const auto dir_x = -sin(player->get_direction() + ALLEGRO_PI);
                const auto dir_y = cos(player->get_direction() + ALLEGRO_PI);

                const auto a = dir_y / dir_x;
                const auto b = player->get_y() - a * player->get_x();
                const auto eq = a * auto_aim_target->get_x() + b;
                char dir = 'L';
                if ((dir_x > 0 && eq < auto_aim_target->get_y()) ||
                    (dir_x < 0 && eq > auto_aim_target->get_y()))
                    dir = 'R';
                player->increment_direction_angle((dir == 'L' ? -1 : 1) * ALLEGRO_PI / 24);
            }
        }
        else
        {
            auto_aim_target = nullptr;
        }
        if (show_inventory)
        {
            if (inventory_counter == 0)
            {
                const auto orig_curs = inventory_cursor;
                const auto orig_cat = inventory_category;
                const auto orig_show_status = inventory_show_status;
                if (key_status[key_config.up] && inventory_cursor > 1)
                {
                    inventory_cursor--;
                    inventory_counter = 5;
                }
                else if (key_status[key_config.down] && inventory_cursor < inventory_cursor_max)
                {
                    inventory_cursor++;
                    inventory_counter = 5;
                }
                else if (key_status[key_config.left])
                {
                    if (inventory_category > Collectable_AMMO)
                    {
                        inventory_cursor = 1;
                        inventory_category = (CollectableType)((int)inventory_category - 1);
                    }
                    else
                        inventory_show_status = true;
                    inventory_counter = 5;
                }
                else if (key_status[key_config.right] && inventory_show_status)
                {
                    inventory_cursor = 1;
                    inventory_category = Collectable_AMMO;
                    inventory_show_status = false;
                    inventory_counter = 5;
                }
                else if (key_status[key_config.right] && inventory_category < Collectable_ARMOR)
                {
                    inventory_cursor = 1;
                    inventory_category = (CollectableType)((int)inventory_category + 1);
                    inventory_counter = 5;
                }

                if (orig_curs != inventory_cursor || orig_cat != inventory_category || orig_show_status != inventory_show_status)
                {
                    midi_tracker.trigger_sfx(60, sfx_select, 100);
                }
            }
        }
        else
        {
            if (key_status[key_config.left])
            {
                player->increment_direction_angle(-ALLEGRO_PI / 24);
                player->set_animation(1);
            }
            else if (key_status[key_config.right])
            {
                player->increment_direction_angle(ALLEGRO_PI / 24);
                player->set_animation(1);
            }
            else
            {
                player->set_animation(1);
            }
            if (key_status[key_config.up])
                player->accelerate_in_direction(0.1);
            if (key_status[key_config.down] && player->get_counter(reload_counter_id) == 0)
            {
                int next_wp = 0;
                int first_wp = 0;
                bool next = false;
                for (const auto &ws : player_weapon_status)
                {
                    if (first_wp == 0 && ws.second.owned)
                        first_wp = ws.first;
                    if (next && ws.second.owned)
                    {
                        next_wp = ws.first;
                        break;
                    }
                    else if (!next)
                    {
                        next = ws.first == player->get_weapon();
                    }
                }
                if (next_wp == 0)
                    next_wp = first_wp;
                if (next_wp != player->get_weapon())
                {
                    midi_tracker.trigger_sfx(39, 12, 100);
                    player->set_counter(reload_counter_id, 30);
                    player->set_weapon(next_wp);
                    text_drawer.add_timed_permanent_text(player->get_x(), player->get_y() + 20, weapon_profiles[next_wp].name, 100);
                }
            }
        }
        if (key_status[key_config.shoot] && player->get_counter(reload_counter_id) == 0)
        {
            const auto ammo = player_weapon_status[player->get_weapon()].ammo;
            if (ammo > 0)
            {
                player_weapon_status[player->get_weapon()].ammo--;
                create_shot(*player);
            }
        }
        if (inventory_counter > 0)
            inventory_counter--;
        if (key_status[key_config.inventory] && inventory_counter == 0 && goal_status > 0)
        {
            midi_tracker.kill_all_sfx();
            midi_tracker.trigger_sfx(60, sfx_select, 100);
            show_inventory = !show_inventory;
            inventory_counter = 10;
        }
        if (show_inventory)
        {
            if (key_status[ALLEGRO_KEY_ENTER] && inventory_counter == 0)
            {
                int i = 0;
                const auto c = get_buy_menu_item(inventory_cursor, inventory_category);
                if (c != nullptr)
                {
                    auto coins = player->get_flag(player_coins_flag);
                    if (coins >= c->cost)
                    {
                        midi_tracker.trigger_sfx(72, sfx_select, 120);
                        const auto new_collectable = spawn_collectable(player->get_x(), player->get_y(), c->id);
                        new_collectable->set_speed(random(-1, 1), random(-1, 1));
                        for (int i = 0; i < 60; i++)
                            new_collectable->progress();
                        new_collectable->set_counter(collectable_not_collectable_counter, 30);
                        new_collectable->set_flag(collectable_buy_value_flag, c->cost);
                        coins -= c->cost;
                        inventory_counter = 10;
                    }
                    player->set_flag(player_coins_flag, coins);
                }
            }
        }
    }
};