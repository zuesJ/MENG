/*==========================================================================*\
 *                           VENG.h - VENG Library Header
 * ===========================================================================
 * This header contains all the function declarations and type definitions
 * required for the VENG library. It acts as the primary interface to access
 * GUI management functionality within SDL.
 *
 * MIT License: See LICENSE for more
\*==========================================================================*/

#ifndef VENG_H
#define VENG_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Forward declarations (VENG.c)
typedef enum VENG_ParentType VENG_ParentType;

typedef enum VENG_Arrangement VENG_Arrangement;
typedef enum VENG_Align VENG_Align;

typedef struct VENG_Screen VENG_Screen;
typedef struct VENG_Layer VENG_Layer;
typedef struct VENG_Element VENG_Element;
typedef struct VENG_Layout VENG_Layout;
typedef struct VENG_Childs VENG_Childs;
typedef struct VENG_Driver VENG_Driver;

// Forward declarations (VENG_listeners.c)
typedef struct VENG_Listeners VENG_Listeners;
typedef struct VENG_Listener VENG_Listener;

typedef void (*VENG_ListenerCallback)(VENG_Element* element, SDL_Event* event);
typedef int (*VENG_ListenerCondition)(VENG_Element* element, SDL_Event* event); // If function returns 0, VENG will call the callback.

/*==========================================================================*\
 *                   VENG.c - Core Functions, structs & enums
\*==========================================================================*/

// Program Hierarchy Example:
// Screen
// ├- Title, icon
// └- Layers (+ size and count)
//	  ├- Layer 1 (could be the main program)
//	  |  ├- Layout (arrangement, align_h and align_v)
//    |  └- Childs
//    |     ├- Element 1
//    |     ├- Element 2
//    |     └- Element ...
//    |         
//    └- Layer 2 (could be a tactil keyboard)
//       ├- Layout (arrangement, align_h and align_v)
//       └- Childs
//          ├- Element 1
//          ├- Element 2
//          └- Element ...
//


typedef enum VENG_ParentType VENG_ParentType;

typedef enum VENG_Arrangement VENG_Arrangement;
typedef enum VENG_Align VENG_Align;

typedef struct VENG_Screen VENG_Screen;
typedef struct VENG_Layer VENG_Layer;
typedef struct VENG_Element VENG_Element;
typedef struct VENG_Layout VENG_Layout;
typedef struct VENG_Childs VENG_Childs;

typedef struct VENG_Driver VENG_Driver;

typedef enum VENG_ParentType
{
	VENG_NOT_CREATED = 0,
	VENG_TYPE_SCREEN,
	VENG_TYPE_LAYER,
	VENG_TYPE_ELEMENT
} VENG_ParentType;

typedef enum VENG_Arrangement
{
	VENG_HORIZONTAL,
	VENG_VERTICAL
} VENG_Arrangement;

typedef enum VENG_Align
{
	VENG_LEFT,
	VENG_TOP,
	VENG_CENTER,
	VENG_RIGHT,
	VENG_BOTTOM
} VENG_Align;

typedef struct VENG_Driver
{
	SDL_Window* window;
	SDL_Renderer* renderer;
} VENG_Driver;

typedef struct VENG_Layout
{
	VENG_Arrangement arrangement;
	VENG_Align align_horizontal;
	VENG_Align align_vertical;
} VENG_Layout;

typedef struct VENG_Childs
{
	VENG_Element** sub_elements;
	size_t sub_elements_size;
	size_t sub_elements_count;
} VENG_Childs;

typedef struct VENG_Screen
{
	VENG_ParentType type;

	char* title;
	SDL_Surface* icon;
	
	VENG_Layer** layers;
	size_t layers_size;
	size_t layers_count;
} VENG_Screen;

typedef struct VENG_Layer
{
	VENG_ParentType type;

	VENG_Layout layout;
	VENG_Childs childs;

	VENG_Listeners* listeners;
} VENG_Layer;

typedef struct VENG_Element
{
	VENG_ParentType type;

	SDL_Rect rect; // Px -> {x, y, w, h}
	float w, h;    // Width and height in % -> [0, 1]
	
	bool stretch_size;
	bool visible;

	bool dirty;
	VENG_Layout layout;
	VENG_Childs childs;
} VENG_Element;

// Start and finish
int VENG_Init(VENG_Driver driver);

void VENG_Destroy(bool closeSDL);

// Creators
VENG_Screen* VENG_CreateScreen(char* title, SDL_Surface* icon, size_t max_layers);

VENG_Layer* VENG_CreateLayer(VENG_Layout layout, size_t max_elements);

VENG_Element* VENG_CreateElement(float w, float h, bool stretch_size, bool visible, VENG_Layout layout, size_t max_sub_elements);

VENG_Layout VENG_CreateLayout(VENG_Arrangement arrangement, VENG_Align align_horizontal, VENG_Align align_vertical);

VENG_Driver VENG_CreateDriver(SDL_Window* window, SDL_Renderer* renderer);

// Add
int VENG_AddLayerToScreen(VENG_Layer* layer, VENG_Screen* screen);

int VENG_AddElementToLayer(VENG_Element* element, VENG_Layer* layer);

int VENG_AddSubElementToElement(VENG_Element* sub_element, VENG_Element* element);

// Prepare
int VENG_PrepareScreen(VENG_Screen* screen);

int VENG_PrepareLayer(VENG_Layer* layer);

void VENG_PrepareElements(void* parent_container, SDL_Rect drawing_rect);

// Drawing
SDL_Rect VENG_StartDrawing(VENG_Element* element);

void VENG_StopDrawing(SDL_Rect* viewport);

// Set
int VENG_SetDriver(VENG_Driver driver);

int VENG_SetScreen(VENG_Screen* screen);

// Get
SDL_Rect VENG_GetElementRect(VENG_Element* element);

SDL_Rect* VENG_GetElementRectPtr(VENG_Element* element);

VENG_Screen* VENG_GetScreen();

VENG_Driver VENG_GetDriver();

// Optimization

// Debug
bool VENG_HasStarted();
void VENG_PrintInternalHierarchy();
int VENG_PrintScreenHierarchy(VENG_Screen* screen);

/*==========================================================================*\
 *                    VENG_listeners.c - Input management 
\*==========================================================================*/

//typedef void (*VENG_ListenerCallback)(VENG_Element* element, SDL_Event* event);
//typedef int (*VENG_ListenerCondition)(VENG_Element* element, SDL_Event* event); // If function returns 0, VENG will call the callback.

// Main
typedef struct VENG_Listeners // Internal usage.
{
	VENG_Listener** listeners;
	size_t listeners_size;
	size_t listeners_count;
} VENG_Listeners;

typedef struct VENG_Listener
{
	SDL_EventType trigger;
	VENG_ListenerCallback callback;
	VENG_ListenerCondition condition;
	VENG_Element* element;
} VENG_Listener;

int VENG_ListenScreen(SDL_Event* event, VENG_Screen* screen);
int VENG_ListenLayer(SDL_Event* event, VENG_Layer* layer);
int VENG_AddListenerToLayer(VENG_Listener* listener, VENG_Layer* layer);
VENG_Listener* VENG_CreateListener(SDL_EventType trigger, VENG_ListenerCallback callback, VENG_ListenerCondition condition, VENG_Element* element);

// Debug
int VENG_PrintListenersInternalHierarchy();
int VENG_PrintLayerListeners(VENG_Layer* layer);

#endif