#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <verovio/c_wrapper.h>

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

int main (int argc, char* argv[])
{
	MENG_Init("./libs/verovio/data");
	MENG_LoadMEIFile("res/MM.mei");
    MENG_PrintMEI();
	return 0;
	//MENG_Init("./libs/verovio/data");

	VRV_Toolkit tk = vrvToolkit_constructorResourcePath("./libs/verovio/data");
	enableLog(false);

    vrvToolkit_loadFile(tk, "./res/MM.mei");

	const char *options = "{\"breaks\": \"none\" }";

	vrvToolkit_setOptions(tk, options);
	vrvToolkit_redoLayout(tk, options);

	//printf("%s\n", vrvToolkit_getOptionUsageString(tk));

	vrvToolkit_renderToSVGFile(tk, "res/output.svg", 1);

    vrvToolkit_destructor(tk);

	
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
	
	Uint64 time0 = SDL_GetTicks64();
	int w;
	int h;
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

		fill_a_rect_with_color(score, (SDL_Color){255, 143, 76, 255});
		fill_a_rect_with_color(piano, (SDL_Color){255, 2, 98, 255});

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_RenderPresent(renderer);

		// FPS management
		int time = (1000 / FPS) - (SDL_GetTicks64() - time0);
		if (time < 0)
		{
			time = 0;
		}
		SDL_Delay(time);

		time0 = SDL_GetTicks64();
	}
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