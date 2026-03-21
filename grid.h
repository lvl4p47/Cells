#ifndef GRID_H
#define GRID_H

#include "utility.h"

#define MAX_ORGANISMS 10000
#define GENOME_SIZE 256

typedef struct {
    uint16_t id;
    int8_t vx;
    int8_t vy;
    int8_t strength;
    int8_t type;
    uint32_t mat;
    uint8_t lifetime; 
    uint16_t life_wave_str;
    uint8_t flag_0;
    uint8_t flag_1;
    uint8_t flag_2;
    uint8_t solid;
    
    uint8_t  cooldown;
} Cell;

typedef struct {
    uint16_t nuc_x;
    uint16_t nuc_y;
    uint32_t material;
    uint16_t min_mat;
    uint32_t volume;
    uint8_t target_vol;
    uint8_t alive;
    uint8_t multiply;
    uint16_t lifetime;
    
    int16_t target_dx;
    int16_t target_dy;
    uint16_t target_str;
    int16_t other_dx;
    int16_t other_dy;
    uint16_t other_str;
    int16_t pain_dx;
    int16_t pain_dy;
    uint16_t pain_str;
    int16_t free_dx;
    int16_t free_dy;
    uint16_t free_dist;
    uint16_t free_str;
    int16_t friend_dx;
    int16_t friend_dy;
    uint16_t friend_str;
    int16_t flag_0_dx;
    int16_t flag_0_dy;
    uint16_t flag_0_str;
    int16_t flag_1_dx;
    int16_t flag_1_dy;
    uint16_t flag_1_str;
    int16_t flag_2_dx;
    int16_t flag_2_dy;
    uint16_t flag_2_str;
    
    uint8_t strength;
    uint8_t state;
    int8_t vx;
    int8_t vy;
    uint8_t move;
    uint8_t take_mat;
    uint8_t attack;
    
    uint8_t flag_0;
    uint8_t flag_1;
    uint8_t flag_2;
    
    uint16_t is_flag_0;
    uint16_t is_flag_1;
    uint16_t is_flag_2;
    
    uint8_t genome[GENOME_SIZE];
    uint8_t gp;
    uint32_t genome_hash;
    uint32_t target_hash;
    uint16_t mutate_chance;
    uint32_t pacifism_treshold;
    
    int16_t min_x, min_y;
    int16_t max_x, max_y;
    
    uint8_t life_wave_timer;
    uint16_t signal_timer;
    uint8_t grow;
    uint8_t shrink;
    
    uint8_t newborn;
    
    uint8_t sex;
    uint16_t partner_id;
    uint8_t child_genome[GENOME_SIZE];
    uint8_t fertilized;
    uint8_t has_reproduced;
    uint8_t solidify;
} Organism;

typedef struct {
    uint32_t step;
    uint32_t alive_count;           // живых организмов
    uint32_t avg_neighbors;         // среднее количество соседей
    uint32_t avg_volume;            // средний объём организма
    uint32_t avg_material;          // средний материал на организм
    uint32_t max_volume;            // максимальный объём
    uint32_t max_material;          // максимальный материал
    uint32_t asexual_reproductions; // делений за шаг   
    uint32_t sexual_reproductions;  // рождений за шаг
    uint32_t deaths;                // смертей за шаг
    uint32_t solidify_count;        // затвердеваний за шаг
} PopulationStats;

typedef struct {
    // Типы организмов
    uint32_t parasite_count;        // с TAKE_MAT_ON
    uint32_t predator_count;        // с ATTACK_ON
    uint32_t social_count;          // с pacifism_threshold > 32
    uint32_t builder_count;         // с SOLIDIFY в геноме
    
    // Ресурсы
    uint32_t total_food;             // всего еды на поле
    uint32_t total_walls;            // всего клеток стен
    uint32_t total_flags;            // всего флагов (сумма flag_0/1/2)
    
    // Информация
    uint32_t flag_0_total;           // сумма flag_0
    uint32_t flag_1_total;
    uint32_t flag_2_total;
    
    // Размеры
    uint32_t tiny_organisms;         // volume < 10
    uint32_t small_organisms;        // 10-50
    uint32_t medium_organisms;       // 50-200
    uint32_t large_organisms;        // 200-500
    uint32_t giant_organisms;        // >500
} EcologyStats;



