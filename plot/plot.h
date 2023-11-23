#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 700

#define NUM_TICKS_Y (WINDOW_HEIGHT / 140)
#define NUM_TICKS_X (WINDOW_WIDTH / 200)

#define ORIGIN_X (WINDOW_WIDTH / 10)
#define ORIGIN_Y (WINDOW_HEIGHT - (WINDOW_HEIGHT / 10))

void create_text(SDL_Renderer *renderer, int x, int y, char *text, TTF_Font *font);
void render_labels(SDL_Renderer *renderer, float x_min, float x_max, float y_min, float y_max, TTF_Font *font);
void plot_scatter(SDL_Renderer *renderer, float *X, float *Y, int n, TTF_Font *font);

void plot(float *X, float *Y, int n, char *font_path, int font_size, void (*plot_type)(SDL_Renderer *renderer, float *X, float *Y, int n, TTF_Font *font));

void plot(float *X, float *Y, int n, char *font_path, int font_size, void (*plot_type)(SDL_Renderer *renderer, float *X, float *Y, int n, TTF_Font *font))
{
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;

	int quit;
	int i;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

	TTF_Init();
	TTF_Font *font = TTF_OpenFont(font_path, font_size);
	if(!font)
	{
		fprintf(stderr, "Font missing\n");
		exit(4);
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderClear(renderer);
	plot_type(renderer, X, Y, 256, font);
	SDL_RenderPresent(renderer);
	
	quit = 0;
	while(!quit)
	{
		while(SDL_WaitEvent(&event) == 1)
		{
			if(event.type == SDL_QUIT)
			{
				quit = 1;
				break;
			}
		}
	}

	TTF_Quit();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
void create_text(SDL_Renderer *renderer, int x, int y, char *text, TTF_Font *font)
{
	SDL_Surface *surface;
	SDL_Color text_color = {0, 0, 0, 0};

	SDL_Texture *out;
	SDL_Rect rect;

	surface = TTF_RenderText_Solid(font, text, text_color);
	out = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_FreeSurface(surface);

	rect.x = x;
	rect.y = y;
	rect.w = surface->w;
	rect.h = surface->h;

	SDL_RenderCopy(renderer, out, NULL, &rect);
	SDL_DestroyTexture(out);
}
void render_labels(SDL_Renderer *renderer, float x_min, float x_max, float y_min, float y_max, TTF_Font *font)
{
	int i;

	for(i = 0; i <= NUM_TICKS_X; i++)
	{
		char *tick;
		float tick_value;
		int num_chars;

		tick_value = x_min + (x_max - x_min) * (((float)i) / ((float)NUM_TICKS_X));
		num_chars = snprintf(NULL, 0, "%f", tick_value);
		tick = (char *)malloc(sizeof(char) * (num_chars + 1));
		if(!tick)
		{
			fprintf(stderr, "render_labels: malloc error\n");
			exit(4);
		}
		snprintf(tick, num_chars + 1, "%f", tick_value);
		create_text(renderer, ORIGIN_X + 0.8*i * ((float)WINDOW_WIDTH)/((float)NUM_TICKS_X), WINDOW_HEIGHT - 30, tick, font);

		free(tick);
	}	
	for(i = 0; i <= NUM_TICKS_Y; i++)
	{
		char *tick;
		float tick_value;
		int num_chars;

		tick_value = y_min + (y_max - y_min) * (((float)i) / ((float)NUM_TICKS_Y));
		num_chars = snprintf(NULL, 0, "%f", tick_value);
		tick = (char *)malloc(sizeof(char) * (num_chars + 1));
		if(!tick)
		{
			fprintf(stderr, "render_labels: malloc error\n");
			exit(4);
		}
		snprintf(tick, num_chars + 1, "%f", tick_value);
		create_text(renderer, 0, ORIGIN_Y - 0.8*i * (((float)WINDOW_HEIGHT) / ((float)NUM_TICKS_Y)), tick, font);

		free(tick);
	}
}
void plot_scatter(SDL_Renderer *renderer, float *X, float *Y, int n, TTF_Font *font)
{
	uint32_t *pixels;

	float x_min, x_max;
	float y_min, y_max;

	int i;


	pixels = (uint32_t *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
	if(!pixels)
	{
		fprintf(stderr, "plot_scatter: malloc error\n");
		exit(4);
	}
	memset(pixels, 255, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));

	for(i = 0; i < n; i++)
	{
		if(i == 0 || X[i] > x_max)
			x_max = X[i];
		if(i == 0 || X[i] < x_min)
			x_min = X[i];
		if(i == 0 || Y[i] > y_max)
			y_max = Y[i];
		if(i == 0 || Y[i] < y_min)
			y_min = Y[i];
	}

	render_labels(renderer, x_min, x_max, y_min, y_max, font);

	for(i = 0; i < n; i++)
	{
		SDL_Texture *texture;
       		SDL_Rect rect;

		float x_scaled, y_scaled;
	
		x_scaled = ORIGIN_X + (X[i] - x_min)/(x_max - x_min) * (WINDOW_WIDTH - ORIGIN_X);
		y_scaled = ORIGIN_Y - (Y[i] - y_min)/(y_max - y_min) * ORIGIN_Y;

		rect.x = x_scaled;
		rect.y = y_scaled;
		rect.w = 10;
		rect.h = 10;

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
		
		SDL_RenderCopy(renderer, texture, NULL, &rect);

		SDL_DestroyTexture(texture);
	}

	free(pixels);
}
