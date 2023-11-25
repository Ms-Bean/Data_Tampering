#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 700

#define NUM_TICKS_Y (WINDOW_HEIGHT / 140)
#define NUM_TICKS_X (WINDOW_WIDTH / 200)

#define ORIGIN_X (WINDOW_WIDTH / 10)
#define ORIGIN_Y (WINDOW_HEIGHT - (WINDOW_HEIGHT / 10))

#define LINE_WIDTH 0

int sort_cmp(const void *a, const void *b);

void create_text(SDL_Renderer *renderer, int x, int y, char *text, TTF_Font *font);
void render_labels(SDL_Renderer *renderer, float x_min, float x_max, float y_min, float y_max, TTF_Font *font);

void plot_scatter(float *X, float *Y, int n, char *font_path, int font_size, int marker_size);
void plot_line(float *X, float *Y, int n, char *font_path, int font_size, int line_width);

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
	SDL_Rect rect;
	SDL_Texture *texture;
	float stride_x, stride_y;

	uint32_t *pixels;

	stride_x = ((float)(WINDOW_WIDTH - ORIGIN_X)) / ((float)NUM_TICKS_X);
	stride_y = ((float)(ORIGIN_Y)) / ((float)(NUM_TICKS_Y));

	pixels = (uint32_t *)calloc(WINDOW_WIDTH * WINDOW_HEIGHT, sizeof(uint32_t));
	if(!pixels)
	{
		fprintf(stderr, "render_labels: calloc error\n");
		exit(4);
	}

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
		create_text(renderer, ORIGIN_X + stride_x*i, WINDOW_HEIGHT - 30, tick, font);

		free(tick);
		
		rect.x = ((float)ORIGIN_X) + stride_x*i;
		rect.y = WINDOW_HEIGHT - 35;
		rect.w = 4;
		rect.h = 8;

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
		SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(uint32_t));
		SDL_RenderCopy(renderer, texture, NULL, &rect);

		SDL_DestroyTexture(texture);
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
		create_text(renderer, 0, ORIGIN_Y - stride_y*i, tick, font);

		free(tick);

		rect.x = ORIGIN_X-5;
		rect.y = ORIGIN_Y - stride_y*i;
		rect.w = 8;
		rect.h = 4;

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
		SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(uint32_t));
		SDL_RenderCopy(renderer, texture, NULL, &rect);

		SDL_DestroyTexture(texture);
	}
}

