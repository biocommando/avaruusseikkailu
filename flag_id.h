#pragma once

constexpr int enemy_type_flag = 10;

constexpr int enemy_type_ship = 1;
constexpr int enemy_type_soldier = 2;
constexpr int enemy_type_tank = 3;

constexpr int reload_counter_id = 100;
constexpr int alive_counter_id = 200;

constexpr int number_of_child_particles_flag = 300;
constexpr int child_particle_id_flag = 301;
constexpr int blast_radius_flag = 302;
constexpr int damage_flag = 303;
constexpr int bouncy_flag = 304;
constexpr int weapon_flag = 305;

constexpr int collectable_type_flag = 400;
constexpr int collectable_bonus_amount_flag = 401;
constexpr int collect_sound_id_flag = 402;
constexpr int collect_sound_key_flag = 403;
constexpr int collectable_original_pos_flag = 404;
constexpr int collectable_float_bounce_amount_flag = 405;

constexpr int player_coins_flag = 1100;
constexpr int player_owns_weapon_flag = 1200;
constexpr int player_ammo_amount_flag = 1300;
constexpr int player_weapon_list_flag = 1400;

// AI flags

constexpr int ai_change_direction_counter_id = 1001;
constexpr int ai_preferred_direction_flag = 1002;
constexpr int ai_wants_to_shoot_flag = 1003;
constexpr int ai_distance_flag = 1004;
constexpr int ai_soldier_jump_counter_id = 1005;
constexpr int ai_soldier_shoot_anim_counter_id = 1006;
constexpr int ai_check_visible_counter_id = 1007;
constexpr int ai_sees_player_flag = 1008;
constexpr int ai_continue_trajectory_after_losing_sight_counter = 1009;

// SFX ids

#define SFX_KEY(id) randomint(sfx_##id##_key_min, sfx_##id##_key_max)

constexpr int sfx_explosion_large = 0;
constexpr int sfx_explosion_medium = 1;
constexpr int sfx_explosion_small = 2;
constexpr int sfx_explosion_var = 3;

constexpr int sfx_explosion_key_min = 12;
constexpr int sfx_explosion_key_max = 30;

constexpr int sfx_thrust = 4;

constexpr int sfx_drop_bomb = 5;

constexpr int sfx_drop_bomb_key = 36;

constexpr int sfx_laser_1 = 6;
constexpr int sfx_laser_2 = 7;

constexpr int sfx_laser_key_min = 48;
constexpr int sfx_laser_key_max = 52;

constexpr int sfx_select = 11;
