#include "MidiTracker.h"
#include <cmath>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Required parameter: midi file\n";
        return 0;
    }

    MidiTracker mt(44100);
    std::cout << "Reading " << argv[1] << '\n';
    mt.read_midi_file(argv[1]);
    std::cout << "File read\n";
    float buf[44100 * 2];
    float max = 0;
    for (int i = 0; i < 60 * 5; i++)
    {
        mt.process_buffer(buf, 44100);
        for (int s = 0; s < 44100 * 2; s++)
        {
            if (fabs(buf[s]) > max)
            {
                max = fabs(buf[s]);
            }
        }
    }

    std::cout << "Peak at " << std::to_string(max) << '\n';
    std::cout << "Scale factor " << std::to_string(1.0 / max) << '\n';

    return 0;
}