#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "grid.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define CELL_SIZE 1

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint8_t display_mode;

void Graphics_Init();
void Graphics_Quit();
void Screen_Clear();
void Screen_Draw();

void Grid_Draw();

#endif