#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

#include "MENG.h"

#define TITLE "Physics Simulator"
#define FPS 144

void StartSDL();

int main (int argc, char* argv[])
{
    StartSDL();
    VENG_PrepareScreen(NULL);
}

void StartSDL ()
{

}