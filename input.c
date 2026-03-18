#include "input.h"

int quit = 0;
int lmb_held = 0;
int rmb_held = 0;
SDL_Event e;

void Events_Handle()
{
    
    while (SDL_PollEvent(&e) != 0) {
        
        if (e.type == SDL_QUIT) {
            quit = 1;
        }
        
        if (e.type == SDL_KEYDOWN) {
        
        }
        
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;
            
            if (e.button.button == SDL_BUTTON_RIGHT) {
                rmb_held = 1;
                Grid_Signal(x, y, 1, 0, -10);
            }
            if (e.button.button == SDL_BUTTON_LEFT) {
                lmb_held = 1;
                Grid_Signal(x, y, 1, 0, 10);
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) {
                Organism_Init(x, y);
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                lmb_held = 0;
            }
            if (e.button.button == SDL_BUTTON_RIGHT) {
                rmb_held = 0;
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;

            if (rmb_held == 1)
            {
                Grid_Set(x, y, 0);
            }
            if (lmb_held == 1)
            {
                
            }
        }
    }
}
