#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <cairo.h>
#include <librsvg/rsvg.h>

#include <verovio/c_wrapper.h>

#include <SDL2/SDL.h>

#include "VENG/VENG.h"

#include "MENG.h"

typedef void* V_Toolkit;

bool init = false;

V_Toolkit tk = NULL;

int MENG_Init(char* verovio_resource_path)
{
    xmlInitParser();
    tk = vrvToolkit_constructorResourcePath(verovio_resource_path);
    if (tk == NULL)
    {
        printf("MENG: Could not load resource path\n");
        return -1;
    }
    init = true;
    return 0;
}

bool MENG_IsInit()
{
    return init;
}

int MENG_Destroy()
{
    xmlCleanupParser();
    vrvToolkit_destructor(tk);
    tk = NULL;
    init = false;
    return 0;
}

// Could create an array to store created renderers to avoid mem leaks.
MENG_Renderer* MENG_CreateRenderer(SDL_Renderer* renderer, MENG_MEI* mei, unsigned int rendering_height, double scale)
{
    if (renderer == NULL || mei == NULL || rendering_height == 0 || !init) return NULL;
    MENG_Renderer* new_renderer = (MENG_Renderer*)calloc(1, sizeof(MENG_Renderer));
    if (new_renderer == NULL) return NULL;
    new_renderer->renderer = renderer;
    new_renderer->mei = mei;
    new_renderer->rendering_height = rendering_height;
    new_renderer->scale = scale;
    return new_renderer;
}

int MENG_SetRenderingHeight(MENG_Renderer* renderer, unsigned int rendering_height)
{
    if (renderer == NULL || rendering_height == 0 || !init) return -1;
    renderer->rendering_height = rendering_height;
    return 0;
}

int MENG_SetRenderingScale(MENG_Renderer* renderer, double scale)
{
    if (renderer == NULL || scale <= 0 || !init) return -1;
    renderer->scale = scale;
    return 0;
}

int MENG_SetMEi(MENG_Renderer* renderer, MENG_MEI* mei)
{
    if (renderer == NULL || mei == NULL || !init) return -1;
    renderer->mei = mei;
    return 0;
}

int MENG_RenderSVG(MENG_Renderer* renderer)
{
    if (!init || renderer == NULL) return -1;
    if (renderer->mei == NULL)     return -2;
    renderer->svg == NULL;
    vrvToolkit_loadData(tk, MENG_OutputMeiAsChar(renderer->mei));
    char* options = (char*)calloc(100, sizeof(char));
    sprintf(options, "{\"breaks\": \"none\", \"svgBoundingBoxes\": true}");
	vrvToolkit_setOptions(tk, options);
    const char* svg = vrvToolkit_renderToSVG(tk, 1, true);
    if (svg == NULL)               return -3;
    renderer->svg = svg;
    return 0;
}

int MENG_RenderTextures(MENG_Renderer* renderer)
{
    if (!init || renderer == NULL)                         return -1;
    if (renderer->svg == NULL || renderer->scale <= 0)     return -2;
    
	GError *error = NULL;
    RsvgHandle *handle = rsvg_handle_new_from_data((const guint8 *)renderer->svg, strlen(renderer->svg), &error);
	if (handle == NULL)
	{
        if (error) g_error_free(error);
		return -3;
	}
    gdouble width, height;
	if (!rsvg_handle_get_intrinsic_size_in_pixels(handle, &width, &height))
	{
        g_object_unref(handle);
        return -4;
    }
    width *= renderer->scale;
    height *= renderer->scale;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
    if (surface == NULL)
    {
        g_object_unref(handle);
        return -5;
    }

	cairo_t* cr = cairo_create(surface);
    if (cr == NULL)
    {
        cairo_surface_destroy(surface);
        g_object_unref(handle);
        return -6;
    }


    RsvgRectangle viewport = {0, 0, (int)width, (int)height};
    if (!rsvg_handle_render_document(handle, cr, &viewport, NULL))
	{
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        g_object_unref(handle);
		return -7;
	}

    cairo_surface_flush(surface);

    unsigned char* data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);

    SDL_Surface* sdl_surface = SDL_CreateRGBSurfaceWithFormatFrom(data, width, height, 32, stride, SDL_PIXELFORMAT_ARGB8888);
	if (sdl_surface == NULL)
	{
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        g_object_unref(handle);
		return -8;
	}

    renderer->surface = sdl_surface;

    renderer->surface_dim.w = (int)width;
    renderer->surface_dim.h = (int)height;

    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer->renderer, &info) != 0)
    {
        return -9;
    }

    unsigned int max_w = info.max_texture_width;
    unsigned int max_h = info.max_texture_height;
    unsigned int tiles_x = (renderer->surface_dim.w + max_w - 1) / max_w;
    unsigned int tiles_y = (renderer->surface_dim.h + max_h - 1) / max_h;

    unsigned int total_tiles = tiles_x * tiles_y;
    renderer->chunks = (MENG_TextureChunk***)calloc(tiles_x, sizeof(MENG_TextureChunk**));
    for (unsigned int i = 0; i < tiles_x; i++)
    {
        renderer->chunks[i] = (MENG_TextureChunk**)calloc(tiles_y, sizeof(MENG_TextureChunk*));
    }
    renderer->chunks_x = tiles_x;
    renderer->chunks_y = tiles_y;

    for (unsigned int x_i = 0; x_i < tiles_x; x_i++)
    {
        for (unsigned int y_i = 0; y_i < tiles_y; y_i++)
        {
            unsigned int x = x_i * max_w;
            unsigned int y = y_i * max_h;
            unsigned int w = (x + max_w > (unsigned int)renderer->surface_dim.w) ? (renderer->surface_dim.w - x) : max_w;
            unsigned int h = (y + max_h > (unsigned int)renderer->surface_dim.h) ? (renderer->surface_dim.h - y) : max_h;
            
            SDL_Surface* tile = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
            
            SDL_Rect src = (SDL_Rect){x, y, w, h};
            if (tile == NULL)
            {
                return -10;
            }

            SDL_BlitSurface(renderer->surface, &src, tile, NULL);
            renderer->chunks[x_i][y_i] = calloc(1, sizeof(MENG_TextureChunk));
            renderer->chunks[x_i][y_i]->texture = SDL_CreateTextureFromSurface(renderer->renderer, tile);
            renderer->chunks[x_i][y_i]->x = x;
            renderer->chunks[x_i][y_i]->y = y;
        }
    }
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);
    return 0;
}

