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
    // 0 = succeeded, -1 = failed, > 0 = goals unmet
    int goal_status = 0;

    World() : key_config("config/key_config.ini"), midi_tracker(44100)
    {
        text_drawer.set_centered(true);
    }

    Sprite load_sprite(const std::string &file)
    {
        if (sprites.find(file) == sprites.end())
        {
            sprites[file] = Sprite(file);
        }

        return sprites[file];
    }

    void load_weapon_profile(const std::string &file)
    {
        WeaponProfile p(file);
        p.sprite = load_sprite(p.sprite_def_file);
        weapon_profiles[p.id] = p;
    }

    void load_weapon_profiles()
    {
        ConfigFile cf([this](const auto &key, const auto &val)
                      {
                          if (key == "file")
                              this->load_weapon_profile(val);
                      },
                      [](const auto &s) {});
        cf.read_config_file("config/weapon_profiles.ini");
    }

    void init_game()
    {
        midi_tracker.read_midi_file(mission_config.music);
        midi_tracker.load_sound_effects("soundfx.mid_meta.ini");
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
            weapon_profile = parent.get_flag(weapon_flag);
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
            auto &shot = game_object_holder.add_object(new GameObject(wp.sprite, type, 6, 6, tile_map));
            shot.increment_direction_angle(angle + random(-wp.random_spread, wp.random_spread));
            shot.set_position(parent.get_x(), parent.get_y());
            if (parentActive)
            {
                shot.increment_position_in_direction(24);
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

    void spawn_enemies()
    {
        auto enemies = EnemyProfile::read_from_file("config/enemy_profiles.ini");

        for (auto &e : enemies)
        {
            load_sprite(e.second.sprite);
            if (e.second.sprite_zoom > 0)
                sprites[e.second.sprite].set_zoom(e.second.sprite_zoom);
        }

        for (auto &iop : tile_map.initial_object_placement)
        {
            if (iop.type == 0)
            {
                auto plr_ship_sprite = load_sprite("config/space_ship_anim.ini");
                plr_ship_sprite.set_animation(1);
                auto &plr = game_object_holder.add_object(new GameObject(plr_ship_sprite, GameObjectType_PLAYER, 24, 24, tile_map));
                player = &plr;
                player->set_position(iop.x + 20, iop.y + 20);
                player->set_flag(weapon_flag, 1);
            }
            else if (enemies.find(iop.type) != enemies.end())
            {
                const auto &e = enemies[iop.type];
                GameObjectType type = GameObjectType_ENM_SHIP;
                if (e.type == "soldier")
                    type = GameObjectType_ENM_SOLDIER;
                if (e.type == "tank")
                    type = GameObjectType_ENM_TANK;

                auto &enemy = game_object_holder.add_object(new GameObject(sprites[e.sprite], type, e.hitbox_w, e.hitbox_h, tile_map));
                enemy.set_position(iop.x + 32 - e.hitbox_w / 2, iop.y + 32 - e.hitbox_h / 2);
                enemy.set_armor(100.0f / e.health);
                enemy.set_flag(weapon_flag, e.weapon);
            }
        }
    }

    void progress_and_draw()
    {
        mission_time++;
        if (time_limit > -1 && mission_time > time_limit)
            goal_status = -1;
        else if (mission_time == survive_limit)
            goal_status--;
        World &world = *this;
        al_clear_to_color(al_map_rgb_f(0, 0, 0));
        tile_map.draw();
        auto &enemies_soldier = game_object_holder.get_category(GameObjectType_ENM_SOLDIER);
        auto &enemies_tank = game_object_holder.get_category(GameObjectType_ENM_TANK);
        if (player)
        {
            for (int enm_type = GameObjectType_ENM_SHIP; enm_type <= GameObjectType_ENM_TANK; enm_type++)
            {
                auto &enemies = game_object_holder.get_category((GameObjectType)enm_type);
                for (auto &enm : enemies)
                {
                    ai_check_visible(*enm, *player, tile_map);
                    if (enm_type == GameObjectType_ENM_SHIP)
                        ship_ai(*enm, *player);
                    else if (enm_type == GameObjectType_ENM_SOLDIER)
                        soldier_ai(*enm, *player, tile_map);
                    else
                        tank_ai(*enm, *player);
                    if (enm->get_flag(ai_wants_to_shoot_flag))
                    {
                        create_shot(*enm);
                    }
                }
            }
        }
        game_object_holder.progress();
        auto &plr_shots = game_object_holder.get_category(GameObjectType_PLAYER_SHOT);
        for (auto &plr_shot : plr_shots)
        {
            for (int enm_type = GameObjectType_ENM_SHIP; enm_type <= GameObjectType_ENM_TANK; enm_type++)
            {
                auto &enemies = game_object_holder.get_category((GameObjectType)enm_type);
                for (auto &enm : enemies)
                {
                    if (plr_shot->collides(*enm))
                    {
                        plr_shot->set_health(-1);
                        enm->deal_damage(plr_shot->get_flag(damage_flag));
                        break;
                    }
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
        }

        game_object_holder.draw();
        progress_and_draw_explosions(explosions);

        text_drawer.draw_timed_permanent_texts();
        if (player)
        {
            //text_drawer.set_color(127, 0, 0);
            //text_drawer.draw_text(player->get_x(), player->get_y() - 34, std::to_string((int)player->get_health()));
            //text_drawer.draw_text(player->get_x(), player->get_y() + 32, weapon_profiles[1].name);
        }

        game_object_holder.clean_up(GameObjectType_PLAYER_SHOT, [&world](GameObject *obj)
                                    {
                                        auto explosion_intensity = (float)obj->get_flag(blast_radius_flag) / 32;
                                        if (explosion_intensity > 1)
                                            world.midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_medium);
                                        else
                                            world.midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_small);
                                        world.explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), explosion_intensity));
                                        if (obj->get_flag(number_of_child_particles_flag) > 0)
                                        {
                                            world.create_shot(*obj, obj->get_flag(child_particle_id_flag));
                                        }

                                        for (int enm_type = GameObjectType_ENM_SHIP; enm_type <= GameObjectType_ENM_TANK; enm_type++)
                                        {
                                            auto &enemies = world.game_object_holder.get_category((GameObjectType)enm_type);
                                            float blast_radius_sqr = obj->get_flag(blast_radius_flag);
                                            blast_radius_sqr *= blast_radius_sqr;
                                            for (auto enm : enemies)
                                            {
                                                if (enm->get_distance_sqr(*obj) < blast_radius_sqr)
                                                {
                                                    enm->deal_damage(obj->get_flag(damage_flag));
                                                }
                                            }
                                        }
                                    });
        game_object_holder.clean_up(GameObjectType_ENM_SHOT, [&world](GameObject *obj)
                                    {
                                        auto explosion_intensity = (float)obj->get_flag(blast_radius_flag) / 32;
                                        if (explosion_intensity > 1)
                                            world.midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_medium);
                                        else
                                            world.midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_small);
                                        world.explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), explosion_intensity));
                                        if (obj->get_flag(number_of_child_particles_flag) > 0)
                                        {
                                            world.create_shot(*obj, obj->get_flag(child_particle_id_flag));
                                        }

                                        float blast_radius_sqr = obj->get_flag(blast_radius_flag);
                                        blast_radius_sqr *= blast_radius_sqr;
                                        if (world.player && world.player->get_distance_sqr(*obj) < blast_radius_sqr)
                                            world.player->deal_damage(obj->get_flag(damage_flag));
                                    });
        game_object_holder.clean_up(GameObjectType_PLAYER, [&world](GameObject *obj)
                                    {
                                        world.midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_large);
                                        for (int x_offs = -1; x_offs <= 1; x_offs++)
                                            for (int y_offs = -1; y_offs <= 1; y_offs++)
                                                world.explosions.push_back(create_explosion(obj->get_x() + x_offs * 40, obj->get_y() + y_offs * 40, 2));
                                        world.player = nullptr;
                                        world.goal_status = -1;
                                    });
        game_object_holder.clean_up(GameObjectType_ENM_SHIP, [this](GameObject *obj)
                                    {
                                        this->midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_large);
                                        this->explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), 1));
                                        if (--this->kill_target == 0)
                                            this->goal_status--;
                                    });
        game_object_holder.clean_up(GameObjectType_ENM_SOLDIER, [this](GameObject *obj)
                                    {
                                        this->midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_medium);
                                        this->explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), 0.5));
                                        if (--this->kill_target == 0)
                                            this->goal_status--;
                                    });
        game_object_holder.clean_up(GameObjectType_ENM_TANK, [this](GameObject *obj)
                                    {
                                        this->midi_tracker.trigger_sfx(SFX_KEY(explosion), sfx_explosion_var);
                                        this->explosions.push_back(create_explosion(obj->get_x(), obj->get_y(), 3));
                                        if (--this->kill_target == 0)
                                            this->goal_status--;
                                    });
    }

    void handle_keys(std::map<int, bool> &key_status)
    {
        bool thrust_key = key_status[key_config.up];
        if (thrust_key != last_thrust_key_status)
        {
            if (thrust_key)
                midi_tracker.trigger_sfx(24, sfx_thrust, 100, 1);
            else
                midi_tracker.trigger_sfx(24, sfx_thrust, 100, 2);
            last_thrust_key_status = thrust_key;
        }
        if (key_status[key_config.up])// && player->get_speed_in_direction() < 10)
            player->accelerate_in_direction(0.1);
            //player->add_speed_in_direction(0.3);
        if (key_status[key_config.left])
        {
            if (key_status[key_config.strafe])
            {
                player->add_speed_in_direction(0.3, -ALLEGRO_PI / 2);
                player->set_animation(2);
            }
            else
            {
                player->increment_direction_angle(-ALLEGRO_PI / 48);
                player->set_animation(1);
            }
        }
        else if (key_status[key_config.right])
        {
            if (key_status[key_config.strafe])
            {
                player->add_speed_in_direction(0.3, ALLEGRO_PI / 2);
                player->set_animation(3);
            }
            else
            {
                player->increment_direction_angle(ALLEGRO_PI / 48);
                player->set_animation(1);
            }
        }
        else
        {
            player->set_animation(1);
        }
        if (key_status[key_config.shoot] && player->get_counter(reload_counter_id) == 0)
        {
            create_shot(*player);
        }
    }
};