extern uint16_t grid_width;
extern uint16_t grid_height;
extern Organism population[MAX_ORGANISMS + 1];
extern uint16_t order[MAX_ORGANISMS];
extern uint16_t test_id;
extern uint8_t timer;
extern Cell **grid_array;

typedef enum
{
    VX_POS,
    VX_NEG,
    VY_POS,
    VY_NEG,
    ACCEL,
    STOP,
    GROW,
    SHRINK,
    STR_POS,
    STR_NEG,
    STR_ONE,
    STR_MAX,
    GOTO_ZONE_0,
    GOTO_ZONE_1,
    GOTO_ZONE_2,
    SKIP,
    CHECK_OTHER_DX,
    CHECK_OTHER_DY,
    CHECK_TARGET_DX,
    CHECK_TARGET_DY,
    CHECK_PAIN_DX,
    CHECK_PAIN_DY,
    CHECK_FREE_DX,
    CHECK_FREE_DY,
    CHECK_FRIEND_DX,
    CHECK_FRIEND_DY,
    CHECK_MAT,
    CHECK_MULT,
    CHECK_VEL,
    CHECK_VOL,
    MULTIPLY,
    SET_STATE_0,
    SET_STATE_1,
    SET_STATE_2,
    IF_STATE_0,
    IF_STATE_1,
    IF_STATE_2,
    MUTATE_POS,
    MUTATE_NEG,
    PACIFISM_POS,
    PACIFISM_NEG,
    TAKE_MAT_ON,
    TAKE_MAT_OFF,
    ATTACK_ON,
    ATTACK_OFF,
    SET_FLAG_0,
    SET_FLAG_1,
    SET_FLAG_2,
    CHECK_FLAG_0,
    CHECK_FLAG_1,
    CHECK_FLAG_2,
    CHECK_FLAG_0_DX,
    CHECK_FLAG_0_DY,
    CHECK_FLAG_1_DX,
    CHECK_FLAG_1_DY,
    CHECK_FLAG_2_DX,
    CHECK_FLAG_2_DY,
    CHECK_SEX,
    FLIP_SEX,
    TARGET_HASH_POS,
    TARGET_HASH_NEG,
    TARGET_HASH_RAND,
    SOLIDIFY,
    OP_COUNT
} OpCode;

typedef enum {
    SCREENSHOT_MATERIALS,
    SCREENSHOT_WALLS,
    SCREENSHOT_FLAGS
} ScreenshotMode;

void Grid_Init(uint16_t w, uint16_t h);
void Grid_Quit();
void Grid_Reset(uint16_t value);

static inline Cell* Grid_Get(int16_t x, int16_t y) {
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    return &grid_array[y1][x1];
}

void Grid_Set(int16_t x, int16_t y, uint16_t id);
void Grid_Set_Food(uint16_t x, uint16_t y);
void Grid_Update();
void Grid_Signal(int16_t x, int16_t y, int8_t vx, int8_t vy, int8_t strength);
void Grid_Life_Wave(int16_t x, int16_t y, uint16_t strength);
void Grid_Add_Cooldown(int16_t x, int16_t y, int8_t cd);

uint16_t Organism_Init(int16_t x, int16_t y);
void Genome_Init(uint16_t id, uint8_t test);
void Genome_Hash(uint16_t id);
void Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate);
void Child_Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate);
void Mutate_Swap_Blocks(uint16_t id);
void Best_Genome_Spread();
uint8_t Organism_Quit(uint16_t id);
uint16_t Is_Membrane(int16_t x, int16_t y);
void Process_Membrane(int16_t x, int16_t y);
uint16_t Id_Count(int16_t x, int16_t y, uint16_t id);
uint8_t Expand(int16_t x, int16_t y, int8_t strength);
uint16_t Most_Common_Neighbor(int16_t x, int16_t y);
void Repopulate();
void Order_Shuffle();
void Stats_CollectAndPrint();

void Save_Screenshot(const char* filename, int mode);

#endif