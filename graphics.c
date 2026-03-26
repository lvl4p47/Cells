#include "graphics.h"
#include <SDL2/SDL_image.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;


void Graphics_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return ;
    }

    window = SDL_CreateWindow("templateSDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
    SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_Quit();
        return ;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ;
    }
}

void Graphics_Quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen_Clear()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Screen_Draw()
{
    Grid_Draw();
    // Organism_Draw();
    SDL_RenderPresent(renderer);
}

void Grid_Draw()
{
    
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;
    if(display_mode)
    {
        for(int id1 = 1; id1 < MAX_ORGANISMS; id1++)
        {
            if(population[id1].alive)
            {
                int16_t min_x = population[id1].min_x;
                int16_t min_y = population[id1].min_y;
                int16_t max_x = population[id1].max_x;
                int16_t max_y = population[id1].max_y;
            
                for(int i = min_y - 1; i <= max_y + 1; i++)
                {
                    for(int j = min_x - 1; j <= max_x + 1; j++)
                    {
                        
                        int id = Grid_Get(j, i)->id;
                        if(id == id1)
                        {
                            int r = 0, g = 0, b = 0;
                        
                            // Обычный организм - цвет зависит от genome_hash
                            int value = population[id].genome_hash / 500;
                            int energy = Grid_Get(j, i)->energy;
                            int life_wave_str = Grid_Get(j, i)->energy;
                            int flag_0 = population[id].flag[0] * 8;
                            int flag_1 = population[id].flag[1] * 8;
                            int flag_2 = population[id].flag[2] * 8; 
                            // r = 0;
                            // g = (255 - value) * energy / 255;
                            // b = value * energy / 255;
                            r = id % 256;
                            g = (id % 2) * 255;
                            b = id / 256;
                            // if(energy != 255)
                            // {
                            //     r = energy;
                            //     g = 0;
                            //     b = 255 - energy;
                            // }
                    
                            rect.x = mod(j, grid_width) * CELL_SIZE;
                            rect.y = mod(i, grid_height) * CELL_SIZE;
                            
                            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                            SDL_RenderFillRect(renderer, &rect);
                        }
                    }
                }
            }
        }
    }
    else
    {
        int id1;
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                int id = Grid_Get(j, i)->id;
                int flag_0 = Grid_Get(j, i)->flag[0];
                int flag_1 = Grid_Get(j, i)->flag[1];
                int flag_2 = Grid_Get(j, i)->flag[2];
                int solid = Grid_Get(j, i)->solid;
                if(id != 0)
                {
                    int r = 0, g = 0, b = 0;
                    
                    if(id == MAX_ORGANISMS)
                    {
                        int material = Grid_Get(j, i)->material;
                        int energy = Grid_Get(j, i)->energy;
                        // Еда - розовый
                        r = material;
                        g = 0;
                        b = energy;
                        if(solid)
                        {
                            r = 255;
                            g = 255;
                            b = 255;
                        }
                    }
                    else if(population[id].alive)
                    {
                        int value = Grid_Get(j, i)->cooldown;
                        int energy = Grid_Get(j, i)->energy;
                        int flag_0 = population[id].flag[0] * 8;
                        int flag_1 = population[id].flag[1] * 8;
                        int flag_2 = population[id].flag[2] * 8; 
                        r = 0;
                        g = 255;
                        b = 0;
                        // r = population[id].attack * 255;
                        // g = population[id].fertilized * 255;
                        // b = population[id].has_reproduced * 255;
                    }
            
                    rect.x = j * CELL_SIZE;
                    rect.y = i * CELL_SIZE;
                    
                    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
                if(flag_0 + flag_1 + flag_2 != 0 && 0)
                {
                    int r = 0, g = 0, b = 0;
                    
                    r = flag_0 * 1;
                    g = flag_1 * 1;
                    b = flag_2 * 1;
                    rect.x = j * CELL_SIZE;
                    rect.y = i * CELL_SIZE;
                    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }
}

void Organism_Draw()
{
    
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;
    
    for(int i = 0; i < MAX_ORGANISMS; i++)
    {
        if(population[i].alive)
        {
            rect.x = population[i].nuc_x * CELL_SIZE;
            rect.y = population[i].nuc_y * CELL_SIZE;
            
            uint32_t value = population[i].energy * 255 / population[i].max_energy;
            
            SDL_SetRenderDrawColor(renderer, value, 0, 255 - value, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void Save_Screenshot(const char* filename, int mode)
{
    int w = grid_width;
    int h = grid_height;
    
    // Создаём поверхность
    SDL_Surface* surface = SDL_CreateRGBSurface(0, w, h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!surface) return;
    
    Uint32* pixels = (Uint32*)surface->pixels;
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Cell* cell = &grid_array[y][x];
            int id = cell->id;
            Uint32 color;
            
            switch (mode) {
                case SCREENSHOT_WALLS:
                {
                    if (cell->solid)
                    {
                        int value = max(cell->material, 127);
                        color = SDL_MapRGB(surface->format, 0, 0, value);
                    }
                    else
                    {
                        color = SDL_MapRGB(surface->format, 0, 0, 0);
                    }
                    break;
                }
                
                case SCREENSHOT_FLAGS:
                {
                    int flag_0 = cell->flag[0];
                    int flag_1 = cell->flag[1];
                    int flag_2 = cell->flag[2];
                    
                    color = SDL_MapRGB(surface->format, flag_0, flag_1, flag_2);
                    break;
                }
                
                case SCREENSHOT_MATERIALS:
                {
                    if (id != 0) {
                        int r = 0, g = 0, b = 0;
                        
                        if (id == MAX_ORGANISMS) {
                            int value = cell->material;
                            r = value;
                            g = 0;
                            b = value;
                        }
                        else if (population[id].alive) {
                            int material = cell->material;
                            r = 0;
                            g = material;
                            b = 0;
                        }
                        
                        color = SDL_MapRGB(surface->format, r, g, b);
                    }
                    else {
                        color = SDL_MapRGB(surface->format, 0, 0, 0);
                    }
                    break;
                }
            }
            
            pixels[y * w + x] = color;
        }
    }
    
    IMG_SavePNG(surface, filename);
    SDL_FreeSurface(surface);
}

