#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

struct Video
{
    int height_pixels;
    int width_pixels;
    int frame_count;
    uint8_t **frames;

    FILE *video_pipe;
};

struct Video video_init(char *video_path, int height_pixels, int width_pixels)
{
    struct Video video;
    char *ffmpeg_command;
    int frame_count, frames_allocated;
    int count;
    
    ffmpeg_command = (char *)malloc(sizeof(char) * (strlen("ffmpeg -i ") + strlen(video_path) + strlen(" -f image2pipe -vcodec rawvideo -pix_fmt rgb24 \"-\" 2>/dev/null") + 1));
    if(!ffmpeg_command)
    {
        fprintf(stderr, "video_init: malloc error\n");
        exit(4);
    }
    strcpy(ffmpeg_command, "ffmpeg -i ");
    strcpy(ffmpeg_command + strlen(ffmpeg_command), video_path);
    strcpy(ffmpeg_command + strlen(ffmpeg_command), " -f image2pipe -vcodec rawvideo -pix_fmt rgb24 \"-\" 2>/dev/null");

    video.height_pixels = height_pixels;
    video.width_pixels = width_pixels;

    video.video_pipe = popen(ffmpeg_command, "r");

    frame_count = 0;
    frames_allocated = 8;
    video.frames = (uint8_t **)malloc(sizeof(uint8_t *) * frames_allocated);    
    if(!video.frames)
    {
        fprintf(stderr, "video_init: malloc error\n");
        exit(4);
    }
    do
    {
        if(frames_allocated <= frame_count)
        {
            frames_allocated *= 2;
            video.frames = (uint8_t **)realloc(video.frames, sizeof(uint8_t *) * frames_allocated);    
            if(!video.frames)
            {
                fprintf(stderr, "video_init: malloc error\n");
                exit(4);
            }
        }
        video.frames[frame_count] = (uint8_t *)malloc(sizeof(uint8_t) * video.height_pixels * video.width_pixels * 3);            
        if(!video.frames[frame_count])
        {
            fprintf(stderr, "video_init: malloc error\n");
            exit(4);
        }
        count = fread(video.frames[frame_count], 1, video.height_pixels * video.width_pixels * 3, video.video_pipe);
        frame_count++;
    } 
    while (count == video.height_pixels * video.width_pixels * 3);
    video.frame_count = frame_count;

    free(ffmpeg_command);
    return video;
}
void video_destroy(struct Video *video)
{
    pclose(video->video_pipe);
    free(video->frames);
}
void video_show_frame(struct Video *video, int frame_index)
{
	int quit;
	SDL_Event event;

	int width = video->width_pixels;
	int height = video->height_pixels;
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);

	quit = 0;
	while(!quit)
	{
		SDL_UpdateTexture(texture, NULL, video->frames[3347], video->width_pixels * sizeof(uint8_t)*3);
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				quit = 1;
				break;
		}
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}
void video_convert_to_grayscale(struct Video *video)
{
    int i;
    for(i = 0; i < video->frame_count; i++)
    {
        int j;
        for(j = 0; j < video->height_pixels * video->width_pixels * 3; j += 3)
        {
            int sum = video->frames[i][j] + video->frames[i][j+1] + video->frames[i][j+2];
            video->frames[i][j] = sum/3;
            video->frames[i][j+1] = sum/3;
            video->frames[i][j+2] = sum/3;
        }
    }
}
