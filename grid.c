#include "grid.h"

Cell **grid_array = NULL;
uint16_t grid_width = 0;
uint16_t grid_height = 0;

Organism population[MAX_ORGANISMS + 1];
uint16_t free_stack[MAX_ORGANISMS];
uint16_t order[MAX_ORGANISMS];
uint16_t test_id;

int organism_count = 0;
int32_t free_top = -1;
uint8_t timer = 0;
uint32_t max_pacifism_threshold = (OP_COUNT * GENOME_SIZE * (GENOME_SIZE + 1) / 2);
uint32_t target_hash_step = (OP_COUNT * GENOME_SIZE * (GENOME_SIZE + 1) / 200);
uint8_t recycle_div = 1;
uint8_t food_mat = 20;
uint16_t min_mat = 20;
uint8_t food_mult = 10;
uint32_t total_mat = 0;
uint32_t alive = 0;
uint16_t lifetime = 32 * GENOME_SIZE;
uint8_t re_frac = 1200;
uint8_t debug = 1;
uint8_t base_mutate_chance = 10;

// Статистика
static uint32_t step_counter = 0;
static uint32_t asexual_reproductions = 0;
static uint32_t sexual_reproductions = 0;
static uint32_t deaths = 0;
static uint32_t solidify_count = 0;

uint8_t display_mode = 1;

FILE *file_ptr;
uint16_t integer;

void Grid_Init(uint16_t w, uint16_t h)
{
    srand(clock());
    grid_width = w;
    grid_height = h;
    
    grid_array = (Cell**)malloc(h * sizeof(Cell*));
    
    for(int i = 0; i < h; i++)
    {
        grid_array[i] = (Cell*)malloc(w * sizeof(Cell));
    }
    for (int i = 0; i < MAX_ORGANISMS; i++)
    {
        order[i] = i;
    }
    
    for (int i = 1; i < MAX_ORGANISMS; i++) {
        free_top++;
        free_stack[free_top] = i;
    }
    
    
    Grid_Reset(0);
    
    // printf("free_top: %d\n", free_top);
    // for (int i = 0; i < MAX_ORGANISMS; i++) {
    //     printf("%d ", free_stack[i]);
    // }
    
    uint16_t id = MAX_ORGANISMS - 1;
    population[id].nuc_x = 0;
    population[id].nuc_y = 0;
    population[id].material = food_mat * recycle_div;
    population[id].min_mat = min_mat;
    population[id].volume = 0;
    population[id].target_vol = 50;
    population[id].alive = 0;
    population[id].multiply = 0;
    population[id].lifetime = 0;
    
    population[id].target_dx = 0;
    population[id].target_dy = 0;
    population[id].target_str = 0;
    population[id].pain_dx = 0;
    population[id].pain_dy = 0;
    population[id].pain_str = 0;
    population[id].strength = 1;
    population[id].vx = 0;
    population[id].vy = 0;
    
    population[id].gp = 0;
    
    Repopulate();
    // Organism_Init(grid_width / 2, grid_height / 2);
    
}

void Grid_Quit()
{
    for(int i = 0; i < MAX_ORGANISMS; i++)
    {
        if(population[i].alive)
        {
            Organism_Quit(i);
        }
    }
    
    char buf[32];
    snprintf(buf, sizeof(buf), "genomes/best_genome.txt");
    
    file_ptr = NULL;
    file_ptr = fopen(buf, "w");

    for(int i = 0; i < GENOME_SIZE; i++)
    {
        fprintf(file_ptr, "%i\n",population[free_top].genome[i]);
        
    }
    fclose(file_ptr);

    for(int i = 0; i < grid_height; i++)
    {
        free(grid_array[i]);
    }
    
    free(grid_array);
}

void Grid_Reset(uint16_t value)
{
    // if(debug) fprintf(stderr, "\nGrid_Reset");
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            Grid_Set(j, i, 0);
        }
    }
}

// Cell* Grid_Get(int16_t x, int16_t y)
// {
//     if(debug > 1) fprintf(stderr, "\nGrid_Get");
//     uint16_t x1 = mod(x, grid_width);
//     uint16_t y1 = mod(y, grid_height);
    
//     return &grid_array[y1][x1];
// }

void Grid_Set(int16_t x, int16_t y, uint16_t id)
{
    // if(debug) fprintf(stderr, "\nGrid_Set");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t temp_id = grid_array[y1][x1].id;
    
    // СЛУЧАЙ 1: Удаление клетки (id = 0)
    if(id == 0)
    {
        if(temp_id != 0)
        {
            if(temp_id == MAX_ORGANISMS)
            {
                grid_array[y1][x1].id = 0;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 0;
                grid_array[y1][x1].mat = 0;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].lifetime = 0;
                grid_array[y1][x1].life_wave_str = 0;
                
                grid_array[y1][x1].flag_0 = 0;
                grid_array[y1][x1].flag_1 = 0;
                grid_array[y1][x1].flag_2 = 0;
                grid_array[y1][x1].solid = 0;
            }
            else if(population[temp_id].volume > 0)
            {
                population[temp_id].volume--;
                population[temp_id].material += grid_array[y1][x1].mat;
                
                grid_array[y1][x1].id = 0;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 0;
                grid_array[y1][x1].mat = 0;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].lifetime = 0;
                grid_array[y1][x1].life_wave_str = 0;
                grid_array[y1][x1].solid = 0;
                
                if(population[temp_id].flag_0)
                {
                    grid_array[y1][x1].flag_0 = min(grid_array[y1][x1].flag_0 + 2, 255);
                }
                if(population[temp_id].flag_1)
                {
                    grid_array[y1][x1].flag_1 = min(grid_array[y1][x1].flag_1 + 2, 255);
                }
                if(population[temp_id].flag_2)
                {
                    grid_array[y1][x1].flag_2 = min(grid_array[y1][x1].flag_2 + 2, 255);
                }
                
                if(population[temp_id].volume == 0 && population[temp_id].alive)
                {
                    // printf("reason of death: weird\n");
                    Organism_Quit(temp_id);
                }
            }
            else if(grid_array[y1][x1].mat != 0)
            {
                printf("weird1 id: %4d alive: %d lifetime: %d\n", 
                grid_array[y1][x1].id, 
                population[grid_array[y1][x1].id].alive,
                grid_array[y1][x1].lifetime);
                
                grid_array[y1][x1].id = 0;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 0;
                grid_array[y1][x1].mat = 0;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].lifetime = 0;
                grid_array[y1][x1].life_wave_str = 0;
                grid_array[y1][x1].solid = 0;
            }
        }
        else if(grid_array[y1][x1].mat != 0)
        {
            grid_array[y1][x1].id = 0;
            grid_array[y1][x1].vx = 0;
            grid_array[y1][x1].vy = 0;
            grid_array[y1][x1].strength = 0;
            grid_array[y1][x1].type = 0;
            grid_array[y1][x1].mat = 0;
            grid_array[y1][x1].cooldown = 0;
            grid_array[y1][x1].lifetime = 0;
            grid_array[y1][x1].life_wave_str = 0;
            grid_array[y1][x1].solid = 0;
            printf("weird2\n");
        }
        else if(temp_id == 0)
        {
            grid_array[y1][x1].flag_0 = 0;
            grid_array[y1][x1].flag_1 = 0;
            grid_array[y1][x1].flag_2 = 0;
            grid_array[y1][x1].solid = 0;
        }
        return;
    }
    
    // СЛУЧАЙ 2: Установка клетки организма (id > 0)
    if(id <= MAX_ORGANISMS && population[id].alive && population[id].material > 0)
    {
        if(grid_array[y1][x1].flag_0)
        {
            grid_array[y1][x1].flag_0 = max(grid_array[y1][x1].flag_0 - 1, 0);
        }
        if(grid_array[y1][x1].flag_1)
        {
            grid_array[y1][x1].flag_1 = max(grid_array[y1][x1].flag_1 - 1, 0);
        }
        if(grid_array[y1][x1].flag_2)
        {
            grid_array[y1][x1].flag_2 = max(grid_array[y1][x1].flag_2 - 1, 0);
        }
        // СЛУЧАЙ 2А: Клетка пуста - всегда можно занять
        if(temp_id == 0)
        {
            population[id].material--;
            
            grid_array[y1][x1].id = id;
            grid_array[y1][x1].vx = 0;
            grid_array[y1][x1].vy = 0;
            grid_array[y1][x1].strength = 0;
            grid_array[y1][x1].type = 2;  // Все новые клетки - тело (type=2)
            grid_array[y1][x1].mat = 1;
            grid_array[y1][x1].cooldown = 0;
            grid_array[y1][x1].lifetime = 255;
            grid_array[y1][x1].life_wave_str = 0;
            grid_array[y1][x1].solid = 0;
            
            population[id].volume = min(population[id].volume + 1, 65535);
            return;
        }
        
        // СЛУЧАЙ 2Б: Клетка занята другим организмом
        else if(temp_id <= MAX_ORGANISMS && temp_id != id)
        {
            if(temp_id == MAX_ORGANISMS)
            {
                
                if(grid_array[y1][x1].solid == 0)
                {
                    population[id].material += grid_array[y1][x1].mat - 1;
                    // population[id].lifetime += 1;
                    grid_array[y1][x1].mat = 1;
                    
                    grid_array[y1][x1].id = id;
                    grid_array[y1][x1].vx = 0;
                    grid_array[y1][x1].vy = 0;
                    grid_array[y1][x1].strength = 0;
                    grid_array[y1][x1].type = 2;  // Захваченная клетка становится телом
                    grid_array[y1][x1].mat = 1;
                    grid_array[y1][x1].cooldown = 0;
                    grid_array[y1][x1].lifetime = 255;
                    grid_array[y1][x1].life_wave_str = 0;
                    
                    population[id].volume = min(population[id].volume + 1, 65535);
                }
                else 
                {
                    if(rand() & 1 == 0)
                    {
                        uint8_t strength = population[id].volume / 20;
                        if(grid_array[y1][x1].mat > 0) // breaking
                        {
                            population[id].material += grid_array[y1][x1].mat;
                            // population[id].lifetime += 5;
                            grid_array[y1][x1].mat = max(grid_array[y1][x1].mat - strength, 0);
                            
                            population[id].vx /= 2;
                            population[id].vy /= 2;
                        }
                        else // breakthrough
                        {
                            grid_array[y1][x1].solid = 0;
                            grid_array[y1][x1].mat = 1;
                            
                            grid_array[y1][x1].id = id;
                            grid_array[y1][x1].vx = 0;
                            grid_array[y1][x1].vy = 0;
                            grid_array[y1][x1].strength = 0;
                            grid_array[y1][x1].type = 2;  // Захваченная клетка становится телом
                            
                            grid_array[y1][x1].cooldown = 0;
                            grid_array[y1][x1].lifetime = 255;
                            grid_array[y1][x1].life_wave_str = 0;
                            
                            population[id].volume = min(population[id].volume + 1, 65535);
                        }
                    }
                }
            }
            // Проверяем генетическую совместимость
            else if((abs(population[id].genome_hash - population[temp_id].genome_hash) 
               > (max_pacifism_threshold * population[id].pacifism_treshold / 64)
               || population[temp_id].alive == 0))
            {
                // Обрабатываем разные типы клеток
                if(grid_array[y1][x1].type == 2)  // Тело
                {
                    // printf("body attack");
                    population[id].material += grid_array[y1][x1].mat - 1;
                    // population[id].lifetime += grid_array[y1][x1].mat;
                    grid_array[y1][x1].mat = 1;
                    
                    if(population[temp_id].volume > 0)
                        population[temp_id].volume--;
                    
                    // Если у атакованного не осталось клеток - он умирает
                    if(population[temp_id].alive && population[temp_id].volume == 0)
                    {
                        // printf("\nreason of death: no cells");
                        Organism_Quit(temp_id);
                    }
                }
                else if(grid_array[y1][x1].type == 1)  // Ядро
                {
                    // printf("nucleus attack");
                    // Атака ядра дает больше ресурсов
                    population[id].material += population[temp_id].material / recycle_div;
                    population[temp_id].material = 0;
                    population[id].material += grid_array[y1][x1].mat - 1;
                    // population[id].lifetime += grid_array[y1][x1].mat;
                    grid_array[y1][x1].mat = 1;
                    
                    if(population[temp_id].volume > 0)
                        population[temp_id].volume--;
                    
                    if(population[temp_id].alive)
                    {
                        // printf("reason of death: nucleus eaten\n");
                        Organism_Quit(temp_id);
                    }
                }
                
                // Атакованный организм чувствует боль
                if(population[temp_id].alive)
                {
                    uint16_t attackers_count = Id_Count(x1, y1, id);
                    if(attackers_count > population[temp_id].pain_str)
                    {
                        population[temp_id].pain_str = attackers_count;
                        population[temp_id].pain_dx = x1 - population[temp_id].nuc_x;
                        population[temp_id].pain_dy = y1 - population[temp_id].nuc_y;
                    }
                }
                
                // Занимаем клетку
                grid_array[y1][x1].id = id;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 2;  // Захваченная клетка становится телом
                grid_array[y1][x1].mat = 1;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].lifetime = 255;
                grid_array[y1][x1].life_wave_str = 0;
                
                population[id].volume = min(population[id].volume + 1, 65535);
            }
        }
    }
}

