#include "../../../wav_handler/wav_handler.h"
#include "wt_sample_loader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct wt_sample wt_sample_slots[MAX_WT_SAMPLE_SLOTS];

int wt_sample_read_all(const char *directory)
{
    //FILE *df = fopen("D:\\debug-wt.txt", "a");
    char path[2048];
    if (strlen(directory) > 2000)
        return 0;
    // All slots contain some safe value even if reading file fails
    for (int i = 0; i < MAX_WT_SAMPLE_SLOTS; i++)
    {
        wt_sample_slots[i].data = (float*)malloc(sizeof(float));
        *wt_sample_slots[i].data = 0;
        wt_sample_slots[i].size = 0;
    }
    int slot;
    for (slot = 0; slot < MAX_WT_SAMPLE_SLOTS; slot++)
    {
        sprintf(path, "%s\\wt_sample_slot_%d.wav", directory, slot);
        //fprintf(df, "reading path '%s'\n", path);
        struct wav_file wav;
        if (read_wav_file(path, &wav) != 0)
            break;
        //fprintf(df, "read result num_frames=%u bit_depth=%u channels=%u\n", wav.num_frames, wav.bit_depth, wav.channels);
        
        wt_sample_slots[slot].data = (float*)realloc(wt_sample_slots[slot].data, sizeof(float) * wav.num_frames);
        //fprintf(df, "allocated %s\n", wt_sample_slots[slot].data ? "PTR" : "NULL");

        //fprintf(df, "reporting sample data...\n");
        wt_sample_slots[slot].size = wav.num_frames;
        float sample[128];
        for (int i = 0; i < wav.num_frames; i++)
        {
            wav_get_normalized(&wav, i, sample);
            //fprintf(df, "%.1f ", sample[0]);
            wt_sample_slots[slot].data[i] = sample[0];
        }
        //fprintf(df, "slot done\n\n");

        free_wav_file(&wav);
    }
    //fprintf(df, "Succesfully read %d slots\n", slot);
    //fclose(df);
    return slot;
}

void wt_sample_free()
{
    for (int i = 0; i < MAX_WT_SAMPLE_SLOTS; i++)
    {
        free(wt_sample_slots[i].data);
        wt_sample_slots[i].data = NULL;
    }
}

struct wt_sample *get_wt_sample(unsigned slot)
{
    if (slot < MAX_WT_SAMPLE_SLOTS)
        return &wt_sample_slots[slot];
    return NULL;
}