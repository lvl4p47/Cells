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
uint8_t food_mat = 100;
uint16_t min_mat = 100;
uint8_t food_mult = 0;
uint32_t total_mat = 0;
uint32_t alive = 0;
uint16_t starting_energy = 381;
uint8_t re_frac = 60000;
uint8_t debug = 0;
uint8_t base_mutate_chance = 5;
uint16_t volumetric_energy_mult = 127;
uint16_t starting_energy_mult = 2;
uint8_t repopulate = 1;
uint8_t statistics = 1;
uint16_t wall_cost = 254;
uint8_t medium_energy = 127;

// Статистика
static uint32_t step_counter = 0;
static uint32_t asexual_reproductions = 0;
static uint32_t sexual_reproductions = 0;
static uint32_t starvation = 0;
static uint32_t overheat = 0;
static uint32_t violent = 0;
static uint32_t solidify_count = 0;

uint8_t display_mode = 1;

FILE *file_ptr;
uint16_t integer;

void Grid_Init(uint16_t w, uint16_t h)
{
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
    if(debug) fprintf(stderr, "\nGrid_Reset"), fflush(stderr);
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
    if(debug) fprintf(stderr, "\nGrid_Set"), fflush(stderr);
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
                grid_array[y1][x1].material = 0;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].energy = 0;
                grid_array[y1][x1].life_wave_str = 0;
                grid_array[y1][x1].membrane = 0;
                
                for(int f = 0; f < FLAG_AMOUNT; f++)
                {
                    grid_array[y1][x1].flag[f] = 0;
                }
                
                grid_array[y1][x1].solid = 0;
            }
            else if(population[temp_id].volume > 0)
            {
                population[temp_id].volume--;
                population[temp_id].material += grid_array[y1][x1].material;
                population[temp_id].energy += grid_array[y1][x1].energy;
                
                grid_array[y1][x1].id = 0;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 0;
                grid_array[y1][x1].material = 0;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].energy = 0;
                grid_array[y1][x1].life_wave_str = 0;
                grid_array[y1][x1].solid = 0;
                grid_array[y1][x1].membrane = 0;
                
                for(int f = 0; f < FLAG_AMOUNT; f++)
                {
                    if(population[temp_id].flag_other[f][STRENGTH])
                    {
                        grid_array[y1][x1].flag[f] = min(grid_array[y1][x1].flag[f] + 2, 255);
                    }
                }
                
                // if(population[temp_id].volume == 0 && population[temp_id].alive)
                // {
                //     // printf("reason of death: weird\n");
                //     Organism_Quit(temp_id);
                // }
            }
            else if(grid_array[y1][x1].material != 0)
            {
                printf("weird1 id: %4d alive: %d energy: %d\n", 
                grid_array[y1][x1].id, 
                population[grid_array[y1][x1].id].alive,
                grid_array[y1][x1].energy);
                
                grid_array[y1][x1].id = 0;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 0;
                grid_array[y1][x1].material = 0;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].energy = 0;
                grid_array[y1][x1].life_wave_str = 0;
                grid_array[y1][x1].solid = 0;
                grid_array[y1][x1].membrane = 0;
            }
        }
        else if(grid_array[y1][x1].material != 0)
        {
            grid_array[y1][x1].id = 0;
            grid_array[y1][x1].vx = 0;
            grid_array[y1][x1].vy = 0;
            grid_array[y1][x1].strength = 0;
            grid_array[y1][x1].type = 0;
            grid_array[y1][x1].material = 0;
            grid_array[y1][x1].cooldown = 0;
            grid_array[y1][x1].energy = 0;
            grid_array[y1][x1].life_wave_str = 0;
            grid_array[y1][x1].solid = 0;
            grid_array[y1][x1].membrane = 0;
            printf("weird2\n");
        }
        else if(temp_id == 0)
        {
            for(int f = 0; f < FLAG_AMOUNT; f++)
            {
                grid_array[y1][x1].flag[f] = 0;
            }
            grid_array[y1][x1].solid = 0;
            grid_array[y1][x1].membrane = 0;
        }
        return;
    }
    
    // СЛУЧАЙ 2: Установка клетки организма (id > 0)
    if(id <= MAX_ORGANISMS && population[id].alive && population[id].material > 0)
    {
        for(int f = 0; f < FLAG_AMOUNT; f++)
        {
            if(grid_array[y1][x1].flag[f])
            {
                grid_array[y1][x1].flag[f] = max(grid_array[y1][x1].flag[f] - 1, 0);
            }
        }
        
        // СЛУЧАЙ 2А: Клетка пуста - всегда можно занять
        if(population[id].energy > medium_energy)
        {
            if(temp_id == 0)
            {
                population[id].material--;
                population[id].energy -= medium_energy;
                
                grid_array[y1][x1].id = id;
                grid_array[y1][x1].vx = 0;
                grid_array[y1][x1].vy = 0;
                grid_array[y1][x1].strength = 0;
                grid_array[y1][x1].type = 2;  // Все новые клетки - тело (type=2)
                grid_array[y1][x1].material = 1;
                grid_array[y1][x1].cooldown = 0;
                grid_array[y1][x1].energy = medium_energy;
                grid_array[y1][x1].life_wave_str = 0;
                grid_array[y1][x1].solid = 0;
                grid_array[y1][x1].membrane = 0;
                
                population[id].volume = min(population[id].volume + 1, 65535);
                return;
            }
            
            // СЛУЧАЙ 2Б: Клетка занята другим организмом
            else if(temp_id <= MAX_ORGANISMS && temp_id != id)
            {
                
                {
                    if(temp_id == MAX_ORGANISMS)
                    {
                        
                        if(grid_array[y1][x1].solid == 0)
                        {
                            population[id].material += grid_array[y1][x1].material - 1;
                            population[id].energy -= medium_energy;
                            population[id].energy += grid_array[y1][x1].energy;
                            grid_array[y1][x1].material = 1;
                            
                            grid_array[y1][x1].id = id;
                            grid_array[y1][x1].vx = 0;
                            grid_array[y1][x1].vy = 0;
                            grid_array[y1][x1].strength = 0;
                            grid_array[y1][x1].type = 2;  // Захваченная клетка становится телом
                            grid_array[y1][x1].material = 1;
                            grid_array[y1][x1].cooldown = 0;
                            grid_array[y1][x1].energy = medium_energy;
                            grid_array[y1][x1].life_wave_str = 0;
                            grid_array[y1][x1].membrane = 0;
                            
                            population[id].volume = min(population[id].volume + 1, 65535);
                        }
                        else
                        {
                            {
                                uint32_t strength = population[id].energy;
                                uint32_t mat_taken;
                                uint32_t energy_taken;
                                if(grid_array[y1][x1].material > 0) // breaking
                                {
                                    mat_taken = min(strength, grid_array[y1][x1].material);
                                    energy_taken = min(strength, grid_array[y1][x1].energy);
                                    
                                    // population[id].velocity[X_DIRECTION] /= 2;
                                    // population[id].velocity[Y_DIRECTION] /= 2;
                                    
                                    if(grid_array[y1][x1].energy >= energy_taken)
                                    {
                                        population[id].energy += energy_taken;
                                        grid_array[y1][x1].energy -= energy_taken;
                                    }
                                    else if(grid_array[y1][x1].material >= mat_taken)
                                    {
                                        population[id].material += mat_taken;
                                        grid_array[y1][x1].material -= mat_taken;
                                        printf("breaking\n");
                                    }
                                }
                                else if(population[id].material > 0) // breakthrough
                                {
                                    population[id].energy += wall_cost;
                                    population[id].energy -= medium_energy;
                                    grid_array[y1][x1].solid = 0;
                                    grid_array[y1][x1].membrane = 0;
                                    
                                    grid_array[y1][x1].energy = medium_energy;
                                    population[id].material -= 1; 
                                    printf("breakthrough\n");
                                    
                                    grid_array[y1][x1].material = 1;
                                    
                                    grid_array[y1][x1].id = id;
                                    grid_array[y1][x1].vx = 0;
                                    grid_array[y1][x1].vy = 0;
                                    grid_array[y1][x1].strength = 0;
                                    grid_array[y1][x1].type = 2;  // Захваченная клетка становится телом
                                    
                                    grid_array[y1][x1].cooldown = 0;
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
                            population[id].material += grid_array[y1][x1].material - 1;
                            population[id].energy -= medium_energy;
                            population[id].energy += grid_array[y1][x1].energy;
                            grid_array[y1][x1].material = 1;
                            grid_array[y1][x1].membrane = 0;
                            
                            if(population[temp_id].volume > 0)
                                population[temp_id].volume--;
                            
                            // Если у атакованного не осталось клеток - он умирает
                            // if(population[temp_id].alive && population[temp_id].volume == 0)
                            // {
                            //     // printf("\nreason of death: no cells");
                            //     Organism_Quit(temp_id);
                            // }
                        }
                        else if(grid_array[y1][x1].type == 1)  // Ядро
                        {
                            // printf("nucleus attack");
                            // Атака ядра дает больше ресурсов
                            population[id].material += population[temp_id].material / recycle_div;
                            population[temp_id].material = 0;
                            population[id].material += grid_array[y1][x1].material - 1;
                            population[id].energy -= medium_energy;
                            population[id].energy += grid_array[y1][x1].energy;
                            population[id].energy += population[temp_id].energy;
                            grid_array[y1][x1].material = 1;
                            grid_array[y1][x1].membrane = 0;
                            
                            if(population[temp_id].volume > 1)
                                population[temp_id].volume--;
                            
                            if(population[temp_id].alive)
                            {
                                // printf("reason of death: nucleus eaten\n");
                                // Organism_Quit(temp_id);
                            }
                        }
                        
                        // Атакованный организм чувствует боль
                        if(population[temp_id].alive)
                        {
                            uint16_t attackers_count = Id_Count(x1, y1, id);
                            if(attackers_count > population[temp_id].sense_pain[STRENGTH])
                            {
                                population[temp_id].sense_pain[STRENGTH] = attackers_count;
                                population[temp_id].sense_pain[X_DIRECTION] = x1 - population[temp_id].nuc_x;
                                population[temp_id].sense_pain[Y_DIRECTION] = y1 - population[temp_id].nuc_y;
                            }
                        }
                        
                        // Занимаем клетку
                        grid_array[y1][x1].id = id;
                        grid_array[y1][x1].vx = 0;
                        grid_array[y1][x1].vy = 0;
                        grid_array[y1][x1].strength = 0;
                        grid_array[y1][x1].type = 2;  // Захваченная клетка становится телом
                        grid_array[y1][x1].material = 1;
                        grid_array[y1][x1].cooldown = 0;
                        grid_array[y1][x1].energy = medium_energy;
                        grid_array[y1][x1].life_wave_str = 0;
                        grid_array[y1][x1].membrane = 0;
                        
                        population[id].volume = min(population[id].volume + 1, 65535);
                    }
                }
            }
        }
    }
}

void Grid_Set_Food(uint16_t x, uint16_t y)
{
    if(debug) fprintf(stderr, "\nGrid_Set_Food"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    // Прямая установка, минуя Grid_Set
    grid_array[y1][x1].id = MAX_ORGANISMS;
    grid_array[y1][x1].vx = 0;
    grid_array[y1][x1].vy = 0;
    grid_array[y1][x1].strength = 0;
    grid_array[y1][x1].type = 1;
    grid_array[y1][x1].material = food_mat;
    grid_array[y1][x1].energy = 0;
    grid_array[y1][x1].cooldown = 0;
    grid_array[y1][x1].solid = 0;
    
    population[MAX_ORGANISMS].volume = 0;
}

uint32_t Check_Conservation()
{
    if(debug == 0) return 0;
    uint32_t org_mat = 0, grid_mat = 0, vol_mat = 0;
    uint16_t id;
    
    total_mat = 0, alive = 0;
    org_mat = 0, grid_mat = 0, vol_mat = 0;
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            id = Grid_Get(j, i)->id;
            if(population[id].volume > 0 || id == MAX_ORGANISMS)
                grid_mat += grid_array[i][j].material;
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
    
    printf("before vol_mat: %5d org_mat: %5d grid_mat: %5d total_mat: %5d\n", vol_mat, org_mat, grid_mat, total_mat);
    
    if(total_mat % min_mat)
    {
        printf("%d", 1 / 0);
    }
    
    return total_mat;
}

void Grid_Update()
{
    Check_Conservation();
    
    if(debug) 
    {
        freopen("debug.log", "w", stderr);
        fprintf(stderr, "\nGrid_Update"), fflush(stderr);
    }
    
    step_counter++;
    
    uint32_t volume = 0;
    uint8_t membrane = 0;
    uint16_t id;
    
    int32_t leftover_material;
    int32_t leftover_energy;
    uint32_t cells;
    int32_t material_per_cell;
    int32_t energy_per_cell;
    
    uint32_t photo_area;
    uint16_t photo_radius;
    uint16_t perimeter;
    
    // Order_Shuffle();  
    
    int32_t total_change = 0, solid_change = 0;
    int32_t max_increase = population[id].max_energy - population[id].energy;
    int32_t max_decrease = -population[id].energy;
    
    Cell *cell, *neighbor;
    
    uint16_t id1;
    for(int alive_stack = free_top + 1; alive_stack < MAX_ORGANISMS - 1; alive_stack++)
    {
        id1 = free_stack[alive_stack];
        volume = 0;
        if(population[id1].alive)
        {
            alive++;
            total_change = 0;
            solid_change = 0;
            population[id1].perimeter = 0;
            
            // printf("\nid: %4d energy: %4d\n", id1, population[id1].energy);
            
            int16_t min_x = population[id1].min_x;
            int16_t min_y = population[id1].min_y;
            int16_t max_x = population[id1].max_x;
            int16_t max_y = population[id1].max_y;
            
            int16_t new_min_x = population[id1].min_x + 1;
            int16_t new_min_y = population[id1].min_y + 1;
            int16_t new_max_x = population[id1].max_x - 1;
            int16_t new_max_y = population[id1].max_y - 1;
            
            uint8_t energy;
            
            // perimeter = 2 * (population[id1].max_x - population[id1].min_x) + 
            //             2 * (population[id1].max_y - population[id1].min_y);
                        
            // if(population[id1].energy > population[id1].material) photo_area = perimeter / 2;
            // else photo_area = perimeter;
            
            // photo_radius = max(fast_root(photo_area) / 2, 1);
            
            for(int i = min_y; i <= max_y; i++)
            {
                for(int j = min_x; j <= max_x; j++)
                {
                    cell = Grid_Get(j, i);
                    energy = cell->energy;
                    if(cell->id == id1 && (energy != 0 && energy < 255))
                    {
                        // printf("first ");
                        // if(display_mode == 0) 
                        // {
                        //     printf("%d %d\n", population[id1].energy, population[id1].max_energy);
                        // }
                        membrane = Is_Membrane(j, i);
                        cell->membrane = membrane;
                        
                        if(cell->membrane
                        && (Id_Count(j, i, 0) != 0 || Id_Count(j, i, MAX_ORGANISMS) != 0))
                        {
                            population[id1].perimeter += 8 - Id_Count(j, i, id1);
                            if(population[id1].photosynthesis)
                            {
                                cell->energy = min(energy + 1, 255);
                                if(display_mode == 0) 
                                {
                                    printf("%d %d\n", population[id1].energy, population[id1].max_energy);
                                }
                            }
                            else
                                cell->energy = max(energy - 1, 0);
                        }
                        
                        
                        
                        if(membrane) 
                        {
                            Process_Membrane(j, i);
                            
                            if(j < new_min_x) new_min_x = j;
                            if(j > new_max_x) new_max_x = j;
                            if(i < new_min_y) new_min_y = i;
                            if(i > new_max_y) new_max_y = i;
                        }
                        
                        if(membrane
                        && (population[id1].steal_mat || population[id1].steal_nrg
                        || population[id1].give_mat || population[id1].give_nrg))
                        {
                            for(int dy = -1; dy < 2; dy++)
                            {
                                for(int dx = -1; dx < 2; dx++)
                                {
                                    neighbor = Grid_Get(j + dx, i + dy);
                                    id = neighbor->id;
                                    
                                    int dist = max(abs((j + dx) - population[id1].nuc_x), abs((i + dy) - population[id1].nuc_y));
                                    if (rand() % (dist + 1) == 0) 
                                    {
                                        if(id != id1 && id != 0)
                                        {
                                            if(population[id1].steal_mat && population[id].material > 0)
                                            {
                                                population[id].material--;
                                                population[id1].material++;
                                            }
                                            if(population[id1].steal_nrg && population[id].energy > 0)
                                            {
                                                population[id].energy--;
                                                population[id].starting_energy--;
                                                population[id1].energy++;
                                            }
                                            if(id != MAX_ORGANISMS)
                                            {
                                                if(population[id1].give_mat && population[id1].material > 0)
                                                {
                                                    population[id].material++;
                                                    population[id1].material--;
                                                }
                                                if(population[id1].give_nrg && population[id1].energy > 0)
                                                {
                                                    population[id].energy++;
                                                    population[id1].energy--;
                                                }
                                                if(display_mode == 0) 
                                                {
                                                    printf("id %d energy %d\n", id1, population[id1].energy);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        int16_t life_wave_str = cell->life_wave_str;
                                
                        if(life_wave_str != 0
                        && cell->cooldown == 0)
                        {
                            total_change += cell->energy - medium_energy;
                        }
                        // if(life_wave_str != 0 && membrane && population[id1].solidify == 1
                        //     && (i != population[id1].nuc_y || j != population[id1].nuc_x))
                        // {
                        //     solid_change -= wall_cost;
                        // }
                        
                        if(cell->strength != 0)
                        {
                            if(cell->strength > 0)
                            {
                                if(j - 1 < new_min_x) new_min_x = j - 1;
                                if(j + 1 > new_max_x) new_max_x = j + 1;
                                if(i - 1 < new_min_y) new_min_y = i - 1;
                                if(i + 1 > new_max_y) new_max_y = i + 1;
                            }
                            if(membrane
                            && cell->cooldown == 0)
                            {
                                Expand(j, i, cell->strength);
                            }
                        }
                    }
                    else if(cell->id == id1)
                    {
                        // printf("cell dies off\n");
                        id = cell->id;
                        population[id].volume--;
                        cell->id = MAX_ORGANISMS;
                    }
                    if(cell->id == id1)
                    {
                        membrane = Is_Membrane(j, i);
                        cell->membrane = membrane;
                    }
                    
                }
            }
            population[id1].max_energy = volumetric_energy_mult * population[id1].volume + starting_energy_mult * population[id1].starting_energy;
            max_increase = population[id1].max_energy - population[id1].energy;
            max_decrease = -population[id1].energy;
            // printf("max_decrease: %d total_change: %d max_increase: %d\n", max_decrease, total_change, max_increase);
            uint8_t all_or_nothing = (total_change > max_decrease && total_change < max_increase);
            uint8_t all_or_nothing_solid = (solid_change > max_decrease && solid_change < max_increase);
            
            if(new_min_x > new_max_x
            || new_min_y > new_max_y) 
            {
                // printf("reason of death: noone\n");
                // if(Organism_Quit(id1) == 1)
                //     continue;
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
                        cell = Grid_Get(j, i);
                        energy = cell->energy;
                        if(cell->id == id1)
                        {
                            
                            // if(cell->membrane == 1
                            // && (j != population[id1].nuc_x || i != population[id1].nuc_y))
                            // {
                            //     int16_t life_wave_str = cell->life_wave_str;
                                
                            //     if(life_wave_str != 0 && population[id1].solidify == 1
                            //     && all_or_nothing_solid)
                            //     {
                            //         if(population[id1].energy > wall_cost)
                            //         {
                            //             Grid_Set(j, i, 0);
                            //             cell->id = MAX_ORGANISMS;
                            //             population[id1].energy -= wall_cost;
                            //             cell->solid = 1;
                            //             // printf("crust id: %d\n", cell->solid);
                            //         }
                                    
                            //         // if(population[id].material >= material_per_cell)
                            //         // {
                            //         //     cell->material += material_per_cell;
                            //         //     population[id].material -= material_per_cell;
                            //         // }
                            //         // if(population[id].energy >= energy_per_cell)
                            //         // {
                            //         //     cell->energy += energy_per_cell;
                            //         //     population[id].energy -= energy_per_cell;
                            //         // }
                                    
                            //     }
                            // }
                            
                            if(cell->membrane && cell->cooldown > 0)
                            {
                                int16_t life_wave_str = cell->life_wave_str;
                                
                                if(life_wave_str != 0)
                                {
                                    Grid_Life_Wave(j, i, life_wave_str, all_or_nothing);
                                }
                            }
                            if(cell->cooldown > 0)
                            {
                                cell->cooldown = max(cell->cooldown - 1, 0);
                            } 
                            else if(energy != 0 && energy != 255)
                            {
                                if(cell->strength != 0)
                                {
                                    // printf("second\n");
                                    int8_t strength = cell->strength;
                                    int8_t vx = cell->vx;
                                    int8_t vy = cell->vy;
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
                                            if(Grid_Get(j + sx, i + sy)->energy != 0)
                                                Grid_Signal(j + sx, i + sy, vx, vy, strength);
                                        }
                                        else
                                        {
                                            if(ax > ay) 
                                            {
                                                if(Grid_Get(j + sx, i)->energy != 0)
                                                    Grid_Signal(j + sx, i, vx, vy, strength);
                                            }
                                            else 
                                            {
                                                if(Grid_Get(j, i + sy)->energy != 0)
                                                Grid_Signal(j, i + sy, vx, vy, strength);
                                            }
                                        }
                                    }
                                    Grid_Signal(j, i, 0, 0, 0);
                                }
                                int16_t life_wave_str = cell->life_wave_str;
                                
                                if(life_wave_str != 0)
                                {
                                    Grid_Life_Wave(j, i, life_wave_str, all_or_nothing);
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
    if(timer > (1 - display_mode) * 50)
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
    if(debug) fprintf(stderr, "\nGrid_Signal"), fflush(stderr);
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

void Grid_Life_Wave(int16_t x, int16_t y, uint16_t strength, uint8_t all_or_nothing)
{
    if(debug) fprintf(stderr, "\nGrid_Life_Wave"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t id = grid_array[y1][x1].id;
    Cell* c = Grid_Get(x1, y1);
    
    int32_t max_increase = population[id].max_energy - population[id].energy;
    int32_t max_decrease = -population[id].energy;
    int32_t desired_change = c->energy - medium_energy;
    
    Cell* n;
    
    if(all_or_nothing)
    {
        if(desired_change > max_decrease && desired_change < max_increase)
        {
            population[id].energy += desired_change;
            c->energy = medium_energy;
        }
        else if(desired_change > 0)
        {
            population[id].energy += max_increase; 
            c->energy -= max_increase;
        }
        else
        {
            population[id].energy += max_decrease;
            c->energy -= max_decrease;
        }
        for(int dy = -1; dy <= 1; dy++)
        {
            for(int dx = -1; dx <= 1; dx++)
            {
                n = Grid_Get(x1 + dx, y1 + dy);
                if(n->id == id
                && n->cooldown == 0
                && n->life_wave_str == 0
                && n->life_wave_str < max(strength - 1, 0))
                {
                    Grid_Get(x1 + dx, y1 + dy)->life_wave_str = max(strength - 1, 0);
                    if(dy > 0 || (dy == 0 && dx > 0))
                        Grid_Get(x1 + dx, y1 + dy)->cooldown += 1;
                }
            }
        }
    }
    
    Grid_Get(x1, y1)->life_wave_str = 0;
    Grid_Get(x1, y1)->cooldown += 5;
}

void Grid_Add_Cooldown(int16_t x, int16_t y, int8_t cd)
{
    if(debug) fprintf(stderr, "\nGrid_Add_Cooldown"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    grid_array[y1][x1].cooldown += cd;
}

uint16_t Organism_Init(int16_t x, int16_t y, uint32_t material, uint32_t energy)
{
    if(debug) fprintf(stderr, "\nOrganism_Init"), fflush(stderr);
    if (free_top < 0) {
        printf("no free ids\n");
        return 0; // Стек пуст, свободных мест нет
    }
    if(material == 0 || energy < starting_energy )
    {
        return 0;
    }
    
    // Забираем ID с вершины и опускаем указатель    
    uint16_t id = free_stack[free_top];
    free_top--;

    // printf("id: %d\n", id);
    
    // Сразу сбрасываем данные в population для нового жильца
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    population[id].nuc_x = x1;
    population[id].nuc_y = y1;
    population[id].material = material;
    population[id].min_mat = min_mat;
    population[id].volume = 0;
    population[id].perimeter = 0;
    population[id].target_vol = 50;
    population[id].alive = 1;
    population[id].multiply = 0;
    population[id].energy = energy;
    population[id].starting_energy = energy;
    population[id].max_energy = volumetric_energy_mult * 1 + starting_energy_mult * population[id].starting_energy;
    
    population[id].sense_material[X_DIRECTION] = 0;
    population[id].sense_material[Y_DIRECTION] = 0;
    population[id].sense_material[STRENGTH] = 0;
    population[id].sense_energy[X_DIRECTION] = 0;
    population[id].sense_energy[Y_DIRECTION] = 0;
    population[id].sense_energy[STRENGTH] = 0;
    population[id].sense_other[X_DIRECTION] = 0;
    population[id].sense_other[Y_DIRECTION] = 0;
    population[id].sense_other[STRENGTH] = 0;
    population[id].sense_wall[X_DIRECTION] = 0;
    population[id].sense_wall[Y_DIRECTION] = 0;
    population[id].sense_wall[STRENGTH] = 0;
    population[id].sense_pain[X_DIRECTION] = 0;
    population[id].sense_pain[Y_DIRECTION] = 0;
    population[id].sense_pain[STRENGTH] = 0;
    population[id].sense_free[X_DIRECTION] = 0;
    population[id].sense_free[Y_DIRECTION] = 0;
    population[id].sense_free[STRENGTH] = 0;
    population[id].sense_free[DISTANCE] = 0;
    population[id].sense_friend[X_DIRECTION] = 0;
    population[id].sense_friend[Y_DIRECTION] = 0;
    population[id].sense_friend[STRENGTH] = 0;
    
    for(int f = 0; f < FLAG_AMOUNT; f++)
    {
        population[id].flag_other[f][STRENGTH] = 0;
        population[id].flag_other[f][X_DIRECTION] = 0;
        population[id].flag_other[f][Y_DIRECTION] = 0;
        population[id].flag[f] = 0;
    }
    
    population[id].strength = 1;
    population[id].velocity[X_DIRECTION] = 0;
    population[id].velocity[Y_DIRECTION] = 0;
    population[id].move = 0;
    population[id].steal_mat = 0;
    population[id].steal_nrg = 0;
    population[id].give_mat = 0;
    population[id].give_nrg = 0;
    population[id].attack = 0;
    population[id].photosynthesis = 1;
    
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
    if(debug) fprintf(stderr, "\nGenome_Init"), fflush(stderr);
    char buf[32];
    if(test)
    {
        printf("\nTEST_GENOME\n");
        snprintf(buf, sizeof(buf), "test_genome.txt", id);
        
        file_ptr = fopen(buf, "r");
        if(file_ptr == NULL)
        {
            // printf("file error id: %d\n", id);
            
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id].genome[i] = rand() % GENOME_SIZE;
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
            
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id].genome[i] = rand() % GENOME_SIZE;
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
    if(debug) fprintf(stderr, "\nGenome_Hash"), fflush(stderr);
    uint16_t hash = 0;
    for (int i = 0; i < GENOME_SIZE; i++) {
        hash += population[id].genome[i] * (GENOME_SIZE - i);
    }
    population[id].genome_hash = hash;
    population[id].target_hash = hash;
}

void Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate)
{
    if(debug) fprintf(stderr, "\nGenome_Copy"), fflush(stderr);
    
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        population[id2].genome[i] = population[id1].genome[i];
        population[id2].child_genome[i] = population[id2].genome[i];
        if(mutate)
        {
            if(rand() % (GENOME_SIZE * 1 / mutate) == 0) 
            {
                population[id2].genome[i] = rand() % GENOME_SIZE;
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
    if(debug) fprintf(stderr, "\nChild_Genome_Copy"), fflush(stderr);
    
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        population[id2].genome[i] = population[id1].child_genome[i];
        if(mutate)
        {
            if(rand() % (GENOME_SIZE * 1 / mutate) == 0) 
            {
                population[id2].genome[i] = rand() % GENOME_SIZE;
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
    if(debug) fprintf(stderr, "\nMutate_Swap_Blocks"), fflush(stderr);
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
    if(debug) fprintf(stderr, "\nBest_Genome_Spread"), fflush(stderr);
    
    for(int i = 0; i < MAX_ORGANISMS; i++)
    {
        Organism_Quit(i);
    }
    // printf("best_genome_spread\n");
    uint16_t current_org = 0;
    uint16_t current_ord = 0;
    uint16_t current_best = free_stack[free_top];
    
    
    
    while(current_best >= MAX_ORGANISMS * (re_frac - 1) / re_frac)
    {
        // printf("cur_best: %d ids:", current_best);
        Genome_Copy(current_best, order[current_org], 0);
        uint16_t bot = min(current_org + 1, MAX_ORGANISMS);
        uint16_t mid = min(current_org + (re_frac) / 2, MAX_ORGANISMS);
        uint16_t top = min(current_org + (re_frac) - 1, MAX_ORGANISMS);
        uint16_t last = min(current_org + re_frac, MAX_ORGANISMS);
        Genome_Copy(current_best, order[last], GENOME_SIZE);
        for(int org = bot; org < mid; org++)
        {
            Genome_Copy(current_best, order[org], 2);
            // printf(" %d", order[org]);
            
        }
        for(int org = mid; org < top; org++)
        {
            Genome_Copy(current_best, order[org], 1);
            // printf(" %d", order[org]);
            
        }
        // printf("\n");
        
        current_best--;
        current_org = min(current_org + re_frac, MAX_ORGANISMS - 1);
    }
    
    Order_Shuffle();
    
    free_top = -1;
    for (int i = 1; i < MAX_ORGANISMS; i++) {
        free_top++;
        free_stack[free_top] = order[i];
    }
    
    printf("\n");
}

void Child_Genome_Combine(uint16_t id1, uint16_t id2)
{
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        if(rand() % 2)
            population[id2].child_genome[i] = population[id1].genome[i];
    }
}

uint8_t Organism_Quit(uint16_t id)
{
    if(debug) fprintf(stderr, "\nOrganism_Quit"), fflush(stderr);
    // printf("organism_quit ");
    if (id == 0 || id >= MAX_ORGANISMS) return 0;
    if (population[id].alive == 0) return 0;
    
    if(population[id].energy == 0) starvation++;
    else if(population[id].energy >= population[id].max_energy) overheat++;
    else violent++;
    
    Grid_Signal(population[id].nuc_x, population[id].nuc_y, 0, 0, 0);
    
    int16_t min_x = population[id].min_x;
    int16_t min_y = population[id].min_y;
    int16_t max_x = population[id].max_x;
    int16_t max_y = population[id].max_y;
    
    Cell* nuc = Grid_Get(population[id].nuc_x, population[id].nuc_y);
    {
        nuc->material += population[id].material;
        nuc->energy += population[id].energy;
    }
    
    // if(population[id].material > 0
    // && population[id].volume > 0)
    // {
    //     uint32_t leftover_material = population[id].material;
    //     uint32_t cells = population[id].volume;
    //     uint32_t material_per_cell = leftover_material / cells;
    //     uint32_t leftover_energy = population[id].energy;
    //     uint32_t energy_per_cell = leftover_energy / cells;

    //     // Проходим по всем клеткам организма
    //     for(int i = min_y; i <= max_y; i++)
    //     {
    //         for(int j = min_x; j <= max_x; j++)
    //         {
    //             if(Grid_Get(j, i)->id == id)
    //             {
    //                 if(population[id].material >= material_per_cell 
    //                 && population[id].energy >= energy_per_cell)
    //                 {
    //                     Grid_Get(j, i)->material += material_per_cell;
    //                     population[id].material -= material_per_cell;
                        
    //                     Grid_Get(j, i)->energy += energy_per_cell;
    //                     population[id].energy -= energy_per_cell;
    //                 }
    //             }
    //         }
    //     }

    //     // Добавляем остаток в ядро
    //     Cell* nucleus = Grid_Get(population[id].nuc_x, population[id].nuc_y);
        
    //     if(nucleus->id != 0)
    //     {
    //         nucleus->material += population[id].material;
    //         nucleus->energy += population[id].energy;
    //     }
    //     else 
    //     {
    //         Grid_Set(population[id].nuc_x, population[id].nuc_y, MAX_ORGANISMS);
    //         nucleus->material += population[id].material;
    //         nucleus->energy += population[id].energy;
    //     }
    //     population[id].material = 0;
    //     population[id].energy = 0;
    //     // printf("with volume\n");
    // }
    // else if(population[id].material > 0)
    // {
    //     // printf("without volume\n");
    //     // printf("id below: %d nucleus id: %d\n", Grid_Get(population[id].nuc_x, population[id].nuc_y)->id, id);
    //     // Нет клеток — весь материал кладём в ядро (которое, возможно, тоже не клетка, но это место его последнего положения)
        
    //     if(Grid_Get(population[id].nuc_x, population[id].nuc_y)->id != 0)
    //     {
    //         // printf("1");
    //         Grid_Get(population[id].nuc_x, population[id].nuc_y)->material += population[id].material;
    //         Grid_Get(population[id].nuc_x, population[id].nuc_y)->energy += population[id].energy;
    //     }
    //     else 
    //     {
    //         // printf("2");
    //         Grid_Set(population[id].nuc_x, population[id].nuc_y, MAX_ORGANISMS);
    //         Grid_Get(population[id].nuc_x, population[id].nuc_y)->material += population[id].material;
    //         Grid_Get(population[id].nuc_x, population[id].nuc_y)->energy += population[id].energy;
    //     }
    //     population[id].material = 0;
    //     population[id].energy = 0;
    // }
    // else
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
                Grid_Get(j, i)->id = MAX_ORGANISMS;
                if(population[id].solidify == 2)
                    Grid_Get(j, i)->solid = 1;
            }
        }
    }
    
    // Adjust free_stack
    uint16_t moving_id = free_stack[free_top + 1];
    for(int i = free_top + 1; i < MAX_ORGANISMS - 1; i++)
    {
        if(free_stack[i] == id) free_stack[i] = moving_id;
    }
    
    // Поднимаем указатель и кладем ID обратно в стек
    free_top++;
    free_stack[free_top] = id;
    
    // Опционально: обнуляем данные в массиве организмов
    
    population[id].nuc_x = 0;
    population[id].nuc_y = 0;
    population[id].material = 0;
    population[id].min_mat = 40;
    population[id].volume = 0;
    population[id].perimeter = 0;
    population[id].target_vol = 0;
    population[id].alive = 0;
    population[id].multiply = 0;
    population[id].energy = 0;
    population[id].starting_energy = 0;
    population[id].max_energy = 0;
    population[id].sense_material[X_DIRECTION] = 0;
    population[id].sense_material[Y_DIRECTION] = 0;
    population[id].sense_material[STRENGTH] = 0;
    population[id].sense_energy[X_DIRECTION] = 0;
    population[id].sense_energy[Y_DIRECTION] = 0;
    population[id].sense_energy[STRENGTH] = 0;
    population[id].sense_other[X_DIRECTION] = 0;
    population[id].sense_other[Y_DIRECTION] = 0;
    population[id].sense_other[STRENGTH] = 0;
    population[id].sense_wall[X_DIRECTION] = 0;
    population[id].sense_wall[Y_DIRECTION] = 0;
    population[id].sense_wall[STRENGTH] = 0;
    population[id].sense_pain[X_DIRECTION] = 0;
    population[id].sense_pain[Y_DIRECTION] = 0;
    population[id].sense_pain[STRENGTH] = 0;
    population[id].sense_free[X_DIRECTION] = 0;
    population[id].sense_free[Y_DIRECTION] = 0;
    population[id].sense_free[STRENGTH] = 0;
    population[id].sense_free[DISTANCE] = 0;
    population[id].sense_friend[X_DIRECTION] = 0;
    population[id].sense_friend[Y_DIRECTION] = 0;
    population[id].sense_friend[STRENGTH] = 0;
    population[id].velocity[X_DIRECTION] = 0;
    population[id].velocity[Y_DIRECTION] = 0;
    population[id].move = 0;
    population[id].steal_mat = 0;
    population[id].steal_nrg = 0;
    population[id].give_mat = 0;
    population[id].give_nrg = 0;
    population[id].attack = 0;
    population[id].photosynthesis = 1;
    
    for(int f = 0; f < FLAG_AMOUNT; f++)
    {
        population[id].flag_other[f][STRENGTH] = 0;
        population[id].flag_other[f][X_DIRECTION] = 0;
        population[id].flag_other[f][Y_DIRECTION] = 0;
        population[id].flag[f] = 0;
    }
    
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
    if(debug > 1) fprintf(stderr, "\nIs_Membrane"), fflush(stderr);
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
    uint16_t wall = 0;
    uint16_t material = 0;
    uint16_t energy = 0;
    uint16_t free = 0;
    uint16_t free_dist = 65535;
    uint16_t friends = 0;
    
    uint16_t flag_other[FLAG_AMOUNT][3];
    
    uint32_t hash, hash_id;
    uint16_t partner_id = population[id].partner_id;
    uint32_t partner_hash = population[partner_id].target_hash;
    uint32_t random_hash;
    uint16_t linear_size = max(population[id].max_x - population[id].min_x
    + population[id].max_y - population[id].min_y, 1);
    
    int8_t vx = population[id].velocity[X_DIRECTION];
    int8_t vy = population[id].velocity[Y_DIRECTION];
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
            if(id1 == 0)
            {
                free += 31;
            }
            if(id1 != id
            && (cell->energy != 0
            && abs(population[id].genome_hash - population[id1].genome_hash)
             > (max_pacifism_threshold * population[id].pacifism_treshold / 64)
            || id1 == MAX_ORGANISMS
            || id1 == 0
            || population[id1].alive == 0))
            {
                counter++;
                if(id1 == MAX_ORGANISMS || population[id1].alive == 0) 
                {
                    if(cell->solid == 0)
                    {
                        material += cell->material;
                        energy += cell->energy;
                    }
                    else
                    {
                        wall += cell->energy;
                    }
                    
                }
                else if(id1 != 0)
                {
                    if(population[id1].steal_mat || population[id1].steal_nrg)
                        other += 31;
                    else
                        other += 3;
                    
                    // for(int f = 0; f < FLAG_AMOUNT; f++)
                    // {
                    //     if(population[id1].flag[f])
                    //         flag[f] += population[id1].flag[f];
                    // }
                }
            }
            else if(id1 != id
            && (cell->energy != 0
            && abs(population[id].genome_hash - population[id1].genome_hash)
             <= (max_pacifism_threshold * population[id].pacifism_treshold / 64)))
            {
                friends += 31;   
            }
            
            for(int f = 0; f < FLAG_AMOUNT; f++)
            {
                if(cell->flag[f])
                    flag_other[f][STRENGTH] += cell->flag[f];
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
        
        if(other > population[id].sense_other[STRENGTH])
        {
            population[id].sense_other[STRENGTH] = other;
            population[id].sense_other[X_DIRECTION] = dx_to_nuc;
            population[id].sense_other[Y_DIRECTION] = dy_to_nuc;
        }
        if(wall > population[id].sense_wall[STRENGTH])
        {
            population[id].sense_wall[STRENGTH] = wall;
            population[id].sense_wall[X_DIRECTION] = dx_to_nuc;
            population[id].sense_wall[Y_DIRECTION] = dy_to_nuc;
        }
        if(material > population[id].sense_material[STRENGTH])
        {
            population[id].sense_material[STRENGTH] = material;
            population[id].sense_material[X_DIRECTION] = dx_to_nuc;
            population[id].sense_material[Y_DIRECTION] = dy_to_nuc;
        }
        if(energy > population[id].sense_energy[STRENGTH])
        {
            population[id].sense_energy[STRENGTH] = energy;
            population[id].sense_energy[X_DIRECTION] = dx_to_nuc;
            population[id].sense_energy[Y_DIRECTION] = dy_to_nuc;
        }
        if(free > 0)
        {
            // printf("free: %d\n", free);
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
                if((free_dist < population[id].sense_free[DISTANCE] && free > 0)
                || (free_dist == population[id].sense_free[DISTANCE] && free > population[id].sense_free[STRENGTH]))
                {
                    population[id].sense_free[STRENGTH] = free;
                    population[id].sense_free[DISTANCE] = free_dist;
                    population[id].sense_free[X_DIRECTION] = dx_to_nuc;
                    population[id].sense_free[Y_DIRECTION] = dy_to_nuc;
                }
            }
            else
            {
                int16_t existing_dot = population[id].sense_free[X_DIRECTION] * vx + population[id].sense_free[Y_DIRECTION] * vy;
                
                if(population[id].sense_free[STRENGTH] == 0 || dot > existing_dot)
                {
                    population[id].sense_free[STRENGTH] = free;
                    population[id].sense_free[DISTANCE] = free_dist;
                    population[id].sense_free[X_DIRECTION] = dx_to_nuc;
                    population[id].sense_free[Y_DIRECTION] = dy_to_nuc;
                }
                else if(dot == existing_dot && (free_dist < population[id].sense_free[DISTANCE]
                || free_dist == population[id].sense_free[DISTANCE] && free > population[id].sense_free[STRENGTH])
                
                )
                {
                    population[id].sense_free[STRENGTH] = free;
                    population[id].sense_free[DISTANCE] = free_dist;
                    population[id].sense_free[X_DIRECTION] = dx_to_nuc;
                    population[id].sense_free[Y_DIRECTION] = dy_to_nuc;
                }
            }
        }
        
        for(int f = 0; f < FLAG_AMOUNT; f++)
        {
            if(flag_other[f][STRENGTH] > population[id].flag_other[f][STRENGTH])
            {
                population[id].flag_other[f][STRENGTH] = flag_other[f][STRENGTH];
                population[id].flag_other[f][X_DIRECTION] = dx_to_nuc;
                population[id].flag_other[f][Y_DIRECTION] = dy_to_nuc;
            }
        }
        
        if(friends > population[id].sense_friend[STRENGTH])
        {
            population[id].sense_friend[STRENGTH] = friends;
            population[id].sense_friend[X_DIRECTION] = dx_to_nuc;
            population[id].sense_friend[Y_DIRECTION] = dy_to_nuc;
        }
        
        partner_id = population[id].partner_id;
        partner_hash = population[partner_id].target_hash;
        
        if(population[partner_id].alive == 0) population[id].partner_id = 0;
        
        if(partner_id != 0 && id == population[partner_id].partner_id
         && population[id].sex == 0
         && population[id].solidify != 2 && population[partner_id].solidify != 2
         && population[partner_id].fertilized == 0)
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
    if(debug) fprintf(stderr, "\nId_Count"), fflush(stderr);
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
    if(debug) fprintf(stderr, "\nExpand"), fflush(stderr);
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
                int16_t vx = population[id].velocity[X_DIRECTION];
                int16_t vy = population[id].velocity[Y_DIRECTION];
                uint8_t penetrate = (
                (max(abs(vx) + 1, abs(vy) + 1) * population[id].volume
                 > population[id_to].volume)
                && id_to != 0
                && population[id].attack);
                uint8_t is_full = population[id].material > population[id].min_mat + population[id].volume
                && id_to != 0;
                
                if((penetrate && !is_full && id_to != id
                && (dx != 0 || dy != 0)
                && Grid_Get(x1 + dx, y1 + dy)->energy != 0)
                || id_to == 0
                || id_to == MAX_ORGANISMS)
                {
                    Grid_Set(x1 + dx, y1 + dy, id);
                    expanded = 1;
                    Grid_Add_Cooldown(x1 + dx, y1 + dy, 4);
                    if(penetrate && 0)
                    {
                        population[id].velocity[X_DIRECTION] -= sign(population[id].velocity[X_DIRECTION]);
                        population[id].velocity[Y_DIRECTION] -= sign(population[id].velocity[Y_DIRECTION]);
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
                && Grid_Get(x1 + dx, y1 + dy)->energy != 0)
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
                && Grid_Get(x1 + dx, y1 + dy)->energy != 0
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
    if(debug) fprintf(stderr, "\nOrganism_Update"), fflush(stderr);
    
    Cell *cell;
    
    for(int alive = free_top + 1; alive < MAX_ORGANISMS - 1; alive++)
    {
        uint16_t i = free_stack[alive];
        // if(population[i].alive)
        {
            population[i].nuc_x = mod(population[i].nuc_x, grid_width);
            population[i].nuc_y = mod(population[i].nuc_y, grid_height);
            uint16_t x = population[i].nuc_x;
            uint16_t y = population[i].nuc_y;
            population[i].newborn = 0;
            
            cell = Grid_Get(x, y);
            
            if(cell->id != i)
            {
                if(Organism_Quit(i) == 1)
                    continue;
            }
            // if(population[i].energy >= population[i].max_energy
            // || population[i].energy == 0)
            // {
            //     if(Organism_Quit(i) == 1)
            //         continue;
            // }
            cell->type = 2;
            
            uint16_t flag, flag_other = 16;
            uint32_t step;
            
            uint8_t gp = population[i].gp;
            uint8_t op = population[i].genome[gp];
            
            uint8_t move = population[i].move;
            uint8_t grow = population[i].grow;
            uint8_t shrink = population[i].shrink;
            
            uint8_t multiply = population[i].multiply;
            uint8_t print = 0;
            uint8_t dp = 1; 
            uint8_t command1 = (gp + 1) % GENOME_SIZE;
            uint8_t command2 = (gp + 2) % GENOME_SIZE;
            uint8_t command3 = (gp + 3) % GENOME_SIZE;
            uint8_t command4 = (gp + 4) % GENOME_SIZE;
            uint8_t command5 = (gp + 5) % GENOME_SIZE;
            uint8_t command6 = (gp + 6) % GENOME_SIZE;
            
            uint8_t temp_value;
            uint8_t axis;
            
            uint16_t max_str = max(
            max(
                population[i].max_x - population[i].min_x, 
                population[i].max_y - population[i].min_y),
                1
                );
            // printf("id: %d\n", i);
            
            if(i == test_id)
            {
                print = 1;
                // printf("material: %d\n", population[test_id].material);
            }         
            
            if(1)
            {
            switch (op)
            {
            case SET_VEL: // v
                temp_value = population[i].genome[command1];
                axis = population[i].genome[command2] % 2; // x or y
                
                if(population[i].velocity[axis] + temp_value - 128 <= 127
                && population[i].velocity[axis] + temp_value - 128 >= -128) 
                    population[i].velocity[axis] = population[i].velocity[axis] + temp_value - 128;
                if(population[i].velocity[axis] != 0)
                {
                    population[i].move = 1;
                    move = 1;
                }
                dp = 3;
                if(print) printf("\n SET_VEL ");
                break;
            case ACCEL: // v
                population[i].move = 1;
                move = 1;
                
                if(population[i].velocity[X_DIRECTION] == 0 
                && population[i].velocity[Y_DIRECTION] == 0)
                {
                    population[i].velocity[X_DIRECTION] = rand() % 256 - 128;
                    population[i].velocity[Y_DIRECTION] = rand() % 256 - 128;
                }
            
                if(abs(population[i].velocity[X_DIRECTION]) <= 63 && abs(population[i].velocity[Y_DIRECTION]) <= 63) 
                {
                    population[i].velocity[X_DIRECTION] *= 2;
                    population[i].velocity[Y_DIRECTION] *= 2;
                }
                if(print) printf("\n ACCEL ");
                break;
            case STOP: // v
                population[i].velocity[X_DIRECTION] = 0;
                population[i].velocity[Y_DIRECTION] = 0; 
                if(population[i].velocity[X_DIRECTION] == 0 && population[i].velocity[Y_DIRECTION] == 0)
                {
                    population[i].move = 0;
                    move = 0;
                }
                if(print) printf("\n STOP ");
                break;
            case GROW: // v
                population[i].grow = 1;
                grow = 1;
                
                population[i].move = 1;
                move = 1;
                population[i].signal_timer = abs(population[i].strength);
                if(population[i].velocity[X_DIRECTION] == 0 
                && population[i].velocity[Y_DIRECTION] == 0)
                {
                    population[i].velocity[X_DIRECTION] = rand() % 256 - 128;
                    population[i].velocity[Y_DIRECTION] = rand() % 256 - 128;
                }
                if(print) printf("\n GROW ");
                break;
            case SHRINK: // v
                population[i].shrink = 1;
                shrink = 1;
                
                population[i].move = 1;
                move = 1;
                population[i].signal_timer = abs(population[i].strength);
                if(population[i].velocity[X_DIRECTION] == 0 
                && population[i].velocity[Y_DIRECTION] == 0)
                {
                    population[i].velocity[X_DIRECTION] = rand() % 256 - 128;
                    population[i].velocity[Y_DIRECTION] = rand() % 256 - 128;
                }
                if(print) printf("\n SHRINK ");
                break;
            case SET_STR: // v
                temp_value = (population[i].genome[command1] - 128) * max_str / 128;
                population[i].strength = 
                min(
                    max(population[i].strength + temp_value, 0), max_str
                );
                dp = 2;
                if(print) printf("\n SET_STR ");
                break;
            case CHECK_OTHER: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_other[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_other[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_other[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_OTHER ");
                break;
            case CHECK_WALL: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_wall[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_wall[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_wall[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_WALL ");
                break;
            case CHECK_TARGET: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_material[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_material[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_material[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_TARGET ");
                break;
            case CHECK_ENERGY: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_energy[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_energy[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_energy[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_ENERGY ");
                break;
            case CHECK_PAIN: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_pain[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_pain[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_pain[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_PAIN ");
                break;
            case CHECK_FREE: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_free[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_free[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_free[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_FREE ");
                break;
            case CHECK_FRIEND: // v
                axis = population[i].genome[command2] % 2; // x or y
                if (population[i].sense_friend[STRENGTH] == 0) 
                {
                    dp = 5 + population[i].genome[command1];
                } else if (population[i].sense_friend[axis] < 0)
                {
                    dp = 5 + population[i].genome[command2];
                } else if (population[i].sense_friend[axis] == 0)
                {
                    dp = 5 + population[i].genome[command3];
                }
                else
                    dp = 5 + population[i].genome[command4];
                if(print) printf("\n CHECK_FRIEND ");
                break;
            case CHECK_MAT: // v
                if (population[i].material == 0) 
                {
                    dp = 4 + population[i].genome[command1];          // нет материала
                } else if (population[i].material < population[i].volume) 
                {
                    dp = 4 + population[i].genome[command2];         // материала меньше чем объема
                } else 
                {
                    dp = 4 + population[i].genome[command3];          // материала больше чем объема
                }
                if(print) printf("\n CHECK_MAT ");
                break;
            case CHECK_NRG: // v
                uint32_t max_energy = population[i].max_energy;
                if (population[i].energy < max_energy / 3)
                {
                    dp = 4 + population[i].genome[command1];          // низкая энергия
                } else if (population[i].energy < max_energy * 2 / 3) 
                {
                    dp = 4 + population[i].genome[command2];          // средняя энергия
                } else 
                {
                    dp = 4 + population[i].genome[command3];          // высокая энергия
                }
                if(print) printf("\n CHECK_NRG ");
                break;
            case CHECK_MULT: // v
                if (population[i].multiply == 0) 
                {
                    dp = 3 + population[i].genome[command1];          // не в режиме размножения
                } else 
                {
                    dp = 3 + population[i].genome[command2];          // в режиме размножения
                }
                if(print) printf("\n CHECK_MULT ");
                break;
            case CHECK_VEL: // v
                {
                uint8_t speed = max(abs(population[i].velocity[X_DIRECTION]), abs(population[i].velocity[Y_DIRECTION]));
                if (speed < 43)
                {
                    dp = 4 + population[i].genome[command1];
                } else if (speed < 85)
                {
                    dp = 4 + population[i].genome[command2];
                } else
                {
                    dp = 4 + population[i].genome[command3];
                }
                if(print) printf("\n CHECK_VEL ");
                break;
                }
            case CHECK_VOL: // v
            {
                uint32_t vol = population[i].volume;
                if (vol == 0)
                {
                    dp = 4 + population[i].genome[command1];
                } else if (vol < population[i].material)
                {
                    dp = 4 + population[i].genome[command2];
                } else
                {
                    dp = 4 + population[i].genome[command3];
                }
                if(print) printf("\n CHECK_VOL ");
                break;
            }
            case MULTIPLY: // v
                multiply = 1;
                population[i].multiply = 1;
                if(print) printf("\n MULTIPLY ");
                break;
            case SET_STATE: // v
                population[i].state = population[i].genome[command1] % STATE_AMOUNT;
                if(print) printf("\n SET_STATE ");
                break;
            case IF_STATE: // v
                temp_value = population[i].genome[command1] % STATE_AMOUNT;
                if(population[i].state == temp_value)
                {
                    dp = 4 + population[i].genome[command2];
                } else 
                {
                    dp = 4 + population[i].genome[command3];
                }
                if(print) printf("\n  ");
                break;
            case SET_MUTATION: // v
                temp_value = (population[i].genome[command1] - 128) * GENOME_SIZE / 128;
                population[i].mutate_chance = 
                min(
                    max(population[i].mutate_chance + temp_value, 0), max_str
                );
                dp = 2;
                if(print) printf("\n  ");
                break;
            case SET_PACIFISM: // v
                temp_value = (population[i].genome[command1] - 128) * 64 / 128;
                population[i].pacifism_treshold = 
                min(
                    max(population[i].pacifism_treshold + temp_value, 0), max_str
                );
                dp = 2;
                if(print) printf("\n  ");
                break;
            case STEAL_MAT: // v
                break;
                temp_value = population[i].genome[command1] % 2;
                population[i].steal_mat = temp_value;
                dp = 2;
                if(print) printf("\n STEAL_MAT ");
                break;
            case STEAL_NRG: // v
                temp_value = population[i].genome[command1] % 2;
                population[i].steal_nrg = temp_value;
                dp = 2;
                if(print) printf("\n STEAL_NRG ");
                break;
            case GIVE_MAT: // v
                break;
                temp_value = population[i].genome[command1] % 2;
                population[i].give_mat = temp_value;
                dp = 2;
                if(print) printf("\n GIVE_MAT ");
                break;
            case GIVE_NRG: // v
                temp_value = population[i].genome[command1] % 2;
                population[i].give_nrg = temp_value;
                dp = 2;
                if(print) printf("\n GIVE_NRG ");
                break;
            case ATTACK: // v
                temp_value = population[i].genome[command1] % 2;
                population[i].attack = temp_value;
                dp = 2;
                population[i].attack = 1;
                if(print) printf("\n ATTACK_ON ");
                break;
            case SET_FLAG: // v
                temp_value = population[i].genome[command1] % FLAG_AMOUNT; // which flag
                population[i].flag[temp_value] = population[i].genome[command2];
                if(population[i].energy > 1)
                    population[i].energy = max(population[i].energy - 1, 0);
                dp = 3;
                if(print) printf("\n SET_FLAG ");
                break;
            case CHECK_FLAG: // v
                temp_value = population[i].genome[command1] % FLAG_AMOUNT; // which flag
                axis = population[i].genome[command2] % 2; // x or y
                flag = population[i].flag_other[temp_value][axis];
                if (population[i].flag_other[temp_value][STRENGTH] == 0) 
                {
                    dp = 7 + population[i].genome[command3];
                } else if (flag < 0)
                {
                    dp = 7 + population[i].genome[command4];
                } else if (flag == 0)
                {
                    dp = 7 + population[i].genome[command5];
                } else
                {
                    dp = 7 + population[i].genome[command6];
                }
                if(print) printf("\n CHECK_FLAG ");
                break;
            case CHECK_SEX: // v
                if (population[i].sex == 0) 
                {
                    dp = 3 + population[i].genome[command1];          // самка
                } else 
                {
                    dp = 3 + population[i].genome[command2];          // самец
                }
                if(print) printf("\n CHECK_SEX ");
                break;
            case FLIP_SEX: // v
                if(population[i].fertilized == 0)
                    population[i].sex = 1 - population[i].sex;
                if(print) printf("\n FLIP_SEX ");
                break;
            case SET_TARGET_HASH: // v
                temp_value = (population[i].genome[command1] - 128) * max_pacifism_threshold / 128;
                population[i].target_hash = 
                min(
                    max(population[i].target_hash + temp_value, 0), max_pacifism_threshold
                );
                dp = 2;
                if(print) printf("\n  ");
                break;
            case SOLIDIFY: // v
                temp_value = population[i].genome[command1] % 3;
                
                if(population[i].solidify == 1 && temp_value == 0) 
                {
                    population[i].solidify = 0;
                }
                else if(population[i].solidify == 0 && temp_value == 1) 
                {
                    population[i].solidify = 1;
                }
                else if(population[i].has_reproduced == 0 && population[i].solidify != 2 && temp_value == 2) 
                {
                    population[i].solidify = 2;
                    solidify_count++;
                }
                dp = 2;
                if(print) printf("\n SOLIDIFY ");
                break;
            case PHOTOSYNTHESIS: // v
                if(population[i].photosynthesis == 1) 
                {
                    population[i].photosynthesis = 0;
                }
                if(print) printf("\n PHOTOSYNTHESIS ");
                break;
            
            default:
                dp = op;
                if(print) printf("\n JUMP %d ", dp);
                break;
            }
            }
            population[i].gp = mod(population[i].gp + dp, GENOME_SIZE);
            
            int8_t vx = population[i].velocity[X_DIRECTION];
            int8_t vy = population[i].velocity[Y_DIRECTION];
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
            
            if(population[i].energy >= population[i].max_energy)
            {
                shrink = 0;
                population[i].shrink = 0;
                grow = 0;
                population[i].grow = 0;
            }
            // if(population[i].energy == 0)
            // {
                
            // }
            
            if(population[i].photosynthesis) 
            {
                shrink = 0;
                population[i].shrink = 0;
            }
            
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
                    // population[i].velocity[X_DIRECTION] /= 2;
                    // population[i].velocity[Y_DIRECTION] /= 2;
                    
                    if(Is_Membrane(x, y) == 0)
                    Grid_Signal(population[i].nuc_x, population[i].nuc_y,
                                -vx, -vy,
                                -min(population[i].strength, max_str) - 1); 
                }
                
            }
            
            if (grow && population[i].signal_timer > 0
             && population[i].material > 0 && population[i].energy > medium_energy) 
            {
                Grid_Signal(population[i].nuc_x, population[i].nuc_y,
                            vx, vy,
                            min(population[i].strength, max_str));
                population[i].signal_timer--;
                grid_array[y][x].cooldown = 0;
                if(population[i].signal_timer == 0) population[i].grow = 0;
            }
            if (shrink && population[i].signal_timer > 0) 
            {
                Grid_Signal(population[i].nuc_x, population[i].nuc_y,
                                -vx, -vy,
                                -min(population[i].strength, max_str) - 1); 
                population[i].signal_timer--;
                grid_array[y][x].cooldown = 0;
                if(population[i].signal_timer == 0) population[i].grow = 0;
            }
            
            grid_array[population[i].nuc_y][population[i].nuc_x].type = 1;
            
            
            // if(print)printf("multiply: %d\nsolidify: %d\nfree_str: %d", multiply, population[i].solidify, population[i].free_str);
            if(multiply && population[i].solidify != 2 && population[i].sense_free[STRENGTH] != 0
            && volumetric_energy_mult * population[i].volume > starting_energy_mult * population[i].starting_energy)
            {
                // printf("multiply\n");
                int deltax = population[i].sense_free[X_DIRECTION];
                int deltay = population[i].sense_free[Y_DIRECTION];
                int x = population[i].nuc_x + deltax;
                int y = population[i].nuc_y + deltay;
                
                cell = Grid_Get(x, y);
                
                if(cell->id == i && (deltax != 0 || deltay != 0))
                {
                    Grid_Set(x, y, 0);
                    uint32_t half_material = min(population[i].material, population[i].strength * population[i].material / max_str);
                    uint32_t half_energy = min(population[i].energy, population[i].strength * volumetric_energy_mult * population[i].volume / max_str);
                    
                    if(half_material > 0 &&  half_energy > medium_energy)
                    {
                        
                        
                        uint16_t child_id = 0;
                        child_id = Organism_Init(x, y, half_material, half_energy);
                        
                        if(child_id != 0) {
                            population[i].material -= half_material;
                            population[i].energy -= half_energy;
                            population[i].starting_energy = population[i].starting_energy - half_energy;
                            Child_Genome_Copy(i, child_id, population[i].mutate_chance);
                            population[i].multiply = 0;
                            // printf("multiply success material: %d\n", population[child_id].material);
                            if(population[i].fertilized)
                            {
                                sexual_reproductions++;
                                // printf("mother: %d father: %d\n", i, population[i].partner_id);
                            }
                            else
                                asexual_reproductions++;
                        } else {
                            // printf("multiply fail\n");
                        }
                        multiply = 0;
                        
                        population[i].fertilized = 0;
                        population[i].has_reproduced = 1;
                    }
                }
            }
                
            population[i].sense_material[STRENGTH] = max(population[i].sense_material[STRENGTH] - 1, 0);
            if(population[i].sense_material[STRENGTH] == 0)
            {
                population[i].sense_material[X_DIRECTION] = 0;
                population[i].sense_material[Y_DIRECTION] = 0;
            }
            population[i].sense_energy[STRENGTH] = max(population[i].sense_energy[STRENGTH] - 1, 0);
            if(population[i].sense_energy[STRENGTH] == 0)
            {
                population[i].sense_energy[X_DIRECTION] = 0;
                population[i].sense_energy[Y_DIRECTION] = 0;
            }
            population[i].sense_other[STRENGTH] = max(population[i].sense_other[STRENGTH] - 1, 0);
            if(population[i].sense_other[STRENGTH] == 0)
            {
                population[i].sense_other[X_DIRECTION] = 0;
                population[i].sense_other[Y_DIRECTION] = 0;
            }
            population[i].sense_pain[STRENGTH] = max(population[i].sense_pain[STRENGTH] - 1, 0);
            if(population[i].sense_pain[STRENGTH] == 0)
            {
                population[i].sense_pain[X_DIRECTION] = 0;
                population[i].sense_pain[Y_DIRECTION] = 0;
            }
            population[i].sense_free[STRENGTH] = max(population[i].sense_free[STRENGTH] - 1, 0);
            population[i].sense_free[DISTANCE] = min(population[i].sense_free[DISTANCE] + 1, 65535);
            if(population[i].sense_free[STRENGTH] == 0)
            {
                population[i].sense_free[X_DIRECTION] = 0;
                population[i].sense_free[Y_DIRECTION] = 0;
            }
            population[i].sense_friend[STRENGTH] = max(population[i].sense_friend[STRENGTH] - 1, 0);
            if(population[i].sense_friend[STRENGTH] == 0)
            {
                population[i].sense_friend[X_DIRECTION] = 0;
                population[i].sense_friend[Y_DIRECTION] = 0;
            }
            
            for(int f = 0; f < FLAG_AMOUNT; f++)
            {
                population[i].flag_other[f][STRENGTH] = max(population[i].flag_other[f][STRENGTH] - 3, 0);
                population[i].flag[f] = max(population[i].flag[f] - 1, 0);
                
                if(population[i].flag_other[f][STRENGTH] == 0)
                {
                    population[i].flag_other[f][X_DIRECTION] = 0;
                    population[i].flag_other[f][Y_DIRECTION] = 0;
                }
            }
            
            // if(multiply
            //  && (population[i].material <= 2 * population[i].min_mat
            //  || population[i].energy <= 2 * starting_energy)
            //  )
            // {
            //     multiply = 0;
            //     population[i].multiply = 0;
            // }
            
            
            if(population[i].newborn == 1) 
            {
                if(population[i].volume == 0)
                {
                    printf("volume error!\n");
                }
                population[i].newborn = 0;
            }
            
            // АВТОМАТИЧЕСКАЯ ВОЛНА ЖИЗНИ (каждые N шагов)
            population[i].life_wave_timer++;
            if(population[i].life_wave_timer >= 47)
            {
                // printf("life wave\n");
                population[i].life_wave_timer = 0;
                
                cell->life_wave_str = population[i].perimeter;
            }
            
            // population[i].solidify = 1;
        }
    }
    
    if(free_top >= MAX_ORGANISMS - 2)//MAX_ORGANISMS / re_frac) 
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
    
    // printf("\nfree_top: %d\n", free_top);
    // for(int i = 0; i < MAX_ORGANISMS - 1; i++)
    // {
    //     if(population[free_stack[i]].alive)
    //         printf(GREEN_BG "id: %d" RESET "\n", free_stack[i]);
    //     else 
    //         printf(RED_BG "id: %d" RESET "\n", free_stack[i]);
    // }
}

uint16_t Most_Common_Neighbor(int16_t x, int16_t y)
{
    if(debug) fprintf(stderr, "\nMost_Common_Neighbor"), fflush(stderr);
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
    if(debug) fprintf(stderr, "\nRepopulate"), fflush(stderr);
    if(repopulate == 0) return;
    step_counter = 0;
    
    Best_Genome_Spread();
    
    Grid_Reset(0);
    
    Check_Conservation();
    
    uint16_t x, y;

    
    for(int i = 0; i < STARTING_ORGANISMS * food_mult; i++)
    {
        // x = grid_width / 2 + (rand() % grid_width + rand() % grid_width) / 2;
        // y = grid_height / 2 + (rand() % grid_height + rand() % grid_height) / 2;
        x = rand() % grid_width;
        y = rand() % grid_height;
        Grid_Set_Food(x, y);
        if(rand() % 1 == 1) Grid_Get(x, y)->solid = 1;
    }
    
    for(int i = 0; i < STARTING_ORGANISMS; i++)
    {
        // x = (rand() % grid_width + rand() % grid_width) / 2;
        // y = (rand() % grid_height + rand() % grid_height) / 2;
        x = rand() % grid_width;
        y = rand() % grid_height;
        if(Grid_Get(x, y)->id == 0)
            Organism_Init(x, y, min_mat, starting_energy);
    }
    
    // uint16_t side = fast_root(STARTING_ORGANISMS);
    
    // for(int i = 0; i < side; i++)
    // {
    //     for(int j = 0; j < side; j++)
    //     {
    //         Organism_Init(4 * j, 4 * i, min_mat, starting_energy);
    //     }
    // }
}

void Order_Shuffle()
{
    if(debug) fprintf(stderr, "\nOrder_Shuffle"), fflush(stderr);
    for (uint16_t i = MAX_ORGANISMS - 1; i > 0; i--) 
    {
        uint16_t j = 1 + rand() % i;
        
        uint16_t temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

}

void Stats_CollectAndPrint()
{
    if(statistics == 0) return;
    PopulationStats pop_stats;
    EcologyStats eco_stats;
    
    // Обнуляем структуры
    memset(&pop_stats, 0, sizeof(PopulationStats));
    memset(&eco_stats, 0, sizeof(EcologyStats));
    
    pop_stats.step = step_counter;
    pop_stats.asexual_reproductions = asexual_reproductions;
    pop_stats.sexual_reproductions = sexual_reproductions;
    pop_stats.starvation = starvation;
    pop_stats.overheat = overheat;
    pop_stats.violent = violent;
    
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
        
        if (org->steal_mat || org->steal_nrg) eco_stats.parasite_count++;
        if (org->attack) eco_stats.predator_count++;
        
        if (org->photosynthesis) eco_stats.phototroph_count++;
        else eco_stats.heterotroph_count++;
        
        eco_stats.builder_count = solidify_count;
        
        // Социальные градации
        if (org->pacifism_treshold <= 16) social_egoist++;
        else if (org->pacifism_treshold <= 32) social_low++;
        else if (org->pacifism_treshold <= 48) social_medium++;
        else social_high++;
        
        
        
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
                eco_stats.total_free_food += cell->material;
                eco_stats.total_free_energy += cell->energy;
            }
            if (cell->solid) {
                eco_stats.total_walls++;
            }
        }
    }
    
    // ========== ВЫВОД В КОНСОЛЬ ==========
    printf("\n STEP %u\n", pop_stats.step);
    printf("POPULATION: alive %u\n", pop_stats.alive_count);
    printf("REPRODUCTION: asexual %u sexual %u starvation %u overheat %u violent %u\n",
           pop_stats.asexual_reproductions, pop_stats.sexual_reproductions, 
           pop_stats.starvation, pop_stats.overheat, pop_stats.violent);
    printf("SIZE: avg_vol %u max_vol %u avg_mat %u max_mat %u avg_neighbors %u\n",
           pop_stats.avg_volume, pop_stats.max_volume, pop_stats.avg_material, 
           pop_stats.max_material, pop_stats.avg_neighbors);
    printf("BEHAVIOR: parasites %u predators %u builders %u phototrophs %u heterotrophs %u\n",
           eco_stats.parasite_count, eco_stats.predator_count, eco_stats.builder_count, eco_stats.phototroph_count, eco_stats.heterotroph_count);
    printf("SOCIAL: egoist %u low %u medium %u high %u\n",
           social_egoist, social_low, social_medium, social_high);
    printf("SIZE DIST: tiny %u small %u medium %u large %u giant %u\n",
           eco_stats.tiny_organisms, eco_stats.small_organisms, eco_stats.medium_organisms,
           eco_stats.large_organisms, eco_stats.giant_organisms);
    printf("RESOURCES: food %u energy %u walls %u\n",
           eco_stats.total_free_food, eco_stats.total_free_energy, eco_stats.total_walls);
           
    // ========== ГИСТОГРАММЫ ==========
    #define BAR_WIDTH 64
    
    // 1. Reproduction (asexual, sexual, starvation, solidify)
    printf("\n REPRODUCTION HISTOGRAM\n");
    uint32_t repro_values[5] = {
        pop_stats.asexual_reproductions,
        pop_stats.sexual_reproductions,
        pop_stats.starvation,
        pop_stats.overheat,
        pop_stats.violent
    };
    const char* repro_labels[5] = {"Asexual", "Sexual", "Starvation", "Overheat", "Violent"};
    
    uint32_t repro_max = 1;
    for (int i = 0; i < 5; i++) {
        if (repro_values[i] > repro_max) repro_max = repro_values[i];
    }
    
    for (int i = 0; i < 5; i++) {
        int bar_len = (repro_max > 0) ? (repro_values[i] * BAR_WIDTH / repro_max) : 0;
        printf("%-14s: %8u [", repro_labels[i], repro_values[i]);
        for (int j = 0; j < bar_len; j++) printf("|");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    // 2. Behavior (parasite, predator, builder)
    printf("\n BEHAVIOR HISTOGRAM\n");
    uint32_t behavior_values[5] = {
        eco_stats.parasite_count,
        eco_stats.predator_count,
        eco_stats.builder_count,
        eco_stats.phototroph_count,
        eco_stats.heterotroph_count
    };
    const char* behavior_labels[5] = {"Parasite", "Predator", "Builder", "Phototroph", "Heterotroph"};
    
    uint32_t behavior_max = 1;
    for (int i = 0; i < 5; i++) {
        if (behavior_values[i] > behavior_max) behavior_max = behavior_values[i];
    }
    
    for (int i = 0; i < 5; i++) {
        int bar_len = (behavior_max > 0) ? (behavior_values[i] * BAR_WIDTH / behavior_max) : 0;
        printf("%-14s: %8u [", behavior_labels[i], behavior_values[i]);
        for (int j = 0; j < bar_len; j++) printf("|");
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
        printf("%-14s: %8u [", social_labels[i], social_values[i]);
        for (int j = 0; j < bar_len; j++) printf("|");
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
        printf("%-14s: %8u [", size_labels[i], size_values[i]);
        for (int j = 0; j < bar_len; j++) printf("|");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    // 5. Resources (food, walls, flags)
    printf("\n RESOURCES HISTOGRAM\n");
    uint32_t resource_values[3] = {
        eco_stats.total_free_food,
        eco_stats.total_free_energy,
        eco_stats.total_walls
    };
    const char* resource_labels[3] = {"Food", "Energy", "Walls"};
    
    uint32_t resource_max = 1;
    for (int i = 0; i < 3; i++) {
        if (resource_values[i] > resource_max) resource_max = resource_values[i];
    }
    
    for (int i = 0; i < 3; i++) {
        int bar_len = (resource_max > 0) ? (resource_values[i] * BAR_WIDTH / resource_max) : 0;
        printf("%-14s: %8u [", resource_labels[i], resource_values[i]);
        for (int j = 0; j < bar_len; j++) printf("|");
        for (int j = bar_len; j < BAR_WIDTH; j++) printf(" ");
        printf("]\n");
    }
    
    asexual_reproductions = 0;
    sexual_reproductions = 0;
    starvation = 0;
    overheat = 0;
    violent = 0;
    solidify_count = 0;
}