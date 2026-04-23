#include <stdio.h>
#include "kick_wav.h"
#include "../../../wav_handler/wav_handler.h"

int main(int argc, char **argv)
{
    for (int bit_depth = 8; bit_depth <= 32; bit_depth += 8)
    {
        struct wav_file wf;
        create_wav_file(&wf, 4537, 1, bit_depth, 44100);
        float *kick_wav = get_kick_wav();
        for (int i = 0; i < 4537; i++)
            wav_set_normalized(&wf, i, &kick_wav[i]);
        char oname[32];
        sprintf(oname, "output_%dbit.wav", bit_depth);
        write_wav_file(oname, &wf);
        free_wav_file(&wf);
    }
    return 0;
}
