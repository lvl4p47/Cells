#include "graphics.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void Graphics_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("templateSDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
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
    Organism_Draw();
    SDL_RenderPresent(renderer);
}

void Grid_Draw()
{
    
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;
    uint8_t fast = 1;
    if(fast)
    {
        for(int id1 = 0; id1 < MAX_ORGANISMS; id1++)
        {
            uint16_t min_x = population[id1].min_x;
            uint16_t min_y = population[id1].min_y;
            uint16_t max_x = population[id1].max_x;
            uint16_t max_y = population[id1].max_y;
                
            for(int i = min_y; i <= max_y; i++)
            {
                for(int j = min_x; j <= max_x; j++)
                {
                    int id = Grid_Get(j, i)->id;
                    if(id == id1)
                    {
                        int r = 0, g = 0, b = 0;
                    
                        if(id == MAX_ORGANISMS)
                        {
                            // Еда - розовый
                            r = 255;
                            g = 0;
                            b = 255;
                        }
                        else if(Grid_Get(j, i)->lifetime == 0)
                        {
                            r = 0;
                            g = 0;
                            b = 0;
                        }
                        else
                        {
                            // Обычный организм - цвет зависит от genome_hash
                            int value = population[id].genome_hash / 500;
                            int lifetime = Grid_Get(j, i)->lifetime;
                            int life_wave_str = Grid_Get(j, i)->lifetime;
                            int flag_0 = population[id].flag_0 * 8;
                            int flag_1 = population[id].flag_1 * 8;
                            int flag_2 = population[id].flag_2 * 8; 
                            // r = 0;
                            // g = (255 - value) * lifetime / 255;
                            // b = value * lifetime / 255;
                            r = 127 + flag_0;
                            g = 127 + flag_1;
                            b = 127 + flag_2;
                        }
                        rect.x = j * CELL_SIZE;
                        rect.y = i * CELL_SIZE;
                        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                        SDL_RenderFillRect(renderer, &rect);
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
                if(id != 0)
                {
                    int r = 0, g = 0, b = 0;
                    
                    if(id == MAX_ORGANISMS || population[id].alive == 0 || Grid_Get(j, i)->lifetime == 0)
                    {
                        int material = Grid_Get(j, i)->mat * 10;
                        
                        r = material;
                        g = 0;
                        b = material;
                    }
                    else
                    {
                        // Обычный организм - цвет зависит от genome_hash
                        int value = population[id].genome_hash / 500;
                        int lifetime = Grid_Get(j, i)->lifetime;
                        r = 0;
                        g = (255 - value) * lifetime / 255;
                        b = value * lifetime / 255;
                    }
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
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            if(population[i].multiply) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}