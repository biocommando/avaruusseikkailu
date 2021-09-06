#ifndef SIMPLE_BMP_H
#define SIMPLE_BMP_H
#include<stdio.h>
#include <stdlib.h>

typedef struct{
	int w, h, padding_bytes;
	unsigned long buffer_size, header_size, total_size;
	unsigned char *buffer;	
} BMP;


void BMP_Create(BMP *image, int w, int h)
{
	int padding_bytes = 0;
	if(w * 3 % 4 != 0)
		padding_bytes = (4 - (w * 3 % 4));
	
	unsigned long buffer_size = (w*3 + padding_bytes) * h;
	
	image->buffer = (unsigned char*)malloc(buffer_size);
	image->padding_bytes = padding_bytes;
	image->w = w;
	image->h = h;
	image->buffer_size = buffer_size;
	image->header_size = 54;
	image->total_size = image->header_size + buffer_size;
	
	for(unsigned long i = 0; i < buffer_size; i++)
		image->buffer[i] = 0;
}


void BMP_ReadFromFileStream(BMP *image, FILE *f_in)
{
	int w, h, data_offset;
	// Not interested in the first 10 characters
	fseek(f_in, 10, SEEK_SET);
	fread(&data_offset, 4, 1, f_in);
	// this is again useless data
	fread(&w, 4, 1, f_in);
	// Now read dimensions
	fread(&w, 4, 1, f_in);
	fread(&h, 4, 1, f_in);
	// And then seek to the data
	fseek(f_in, data_offset, SEEK_SET);
	BMP_Create(image, w, h);
	fread(image->buffer, 1, image->buffer_size, f_in);
}

void BMP_ReadFromFile(BMP *image, char *fname)
{
	FILE *f = fopen(fname, "rb");
	BMP_ReadFromFileStream(image, f);
	fclose(f);
}

void BMP_WriteToFileStream(BMP *image, FILE *f_out)
{
	int w = image->w, h = image->h;
	// Write the BMP header
	// for reference see http://www.fastgraph.com/help/bmp_header_format.html
	short i16;
	int i32;
	i16 = 0x4D42;
	int padding_bytes = image->padding_bytes;
	fwrite(&i16, 2, 1, f_out);
	i32 = image->total_size;
	fwrite(&i32, 4, 1, f_out);
	i32 = 0;
	fwrite(&i32, 4, 1, f_out);
	i32 = image->header_size;
	fwrite(&i32, 4, 1, f_out);
	i32 = 40;
	fwrite(&i32, 4, 1, f_out);
	fwrite(&w, 4, 1, f_out);
	fwrite(&h, 4, 1, f_out);
	i16 = 1;
	fwrite(&i16, 2, 1, f_out);
	i16 = 24;
	fwrite(&i16, 2, 1, f_out);
	i32 = 0;
	fwrite(&i32, 4, 1, f_out);
	i32 = image->buffer_size;
	fwrite(&i32, 4, 1, f_out);
	i32 = 0;
	fwrite(&i32, 4, 1, f_out);
	fwrite(&i32, 4, 1, f_out);
	fwrite(&i32, 4, 1, f_out);
	fwrite(&i32, 4, 1, f_out);
	fwrite(image->buffer, 1, image->buffer_size, f_out);
}

void BMP_WriteToFile(BMP *image, char *fname)
{
	FILE *f = fopen(fname, "wb");
	BMP_WriteToFileStream(image, f);
	fclose(f);
}
void BMP_SetPixel(BMP *image, int x, int y, unsigned char *rgb)
{
	if(x < image->w && y < image->h && x >= 0 && y >= 0)
	for(int c = 0; c < 3; c++)
	{
		image->buffer[(image->h-1-y)*(image->w*3 + image->padding_bytes)+x*3+c] = rgb[2-c];
	}
}
// transparency = 1 for full transparency
void BMP_BlendPixel(BMP *image, int x, int y, unsigned char *rgb, double transparency)
{
	if(x < image->w && y < image->h && x >= 0 && y >= 0)
	for(int c = 0; c < 3; c++)
	{
		unsigned char *current_rgb = &rgb[2-c];
		unsigned char *current_buffer = &image->buffer[(image->h-1-y)*(image->w*3 + image->padding_bytes)+x*3+c];
		double colA = (double)*current_buffer;
		double colB = (double)*current_rgb;
		double colResult = colA * transparency + colB * (1 - transparency);
		*current_buffer = (unsigned char)colResult;
	}
}

void BMP_GetPixel(BMP *image, int x, int y, unsigned char *rgb)
{
	if(x < image->w && y < image->h && x >= 0 && y >= 0)
	for(int c = 0; c < 3; c++)
	{
		rgb[2-c] = image->buffer[(image->h-1-y)*(image->w*3 + image->padding_bytes)+x*3+c];
	}
}

void BMP_Blit(BMP *image_from, int x_from, int y_from, BMP *image_to, int x_to, int y_to, int blit_width, int blit_height, unsigned char *transparent_rgb)
{
	for(int y = 0; y < blit_height; y++)
	for(int x = 0; x < blit_width; x++)
	{
		unsigned char rgb[3];
		BMP_GetPixel(image_from, x_from + x, y_from + y, rgb);
		if(rgb[0] != transparent_rgb[0] || rgb[1] != transparent_rgb[1] || rgb[2] != transparent_rgb[2])
		BMP_SetPixel(image_to, x_to + x, y_to + y, rgb);
	}
}

void BMP_Fill(BMP *image, unsigned char *rgb)
{
	for(int x = 0; x < image->w; x++)
	for(int y = 0; y < image->h; y++)
		BMP_SetPixel(image, x, y, rgb);
}

void BMP_Free(BMP *image)
{
	free(image->buffer);
}
#endif
