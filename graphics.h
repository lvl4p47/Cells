#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "grid.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define CELL_SIZE 1

// #define WINDOW_WIDTH 1366 // for small resolution screen
// #define WINDOW_HEIGHT 768
// #define CELL_SIZE 1

// #define WINDOW_WIDTH 960 // for debugging
// #define WINDOW_HEIGHT 540
// #define CELL_SIZE 2

extern SDL_Window* window;
extern SDL_Renderer* renderer;

void Graphics_Init();
void Graphics_Quit();
void Screen_Clear();
void Screen_Draw();
void Grid_Draw();

#endif