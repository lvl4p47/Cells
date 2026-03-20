#include "grid.h"

Cell **grid_array = NULL;
uint16_t grid_width = 0;
uint16_t grid_height = 0;

Organism population[MAX_ORGANISMS + 1];
uint16_t free_stack[MAX_ORGANISMS];
uint16_t order[MAX_ORGANISMS];
uint16_t test_id;

int organism_count = 0;
int16_t free_top = -1;
uint8_t timer = 0;
uint32_t max_pacifism_threshold = (OP_COUNT * GENOME_SIZE * (GENOME_SIZE + 1) / 2);
uint8_t recycle_div = 1;
uint8_t food_mat = 30;
uint16_t min_mat = 30;
uint8_t food_mult = 1;
uint32_t total_mat = 0;
uint16_t lifetime = 2 * GENOME_SIZE;
uint8_t re_frac = 100;
uint8_t debug = 1;


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
    if(debug) fprintf(stderr, "\nGrid_Reset");
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            Grid_Set(j, i, 0);
        }
    }
}

Cell* Grid_Get(int16_t x, int16_t y)
{
    if(debug > 1) fprintf(stderr, "\nGrid_Get");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    return &grid_array[y1][x1];
}

void Grid_Set(int16_t x, int16_t y, uint16_t id)
{
    if(debug) fprintf(stderr, "\nGrid_Set");
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
            printf("weird2\n");
        }
        return;
    }
    
    // СЛУЧАЙ 2: Установка клетки организма (id > 0)
    if(id <= MAX_ORGANISMS && population[id].alive && population[id].material > 0)
    {
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
            
            population[id].volume = min(population[id].volume + 1, 65535);
            return;
        }
        
        // СЛУЧАЙ 2Б: Клетка занята другим организмом
        else if(temp_id <= MAX_ORGANISMS && temp_id != id)
        {
            if(temp_id == MAX_ORGANISMS)
            {
                population[id].material += grid_array[y1][x1].mat - 1;
                population[id].lifetime += grid_array[y1][x1].mat > 0;
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
                    population[id].lifetime += grid_array[y1][x1].mat > 0;
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
                    population[id].lifetime += grid_array[y1][x1].mat;
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
    if(debug) fprintf(stderr, "\nGrid_Set_Food");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    // Прямая установка, минуя Grid_Set
    grid_array[y1][x1].id = MAX_ORGANISMS;
    grid_array[y1][x1].vx = 0;
    grid_array[y1][x1].vy = 0;
    grid_array[y1][x1].strength = 0;
    grid_array[y1][x1].type = 1;
    grid_array[y1][x1].mat = food_mat;
    grid_array[y1][x1].cooldown = 0;
    
    population[MAX_ORGANISMS].volume = 0;
}

void Grid_Update()
{
    freopen("debug.log", "w", stderr);
    if(debug) fprintf(stderr, "\nGrid_Update");
    total_mat = 0;
    uint32_t org_mat = 0, grid_mat = 0, vol_mat = 0, volume = 0;
    uint8_t membrane = 0;
    uint16_t id;
    
    Order_Shuffle();
    
    total_mat = 0;
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
    
    if(total_mat % 10 != 0)
    {
        printf("%d", 1 / 0);
    }    
    
    uint16_t id1;
    for(int id0 = 1; id0 < MAX_ORGANISMS; id0++)
    {
        id1 = order[id0];
        volume = 0;
        if(population[id1].alive && population[id1].newborn == 0)
        {
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
                        
                        if(membrane && population[id1].take_mat)
                        {
                            for(int dy = -1; dy < 2; dy++)
                            {
                                for(int dx = -1; dx < 2; dx++)
                                {
                                    id = Grid_Get(j + dx, i + dy)->id;
                                    if(id != id1 && id != 0
                                    && population[id].alive && id != MAX_ORGANISMS
                                    && population[id].material > 0)
                                    {
                                        population[id].material--;
                                        population[id1].material++;
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
                        id = Grid_Get(j, i)->id;
                        population[id].volume--;
                        
                        if(population[id].alive && population[id].volume == 0)
                        {
                            // printf("reason of death: dry\n");
                            if(Organism_Quit(id) == 1)
                                continue;
                        }
                        
                        Grid_Get(j, i)->id = MAX_ORGANISMS;
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
                            else
                            {
                                uint16_t id = Grid_Get(j, i)->id;
                                population[id].volume--;
                                
                                if(population[id].alive && population[id].volume == 0)
                                {
                                    // printf("reason of death: dry too\n");
                                    if(Organism_Quit(id) == 1)
                                        continue;
                                }
                                
                                Grid_Get(j, i)->id = MAX_ORGANISMS;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // total_mat = 0;
    // org_mat = 0, grid_mat = 0, vol_mat = 0;
    // for(int i = 0; i < grid_height; i++)
    // {
    //     for(int j = 0; j < grid_width; j++)
    //     {
    //         id = Grid_Get(j, i)->id;
    //         if(population[id].volume > 0 || id == MAX_ORGANISMS)
    //             grid_mat += grid_array[i][j].mat;
    //     }
    // }
    
    // for(int i = 1; i < MAX_ORGANISMS; i++)
    // {
    //     if(population[i].alive)
    //     {
    //         org_mat += population[i].material;
    //         vol_mat += population[i].volume;
    //     }
    // }
    // total_mat = org_mat + grid_mat;
    
    // printf("middle vol_mat: %5d org_mat: %5d grid_mat: %5d total_mat: %5d\n", vol_mat, org_mat, grid_mat, total_mat);
    Organism_Update();
    
    timer++;
    if(timer > 0)
    {
        // if(free_top == MAX_ORGANISMS - 2) Repopulate();
        
        timer = 0;
    }
    
    // total_mat = 0;
    // org_mat = 0, grid_mat = 0, vol_mat = 0;
    // for(int i = 0; i < grid_height; i++)
    // {
    //     for(int j = 0; j < grid_width; j++)
    //     {
    //         id = Grid_Get(j, i)->id;
    //         if(population[id].volume > 0 || id == MAX_ORGANISMS)
    //             grid_mat += grid_array[i][j].mat;
    //     }
    // }
    
    // for(int i = 1; i < MAX_ORGANISMS; i++)
    // {
    //     if(population[i].alive)
    //     {
    //         org_mat += population[i].material;
    //         vol_mat += population[i].volume;
    //     }
    // }
    // total_mat = org_mat + grid_mat;
    
    // printf("after  vol_mat: %5d org_mat: %5d grid_mat: %5d total_mat: %5d\n", vol_mat, org_mat, grid_mat, total_mat);
    
}

void Grid_Signal(int16_t x, int16_t y, int8_t vx, int8_t vy, int8_t strength)
{
    if(debug) fprintf(stderr, "\nGrid_Signal");
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
    if(debug) fprintf(stderr, "\nGrid_Life_Wave");
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
    if(debug) fprintf(stderr, "\nGrid_Add_Cooldown");
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    grid_array[y1][x1].cooldown += cd;
}

uint16_t Organism_Init(int16_t x, int16_t y)
{
    if(debug) fprintf(stderr, "\nOrganism_Init");
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
    
    population[id].strength = 1;
    population[id].vx = 0;
    population[id].vy = 0;
    population[id].move = 0;
    population[id].take_mat = 0;
    
    population[id].flag_0 = 0;
    population[id].flag_1 = 0;
    population[id].flag_2 = 0;
    population[id].is_flag_0 = 0;
    population[id].is_flag_1 = 0;
    population[id].is_flag_2 = 0;
    
    population[id].gp = 0;
    population[id].mutate_chance = 1;
    population[id].pacifism_treshold = 1;
    
    population[id].life_wave_timer = 0;
    population[id].signal_timer = 0;
    population[id].grow = 0;
    population[id].shrink = 0;
    
    population[id].newborn = 1;
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
    if(debug) fprintf(stderr, "\nGenome_Init");
    char buf[32];
    if(test)
    {
        printf("\nTEST_GENOME\n");
        snprintf(buf, sizeof(buf), "genomes/test_genome.txt", id);
        
        file_ptr = fopen(buf, "r");
        if(file_ptr == NULL)
        {
            printf("file error id: %d\n", id);
            srand(clock());
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id].genome[i] = rand() % OP_COUNT;
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
            printf("file error id: %d\n", id);
            srand(clock());
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                population[id].genome[i] = rand() % OP_COUNT;
            }
            Genome_Copy(id, id, 1);
        }
        else
        {
            for(int i = 0; i < GENOME_SIZE; i++)
            {
                fscanf(file_ptr, "%d", &integer);
                population[id].genome[i] = integer;
            }
            fclose(file_ptr); 
            Genome_Copy(id, id, 1);
        }
    }
    
    Genome_Hash(id);
}

void Genome_Hash(uint16_t id)
{
    if(debug) fprintf(stderr, "\nGenome_Hash");
    uint16_t hash = 0;
    for (int i = 0; i < GENOME_SIZE; i++) {
        hash += population[id].genome[i] * (GENOME_SIZE - i);
    }
    population[id].genome_hash = hash;
}

void Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate)
{
    if(debug) fprintf(stderr, "\nGenome_Copy");
    srand(clock());
    for(int i = 0; i < GENOME_SIZE; i++)
    {
        population[id2].genome[i]= population[id1].genome[i];
        if(mutate)
        {
            if(rand() % (GENOME_SIZE * 1 / mutate) == 0) population[id2].genome[i] = rand() % OP_COUNT;
        }
    }
    if(mutate)
    {
        if(rand() % (GENOME_SIZE * 10 / mutate) == 0) Mutate_Swap_Blocks(id2);
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
    if(debug) fprintf(stderr, "\nMutate_Swap_Blocks");
    uint8_t len = 2 + rand() % 4;
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
    if(debug) fprintf(stderr, "\nBest_Genome_Spread");
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
            Genome_Copy(current_best, org, GENOME_SIZE / 2);
            
        }
        for(int org = mid; org < top; org++)
        {
            Genome_Copy(current_best, org, 1);
            
        }
        current_best--;
        current_org += MAX_ORGANISMS / re_frac;
    }
}

uint8_t Organism_Quit(uint16_t id)
{
    if(debug) fprintf(stderr, "\nOrganism_Quit");
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
    population[id].vx = 0;
    population[id].vy = 0;
    population[id].move = 0;
    population[id].take_mat = 0;
    population[id].flag_0 = 0;
    population[id].flag_1 = 0;
    population[id].flag_2 = 0;
    population[id].is_flag_0 = 0;
    population[id].is_flag_1 = 0;
    population[id].is_flag_2 = 0;
    population[id].gp = 0;
    population[id].mutate_chance = 1;
    population[id].pacifism_treshold = 1;
    population[id].life_wave_timer = 0;
    population[id].signal_timer = 0;
    population[id].grow = 0;
    population[id].shrink = 0;
    population[id].newborn = 0;
    
    return 1;
}

uint16_t Is_Membrane(int16_t x, int16_t y)
{
    if(debug > 1) fprintf(stderr, "\nIs_Membrane");
    uint8_t id = Grid_Get(x, y)->id;
    uint8_t id1;
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t counter = 0;
    uint16_t other = 0;
    uint16_t target = 0;
    uint16_t free = 0;
    uint16_t free_dist = 65535;
    
    uint16_t flag_0 = 0;
    uint16_t flag_1 = 0;
    uint16_t flag_2 = 0;
    
    for(int dy = -1; dy < 2; dy++)
    {
        for(int dx = -1; dx < 2; dx++)
        {
            id1 = Grid_Get(x1 + dx, y1 + dy)->id;
            if(id1 != id
            && (Grid_Get(x1 + dx, y1 + dy)->lifetime != 0
            && abs(population[id].genome_hash
             - population[id1].genome_hash)
             > (max_pacifism_threshold * population[id].pacifism_treshold / 64)
            || id1 == MAX_ORGANISMS
            || id1 == 0
            || population[id1].alive == 0))
            {
                counter++;
                if(id1 == MAX_ORGANISMS || population[id1].alive == 0) 
                {
                    target += Grid_Get(x1 + dx, y1 + dy)->mat;
                }
                else if(id1 != 0)
                {
                    if(population[id1].take_mat)
                        other += 10;
                    else
                        other += 1;
                    if(population[id1].flag_0)
                        flag_0 += population[id1].flag_0;
                    if(population[id1].flag_1)
                        flag_1 += population[id1].flag_1;
                    if(population[id1].flag_2)
                        flag_2 += population[id1].flag_2;
                }
                else if(id1 == 0)
                {
                    free += 1;
                }
            }
        }
    }
    if(counter == 0
    && Grid_Get(x1, y1)->vx == 0
    && Grid_Get(x1, y1)->vy == 0)
    {
        Grid_Signal(x1, y1, 0, 0, 0);
    }
    if(population[id].alive)
    {
        if(other > population[id].other_str)
        {
            population[id].other_str = other;
            population[id].other_dx = x1 - population[id].nuc_x;
            population[id].other_dy = y1 - population[id].nuc_y;
        }
        if(target > population[id].target_str)
        {
            population[id].target_str = target;
            population[id].target_dx = x1 - population[id].nuc_x;
            population[id].target_dy = y1 - population[id].nuc_y;
        }
        free_dist = max(abs(x1 - population[id].nuc_x), abs(y1 - population[id].nuc_y));
        if((free_dist < population[id].free_dist && free > 0)
         || (free_dist == population[id].free_dist && free > population[id].free_str))
        {
            population[id].free_str = free;
            population[id].free_dist = free_dist;
            population[id].free_dx = x1 - population[id].nuc_x;
            population[id].free_dy = y1 - population[id].nuc_y;
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
    }
    
    return counter;
}

uint16_t Id_Count(int16_t x, int16_t y, uint16_t id)
{
    if(debug) fprintf(stderr, "\nId_Count");
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
    if(debug) fprintf(stderr, "\nExpand");
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
                && id_to != 0);
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
            Grid_Set(x1, y1, 0);
        
        
        
    }
    return str_available;
}

void Organism_Update()
{
    if(debug) fprintf(stderr, "\nOrganism_Update");
    population[MAX_ORGANISMS].material = food_mat * recycle_div;
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
            case GOTO_0:
                population[i].gp = 0;
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
            if(rand() % 256 < move_prob)
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
                && (Is_Membrane(x + dx, y + dy) == 0 || multiply))
                { 
                    population[i].nuc_x = mod(population[i].nuc_x + dx, grid_width);
                    population[i].nuc_y = mod(population[i].nuc_y + dy, grid_height);
                }
                else if(move) // если ядро хочет двигаться, но не может, его надо тормозить
                {
                    population[i].vx -= sign(population[i].vx);
                    population[i].vy -= sign(population[i].vy);
                    
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
            
            
            if(multiply && (Is_Membrane(population[i].nuc_x, population[i].nuc_y)))
            {
                int deltax = rand() % 3 - 1;
                int deltay = rand() % 3 - 1;
                
                for(int t = 0; t < 10; t++)
                {
                    if(Grid_Get(population[i].nuc_x + deltax, population[i].nuc_y + deltay)->id == 0)
                        t = 10;
                    else
                    {
                        deltax = rand() % 3 - 1;
                        deltay = rand() % 3 - 1;
                    }
                }
                
                if(Grid_Get(population[i].nuc_x + deltax, population[i].nuc_y + deltay)->id == 0)
                {
                    if(population[i].material > 2 * population[i].min_mat)
                    {
                        population[i].material -= population[i].min_mat;
                        
                        uint16_t child_id = 0;
                        child_id = Organism_Init(population[i].nuc_x + deltax, population[i].nuc_y + deltay);
                        
                        if(child_id != 0) {
                            Genome_Copy(i, child_id, population[i].mutate_chance);
                            population[i].multiply = 0;
                            // population[i].lifetime = lifetime;
                            // printf("multiply success mat: %d\n", population[child_id].material);
                        } else {
                            population[i].material += population[i].min_mat;
                            // printf("multiply fail\n");
                        }
                    }
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
            
            population[i].is_flag_0 = max(population[i].is_flag_0 - 3, 0);
            population[i].is_flag_1 = max(population[i].is_flag_1 - 3, 0);
            population[i].is_flag_2 = max(population[i].is_flag_2 - 3, 0);
            
            population[i].flag_0 = max(population[i].flag_0 - 1, 0);
            population[i].flag_1 = max(population[i].flag_1 - 1, 0);
            population[i].flag_2 = max(population[i].flag_2 - 1, 0);
            
            // if(grow == 0 && population[i].volume < population[i].material) population[i].material = max(population[i].material - 1, 0);
            
            if(multiply && population[i].material <= 2 * population[i].min_mat)
            {
                multiply = 0;
                population[i].multiply = 0;
            }
            
            uint16_t volume = population[i].volume;
            uint16_t base_volume = 1 * population[i].min_mat;
            uint16_t volume_chance = 0;
            
            uint16_t ratio;
            uint16_t r;
            
            if(volume <= base_volume) {
                volume_chance = 0;
            } else {
                ratio = volume / base_volume;
                r = fast_root(ratio);
                volume_chance = 100 * (r - 1) / r;
            }
            
            uint8_t speed = max(ax, ay);
            
            uint8_t speed_chance = (speed * 100) / 129;

            // Общий шанс не старения — среднее
            uint16_t total_chance = max(volume_chance, speed_chance);

            if(rand() % 101 >= total_chance) {
                population[i].lifetime = max(population[i].lifetime - 1, 0);
            }
            
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
    if(free_top >= MAX_ORGANISMS * (re_frac - 1) / re_frac) Repopulate();
}

uint16_t Most_Common_Neighbor(int16_t x, int16_t y)
{
    if(debug) fprintf(stderr, "\nMost_Common_Neighbor");
    uint16_t id_count[MAX_ORGANISMS];
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
    if(debug) fprintf(stderr, "\nRepopulate");
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

    
    for(int i = 0; i < MAX_ORGANISMS * food_mult; i++)
    {
        x = (rand() % grid_width + 0 * rand() % grid_width) / 1;
        y = (rand() % grid_height + 0 * rand() % grid_height) / 1;
        
        Grid_Set_Food(x, y);
    }
    
    for(int i = 0; i < MAX_ORGANISMS; i++)
    {
        x = (rand() % grid_width + rand() % grid_width) / 2;
        y = (rand() % grid_height + rand() % grid_height) / 2;
        if(Grid_Get(x, y)->id == 0)
            Organism_Init(x, y);
    }
}

void Order_Shuffle()
{
    if(debug) fprintf(stderr, "\nOrder_Shuffle");
    for (int i = MAX_ORGANISMS - 1; i > 0; i--) 
    {
        int j = rand() % (i + 1);
        
        uint16_t temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

}