#include "log.h"
#include "World.h"
#include "MissionMenu.h"
#include "allegro5/allegro_audio.h"

float camera_offset_x = 0, camera_offset_y = 50;
int screen_w = 640 * 2, screen_h = 480 * 2;

void calc_camera_offset_correction(float plr_pos, float *camera_offset, float screen_size, float level_size)
{
    // camera correction not needed if whole level fits on screen at once
    if (level_size <= screen_size)
        return;
    const auto target = plr_pos - screen_size / 2;

    const auto offset_correction = target - *camera_offset;

    *camera_offset = (int)(*camera_offset + offset_correction);

    if (*camera_offset < 0)
        *camera_offset = 0;
    else if (*camera_offset > level_size - screen_size)
        *camera_offset = level_size - screen_size;
}

int play_mission(MissionConfig &mission_config, ALLEGRO_EVENT_QUEUE *queue)
{

    auto world = new World();

    world->mission_config = mission_config;

    world->init_game();

    std::map<int, bool> key_status;
    float target_camera_y = camera_offset_y;
    float target_camera_x = camera_offset_x;
    int redraw_count = 0;

    while (true)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event); // Wait for and get an event.

        if (event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
        {
            auto stream = (ALLEGRO_AUDIO_STREAM *)event.any.source;
            float *buf = (float *)al_get_audio_stream_fragment(stream);
            if (buf)
            {
                world->midi_tracker.process_buffer(buf, 1024);
                al_set_audio_stream_fragment(stream, buf);
            }
        }

        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            if (redraw_count == 0)
            {
                if (world->player)
                {
                    world->handle_keys(key_status);
                    calc_camera_offset_correction(world->player->get_y(), &camera_offset_y, screen_h, world->tile_map.get_h());
                    calc_camera_offset_correction(world->player->get_x(), &camera_offset_x, screen_w, world->tile_map.get_w());
                }
                world->progress_and_draw();
                redraw_count++;
            }
            else
            {
                al_flip_display();
                redraw_count = 0;
            }
        }

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            return -2;

        if (event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                break; // Break the loop and quit on escape key.
            key_status[event.keyboard.keycode] = true;
        }
        if (event.type == ALLEGRO_EVENT_KEY_UP)
        {
            key_status[event.keyboard.keycode] = false;
        }
        if (world->goal_status == -1)
        {
            if (key_status[ALLEGRO_KEY_Y])
            {
                delete world;
                world = new World();
                world->mission_config = mission_config;
                world->init_game();
            }
            else if (key_status[ALLEGRO_KEY_N])
            {
                break;
            }
        }
        else if (world->goal_status == 0 && key_status[ALLEGRO_KEY_ENTER])
        {
            break;
        }
    }

    const auto ret = world->goal_status;
    delete world;
    return ret;
}

int main(int argc, char **argv)
{
    srand((int)time(nullptr));
    ALLEGRO_DISPLAY *display;
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;

    if (!al_init())
    {
        return 0;
    }
    al_install_mouse();
    al_install_keyboard();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();

    al_install_audio();
    al_reserve_samples(0);
    ALLEGRO_AUDIO_STREAM *audio_stream = al_create_audio_stream(8, 1024, 44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
    ALLEGRO_MIXER *mixer = al_get_default_mixer();
    al_attach_audio_stream_to_mixer(audio_stream, mixer);

    std::cout << al_get_new_display_flags() << std::endl;
    al_set_new_window_title("The Space Game");
    al_set_new_display_refresh_rate(60);
    //al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);
    al_set_new_display_flags(ALLEGRO_OPENGL);
    display = al_create_display(screen_w, screen_h);
    if (!display)
    {
        return 0;
    }
    timer = al_create_timer(1.0 / 60);
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_audio_stream_event_source(audio_stream));
    al_start_timer(timer); // Start the timer
    Playlist playlist;
    set_singleton<Playlist>(&playlist);
    playlist.read_from_file("sounds/playlist.ini");
    playlist.randomize_order();

    auto mission_configs = MissionConfig::read_from_file("config/missions.ini");
    int last_accomplished = -1;
    while (true)
    {
        playlist.next_file(0);
        auto selected = show_mission_selector(mission_configs, queue, last_accomplished);
        if (selected == -1)
            break;
        auto status = play_mission(mission_configs[selected], queue);
        if (status == -2)
            break;
        if (status == 0 && selected > last_accomplished)
            last_accomplished = selected;
    }
    unload_bitmaps();
    al_uninstall_audio();

    return 0;
}