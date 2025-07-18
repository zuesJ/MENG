#ifndef MENG_H
#define MENG_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "VENG/VENG.h"
#include "MEI.h"

/*Might return -1 on failure, but verovio will output an Error message*/
int MENG_Init(char* verovio_resource_path);
bool MENG_IsInit();
int MENG_Destroy();

// MENG Renderer
typedef struct MENG_Dimentions
{
    int w;
    int h;
} MENG_Dimentions;

typedef struct MENG_TextureChunk
{
    int x, y;
    SDL_Texture* texture;
} MENG_TextureChunk;

typedef struct MENG_Renderer
{
    SDL_Renderer* renderer;
    unsigned int rendering_height;
    MENG_MEI* mei;
    const char* svg;
    double scale;
    SDL_Surface* surface;
    MENG_Dimentions surface_dim;
    MENG_TextureChunk*** chunks;
    unsigned int chunks_x;
    unsigned int chunks_y;
} MENG_Renderer;

MENG_Renderer* MENG_CreateRenderer(SDL_Renderer* renderer, MENG_MEI* mei, unsigned int rendering_height, double scale);
int MENG_SetRenderingHeight(MENG_Renderer* renderer, unsigned int rendering_height);
int MENG_SetRenderingScale(MENG_Renderer* renderer, double scale);
int MENG_SetMEi(MENG_Renderer* renderer, MENG_MEI* mei);
int MENG_RenderSVG(MENG_Renderer* renderer);
int MENG_RenderTextures(MENG_Renderer* renderer);
MENG_Dimentions MENG_GetSurfaceDimentions(MENG_Renderer* renderer);
int MENG_Render(MENG_Renderer* renderer, VENG_Element* element, SDL_Rect* viewport); // If SDL_Rect is null then MENG will render the entire square
void MENG_DestroyRenderer(MENG_Renderer* renderer);
void MENG_PrintRenderer(MENG_Renderer* renderer);

// MEI
MENG_MEI MENG_LoadMEIFile(char* file);
const char* MENG_OutputMeiAsChar(MENG_MEI* mei);
void MENG_PrintMEI(MENG_MEI mei);


#endif