#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

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
	StartSDL();
	
	/*MENG_MEI test = MENG_LoadMEIFile("res/MM.mei");
	const char* out = MENG_OutputMeiAsChar(&test);
	printf("%s\n", out);*/

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

	float scale = 10.0f;
	MENG_MEI mei = MENG_LoadMEIFile("./res/MM.mei");
	MENG_Renderer* m_renderer = MENG_CreateRenderer(renderer, &mei, score->rect.h, scale);
	if (MENG_RenderSVG(m_renderer) < 0)
	{
		printf("SVG went wrong\n");
		return -1;
	}
	//m_renderer->svg = score_svg;
	if (MENG_RenderTextures(m_renderer) < 0)
	{
		printf("texture went wrong\n");
		return -1;
	}
	MENG_PrintRenderer(m_renderer);

	MENG_Dimentions dim = MENG_GetSurfaceDimentions(m_renderer);
	SDL_Rect viewport = (SDL_Rect){0, 0, dim.w, dim.h};

	SDL_Event event;
	int close_requested = 0;
	float last_finger_x = 0;
	float last_finger_y = 0;
	int finger_count = 0;

	typedef struct Finger
	{
		SDL_FingerID id;
		float x, y;
	} Finger;

	Finger fingers[2];
	int active_fingers = 0;
	float last_distance = 0.0f;

	Uint64 time0 = SDL_GetTicks64();
	while (!close_requested)
	{
		while (SDL_PollEvent(&event))
		{
			VENG_ListenScreen(&event, screen);

			switch (event.type)
			{
				case SDL_FINGERDOWN:
					if (active_fingers < 2) {
						fingers[active_fingers].id = event.tfinger.fingerId;
						fingers[active_fingers].x = event.tfinger.x;
						fingers[active_fingers].y = event.tfinger.y;
						active_fingers++;

						if (active_fingers == 1) {
							last_finger_x = event.tfinger.x;
							last_finger_y = event.tfinger.y;
						}
						else if (active_fingers == 2) {
							float dx = fingers[1].x - fingers[0].x;
							float dy = fingers[1].y - fingers[0].y;
							last_distance = sqrtf(dx * dx + dy * dy);
						}
					}
					break;

				case SDL_FINGERMOTION:
					for (int i = 0; i < active_fingers; ++i) {
						if (event.tfinger.fingerId == fingers[i].id) {
							fingers[i].x = event.tfinger.x;
							fingers[i].y = event.tfinger.y;
						}
					}

					if (active_fingers == 1) {
						float dx = event.tfinger.x - last_finger_x;
						float dy = event.tfinger.y - last_finger_y;

						viewport.x -= dx * viewport.w;
						viewport.y -= dy * viewport.h;

						last_finger_x = event.tfinger.x;
						last_finger_y = event.tfinger.y;
					}
					else if (active_fingers == 2) {
						float dx = fingers[1].x - fingers[0].x;
						float dy = fingers[1].y - fingers[0].y;
						float new_distance = sqrtf(dx * dx + dy * dy);
						float zoom_factor = new_distance / last_distance;

						float center_x = (fingers[0].x + fingers[1].x) / 2.0f;
						float center_y = (fingers[0].y + fingers[1].y) / 2.0f;

						float world_x = viewport.x + center_x * viewport.w;
						float world_y = viewport.y + center_y * viewport.h;

						viewport.w /= zoom_factor;
						viewport.h /= zoom_factor;

						viewport.x = world_x - center_x * viewport.w;
						viewport.y = world_y - center_y * viewport.h;

						last_distance = new_distance;
					}
					break;

				case SDL_FINGERUP:
					for (int i = 0; i < active_fingers; ++i) {
						if (event.tfinger.fingerId == fingers[i].id) {
							fingers[i] = fingers[active_fingers - 1];
							active_fingers--;
							break;
						}
					}
					break;

				case SDL_QUIT:
					close_requested = 1;
					break;
			}
		}

		SDL_RenderClear(renderer);

		

		fill_a_rect_with_color(score, (SDL_Color){255, 255, 255, 255});
		
		MENG_Render(m_renderer, score, &viewport);

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