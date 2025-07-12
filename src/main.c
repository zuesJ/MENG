#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <verovio/c_wrapper.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <cairo.h>
#include <librsvg/rsvg.h>

#include "VENG/VENG.h"
#include "MENG.h"

#define TITLE "MENG"
#define FPS 144

typedef void* VRV_Toolkit;

SDL_Window* window;
SDL_Renderer* renderer;
VENG_Driver driver;

void StartSDL();
void fill_a_rect_with_color (VENG_Element* element, SDL_Color color);
SDL_Texture* render_svg_into_texture (const char* svg, int* w, int* h);

int main (int argc, char* argv[])
{
	MENG_Init("./libs/verovio/data");
	MENG_MEI mei = MENG_LoadMEIFile("res/MM.mei");
	MENG_PrintMEI(mei);

	
	char* mei_str = MENG_OutputMeiAsChar(mei);
	printf("[MEI str]:\n%s", (xmlChar*)mei_str);
	xmlFree(mei_str);

	VRV_Toolkit tk = vrvToolkit_constructorResourcePath("./libs/verovio/data");
	enableLog(false);

    vrvToolkit_loadFile(tk, "./res/MM.mei");
	const char *options = "{\"breaks\": \"none\" }";
	vrvToolkit_setOptions(tk, options);
	vrvToolkit_redoLayout(tk, options);
	
	const char* score_svg = vrvToolkit_renderToSVG(tk, 1, true);
	vrvToolkit_renderToSVGFile(tk, "res/output.svg", 1);

    StartSDL();

	SDL_ShowCursor(SDL_DISABLE);
	VENG_Init(driver);

	VENG_Screen* screen = VENG_CreateScreen(TITLE, NULL, 1);
	VENG_Layer* layer = VENG_CreateLayer(VENG_CreateLayout(VENG_VERTICAL, VENG_LEFT, VENG_TOP), 2);
	VENG_Element* score = VENG_CreateElement(1.0f, 0.8f, true, true, VENG_CreateLayout(VENG_VERTICAL, VENG_LEFT, VENG_TOP), 0);
	VENG_Element* piano = VENG_CreateElement(1.0f, 0.2f, true, true, VENG_CreateLayout(VENG_VERTICAL, VENG_LEFT, VENG_TOP), 0);

	VENG_AddLayerToScreen(layer, screen);
	VENG_AddElementToLayer(score, layer);
	VENG_AddElementToLayer(piano, layer);

    VENG_PrepareScreen(screen);

	SDL_Event event;
	int close_requested = 0;

	int render_w, render_h;
	SDL_Texture* texture = render_svg_into_texture(score_svg, &render_w, &render_h);
	if (texture == NULL)
	{
		printf("Something failed\n");
	}

	Uint64 time0 = SDL_GetTicks64();
	while(!close_requested)
	{
		while (SDL_PollEvent(&event))
		{	
			VENG_ListenScreen(&event, screen);
			switch (event.type)
			{
				case SDL_QUIT:
					close_requested = 1;
					break;
			}
		}

		SDL_RenderClear(renderer);

		VENG_PrepareScreen(screen);

		fill_a_rect_with_color(score, (SDL_Color){255, 255, 255, 255});

		SDL_Rect rectan = (SDL_Rect){0, 0, render_w, render_h};
		SDL_SetRenderDrawColor(renderer, 23, 123, 50, 255);
		SDL_RenderCopy(renderer, texture, NULL, &rectan);

		

        SDL_RenderPresent(renderer);
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		// FPS management
		int time = (1000 / FPS) - (SDL_GetTicks64() - time0);
		if (time < 0)
		{
			time = 0;
		}
		SDL_Delay(time);

		time0 = SDL_GetTicks64();
	}
	vrvToolkit_destructor(tk);
	VENG_Destroy(true);
	return 0;
}

void StartSDL ()
{
	if (TTF_Init() != 0)
	{
		printf("The system has failed to initialize the image subsystem: %s\n", IMG_GetError());
		exit(-1);
	}

	if (IMG_Init(IMG_INIT_PNG) == 0)
	{
		printf("The system has failed to initialize the image subsystem: %s\n", IMG_GetError());
		exit(-1);
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		printf("The system has failed to initialize the subsystems: %s\n", SDL_GetError());
		exit(-1);	
	}

	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED,
						0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (window == NULL)
	{
		printf("The system has failed to initialize the window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(-1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == NULL)
	{
		printf("The system has failed to initialize the window: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(-1);
	}

	driver = VENG_CreateDriver(window, renderer);
}
void fill_a_rect_with_color (VENG_Element* element, SDL_Color color)
{
	SDL_Rect area = VENG_StartDrawing(element);

	SDL_SetRenderDrawColor(VENG_GetDriver().renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(VENG_GetDriver().renderer, &area);
	SDL_SetRenderDrawColor(VENG_GetDriver().renderer, 0, 0, 0, 0);

	VENG_StopDrawing(NULL);
}

SDL_Texture* render_svg_into_texture (const char* svg, int* w, int* h)
{
	SDL_Texture* texture = NULL;
	GError *error = NULL;

	RsvgHandle *handle = rsvg_handle_new_from_data((const guint8 *)svg, strlen(svg), &error);
	if (handle == NULL)
	{
		return NULL;
	}

	gdouble width, height;
	if (!rsvg_handle_get_intrinsic_size_in_pixels(handle, &width, &height))
	{
		return NULL;
	}
	*w = (int)width;
	*h = (int)height;

	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
	cairo_t* cr = cairo_create(surface);

	RsvgRectangle viewport = {0, 0, (int)width, (int)height};

	if (!rsvg_handle_render_document(handle, cr, &viewport, NULL))
	{
		return NULL;
	}

	cairo_surface_flush(surface);

	unsigned char* data = cairo_image_surface_get_data(surface);
	int stride = cairo_image_surface_get_stride(surface);
	int render_w = (int)width;
	int render_h = (int)height;

	SDL_Surface* sdl_surface = SDL_CreateRGBSurfaceWithFormatFrom(data, render_w, render_h, 32, stride, SDL_PIXELFORMAT_ARGB8888);
	if (!sdl_surface)
	{
		return NULL;
	}

	texture = SDL_CreateTextureFromSurface(renderer, sdl_surface);

	SDL_FreeSurface(sdl_surface);
	return texture;
}