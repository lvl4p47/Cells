#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "grid.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define CELL_SIZE 1

extern SDL_Window* window;
extern SDL_Renderer* renderer;

void Graphics_Init();
void Graphics_Quit();
void Screen_Clear();
void Screen_Draw();

void Grid_Draw();

#endif