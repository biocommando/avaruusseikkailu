#pragma once

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_audio.h"

#include "MidiTracker.h"

#include "TextDrawer.h"
#include "MissionConfig.h"

inline int show_mission_selector(std::vector<MissionConfig> &mission_configs, ALLEGRO_EVENT_QUEUE *queue, int last_accomplished)
{
    int delay = 10;
    int show_mission = last_accomplished + 1;
    if (show_mission > mission_configs.size() - 1)
        show_mission = 0;
    TextDrawer text_drawer;
    text_drawer.set_use_camera_offset(false);

    MidiTracker midi_tracker(44100);
    midi_tracker.read_midi_file("sounds/dark_ambient_theme.mid");

    while (true)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
        {
            auto stream = (ALLEGRO_AUDIO_STREAM *) event.any.source;
            float *buf = (float*)al_get_audio_stream_fragment(stream);
            if (buf) {
                midi_tracker.process_buffer(buf, 1024);
                al_set_audio_stream_fragment(stream, buf);
            }
        }
        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            al_clear_to_color(al_map_rgb_f(0, 0, 0));
            text_drawer.draw_text(10, 10, "Select mission");
            text_drawer.draw_text(10, 20, "Mission #" +                                                      //
                                              std::to_string(mission_configs[show_mission].mission_number) + //
                                              " " + mission_configs[show_mission].name);
            if (show_mission > last_accomplished + 1)
                text_drawer.draw_text(10, 30, "LOCKED");
            else
            {
                if (show_mission < last_accomplished + 1)
                    text_drawer.draw_text(10, 30, "Completed");
                
                for (int i = 0; i < mission_configs[show_mission].mission_goals.size(); i++)
                {
                    const auto &goal = mission_configs[show_mission].mission_goals[i];
                    text_drawer.draw_text(10, 40 + i * 10, "Goal: " + goal.name + " - " + std::to_string(goal.value));
                }

            }
            al_flip_display();
        }
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            return -1;
        if (delay > 0)
        {
            delay--;
            continue;
        }
        if (event.type == ALLEGRO_EVENT_KEY_CHAR)
        {
            delay = 10;
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT && show_mission < mission_configs.size() - 1)
                show_mission++;
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT && show_mission > 0)
                show_mission--;
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER && show_mission <= last_accomplished + 1)
                return show_mission;
        }
    }
}