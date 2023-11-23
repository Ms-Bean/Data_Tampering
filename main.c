#include <SDL.h>
#include <stdlib.h>
#include "videos.h"
int convert_ascii_to_int(char *n);
int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr, "Expected arguments: video_width, video_height, video_path\n");
		exit(4);
	}

	int width = convert_ascii_to_int(argv[1]);
	int height = convert_ascii_to_int(argv[2]);

	struct Video video = video_init(argv[3], height, width);
	
	video_convert_to_grayscale(&video);
	video_show_frame(&video, 134);

	return 0;
}

int convert_ascii_to_int(char *n)
{
	int sum;

	sum = 0;
	while(*n != '\0')
		sum = sum * 10 + *(n++) - '0';
	return sum;
}