int sort_cmp(const void *a, const void *b)
{
	if (*(float *)a - *(float *)b > 0)
		return 1;
	return -1;
}
void plot_line(float *X, float *Y, int n, char *font_path, int font_size, int line_width)
{
	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_Texture *texture;
	TTF_Font *font;

	int quit;
	uint32_t *pixels;
	
	int i, j;

	float x_min, x_max;
	float y_min, y_max;
	float *packed_floats;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

	TTF_Init();
	font = TTF_OpenFont(font_path, font_size);
	if(!font)
	{
		fprintf(stderr, "Font missing\n");
		exit(4);
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderClear(renderer);
	
	y_min = y_max = x_min = x_max = 0;
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
	if(y_min == y_max)
	{
		y_min -= 0.5;
		y_max += 0.5;
	}
	if(x_min == x_max)
	{
		x_min -= 0.5;
		x_max += 0.5;
	}
	
	/* [x1 y1] [x2 y2] [x3 y3] ... */
	packed_floats = (float *)malloc(sizeof(float) * 2 * n);
	if(!packed_floats)
	{
		fprintf(stderr, "plot_line: malloc error\n");
		exit(4);
	}

	for(i = 0; i < n; i++)
	{
		float x_scaled, y_scaled;
		
		x_scaled = ORIGIN_X + (X[i] - x_min)/(x_max - x_min) * (WINDOW_WIDTH - ORIGIN_X);
		y_scaled = ORIGIN_Y - (Y[i] - y_min)/(y_max - y_min) * ORIGIN_Y;

		packed_floats[i * 2]  = x_scaled;
		packed_floats[i * 2 + 1] = y_scaled;
	}

	qsort(packed_floats, n, sizeof(float) * 2, sort_cmp);
	
	pixels = (uint32_t *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
	if(!pixels)
	{
		fprintf(stderr, "draw_line: malloc error\n");
		exit(4);
	}
	memset(pixels, 255, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
	
	for(i = 0; i < n - 1; i++)
	{
		float x1, y1, x2, y2;

		float dist;
		float dx, dy;
		float curr_x, curr_y;

		int x, y;

		x1 = packed_floats[2*i];
		y1 = packed_floats[2*i + 1];

		x2 = packed_floats[2*i + 2];
		y2 = packed_floats[2*i + 3];
		
		dx = x2 - x1;
		dy = y2 - y1;

		dist = sqrtf(dx * dx + dy * dy);

		dx /= dist;
		dy /= dist;

		curr_x = x1;
		curr_y = y1;

		for(j = 0; j < dist; j++)
		{
			x = (int)curr_x;
			y = (int)curr_y;
			if(x < WINDOW_WIDTH && y < WINDOW_HEIGHT)
				pixels[WINDOW_WIDTH * y + x] = 0;

			curr_x += dx;
			curr_y += dy;
		}
	}	
	for(i = 0; i < line_width - 1; i++)
	{
		uint32_t *pixels_new;
		int x, y;

		pixels_new = (uint32_t *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
		if(!pixels_new)
		{
			fprintf(stderr, "draw_line: malloc error\n");
			exit(4);
		}
		memset(pixels_new, 255, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));

		for(x = 1; x < WINDOW_WIDTH - 1; x++)
		{
			for(y = 1; y < WINDOW_HEIGHT - 1; y++)
			{
				if(pixels[(y + 1) * WINDOW_WIDTH + x] == 0)
					pixels_new[y * WINDOW_WIDTH + x] = 0;
				if(pixels[(y - 1) * WINDOW_WIDTH + x] == 0)
					pixels_new[y * WINDOW_WIDTH + x] = 0;
				if(pixels[y * WINDOW_WIDTH + x + 1] == 0)
					pixels_new[y * WINDOW_WIDTH + x] = 0;
				if(pixels[y * WINDOW_WIDTH + x - 1] == 0)
					pixels_new[y * WINDOW_WIDTH + x] = 0;
				if(pixels[y * WINDOW_WIDTH + x] == 0)
					pixels_new[y * WINDOW_WIDTH + x] = 0;
			}
		}
		free(pixels);
		pixels = pixels_new;
	}

	SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(uint32_t));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_DestroyTexture(texture);

	render_labels(renderer, x_min, x_max, y_min, y_max, font);
	
	free(pixels);
	free(packed_floats);
	
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
void plot_scatter(float *X, float *Y, int n, char *font_path, int font_size, int marker_size)
{
	uint32_t *pixels;

	float x_min, x_max;
	float y_min, y_max;

	int i;

	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;
	TTF_Font *font;

	int quit;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

	TTF_Init();
	font = TTF_OpenFont(font_path, font_size);
	if(!font)
	{
		fprintf(stderr, "Font missing\n");
		exit(4);
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderClear(renderer);
	pixels = (uint32_t *)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
	if(!pixels)
	{
		fprintf(stderr, "plot_scatter: malloc error\n");
		exit(4);
	}
	memset(pixels, 0, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));

	y_min = y_max = x_min = x_max = 0;
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
	if(y_min == y_max)
	{
		y_min -= 0.5;
		y_max += 0.5;
	}
	if(x_min == x_max)
	{
		x_min -= 0.5;
		x_max += 0.5;
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
		rect.w = marker_size;
		rect.h = marker_size;

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
		SDL_UpdateTexture(texture, NULL, pixels, WINDOW_WIDTH * sizeof(uint32_t));
		SDL_RenderCopy(renderer, texture, NULL, &rect);

		SDL_DestroyTexture(texture);
	}

	free(pixels);
	
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