void Grid_Set_Food(uint16_t x, uint16_t y)
{
    // if(debug) fprintf(stderr, "\nGrid_Set_Food");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    // Прямая установка, минуя Grid_Set
    grid_array[y1][x1].id = MAX_ORGANISMS;
    grid_array[y1][x1].vx = 0;
    grid_array[y1][x1].vy = 0;
    grid_array[y1][x1].strength = 0;
    grid_array[y1][x1].type = 1;
    grid_array[y1][x1].mat = rand() % food_mat;
    grid_array[y1][x1].cooldown = 0;
    grid_array[y1][x1].solid = 0;
    
    population[MAX_ORGANISMS].volume = 0;
}

void Grid_Update()
{
    // if(debug) 
    // {
    //     freopen("debug.log", "w", stderr);
    //     fprintf(stderr, "\nGrid_Update");
    // }
    
    step_counter++;
    
    total_mat = 0;
    uint32_t org_mat = 0, grid_mat = 0, vol_mat = 0, volume = 0;
    uint8_t membrane = 0;
    uint16_t id;
    
    uint32_t leftover;
    uint32_t cells;
    uint32_t per_cell;
    
    Order_Shuffle();
    
    total_mat = 0, alive = 0;
    org_mat = 0, grid_mat = 0, vol_mat = 0;
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            id = Grid_Get(j, i)->id;
            if(population[id].volume > 0 || id == MAX_ORGANISMS)
                grid_mat += grid_array[i][j].mat;
        }
    }
    
    for(int i = 1; i < MAX_ORGANISMS; i++)
    {
        if(population[i].alive)
        {
            org_mat += population[i].material;
            vol_mat += population[i].volume;
        }
    }
    total_mat = org_mat + grid_mat;
    
    // printf("before vol_mat: %5d org_mat: %5d grid_mat: %5d total_mat: %5d\n", vol_mat, org_mat, grid_mat, total_mat);
    
    // if(total_mat % 10 != 0)
    // {
    //     printf("%d", 1 / 0);
    // }    
    
    uint16_t id1;
    for(int id0 = 1; id0 < MAX_ORGANISMS; id0++)
    {
        id1 = order[id0];
        volume = 0;
        if(population[id1].alive)
        {
            alive++;
            // printf("\nid: %4d lifetime: %4d\n", id1, population[id1].lifetime);
            int16_t min_x = population[id1].min_x;
            int16_t min_y = population[id1].min_y;
            int16_t max_x = population[id1].max_x;
            int16_t max_y = population[id1].max_y;
            
            int16_t new_min_x = population[id1].min_x + 1;
            int16_t new_min_y = population[id1].min_y + 1;
            int16_t new_max_x = population[id1].max_x - 1;
            int16_t new_max_y = population[id1].max_y - 1;
            
            uint8_t lifetime;
            
            for(int i = min_y; i <= max_y; i++)
            {
                for(int j = min_x; j <= max_x; j++)
                {
                    lifetime = Grid_Get(j, i)->lifetime;
                    if(Grid_Get(j, i)->id == id1 && lifetime != 0)
                    {
                        // printf("first ");
                        // printf("%*d %*d %*d\n", 3, j, 3, i, 4, population[id1].lifetime);
                        Grid_Get(j, i)->lifetime = max(lifetime - 1, 0);
                        
                        if(j < new_min_x) new_min_x = j;
                        if(j > new_max_x) new_max_x = j;
                        if(i < new_min_y) new_min_y = i;
                        if(i > new_max_y) new_max_y = i;
                        
                        membrane = Is_Membrane(j, i);
                        
                        if(membrane) Process_Membrane(j, i);
                        
                        if(membrane && population[id1].take_mat)
                        {
                            for(int dy = -1; dy < 2; dy++)
                            {
                                for(int dx = -1; dx < 2; dx++)
                                {
                                    id = Grid_Get(j + dx, i + dy)->id;
                                    
                                    int dist = max(abs((j + dx) - population[id1].nuc_x), abs((i + dy) - population[id1].nuc_y));
                                    if (rand() % (dist + 1) == 0) 
                                    {
                                        if(id != id1 && id != 0
                                        && population[id].alive && id != MAX_ORGANISMS
                                        && population[id].material > 0)
                                        {
                                            population[id].material--;
                                            population[id1].material++;
                                            
                                            if(population[id].material == 0)
                                            {
                                                Organism_Quit(id);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        if(Grid_Get(j, i)->strength != 0)
                        {
                            if(Grid_Get(j, i)->strength > 0)
                            {
                                if(j - 1 < new_min_x) new_min_x = j - 1;
                                if(j + 1 > new_max_x) new_max_x = j + 1;
                                if(i - 1 < new_min_y) new_min_y = i - 1;
                                if(i + 1 > new_max_y) new_max_y = i + 1;
                            }
                            if(membrane
                            && Grid_Get(j, i)->cooldown == 0)
                            {
                                Expand(j, i, Grid_Get(j, i)->strength);
                            }
                        }
                    }
                    else if(lifetime == 0)
                    {
                        uint16_t id = Grid_Get(j, i)->id;
                        
                        leftover = population[id].material;
                        cells = population[id].volume;
                        
                        population[id].volume--;
                        if(cells != 0)
                        {
                            per_cell = leftover / cells;
                            
                            Grid_Get(j, i)->id = MAX_ORGANISMS;
                            if(population[id].material >= per_cell)
                            {
                                Grid_Get(j, i)->mat += per_cell;
                                population[id].material -= per_cell;
                            }
                            if(population[id].alive && population[id].volume == 0)
                            {
                                // printf("reason of death: dry\n");
                                if(Organism_Quit(id) == 1)
                                    continue;
                            }
                        }
                    }
                }
            }
            if(new_min_x > new_max_x
            || new_min_y > new_max_y) 
            {
                // printf("reason of death: noone\n");
                if(Organism_Quit(id1) == 1)
                    continue;
            }
            else
            {
                population[id1].min_x = new_min_x;
                population[id1].min_y = new_min_y;
                population[id1].max_x = new_max_x;
                population[id1].max_y = new_max_y;
            
                for(int i = min_y; i <= max_y; i++)
                {
                    for(int j = min_x; j <= max_x; j++)
                    {
                        lifetime = Grid_Get(j, i)->lifetime;
                        if(Grid_Get(j, i)->id == id1)
                        {
                            if(Grid_Get(j, i)->cooldown > 0)
                            {
                                // printf("\ncooldown %3d %4d %4d", Grid_Get(j, i)->cooldown, j, i);
                                Grid_Get(j, i)->cooldown = max(Grid_Get(j, i)->cooldown - 1, 0);
                            } 
                            else if(lifetime != 0)
                            {
                                if(Grid_Get(j, i)->strength != 0)
                                {
                                    // printf("second\n");
                                    int8_t strength = Grid_Get(j, i)->strength;
                                    int8_t vx = Grid_Get(j, i)->vx;
                                    int8_t vy = Grid_Get(j, i)->vy;
                                    uint8_t ax = abs(vx);
                                    uint8_t ay = abs(vy);
                                    int8_t sx = sign(vx);
                                    int8_t sy = sign(vy);
                                    uint8_t w_diag = min(ax, ay);
                                    uint8_t w_total = max(ax, ay);
                                    uint8_t w_axis = w_total - w_diag;
                                    
                                    if(w_total != 0)
                                    {
                                        uint8_t r = rand() % w_total;
                                        if (r < w_diag)
                                        {
                                            if(Grid_Get(j + sx, i + sy)->lifetime != 0)
                                                Grid_Signal(j + sx, i + sy, vx, vy, strength);
                                        }
                                        else
                                        {
                                            if(ax > ay) 
                                            {
                                                if(Grid_Get(j + sx, i)->lifetime != 0)
                                                    Grid_Signal(j + sx, i, vx, vy, strength);
                                            }
                                            else 
                                            {
                                                if(Grid_Get(j, i + sy)->lifetime != 0)
                                                Grid_Signal(j, i + sy, vx, vy, strength);
                                            }
                                        }
                                    }
                                    Grid_Signal(j, i, 0, 0, 0);
                                }
                                int16_t life_wave_str = Grid_Get(j, i)->life_wave_str;
                                if(life_wave_str != 0)
                                {
                                    Grid_Life_Wave(j, i, life_wave_str);
                                }
                            }
                            else if(lifetime == 0)
                            {
                                uint16_t id = Grid_Get(j, i)->id;
                                
                                leftover = population[id].material;
                                cells = population[id].volume;
                                
                                population[id].volume--;
                                if(cells != 0)
                                {
                                    per_cell = leftover / cells;
                                    
                                    Grid_Get(j, i)->id = MAX_ORGANISMS;
                                    if(population[id].material > per_cell)
                                    {
                                        Grid_Get(j, i)->mat += per_cell;
                                        population[id].material -= per_cell;
                                    }
                                    if(population[id].alive && population[id].volume == 0)
                                    {
                                        // printf("reason of death: dry\n");
                                        if(Organism_Quit(id) == 1)
                                            continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    Organism_Update();
    
    timer++;
    if(timer > (1 - display_mode) * 100)
    {
        timer = 0;
    }
    
    if (step_counter % 1000 == 0)
    {
        Stats_CollectAndPrint(step_counter);
    }
}

void Grid_Signal(int16_t x, int16_t y, int8_t vx, int8_t vy, int8_t strength)
{
    // if(debug) fprintf(stderr, "\nGrid_Signal");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    if(grid_array[y1][x1].id != 0)
    {
        grid_array[y1][x1].vx = vx;
        grid_array[y1][x1].vy = vy;
        grid_array[y1][x1].strength = strength;
        if(strength != 0 && grid_array[y1][x1].cooldown == 0)
            grid_array[y1][x1].cooldown += 1;
    }
}

void Grid_Life_Wave(int16_t x, int16_t y, uint16_t strength)
{
    // if(debug) fprintf(stderr, "\nGrid_Life_Wave");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t id = grid_array[y1][x1].id;
    uint8_t spread = 0;
    Grid_Get(x1, y1)->lifetime = 255;
    
    for(int dy = -1; dy <= 1; dy++)
    {
        for(int dx = -1; dx <= 1; dx++)
        {
            if(Grid_Get(x1 + dx, y1 + dy)->id == id
            && Grid_Get(x1 + dx, y1 + dy)->cooldown == 0
            && Grid_Get(x1 + dx, y1 + dy)->life_wave_str < max(strength - 1, 0))
            {
                Grid_Get(x1 + dx, y1 + dy)->lifetime = 255;
                Grid_Get(x1 + dx, y1 + dy)->life_wave_str = max(strength - 1, 0);
                // Grid_Get(x1 + dx, y1 + dy)->cooldown += 1;
                if(dy > 0 || (dy == 0 && dx > 0))
                    Grid_Get(x1 + dx, y1 + dy)->cooldown += 1;
                spread = 1;
            }
        }
    }
    
    Grid_Get(x1, y1)->life_wave_str = 0;
    Grid_Get(x1, y1)->cooldown += 2;
}

void Grid_Add_Cooldown(int16_t x, int16_t y, int8_t cd)
{
    // if(debug) fprintf(stderr, "\nGrid_Add_Cooldown");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    grid_array[y1][x1].cooldown += cd;
}

uint16_t Organism_Init(int16_t x, int16_t y)
{
    // if(debug) fprintf(stderr, "\nOrganism_Init");
    if (free_top < 0) {
        return 0; // Стек пуст, свободных мест нет
    }
    
    // Забираем ID с вершины и опускаем указатель    
    uint16_t id = free_stack[free_top];
    free_stack[free_top] = 0;
    free_top--;

    // printf("id: %d\n", id);
    
    // Сразу сбрасываем данные в population для нового жильца
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    population[id].nuc_x = x1;
    population[id].nuc_y = y1;
    population[id].material = min_mat;
    population[id].min_mat = min_mat;
    population[id].volume = 0;
    population[id].target_vol = 50;
    population[id].alive = 1;
    population[id].multiply = 0;
    population[id].lifetime = lifetime;
    
    population[id].target_dx = 0;
    population[id].target_dy = 0;
    population[id].target_str = 0;
    population[id].other_dx = 0;
    population[id].other_dy = 0;
    population[id].other_str = 0;
    population[id].pain_dx = 0;
    population[id].pain_dy = 0;
    population[id].pain_str = 0;
    population[id].free_dx = 0;
    population[id].free_dy = 0;
    population[id].free_dist = 0;
    population[id].free_str = 0;
    population[id].friend_dx = 0;
    population[id].friend_dy = 0;
    population[id].friend_str = 0;
    population[id].flag_0_dx = 0;
    population[id].flag_0_dy = 0;
    population[id].flag_0_str = 0;
    population[id].flag_1_dx = 0;
    population[id].flag_1_dy = 0;
    population[id].flag_1_str = 0;
    population[id].flag_2_dx = 0;
    population[id].flag_2_dy = 0;
    population[id].flag_2_str = 0;
    
    population[id].strength = 1;
    population[id].vx = 0;
    population[id].vy = 0;
    population[id].move = 0;
    population[id].take_mat = 0;
    population[id].attack = 0;
    
    population[id].flag_0 = 0;
    population[id].flag_1 = 0;
    population[id].flag_2 = 0;
    population[id].is_flag_0 = 0;
    population[id].is_flag_1 = 0;
    population[id].is_flag_2 = 0;
    
    population[id].gp = 0;
    population[id].genome_hash = 0;
    population[id].target_hash = 0;
    population[id].mutate_chance = base_mutate_chance;
    population[id].pacifism_treshold = 1;
    
    population[id].life_wave_timer = 0;
    population[id].signal_timer = 0;
    population[id].grow = 0;
    population[id].shrink = 0;
    
    population[id].newborn = 1;
    
    population[id].sex = 0;
    population[id].partner_id = 0;
    population[id].fertilized = 0;
    population[id].has_reproduced = 0;
    population[id].solidify = 0;
    Genome_Init(id, 0);
    
    int range = 0;
    
    population[id].min_x = population[id].nuc_x - range;
    population[id].min_y = population[id].nuc_y - range;
    population[id].max_x = population[id].nuc_x + range;
    population[id].max_y = population[id].nuc_y + range;
    
    // printf("\norg_init:");
    
    for(int dy = -range; dy <= range; dy++)
    {
        for(int dx = -range; dx <= range; dx++)
        {
            Grid_Set(x1 + dx, y1 + dy, id);
        }
    }
    if(Grid_Get(x1, y1)->id != id
    || population[id].volume == 0)
    {
        printf("Organism_Init error\n");
    }
    Grid_Get(x1, y1)->type = 1;
    // printf("id: %d volume: %d", id, population[id].volume);
    
    return id;
}

void Genome_Init(uint16_t id, uint8_t test)
{
    // if(debug) fprintf(stderr, "\nGenome_Init");
    char buf[32];
    if(test)
    {
        printf("\nTEST_GENOME\n");
        snprintf(buf, sizeof(buf), "genomes/test_genome.txt", id);
        
        file_ptr = fopen(buf, "r");
        if(file_ptr == NULL)
        {
            // printf("file error id: %d\n", id);
            srand(clock());
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id].genome[i] = rand() % OP_COUNT;
                population[id].child_genome[i] = population[id].genome[i];
            }
            Genome_Copy(id, id, 1);
        }
        else
        {
            printf("\nTEST_GENOME FOUND\n");
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                fscanf(file_ptr, "%d", &integer);
                population[id].genome[i] = integer;
                population[id].child_genome[i] = population[id].genome[i];
            }
            fclose(file_ptr);
        }
    }
    else
    {
        snprintf(buf, sizeof(buf), "genomes/genome_%d.txt", id);
        
        file_ptr = fopen(buf, "r");
        if(file_ptr == NULL)
        {
            // printf("file error id: %d\n", id);
            srand(clock());
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id].genome[i] = rand() % OP_COUNT;
                population[id].child_genome[i] = population[id].genome[i];
            }
            Genome_Copy(id, id, 1);
        }
        else
        {
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                fscanf(file_ptr, "%d", &integer);
                population[id].genome[i] = integer;
                population[id].child_genome[i] = population[id].genome[i];
            }
            fclose(file_ptr); 
            Genome_Copy(id, id, 1);
        }
    }
    
    Genome_Hash(id);
}

void Genome_Hash(uint16_t id)
{
    // if(debug) fprintf(stderr, "\nGenome_Hash");
    uint16_t hash = 0;
    for (int i = 0; i < GENOME_SIZE; i++) {
        hash += population[id].genome[i] * (GENOME_SIZE - i);
    }
    population[id].genome_hash = hash;
    population[id].target_hash = hash;
}

void Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate)
{
    // if(debug) fprintf(stderr, "\nGenome_Copy");
    srand(clock());
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        population[id2].genome[i] = population[id1].genome[i];
        population[id2].child_genome[i] = population[id2].genome[i];
        if(mutate)
        {
            if(rand() % (GENOME_SIZE * 1 / mutate) == 0) 
            {
                population[id2].genome[i] = rand() % OP_COUNT;
                population[id2].child_genome[i] = population[id2].genome[i];
            }
        }
    }
    if(mutate)
    {
        if(rand() % (GENOME_SIZE * 10 / mutate) == 0) 
        {
            Mutate_Swap_Blocks(id2);
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id2].child_genome[i] = population[id2].genome[i];
            }
        }
        
    }
    
    char buf[32];
    snprintf(buf, sizeof(buf), "genomes/genome_%d.txt", id2);
    
    file_ptr = NULL;
    file_ptr = fopen(buf, "w");

    for(int i = 0; i < GENOME_SIZE; i++)
    {
        fprintf(file_ptr, "%i\n",population[id2].genome[i]);
        
    }
    fclose(file_ptr);
    Genome_Hash(id2);
    // printf("\n%d", mutate);
}

void Child_Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate)
{
    // if(debug) fprintf(stderr, "\nChild_Genome_Copy");
    srand(clock());
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        population[id2].genome[i] = population[id1].child_genome[i];
        if(mutate)
        {
            if(rand() % (GENOME_SIZE * 1 / mutate) == 0) 
            {
                population[id2].genome[i] = rand() % OP_COUNT;
            }
        }
    }
    if(mutate)
    {
        if(rand() % (GENOME_SIZE * 10 / mutate) == 0) 
        {
            Mutate_Swap_Blocks(id2);
        }
        
    }
    
    char buf[32];
    snprintf(buf, sizeof(buf), "genomes/genome_%d.txt", id2);
    
    file_ptr = NULL;
    file_ptr = fopen(buf, "w");

    for(int i = 0; i < GENOME_SIZE; i++)
    {
        fprintf(file_ptr, "%i\n",population[id2].genome[i]);
        
    }
    fclose(file_ptr);
    Genome_Hash(id2);
    // printf("\n%d", mutate);
}

void Mutate_Swap_Blocks(uint16_t id)
{
    // if(debug) fprintf(stderr, "\nMutate_Swap_Blocks");
    uint8_t len = 2 + rand() % GENOME_SIZE / 32;
    uint8_t pos1 = rand() % (GENOME_SIZE - len);
    uint8_t pos2 = rand() % (GENOME_SIZE - len);
    for (int i = 0; i < len; i++) {
        uint8_t temp = population[id].genome[pos1 + i];
        population[id].genome[pos1 + i] = population[id].genome[pos2 + i];
        population[id].genome[pos2 + i] = temp;
    }
}

void Best_Genome_Spread()
{
    // if(debug) fprintf(stderr, "\nBest_Genome_Spread");
    // printf("best_genome_spread\n");
    uint8_t current_org = 0;
    uint16_t current_best = free_top;
    
    while(current_best >= MAX_ORGANISMS * (re_frac - 1) / re_frac)
    {
        Genome_Copy(current_best, current_org, 0);
        uint8_t bot = min(current_org + 1, MAX_ORGANISMS);
        uint8_t mid = min(current_org + (re_frac - 1) / 2, MAX_ORGANISMS);
        uint8_t top = min(current_org + (re_frac - 1) - 1, MAX_ORGANISMS);
        uint8_t last = min(current_org + re_frac - 1, MAX_ORGANISMS);
        Genome_Copy(current_best, last, GENOME_SIZE);
        for(int org = bot; org < mid; org++)
        {
            Genome_Copy(current_best, org, GENOME_SIZE);
            
        }
        for(int org = mid; org < top; org++)
        {
            Genome_Copy(current_best, org, 1);
            
        }
        current_best--;
        current_org += MAX_ORGANISMS / re_frac;
    }
}

void Child_Genome_Combine(uint16_t id1, uint16_t id2)
{
    srand(clock());
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        if(rand() % 2)
            population[id2].child_genome[i] = population[id1].genome[i];
    }
}

uint8_t Organism_Quit(uint16_t id)
{
    // if(debug) fprintf(stderr, "\nOrganism_Quit");
    // printf("organism_quit\n");
    if (id == 0 || id >= MAX_ORGANISMS) return 0;
    if (population[id].alive == 0) return 0;
    
    Grid_Signal(population[id].nuc_x, population[id].nuc_y, 0, 0, 0);
    
    int16_t min_x = population[id].min_x;
    int16_t min_y = population[id].min_y;
    int16_t max_x = population[id].max_x;
    int16_t max_y = population[id].max_y;
    
    if(population[id].material > 0
    && population[id].volume > 0)
    {
        uint32_t leftover = population[id].material;
        uint32_t cells = population[id].volume;
        uint32_t per_cell = leftover / cells;  // материал на каждую клетку

        // Проходим по всем клеткам организма
        for(int i = min_y; i <= max_y; i++)
        {
            for(int j = min_x; j <= max_x; j++)
            {
                if(Grid_Get(j, i)->id == id)
                {
                    if(population[id].material >= per_cell)
                    {
                        Grid_Get(j, i)->mat += per_cell;
                        population[id].material -= per_cell;
                    }
                }
            }
        }

        // Добавляем остаток в ядро
        Cell* nucleus = Grid_Get(population[id].nuc_x, population[id].nuc_y);
        
        if(nucleus->id != 0)
            nucleus->mat += population[id].material;
        else 
        {
            Grid_Set(population[id].nuc_x, population[id].nuc_y, MAX_ORGANISMS);
            nucleus->mat += population[id].material;
        }
        population[id].material = 0;
        // printf("with volume\n");
    }
    else if(population[id].material > 0)
    {
        // printf("without volume\n");
        // printf("id below: %d nucleus id: %d\n", Grid_Get(population[id].nuc_x, population[id].nuc_y)->id, id);
        // Нет клеток — весь материал кладём в ядро (которое, возможно, тоже не клетка, но это место его последнего положения)
        
        if(Grid_Get(population[id].nuc_x, population[id].nuc_y)->id != 0)
        {
            // printf("1");
            Grid_Get(population[id].nuc_x, population[id].nuc_y)->mat += population[id].material;
        }
        else 
        {
            // printf("2");
            Grid_Set(population[id].nuc_x, population[id].nuc_y, MAX_ORGANISMS);
            Grid_Get(population[id].nuc_x, population[id].nuc_y)->mat += population[id].material;
        }
        population[id].material = 0;
    }
    else
    {
        // printf("without material and volume\n");
        // printf("id below: %d nucleus id: %d\n", Grid_Get(population[id].nuc_x, population[id].nuc_y)->id, id);
        
    }
    for(int i = min_y - 1; i <= max_y + 1; i++)
    {
        for(int j = min_x - 1; j <= max_x + 1; j++)
        {
            if(Grid_Get(j, i)->id == id)
            {
                Grid_Get(j, i)->lifetime = 0;
                Grid_Get(j, i)->id = MAX_ORGANISMS;
                if(population[id].solidify)
                    Grid_Get(j, i)->solid = 1;
            }
        }
    }
    
    Cell* nuc = Grid_Get(population[id].nuc_x, population[id].nuc_y);
    if (nuc->id == id) {
        nuc->lifetime = 0;
        nuc->id = MAX_ORGANISMS;
        // mat остаётся
    }
    
    // Поднимаем указатель и кладем ID обратно в стек
    free_top++;
    free_stack[free_top] = id;
    
    deaths++;
    
    // Опционально: обнуляем данные в массиве организмов
    
    population[id].nuc_x = 0;
    population[id].nuc_y = 0;
    population[id].material = 0;
    population[id].min_mat = 40;
    population[id].volume = 0;
    population[id].target_vol = 0;
    population[id].alive = 0;
    population[id].multiply = 0;
    population[id].lifetime = 0;
    population[id].target_dx = 0;
    population[id].target_dy = 0;
    population[id].target_str = 0;
    population[id].other_dx = 0;
    population[id].other_dy = 0;
    population[id].other_str = 0;
    population[id].pain_dx = 0;
    population[id].pain_dy = 0;
    population[id].pain_str = 0;
    population[id].free_dx = 0;
    population[id].free_dy = 0;
    population[id].free_dist = 0;
    population[id].free_str = 0;
    population[id].friend_dx = 0;
    population[id].friend_dy = 0;
    population[id].friend_str = 0;
    population[id].vx = 0;
    population[id].vy = 0;
    population[id].move = 0;
    population[id].take_mat = 0;
    population[id].attack = 0;
    population[id].flag_0 = 0;
    population[id].flag_1 = 0;
    population[id].flag_2 = 0;
    population[id].is_flag_0 = 0;
    population[id].is_flag_1 = 0;
    population[id].is_flag_2 = 0;
    population[id].gp = 0;
    population[id].genome_hash = 0;
    population[id].target_hash = 0;
    population[id].mutate_chance = base_mutate_chance;
    population[id].pacifism_treshold = 1;
    population[id].life_wave_timer = 0;
    population[id].signal_timer = 0;
    population[id].grow = 0;
    population[id].shrink = 0;
    population[id].newborn = 0;
    population[id].sex = 0;
    population[id].partner_id = 0;
    population[id].fertilized = 0;
    population[id].has_reproduced = 0;
    population[id].solidify = 0;
    
    return 1;
}

uint16_t Is_Membrane(int16_t x, int16_t y)
{
    if(debug > 1) fprintf(stderr, "\nIs_Membrane");
    uint16_t id = Grid_Get(x, y)->id;
    uint16_t id1;
    uint16_t counter = 0;
    
    for(int dy = -1; dy < 2; dy++)
    {
        for(int dx = -1; dx < 2; dx++)
        {
            id1 = Grid_Get(x + dx, y + dy)->id;
            if(id1 != id)
            {
                return 1; 
            }
        }
    }
    
    return 0;
}

void Process_Membrane(int16_t x, int16_t y)
{
    Cell* current = Grid_Get(x, y);
    uint16_t id = current->id;
    
    uint16_t id1;
    uint16_t counter = 0;
    uint16_t other = 0;
    uint16_t target = 0;
    uint16_t free = 0;
    uint16_t free_dist = 65535;
    uint16_t friends = 0;
    
    uint8_t flag_0 = 0;
    uint8_t flag_1 = 0;
    uint8_t flag_2 = 0;
    
    uint16_t flag_0_str = 0;
    uint16_t flag_1_str = 0;
    uint16_t flag_2_str = 0;
    
    uint32_t hash, hash_id;
    uint16_t partner_id = population[id].partner_id;
    uint32_t partner_hash = population[partner_id].target_hash;
    uint32_t random_hash;
    uint16_t linear_size = max(population[id].max_x - population[id].min_x
    + population[id].max_y - population[id].min_y, 1);
    
    int8_t vx = population[id].vx;
    int8_t vy = population[id].vy;
    uint8_t ax = abs(vx);
    uint8_t ay = abs(vy);
    
    for(int dy = -1; dy < 2; dy++)
    {
        for(int dx = -1; dx < 2; dx++)
        {
            Cell* cell = Grid_Get(x + dx, y + dy);
            id1 = cell->id;
            
            if(population[id1].alive == 1
            && id1 != id && id1 != 0 && id1 != MAX_ORGANISMS)
            {
                hash = population[id1].genome_hash;
                partner_id = population[id].partner_id;
                partner_hash = population[partner_id].target_hash;
            
                uint32_t diff_candidate = abs(hash - population[id].target_hash);
                uint32_t diff_existing = abs(partner_hash - population[id].target_hash);
                uint32_t pacifism_limit = max(max_pacifism_threshold * population[id].pacifism_treshold / 64, 1);
                uint32_t difference = abs(hash - population[id].genome_hash);
                int chance = difference * 100 / pacifism_limit;
                
                if(rand() % 100 < chance && rand() % linear_size == 0)
                {
                    if (partner_id == 0) 
                    {
                        population[id].partner_id = id1;
                    } 
                    else if (diff_candidate < diff_existing) 
                    {
                        population[id].partner_id = id1;
                    }
                }
            }
            
            if(id1 != id
            && (cell->lifetime != 0
            && abs(population[id].genome_hash - population[id1].genome_hash)
             > (max_pacifism_threshold * population[id].pacifism_treshold / 64)
            || id1 == MAX_ORGANISMS
            || id1 == 0
            || population[id1].alive == 0))
            {
                counter++;
                if(id1 == MAX_ORGANISMS || population[id1].alive == 0) 
                {
                    if(cell->solid)
                        target += 1;
                    else
                        target += cell->mat;
                }
                else if(id1 != 0)
                {
                    if(population[id1].take_mat)
                        other += 31;
                    else
                        other += 3;
                    if(population[id1].flag_0)
                        flag_0 += population[id1].flag_0;
                    if(population[id1].flag_1)
                        flag_1 += population[id1].flag_1;
                    if(population[id1].flag_2)
                        flag_2 += population[id1].flag_2;
                }
                else if(id1 == 0)
                {
                    free += 31;
                }
            }
            else if(id1 != id
            && (cell->lifetime != 0
            && abs(population[id].genome_hash - population[id1].genome_hash)
             <= (max_pacifism_threshold * population[id].pacifism_treshold / 64)))
            {
                friends += 31;   
            }
            
            if(cell->flag_0)
            {
                flag_0_str += cell->flag_0;
            }
            if(cell->flag_1)
            {
                flag_1_str += cell->flag_1;
            }
            if(cell->flag_2)
            {
                flag_2_str += cell->flag_2;
            }
        }
    }
    
    if(counter == 0 && current->vx == 0 && current->vy == 0)
    {
        Grid_Signal(x, y, 0, 0, 0);
    }
    
    if(population[id].alive)
    {
        int16_t dx_to_nuc = x - population[id].nuc_x;
        int16_t dy_to_nuc = y - population[id].nuc_y;
        
        if(other > population[id].other_str)
        {
            population[id].other_str = other;
            population[id].other_dx = dx_to_nuc;
            population[id].other_dy = dy_to_nuc;
        }
        if(target > population[id].target_str)
        {
            population[id].target_str = target;
            population[id].target_dx = dx_to_nuc;
            population[id].target_dy = dy_to_nuc;
        }
        
        free_dist = max(abs(dx_to_nuc), abs(dy_to_nuc));
        
        int16_t dot = 0;
        int16_t vel_len_sq = vx*vx + vy*vy;
        
        if(vel_len_sq > 0)
        {
            dot = dx_to_nuc * vx + dy_to_nuc * vy;
        }
        
        if(vel_len_sq == 0)
        {
            // Старая логика: выбираем ближайшую свободную клетку
            if((free_dist < population[id].free_dist && free > 0)
            || (free_dist == population[id].free_dist && free > population[id].free_str))
            {
                population[id].free_str = free;
                population[id].free_dist = free_dist;
                population[id].free_dx = dx_to_nuc;
                population[id].free_dy = dy_to_nuc;
            }
        }
        else
        {
            int16_t existing_dot = population[id].free_dx * vx + population[id].free_dy * vy;
            
            if(population[id].free_str == 0 || dot > existing_dot)
            {
                population[id].free_str = free;
                population[id].free_dist = free_dist;
                population[id].free_dx = dx_to_nuc;
                population[id].free_dy = dy_to_nuc;
            }
            else if(dot == existing_dot && (free_dist < population[id].free_dist
            || free_dist == population[id].free_dist && free > population[id].free_str)
            
            )
            {
                population[id].free_str = free;
                population[id].free_dist = free_dist;
                population[id].free_dx = dx_to_nuc;
                population[id].free_dy = dy_to_nuc;
            }
        }
        
        if(flag_0 > population[id].is_flag_0)
        {
            population[id].is_flag_0 = flag_0;
        }
        if(flag_1 > population[id].is_flag_1)
        {
            population[id].is_flag_1 = flag_1;
        }
        if(flag_2 > population[id].is_flag_2)
        {
            population[id].is_flag_2 = flag_2;
        }
        if(flag_0_str > population[id].flag_0_str)
        {
            population[id].flag_0_str = flag_0_str;
            population[id].flag_0_dx = dx_to_nuc;
            population[id].flag_0_dy = dy_to_nuc;
        }
        if(flag_1_str > population[id].flag_1_str)
        {
            population[id].flag_1_str = flag_1_str;
            population[id].flag_1_dx = dx_to_nuc;
            population[id].flag_1_dy = dy_to_nuc;
        }
        if(flag_2_str > population[id].flag_2_str)
        {
            population[id].flag_2_str = flag_2_str;
            population[id].flag_2_dx = dx_to_nuc;
            population[id].flag_2_dy = dy_to_nuc;
        }
        if(friends > population[id].friend_str)
        {
            population[id].friend_str = friends;
            population[id].friend_dx = dx_to_nuc;
            population[id].friend_dy = dy_to_nuc;
        }
        
        partner_id = population[id].partner_id;
        partner_hash = population[partner_id].target_hash;
        
        if(population[partner_id].alive == 0) population[id].partner_id = 0;
        
        if(partner_id != 0 && id == population[partner_id].partner_id
         && population[id].sex == 0
         && population[id].solidify == 0 && population[partner_id].solidify == 0)
        {
            Child_Genome_Combine(partner_id, id);
            population[id].fertilized = 1;
            population[id].partner_id = 0;
            population[partner_id].partner_id = 0;
            population[partner_id].has_reproduced = 1;
            // printf("%5d fertilized by %5d\n", id, partner_id);
        }
    }
}

uint16_t Id_Count(int16_t x, int16_t y, uint16_t id)
{
    // if(debug) fprintf(stderr, "\nId_Count");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint8_t counter = 0;
    
    for(int dy = -1; dy < 2; dy++)
    {
        for(int dx = -1; dx < 2; dx++)
        {
            if(Grid_Get(x1 + dx, y1 + dy)->id == id)
            {
                counter++;
            }
        }
    }
    return counter;
}

uint8_t Expand(int16_t x, int16_t y, int8_t strength)
{
    // if(debug) fprintf(stderr, "\nExpand");
    uint16_t id = Grid_Get(x, y)->id;
    uint16_t id_to = Grid_Get(x, y)->id;
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    int8_t str_available = strength;
    
    if(strength > 0)
    {
        uint8_t expanded = 0;
        for(int dy = -1; dy < 2; dy++)
        {
            for(int dx = -1; dx < 2; dx++)
            {
                id_to = Grid_Get(x + dx, y + dy)->id;
                int16_t vx = population[id].vx;
                int16_t vy = population[id].vy;
                uint8_t penetrate = (
                (max(abs(vx) + 1, abs(vy) + 1) * population[id].volume
                 > population[id_to].volume)
                && id_to != 0
                && population[id].attack);
                uint8_t is_full = population[id].material > population[id].min_mat + population[id].volume
                && id_to != 0;
                
                if((penetrate && !is_full && id_to != id
                && (dx != 0 || dy != 0)
                && Grid_Get(x1 + dx, y1 + dy)->lifetime != 0)
                || id_to == 0
                || id_to == MAX_ORGANISMS)
                {
                    Grid_Set(x1 + dx, y1 + dy, id);
                    expanded = 1;
                    Grid_Add_Cooldown(x1 + dx, y1 + dy, 4);
                    if(penetrate && 0)
                    {
                        population[id].vx -= sign(population[id].vx);
                        population[id].vy -= sign(population[id].vy);
                    }
                }
            }
        }
        for(int dy = -1; dy < 2; dy++)
        {
            for(int dx = -1; dx < 2; dx++)
            {
                id_to = Grid_Get(x + dx, y + dy)->id;
                // Grid_Add_Cooldown(x1 + dx, y1 + dy, 1);
                if(id_to == id
                && Is_Membrane(x1 + dx, y1 + dy)
                && Grid_Get(x1 + dx, y1 + dy)->cooldown == 0
                && Grid_Get(x1 + dx, y1 + dy)->lifetime != 0)
                {
                    Grid_Signal(x1 + dx, y1 + dy, 0, 0, strength - 1);
                    // if(Grid_Get(x1 + dx, y1 + dy)->flag_0)
                    // {
                    //     Grid_Get(x1, y1)->flag_0 = max(Grid_Get(x1, y1)->flag_0 - 1, 0);
                    // }
                    // if(Grid_Get(x1 + dx, y1 + dy)->flag_1)
                    // {
                    //     Grid_Get(x1, y1)->flag_1 = max(Grid_Get(x1, y1)->flag_1 - 1, 0);
                    // }
                    // if(Grid_Get(x1 + dx, y1 + dy)->flag_2)
                    // {
                    //     Grid_Get(x1, y1)->flag_2 = max(Grid_Get(x1, y1)->flag_2 - 1, 0);
                    // }
                }
            }
        }
        if(expanded)
        {
            Grid_Add_Cooldown(x1, y1, strength);
            Grid_Signal(x1, y1, 0, 0, 0);
        }
    }
    else if(strength < 0)
    {
        uint8_t is_membrane = Is_Membrane(x1, y1);
        for(int dy = -1; dy < 2; dy++)
        {
            for(int dx = -1; dx < 2; dx++)
            {
                if(Grid_Get(x1 + dx, y1 + dy)->id == id
                && Is_Membrane(x1 + dx, y1 + dy)
                && Grid_Get(x1 + dx, y1 + dy)->cooldown == 0
                && Grid_Get(x1 + dx, y1 + dy)->strength == 0
                && Grid_Get(x1 + dx, y1 + dy)->type == 2
                && Grid_Get(x1 + dx, y1 + dy)->lifetime != 0
                && population[id].alive)
                {
                    Grid_Signal(x1 + dx, y1 + dy, 0, 0, strength + 1);
                    
                }
                if(Grid_Get(x1 + dx, y1 + dy)->id == id
                && Is_Membrane(x1 + dx, y1 + dy) == 0)
                {
                    Grid_Add_Cooldown(x1 + dx, y1 + dy, -strength - 1);
                    
                }
            }
        }
        
        Grid_Add_Cooldown(x1, y1, -strength);
        if(Grid_Get(x1, y1)->type == 2
        && population[id].alive
        && is_membrane
        && Grid_Get(x1, y1)->id != MAX_ORGANISMS)
        {
            Grid_Set(x1, y1, 0);
        }
        
        
        
    }
    return str_available;
}

void Organism_Update()
{
    // if(debug) fprintf(stderr, "\nOrganism_Update");
    for(int id = 0; id < MAX_ORGANISMS; id++)
    {
        uint16_t i = order[id];
        if(population[i].alive)
        {
            population[i].nuc_x = mod(population[i].nuc_x, grid_width);
            population[i].nuc_y = mod(population[i].nuc_y, grid_height);
            uint16_t x = population[i].nuc_x;
            uint16_t y = population[i].nuc_y;
            
            if(Grid_Get(x, y)->id != i || Grid_Get(x, y)->id == 0)
            {
                // printf("\n reason of death: nucleus without body");
                if(Organism_Quit(i) == 1)
                    continue;
            }
            
            // АВТОМАТИЧЕСКАЯ ВОЛНА ЖИЗНИ (каждые N шагов)
            population[i].life_wave_timer++;
            if(population[i].life_wave_timer >= 127)
            {
                population[i].life_wave_timer = 0;
                
                grid_array[y][x].life_wave_str = max(
                population[i].max_x - population[i].min_x, 
                population[i].max_y - population[i].min_y);
                grid_array[y][x].lifetime = 255;
            }
            
            grid_array[y][x].type = 2;
            
            uint16_t flag, flag_str = 16;
            uint32_t step;
            
            uint8_t gp = population[i].gp;
            uint8_t op = population[i].genome[gp];
            
            uint8_t move = population[i].move;
            uint8_t grow = population[i].grow;
            uint8_t shrink = population[i].shrink;
            
            uint8_t multiply = population[i].multiply;
            uint8_t print = 0;
            uint8_t dp = 1; 
            
            uint16_t max_str = max(
            max(
                population[i].max_x - population[i].min_x, 
                population[i].max_y - population[i].min_y),
                1
                );
            
            
            if(i == test_id && 0)
            {
                print = 1;
            }         
            
            switch (op)
            {
            case VX_POS:
                population[i].vx = min(population[i].vx + 1, 127);
                if(population[i].vx != 0)
                {
                    population[i].move = 1;
                    move = 1;
                }
                if(print) printf("\n VX_POS ");
                break;
            case VX_NEG:
                population[i].vx = max(population[i].vx - 1, -128);
                if(population[i].vx != 0)
                {
                    population[i].move = 1;
                    move = 1;
                }
                if(print) printf("\n VX_NEG ");
                break;
            case VY_POS:
                population[i].vy = min(population[i].vy + 1, 127);
                if(population[i].vy != 0)
                {
                    population[i].move = 1;
                    move = 1;
                }
                if(print) printf("\n VY_POS ");
                break;
            case VY_NEG:
                population[i].vy = max(population[i].vy - 1, -128);
                if(population[i].vy != 0)
                {
                    population[i].move = 1;
                    move = 1;
                }
                if(print) printf("\n VY_NEG ");
                break;
            case ACCEL:
                population[i].move = 1;
                move = 1;
                
                if(population[i].vx == 0 
                && population[i].vy == 0)
                {
                    population[i].vx = rand() % 256 - 128;
                    population[i].vy = rand() % 256 - 128;
                }
            
                if(abs(population[i].vx) <= 63 && abs(population[i].vy) <= 63) 
                {
                    population[i].vx *= 2;
                    population[i].vy *= 2;
                }
                if(print) printf("\n ACCEL ");
                break;
            case STOP:
                population[i].vx /= 2;
                population[i].vy /= 2; 
                if(population[i].vx == 0 && population[i].vy == 0)
                {
                    population[i].move = 0;
                    move = 0;
                }
                if(print) printf("\n STOP ");
                break;
            case GROW:
                population[i].grow = 1;
                grow = 1;
                
                population[i].move = 1;
                move = 1;
                population[i].signal_timer = abs(population[i].strength);
                if(population[i].vx == 0 
                && population[i].vy == 0)
                {
                    population[i].vx = rand() % 256 - 128;
                    population[i].vy = rand() % 256 - 128;
                }
                if(print) printf("\n GROW ");
                break;
            case SHRINK:
                population[i].shrink = 1;
                shrink = 1;
                
                population[i].move = 1;
                move = 1;
                population[i].signal_timer = abs(population[i].strength);
                if(population[i].vx == 0 
                && population[i].vy == 0)
                {
                    population[i].vx = rand() % 256 - 128;
                    population[i].vy = rand() % 256 - 128;
                }
                if(print) printf("\n SHRINK ");
                break;
            case STR_POS:
                population[i].strength = min(population[i].strength + 1, max_str);
                if(print) printf("\n STR_POS ");
                break;
            case STR_NEG:
                population[i].strength = max(population[i].strength - 1, 1);
                if(print) printf("\n STR_NEG ");
                break;
            case STR_ONE:
                population[i].strength = 1;
                if(print) printf("\n STR_ONE ");
                break;
            case STR_MAX:
                population[i].strength = max_str;
                if(print) printf("\n STR_MAX ");
                break;
            case GOTO_ZONE_0:
                population[i].gp = 0;
                dp = 0;
                if(print) printf("\n GOTO_0 ");
                break;
            case GOTO_ZONE_1:
                population[i].gp = GENOME_SIZE / 3;
                dp = 0;
                if(print) printf("\n GOTO_0 ");
                break;
            case GOTO_ZONE_2:
                population[i].gp = GENOME_SIZE * 2 / 3;
                dp = 0;
                if(print) printf("\n GOTO_0 ");
                break;
            case SKIP:
                dp = 2;
                if(print) printf("\n SKIP ");
                break;
            case CHECK_OTHER_DX:
                if (population[i].other_str == 0) 
                {
                    dp = 5;
                } else if (population[i].other_dx > 0)
                {
                    dp = 3;
                } else if (population[i].other_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_TARGET_DX ");
                break;
            case CHECK_OTHER_DY:
                if (population[i].other_str == 0) 
                {
                    dp = 5;
                } else if (population[i].other_dy > 0)
                {
                    dp = 3;
                } else if (population[i].other_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_TARGET_DY ");
                break;
            case CHECK_TARGET_DX:
                if (population[i].target_str == 0) 
                {
                    dp = 5;
                } else if (population[i].target_dx > 0)
                {
                    dp = 3;
                } else if (population[i].target_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_TARGET_DX ");
                break;
            case CHECK_TARGET_DY:
                if (population[i].target_str == 0) 
                {
                    dp = 5;
                } else if (population[i].target_dy > 0)
                {
                    dp = 3;
                } else if (population[i].target_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_TARGET_DY ");
                break;
            case CHECK_PAIN_DX:
                if (population[i].pain_str == 0) 
                {
                    dp = 5;
                } else if (population[i].pain_dx > 0)
                {
                    dp = 3;
                } else if (population[i].pain_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_PAIN_DX ");
                break;
            case CHECK_PAIN_DY:
                if (population[i].pain_str == 0) 
                {
                    dp = 5;
                } else if (population[i].pain_dy > 0)
                {
                    dp = 3;
                } else if (population[i].pain_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_PAIN_DY ");
                break;
            case CHECK_FREE_DX:
                if (population[i].free_str == 0) 
                {
                    dp = 5;
                } else if (population[i].free_dx > 0)
                {
                    dp = 3;
                } else if (population[i].free_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_FREE_DX ");
                break;
            case CHECK_FREE_DY:
                if (population[i].free_str == 0) 
                {
                    dp = 5;
                } else if (population[i].free_dy > 0)
                {
                    dp = 3;
                } else if (population[i].free_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_FREE_DY ");
                break;
            case CHECK_FRIEND_DX:
                if (population[i].friend_str == 0) 
                {
                    dp = 5;
                } else if (population[i].friend_dx > 0)
                {
                    dp = 3;
                } else if (population[i].friend_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_FRIEND_DX ");
                break;
            case CHECK_FRIEND_DY:
                if (population[i].friend_str == 0) 
                {
                    dp = 5;
                } else if (population[i].friend_dy > 0)
                {
                    dp = 3;
                } else if (population[i].friend_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_FRIEND_DY ");
                break;
            case CHECK_MAT:
                if (population[i].material == 0) 
                {
                    dp = 5;          // нет материала
                } else if (population[i].material < population[i].min_mat) 
                {
                    dp = 3;          // мало материала
                } else 
                {
                    dp = 1;          // достаточно для размножения
                }
                if(print) printf("\n CHECK_MAT ");
                break;
            case CHECK_MULT:
                if (population[i].multiply == 0) 
                {
                    dp = 3;          // не в режиме размножения
                } else 
                {
                    dp = 1;          // в режиме размножения
                }
                if(print) printf("\n CHECK_MULT ");
                break;
            case CHECK_VEL:
                {
                uint8_t speed = max(abs(population[i].vx), abs(population[i].vy));
                if (speed < 43)
                {
                    dp = 1;
                } else if (speed < 85)
                {
                    dp = 3;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_VEL ");
                break;
                }
            case CHECK_VOL:
            {
                uint32_t vol = population[i].volume;
                if (vol < population[i].min_mat)
                {
                    dp = 1;
                } else if (vol < 3 * population[i].min_mat)
                {
                    dp = 3;
                } else
                {
                    dp = 5;
                }
                if(print) printf("\n CHECK_VOL ");
                break;
            }
            case MULTIPLY:
                if(population[i].material > 2 * population[i].min_mat) 
                {
                    multiply = 1;
                    population[i].multiply = 1;
                    
                    move = 1;
                    population[i].move = 1;
                    if(population[i].vx == 0 
                    && population[i].vy == 0)
                    {
                        population[i].vx = rand() % 256 - 128;
                        population[i].vy = rand() % 256 - 128;
                    }
                }
                if(print) printf("\n MULTIPLY ");
                break;
            case SET_STATE_0:
                population[i].state = 0;
                break;
            case SET_STATE_1:
                population[i].state = 1;
                break;
            case SET_STATE_2:
                population[i].state = 2;
                break;
            case IF_STATE_0:
                if(population[i].state == 0)
                {
                    dp = 3;
                } else 
                {
                    dp = 1;
                }
                break;
            case IF_STATE_1:
                if(population[i].state == 1)
                {
                    dp = 3;
                } else 
                {
                    dp = 1;
                }
                break;
                break;
            case IF_STATE_2:
                if(population[i].state == 2)
                {
                    dp = 3;
                } else 
                {
                    dp = 1;
                }
                break;
                break;
            case MUTATE_POS:
                population[i].mutate_chance = min(population[i].mutate_chance + 1, GENOME_SIZE);
                break;
            case MUTATE_NEG:
                population[i].mutate_chance = max(population[i].mutate_chance - 1, 1);
                break;
            case PACIFISM_POS:
                population[i].pacifism_treshold = min(population[i].mutate_chance + 1, 64);
                break;
            case PACIFISM_NEG:
                population[i].pacifism_treshold = max(population[i].mutate_chance - 1, 0);
                break;
            case TAKE_MAT_ON:
                population[i].take_mat = 1;
                break;
            case TAKE_MAT_OFF:
                population[i].take_mat = 0;
                break;
            case ATTACK_ON:
                population[i].attack = 1;
                break;
            case ATTACK_OFF:
                population[i].attack = 0;
                break;
            case SET_FLAG_0:
                population[i].flag_0 = flag_str;
                break;
            case SET_FLAG_1:
                population[i].flag_1 = flag_str;
                break;
            case SET_FLAG_2:
                population[i].flag_2 = flag_str;
                break;
            case CHECK_FLAG_0:
                flag = population[i].is_flag_0;
                if (flag == 0) 
                {
                    dp = 5;
                } else if (flag <= 3 * flag_str) 
                {
                    dp = 3;
                } else 
                {
                    dp = 1;
                }
                break;
            case CHECK_FLAG_1:
                flag = population[i].is_flag_1;
                if (flag == 0) 
                {
                    dp = 5;
                } else if (flag <= 3 * flag_str)
                {
                    dp = 3;
                } else 
                {
                    dp = 1;
                }
                break;
            case CHECK_FLAG_2:
                flag = population[i].is_flag_2;
                if (flag == 0) 
                {
                    dp = 5;
                } else if (flag <= 3 * flag_str)
                {
                    dp = 3;
                } else 
                {
                    dp = 1;
                }
                break;
            case CHECK_FLAG_0_DX:
                if (population[i].flag_0_str == 0) 
                {
                    dp = 5;
                } else if (population[i].flag_0_dx > 0)
                {
                    dp = 3;
                } else if (population[i].flag_0_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                break;
            case CHECK_FLAG_0_DY:
                if (population[i].flag_0_str == 0) 
                {
                    dp = 5;
                } else if (population[i].flag_0_dy > 0)
                {
                    dp = 3;
                } else if (population[i].flag_0_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                break;
            case CHECK_FLAG_1_DX:
                if (population[i].flag_1_str == 0) 
                {
                    dp = 5;
                } else if (population[i].flag_1_dx > 0)
                {
                    dp = 3;
                } else if (population[i].flag_1_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                break;
            case CHECK_FLAG_1_DY:
                if (population[i].flag_1_str == 0) 
                {
                    dp = 5;
                } else if (population[i].flag_1_dy > 0)
                {
                    dp = 3;
                } else if (population[i].flag_1_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                break;
            case CHECK_FLAG_2_DX:
                if (population[i].flag_2_str == 0) 
                {
                    dp = 5;
                } else if (population[i].flag_2_dx > 0)
                {
                    dp = 3;
                } else if (population[i].flag_2_dx < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                break;
            case CHECK_FLAG_2_DY:
                if (population[i].flag_2_str == 0) 
                {
                    dp = 5;
                } else if (population[i].flag_2_dy > 0)
                {
                    dp = 3;
                } else if (population[i].flag_2_dy < 0)
                {
                    dp = 1;
                } else
                {
                    dp = 5;
                }
                break;
            case CHECK_SEX:
                if (population[i].sex == 0) 
                {
                    dp = 1;
                } else 
                {
                    dp = 3;
                }
                if(print) printf("\n CHECK_SEX ");
                break;
            case FLIP_SEX:
                if(population[i].fertilized == 0)
                    population[i].sex = 1 - population[i].sex;
                if(print) printf("\n FLIP_SEX ");
                break;
            case TARGET_HASH_POS:
                step = max(target_hash_step, 1) * (population[i].pacifism_treshold + 1);
                population[i].target_hash = min(population[i].target_hash
                 + step, max_pacifism_threshold);
                break;
            case TARGET_HASH_NEG:
                step = max(target_hash_step, 1) * (population[i].pacifism_treshold + 1);
                population[i].target_hash = max(population[i].target_hash
                 - step, 0);
                break;
            case TARGET_HASH_RAND:
                population[i].target_hash = rand() % max_pacifism_threshold;
                break;
            case SOLIDIFY:
                if(population[i].has_reproduced == 0 && population[i].solidify == 0) 
                {
                    population[i].solidify = 1;
                    population[i].lifetime = 0;
                    solidify_count++;
                }
                break;
            
            default:
                break;
            }
            population[i].gp = mod(population[i].gp + dp, GENOME_SIZE);
            
            int8_t vx = population[i].vx;
            int8_t vy = population[i].vy;
            uint8_t ax = abs(vx);
            uint8_t ay = abs(vy);
            int8_t sx = sign(vx);
            int8_t sy = sign(vy);
            int8_t dx = 0;
            int8_t dy = 0;
            uint8_t w_diag = min(ax, ay);
            uint8_t w_total = max(ax, ay);
            
            if(i == test_id && 0)
            {
                printf("\nvx: %3d vy: %3d str: %4d", vx, vy, population[i].strength);
            }  
            // move = 0;
            // grow = 0;
            // shrink = 0;
            // multiply = 0;
            
            // ВЕРОЯТНОСТЬ движения (скорость 127 = 100%, 64 = 50%, 32 = 25%, и т.д.)
            uint8_t move_prob = max(ax, ay);
            if(rand() % 508 < move_prob)
            {
                if(i == test_id && 0)
                {
                    printf("\ncan move");
                }  
                // Движение разрешено, определяем направление
                if(w_total != 0) {
                    uint8_t r = rand() % w_total;
                    if (r < w_diag) {
                        dx = sx;
                        dy = sy;
                    } else {
                        if(ax > ay) dx = sx;
                        else dy = sy;
                    }
                }
                
                if(move && Grid_Get(x + dx, y + dy)->id == i
                && (Is_Membrane(x + dx, y + dy) == 0))
                { 
                    population[i].nuc_x = mod(population[i].nuc_x + dx, grid_width);
                    population[i].nuc_y = mod(population[i].nuc_y + dy, grid_height);
                }
                else if(move) // если ядро хочет двигаться, но не может, его надо тормозить
                {
                    population[i].vx /= 2;
                    population[i].vy /= 2;
                    
                    if(Is_Membrane(x, y) == 0)
                    Grid_Signal(population[i].nuc_x, population[i].nuc_y,
                                -population[i].vx, -population[i].vy,
                                -min(population[i].strength, max_str) - 1); 
                }
                
            }
            
            if (grow && population[i].signal_timer > 0) 
            {
                Grid_Signal(population[i].nuc_x, population[i].nuc_y,
                            population[i].vx, population[i].vy,
                            min(population[i].strength, max_str));
                population[i].signal_timer--;
                if(population[i].signal_timer == 0) population[i].grow = 0;
            }
            if (shrink && population[i].signal_timer > 0) 
            {
                Grid_Signal(population[i].nuc_x, population[i].nuc_y,
                                -population[i].vx, -population[i].vy,
                                -min(population[i].strength, max_str) - 1); 
                population[i].signal_timer--;
                if(population[i].signal_timer == 0) population[i].grow = 0;
            }
            
            // if(multiply) 
            // {
            //     population[i].signal_timer = 0;
            //     population[i].grow = 0;
            //     population[i].shrink = 0;
            // }
            
            grid_array[population[i].nuc_y][population[i].nuc_x].type = 1;
            
            if(population[i].solidify == 0 && multiply)
            {
                int deltax = population[i].free_dx;
                int deltay = population[i].free_dy;
                int x = population[i].nuc_x + deltax;
                int y = population[i].nuc_y + deltay;
                
                if(Grid_Get(x, y)->id == i)
                {
                    if(population[i].material > 2 * population[i].min_mat)
                    {
                        population[i].material -= population[i].min_mat;
                        
                        Grid_Set(x, y, 0);
                        
                        uint16_t child_id = 0;
                        child_id = Organism_Init(x, y);
                        
                        if(child_id != 0) {
                            Child_Genome_Copy(i, child_id, population[i].mutate_chance);
                            population[i].multiply = 0;
                            // if(population[i].has_reproduced == 0)
                                // population[i].lifetime = lifetime;
                            // printf("multiply success mat: %d\n", population[child_id].material);
                            if(population[i].fertilized)
                                sexual_reproductions++;
                            else
                                asexual_reproductions++;
                        } else {
                            population[i].material += population[i].min_mat;
                            // printf("multiply fail\n");
                        }
                        multiply = 0;
                    }
                    population[i].fertilized = 0;
                    population[i].has_reproduced = 1;
                }
            }
                
            population[i].target_str = max(population[i].target_str - 1, 0);
            if(population[i].target_str == 0)
            {
                population[i].target_dx = 0;
                population[i].target_dy = 0;
            }
            population[i].other_str = max(population[i].other_str - 1, 0);
            if(population[i].other_str == 0)
            {
                population[i].other_dx = 0;
                population[i].other_dy = 0;
            }
            population[i].pain_str = max(population[i].pain_str - 1, 0);
            if(population[i].pain_str == 0)
            {
                population[i].pain_dx = 0;
                population[i].pain_dy = 0;
            }
            population[i].free_str = max(population[i].free_str - 1, 0);
            population[i].free_dist = min(population[i].free_dist + 1, 65535);
            if(population[i].free_str == 0)
            {
                population[i].free_dx = 0;
                population[i].free_dy = 0;
            }
            population[i].friend_str = max(population[i].friend_str - 1, 0);
            if(population[i].friend_str == 0)
            {
                population[i].friend_dx = 0;
                population[i].friend_dy = 0;
            }
            
            population[i].is_flag_0 = max(population[i].is_flag_0 - 3, 0);
            population[i].is_flag_1 = max(population[i].is_flag_1 - 3, 0);
            population[i].is_flag_2 = max(population[i].is_flag_2 - 3, 0);
            
            population[i].flag_0 = max(population[i].flag_0 - 1, 0);
            population[i].flag_1 = max(population[i].flag_1 - 1, 0);
            population[i].flag_2 = max(population[i].flag_2 - 1, 0);
            
            population[i].flag_0_str = max(population[i].flag_0 - 1, 0);
            if(population[i].flag_0_str == 0)
            {
                population[i].flag_0_dx = 0;
                population[i].flag_0_dy = 0;
            }
            population[i].flag_1_str = max(population[i].flag_1 - 1, 0);
            if(population[i].flag_1_str == 0)
            {
                population[i].flag_1_dx = 0;
                population[i].flag_1_dy = 0;
            }
            population[i].flag_2_str = max(population[i].flag_2 - 1, 0);
            if(population[i].flag_2_str == 0)
            {
                population[i].flag_2_dx = 0;
                population[i].flag_2_dy = 0;
            }
            
            // if(grow == 0 && population[i].volume < population[i].material) population[i].material = max(population[i].material - 1, 0);
            
            if(multiply && population[i].material <= 2 * population[i].min_mat)
            {
                multiply = 0;
                population[i].multiply = 0;
            }
            
            // uint16_t volume = population[i].volume;
            // uint16_t base_volume = 1;
            // uint16_t volume_chance = 0;
            
            // uint16_t ratio;
            // uint16_t r;
            
            // if(volume <= base_volume) {
            //     volume_chance = 0;
            // } else {
            //     ratio = volume / base_volume;
            //     r = fast_root(ratio);
            //     volume_chance = 100 * (r - 1) / r;
            // }
            
            // uint8_t speed = max(ax, ay);
            
            // uint8_t speed_chance = (speed * 100) / 128;

            // uint16_t total_chance = max(volume_chance, speed_chance);

            // if(rand() % 100 >= volume_chance * 0) {
            //     population[i].lifetime = max(population[i].lifetime - 1, 0);
            // }
            // if(rand() % 100 < speed_chance * 0) {
            //     population[i].lifetime = max(population[i].lifetime - 1, 0);
            // }
            
            population[i].lifetime = max(population[i].lifetime - 1, 0);
            
            if(population[i].newborn == 1) 
            {
                if(population[i].volume == 0)
                {
                    printf("volume error!\n");
                }
                population[i].newborn = 0;
            }
            
            if(population[i].lifetime == 0)
            {
                // printf("\nreason of death: old age");
                Organism_Quit(i);
            }
        }
    }
    if(free_top >= MAX_ORGANISMS * (re_frac - 1) / re_frac) 
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "screenshots/material_steps:%d.png", step_counter);
        Save_Screenshot(buf, SCREENSHOT_MATERIALS);
        snprintf(buf, sizeof(buf), "screenshots/walls_steps:%d.png", step_counter);
        Save_Screenshot(buf, SCREENSHOT_WALLS);
        snprintf(buf, sizeof(buf), "screenshots/flags_steps:%d.png", step_counter);
        Save_Screenshot(buf, SCREENSHOT_FLAGS);
        Repopulate();
    }
}

uint16_t Most_Common_Neighbor(int16_t x, int16_t y)
{
    // if(debug) fprintf(stderr, "\nMost_Common_Neighbor");
    static uint16_t id_count[MAX_ORGANISMS];
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t id;
    uint16_t max_value = 0;
    uint16_t max_id = 0;
    uint8_t strength;
    
    for(int i = 0; i < MAX_ORGANISMS; i++)
    {
        id_count[i] = 0;
    }
    
    for(int dy = -1; dy < 2; dy++)
    {
        for(int dx = -1; dx < 2; dx++)
        {
            strength = (Grid_Get(x1 + dx, y1 + dy)->strength);
            id = Grid_Get(x1 + dx, y1 + dy)->id;
            if(id != 0) id_count[id] += 1 + strength;
        }
    }
    
    for(int i = 1; i < MAX_ORGANISMS; i++)
    {
        if(id_count[i] > max_value)
        {
            max_value = id_count[i];
            max_id = i;
        }
        else if(id_count[i] == max_value)
        {
            max_id = 0;
        }
    }
    return max_id;
}

void Repopulate()
{
    // if(debug) fprintf(stderr, "\nRepopulate");
    step_counter = 0;
    Best_Genome_Spread();
    
    for(int i = 0; i < MAX_ORGANISMS; i++)
    {
        Organism_Quit(i);
    }
    
    Grid_Reset(0);
    
    total_mat = 0;
    uint32_t org_mat = 0, grid_mat = 0, vol_mat = 0;
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            grid_mat += grid_array[i][j].mat;
        }
    }
    
    for(int i = 1; i < MAX_ORGANISMS; i++)
    {
        if(population[i].alive)
        {
            org_mat += population[i].material;
            vol_mat += population[i].volume;
        }
    }
    total_mat = org_mat + grid_mat;
    
    // printf("repop vol_mat: %5d org_mat: %5d grid_mat: %5d total_mat: %5d\n", vol_mat, org_mat, grid_mat, total_mat);
    
    if(total_mat != 0)
    {
        printf("%d", 1 / 0);
    }  
    
    uint16_t x, y;

    
    for(int i = 0; i < STARTING_ORGANISMS * food_mult; i++)
    {
        x = grid_width / 2 + (rand() % grid_width + rand() % grid_width) / 2;
        y = grid_height / 2 + (rand() % grid_height + rand() % grid_height) / 2;
        
        Grid_Set_Food(x, y);
        if(rand() % 10 == 0) Grid_Get(x, y)->solid = 1;
    }
    
    for(int i = 0; i < STARTING_ORGANISMS; i++)
    {
        // x = (rand() % grid_width + rand() % grid_width) / 2;
        // y = (rand() % grid_height + rand() % grid_height) / 2;
        x = rand() % grid_width;
        y = rand() % grid_height;
        if(Grid_Get(x, y)->id == 0)
            Organism_Init(x, y);
    }
}

void Order_Shuffle()
{
    // if(debug) fprintf(stderr, "\nOrder_Shuffle");
    for (int i = MAX_ORGANISMS - 1; i > 0; i--) 
    {
        int j = rand() % (i + 1);
        
        uint16_t temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

}

void Stats_CollectAndPrint()
{
    PopulationStats pop_stats;
    EcologyStats eco_stats;
    
    // Обнуляем структуры
    memset(&pop_stats, 0, sizeof(PopulationStats));
    memset(&eco_stats, 0, sizeof(EcologyStats));
    
    pop_stats.step = step_counter;
    pop_stats.asexual_reproductions = asexual_reproductions;
    pop_stats.sexual_reproductions = sexual_reproductions;
    pop_stats.deaths = deaths;
    pop_stats.solidify_count = solidify_count;
    
    uint32_t total_volume = 0;
    uint32_t total_material = 0;
    uint32_t total_neighbors = 0;
    uint32_t organism_count = 0;
    
    // Социальные градации
    uint32_t social_egoist = 0;      // pacifism <= 16
    uint32_t social_low = 0;         // 16 < pacifism <= 32
    uint32_t social_medium = 0;      // 32 < pacifism <= 48
    uint32_t social_high = 0;        // pacifism > 48
    
    // ========== ПРОХОД ПО ОРГАНИЗМАМ ==========
    for (int i = 1; i < MAX_ORGANISMS; i++) {
        if (!population[i].alive) continue;
        
        Organism* org = &population[i];
        organism_count++;
        
        total_volume += org->volume;
        total_material += org->material;
        
        if (org->volume > pop_stats.max_volume) pop_stats.max_volume = org->volume;
        if (org->material > pop_stats.max_material) pop_stats.max_material = org->material;
        
        if (org->take_mat) eco_stats.parasite_count++;
        if (org->attack) eco_stats.predator_count++;
        
        // Социальные градации
        if (org->pacifism_treshold <= 16) social_egoist++;
        else if (org->pacifism_treshold <= 32) social_low++;
        else if (org->pacifism_treshold <= 48) social_medium++;
        else social_high++;
        
        uint8_t is_builder = 0;
        
        for (int g = 0; g < GENOME_SIZE; g++) {
            switch (org->genome[g])
            {
            case SOLIDIFY:
                if(is_builder == 0)
                {
                    eco_stats.builder_count++;
                    is_builder = 1;
                }
                break;
            default:
                break;
            }
        }
        
        if (org->volume < 10) eco_stats.tiny_organisms++;
        else if (org->volume < 50) eco_stats.small_organisms++;
        else if (org->volume < 200) eco_stats.medium_organisms++;
        else if (org->volume < 500) eco_stats.large_organisms++;
        else eco_stats.giant_organisms++;
        
        // Подсчёт соседей
        for (int y = org->min_y; y <= org->max_y; y++) {
            for (int x = org->min_x; x <= org->max_x; x++)
            {
                Cell* cell = &grid_array[mod(y, grid_height)][mod(x, grid_width)];
                if (cell->id == i) {
                    
                    int neighbors = 0;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            Cell* neighbor = &grid_array[mod(y + dy, grid_height)][mod(x + dx, grid_width)];
                            if (neighbor->id == i) neighbors++;
                        }
                    }
                    total_neighbors += neighbors;
                }
            }
        }
    }
    
    if (organism_count > 0) {
        pop_stats.alive_count = organism_count;
        pop_stats.avg_volume = total_volume / organism_count;
        pop_stats.avg_material = total_material / organism_count;
        pop_stats.avg_neighbors = total_neighbors / organism_count;
    }
    
    // ========== ПРОХОД ПО ПОЛЮ ==========
    for (int y = 0; y < grid_height; y++) {
        for (int x = 0; x < grid_width; x++) {
            Cell* cell = &grid_array[y][x];
            
            if (cell->id == MAX_ORGANISMS) {
                eco_stats.total_food += cell->mat;
            }
            if (cell->solid) {
                eco_stats.total_walls++;
            }
            
            eco_stats.total_flags += cell->flag_0 + cell->flag_1 + cell->flag_2;
            eco_stats.flag_0_total += cell->flag_0;
            eco_stats.flag_1_total += cell->flag_1;
            eco_stats.flag_2_total += cell->flag_2;
        }
    }
    
    // ========== ВЫВОД В КОНСОЛЬ ==========
    printf("\n STEP %u\n", pop_stats.step);
    printf("POPULATION: alive = %u\n", pop_stats.alive_count);
    printf("REPRODUCTION: asexual = %u sexual = %u deaths = %u solidify = %u\n",
           pop_stats.asexual_reproductions, pop_stats.sexual_reproductions, 
           pop_stats.deaths, pop_stats.solidify_count);
    printf("SIZE: avg_vol = %u max_vol = %u avg_mat = %u max_mat = %u avg_neighbors = %u\n",
           pop_stats.avg_volume, pop_stats.max_volume, pop_stats.avg_material, 
           pop_stats.max_material, pop_stats.avg_neighbors);
    printf("BEHAVIOR: parasites = %u predators = %u builders = %u\n",
           eco_stats.parasite_count, eco_stats.predator_count, eco_stats.builder_count);
    printf("SOCIAL: egoist = %u low( = %u medium = %u high = %u\n",
           social_egoist, social_low, social_medium, social_high);
    printf("SIZE DIST: tiny = %u small = %u medium = %u large = %u giant = %u\n",
           eco_stats.tiny_organisms, eco_stats.small_organisms, eco_stats.medium_organisms,
           eco_stats.large_organisms, eco_stats.giant_organisms);
    printf("RESOURCES: food = %u walls = %u flags = %u (R: %u G: %u B: %u)\n",
           eco_stats.total_food, eco_stats.total_walls, eco_stats.total_flags,
           eco_stats.flag_0_total, eco_stats.flag_1_total, eco_stats.flag_2_total);
           
    // ========== ГИСТОГРАММЫ ==========
    #define BAR_WIDTH 64
    
    // 1. Reproduction (asexual, sexual, deaths, solidify)
    printf("\n REPRODUCTION HISTOGRAM\n");
    uint32_t repro_values[4] = {
        pop_stats.asexual_reproductions,
        pop_stats.sexual_reproductions,
        pop_stats.deaths,
        pop_stats.solidify_count
    };
    const char* repro_labels[4] = {"Asexual", "Sexual", "Deaths", "Solidify"};
    
    uint32_t repro_max = 1;
    for (int i = 0; i < 4; i++) {
        if (repro_values[i] > repro_max) repro_max = repro_values[i];
    }
    
    for (int i = 0; i < 4; i++) {
        int bar_len = (repro_max > 0) ? (repro_values[i] * BAR_WIDTH / repro_max) : 0;
        printf("%-8s: %6u [", repro_labels[i], repro_values[i]);
        for (int j = 0; j < bar_len; j++) printf("#");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    // 2. Behavior (parasite, predator, builder)
    printf("\n BEHAVIOR HISTOGRAM\n");
    uint32_t behavior_values[3] = {
        eco_stats.parasite_count,
        eco_stats.predator_count,
        eco_stats.builder_count
    };
    const char* behavior_labels[3] = {"Parasite", "Predator", "Builder"};
    
    uint32_t behavior_max = 1;
    for (int i = 0; i < 3; i++) {
        if (behavior_values[i] > behavior_max) behavior_max = behavior_values[i];
    }
    
    for (int i = 0; i < 3; i++) {
        int bar_len = (behavior_max > 0) ? (behavior_values[i] * BAR_WIDTH / behavior_max) : 0;
        printf("%-8s: %6u [", behavior_labels[i], behavior_values[i]);
        for (int j = 0; j < bar_len; j++) printf("#");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    // 3. Social gradation
    printf("\n SOCIAL HISTOGRAM\n");
    uint32_t social_values[4] = {
        social_egoist,
        social_low,
        social_medium,
        social_high
    };
    const char* social_labels[4] = {"Egoist", "Low", "Medium", "High"};
    
    uint32_t social_max = 1;
    for (int i = 0; i < 4; i++) {
        if (social_values[i] > social_max) social_max = social_values[i];
    }
    
    for (int i = 0; i < 4; i++) {
        int bar_len = (social_max > 0) ? (social_values[i] * BAR_WIDTH / social_max) : 0;
        printf("%-8s: %6u [", social_labels[i], social_values[i]);
        for (int j = 0; j < bar_len; j++) printf("#");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    // 4. Size distribution (tiny, small, medium, large, giant)
    printf("\n SIZE DISTRIBUTION HISTOGRAM\n");
    uint32_t size_values[5] = {
        eco_stats.tiny_organisms,
        eco_stats.small_organisms,
        eco_stats.medium_organisms,
        eco_stats.large_organisms,
        eco_stats.giant_organisms
    };
    const char* size_labels[5] = {"Tiny", "Small", "Medium", "Large", "Giant"};
    
    uint32_t size_max = 1;
    uint32_t log_values;
    for (int i = 0; i < 5; i++)
    {
        log_values = lg(size_values[i], 2);
        if (log_values > size_max) size_max = log_values;
    }
    
    for (int i = 0; i < 5; i++)
    {
        log_values = lg(size_values[i], 2);
        int bar_len = (size_max > 0) ? (log_values * BAR_WIDTH / size_max) : 0;
        printf("%-6s: %8u [", size_labels[i], size_values[i]);
        for (int j = 0; j < bar_len; j++) printf("#");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    // 5. Resources (food, walls, flags)
    printf("\n RESOURCES HISTOGRAM\n");
    uint32_t resource_values[3] = {
        eco_stats.total_food,
        eco_stats.total_walls,
        eco_stats.total_flags
    };
    const char* resource_labels[3] = {"Food", "Walls", "Flags"};
    
    uint32_t resource_max = 1;
    for (int i = 0; i < 3; i++) {
        if (resource_values[i] > resource_max) resource_max = resource_values[i];
    }
    
    for (int i = 0; i < 3; i++) {
        int bar_len = (resource_max > 0) ? (resource_values[i] * BAR_WIDTH / resource_max) : 0;
        printf("%-6s: %8u [", resource_labels[i], resource_values[i]);
        for (int j = 0; j < bar_len; j++) printf("#");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    asexual_reproductions = 0;
    sexual_reproductions = 0;
    deaths = 0;
    solidify_count = 0;
}