MENG_Dimentions MENG_GetSurfaceDimentions(MENG_Renderer* renderer)
{
    if (!init || renderer == NULL) return (MENG_Dimentions){0};
    return renderer->surface_dim;
}

int MENG_Render(MENG_Renderer* renderer, VENG_Element* element, SDL_Rect* viewport)
{
    if (!init || renderer == NULL || element == NULL || viewport == NULL) return -1;
    if (renderer->renderer == NULL || renderer->chunks == NULL) return -2;
	SDL_RendererInfo info;
    if (SDL_GetRendererInfo(renderer->renderer, &info) != 0)
    {
        return -3;
    }
    unsigned int max_w = info.max_texture_width;
    unsigned int max_h = info.max_texture_height;

    for (unsigned int x = 0; x < renderer->chunks_x; x++)
    {
        for (unsigned int y = 0; y < renderer->chunks_y; y++)
        {
            // Check if the texture is relevant
            int text_w, text_h;
            SDL_QueryTexture(renderer->chunks[x][y]->texture, NULL, NULL, &text_w, &text_h);
            SDL_Rect texture_rect = (SDL_Rect){renderer->chunks[x][y]->x, renderer->chunks[x][y]->y, text_w, text_h};
            if (!SDL_HasIntersection(&texture_rect, viewport)) continue;
            // Get srcrect
            SDL_Rect srcrect;
            SDL_IntersectRect(viewport, &texture_rect, &srcrect);
            srcrect.x -= max_w*x;
            srcrect.y -= max_w*y;

            // Get dstrect
            SDL_Rect fullsrcrect;
            SDL_IntersectRect(viewport, &texture_rect, &fullsrcrect);
            SDL_Rect dstrect = (SDL_Rect){round((((float)(fullsrcrect.x - viewport->x) / (float)viewport->w) * element->rect.w)), round((((float)(fullsrcrect.y - viewport->y) / (float)viewport->h) * element->rect.h)), round((((float)fullsrcrect.w / (float)viewport->w) * element->rect.w)), round((((float)fullsrcrect.h / (float)viewport->h) * element->rect.h))};
            
            VENG_StartDrawing(element);
            SDL_RenderCopy(renderer->renderer, renderer->chunks[x][y]->texture, &srcrect, &dstrect);
            
            // Debug Lines
            //SDL_SetRenderDrawColor(renderer->renderer, x* 50, y * 50, x*y*30, 255);
            //SDL_RenderFillRect(renderer->renderer, &dstrect);
            //SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 255);
            //printf("Chunk (%d,%d): srcrect = x:%d y:%d w:%d h:%d\n", x, y, srcrect.x, srcrect.y, srcrect.w, srcrect.h);
            //printf("Chunk (%d,%d): dstrect = x:%d y:%d w:%d h:%d\n\n", x, y, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
            VENG_StopDrawing(NULL);
        }
    }

    return 0;
}

void MENG_DestroyRenderer(MENG_Renderer* renderer);

void MENG_PrintRenderer(MENG_Renderer* renderer)
{
    if (renderer == NULL) return;
    printf("[MENG Renderer]: %p\n", renderer);
    printf("\t[SDL Renderer]: %p\n", renderer->renderer);
    printf("\t[MEI]: %p\n", renderer->mei);
    printf("\t[Scale]: %.2f\n", renderer->scale);
    printf("\t[SVG]: %p\n", renderer->svg);
    printf("\t[Surface]: %p\n", renderer->surface);
    printf("\t[Surface DIM]: %d %d\n", renderer->surface_dim.w, renderer->surface_dim.h);
    printf("\t[Texture Chunks]: %p (count: %d %d)\n", renderer->chunks, renderer->chunks_x, renderer->chunks_y);
    if (renderer->chunks)
    {
        for (unsigned int x = 0; x < renderer->chunks_x; x++)
        {
            for (unsigned int y = 0; y < renderer->chunks_y; y++)
            {
                if (renderer->chunks[x][y] != NULL)
                {
                    int w, h;
                    SDL_QueryTexture(renderer->chunks[x][y]->texture, NULL, NULL, &w, &h);
                    printf("\t\t[Chunk %d %d]: %p x & y: %d %d Dim:(%d %d)\n", x, y, renderer->chunks[x][y]->texture, renderer->chunks[x][y]->x, renderer->chunks[x][y]->y, w, h);
                }
                else
                {
                    printf("\t\t[Chunk %d %d]: (NULL)\n", x, y, renderer->chunks[x][y]);
                }
            }
        }
    }
}