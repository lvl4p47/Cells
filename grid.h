#ifndef GRID_H
#define GRID_H

#include "utility.h"

#define MAX_ORGANISMS 60000
#define STARTING_ORGANISMS 30000
#define GENOME_SIZE 256

#define STATE_AMOUNT 8
#define FLAG_AMOUNT 8

typedef struct {
    uint16_t id;
    int8_t vx;
    int8_t vy;
    int8_t strength;
    int8_t type;
    uint32_t material;
    uint32_t energy; 
    uint16_t life_wave_str;
    uint8_t flag[FLAG_AMOUNT];
    uint8_t solid;
    uint8_t membrane;
    
    uint8_t  cooldown;
} Cell;

typedef struct {
    uint16_t nuc_x;
    uint16_t nuc_y;
    uint32_t material;
    uint16_t min_mat;
    uint32_t volume;
    uint32_t perimeter;
    uint8_t target_vol;
    uint8_t alive;
    uint8_t multiply;
    uint32_t energy;
    uint32_t starting_energy;
    uint32_t max_energy;
    
    int16_t sense_material[3];
    int16_t sense_energy[3];
    int16_t sense_other[3];
    int16_t sense_wall[3];
    int16_t sense_pain[3];
    int16_t sense_free[4];
    int16_t sense_friend[3];
    
    int16_t flag_other[FLAG_AMOUNT][3];
    
    uint8_t strength;
    uint8_t state;
    int8_t velocity[2];
    uint8_t move;
    uint8_t steal_mat;
    uint8_t steal_nrg;
    uint8_t give_mat;
    uint8_t give_nrg;
    uint8_t attack;
    uint8_t photosynthesis;
    
    uint8_t flag[FLAG_AMOUNT];
    
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
    uint32_t starvation;            // смертей от голода
    uint32_t overheat;              // смертей от перегрева
    uint32_t violent;               // насильственных смертей
} PopulationStats;

typedef struct {
    // Типы организмов
    uint32_t parasite_count;        // с TAKE_MAT_ON
    uint32_t predator_count;        // с ATTACK_ON
    uint32_t social_count;          // с pacifism_threshold > 32
    uint32_t builder_count;         // с SOLIDIFY в геноме
    uint32_t phototroph_count;      // с photosynthesis = 1
    uint32_t heterotroph_count;     // с photosynthesis = 0
    
    // Ресурсы
    uint32_t total_free_food;        // всего еды на поле
    uint32_t total_free_energy;      // всего еды на поле
    uint32_t total_walls;            // всего клеток стен
    
    // Размеры
    uint32_t tiny_organisms;         // volume < 10
    uint32_t small_organisms;        // 10-50
    uint32_t medium_organisms;       // 50-200
    uint32_t large_organisms;        // 200-500
    uint32_t giant_organisms;        // >500
} EcologyStats;

extern uint8_t display_mode;


extern uint16_t grid_width;
extern uint16_t grid_height;
extern Organism population[MAX_ORGANISMS + 1];
extern uint16_t order[MAX_ORGANISMS];
extern uint16_t test_id;
extern uint8_t timer;
extern Cell **grid_array;

typedef enum
{
    X_DIRECTION,
    Y_DIRECTION,
    STRENGTH,
    DISTANCE
} indeces;

typedef enum
{
    SET_VEL,
    ACCEL,
    STOP,
    GROW,
    SHRINK,
    SET_STR,
    CHECK_OTHER,
    CHECK_WALL,
    CHECK_TARGET,
    CHECK_ENERGY,
    CHECK_PAIN,
    CHECK_FREE,
    CHECK_FRIEND,
    CHECK_MAT,
    CHECK_NRG,
    CHECK_MULT,
    CHECK_VEL,
    CHECK_VOL,
    MULTIPLY,
    SET_STATE,
    IF_STATE,
    SET_MUTATION,
    SET_PACIFISM,
    STEAL_MAT,
    STEAL_NRG,
    GIVE_MAT,
    GIVE_NRG,
    ATTACK,
    SET_FLAG,
    CHECK_FLAG,
    CHECK_SEX,
    FLIP_SEX,
    SET_TARGET_HASH,
    SOLIDIFY,
    PHOTOSYNTHESIS,
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
uint32_t Check_Conservation();
void Grid_Update();
void Grid_Signal(int16_t x, int16_t y, int8_t vx, int8_t vy, int8_t strength);
void Grid_Life_Wave(int16_t x, int16_t y, uint16_t strength, uint8_t all_or_nothing);
void Grid_Add_Cooldown(int16_t x, int16_t y, int8_t cd);

uint16_t Organism_Init(int16_t x, int16_t y, uint32_t material, uint32_t energy);
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