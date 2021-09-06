#include "simple_bmp.h"

#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Provide input file name\n");
        return 0;
    }
    BMP b;
    BMP_ReadFromFile(&b, argv[1]);
    char fout_name[1024];
    sprintf(fout_name, "%s.json", argv[1]);
    FILE *fout = fopen(fout_name, "w");
    fprintf(fout, "[");
    for (int x = 0; x < b.w; x++)
    {
        for (int y = 0; y < b.h; y++)
        {
            unsigned char rgb[3];
            BMP_GetPixel(&b, x, y, rgb);
            if (x != 0 || y != 0)
                fprintf(fout, ",\n");
            fprintf(fout, "{\"x\":%d, \"y\":%d, \"color\": [%d,%d,%d]}", x, y, rgb[0], rgb[1], rgb[2]);
        }
    }
    fprintf(fout, "]");
    fclose(fout);
    BMP_Free(&b);
    return 0;
}