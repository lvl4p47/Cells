#ifndef GRID_H
#define GRID_H

#include "utility.h"

#define MAX_ORGANISMS 2000
#define GENOME_SIZE 128

typedef struct {
    uint16_t id;
    int8_t vx;
    int8_t vy;
    int8_t strength;
    int8_t type;
    uint16_t mat;
    uint8_t lifetime; 
    uint16_t life_wave_str;
    
    uint8_t  cooldown;
} Cell;

typedef struct {
    uint16_t nuc_x;
    uint16_t nuc_y;
    uint16_t material;
    uint16_t min_mat;
    uint16_t volume;
    uint8_t target_vol;
    uint8_t alive;
    uint8_t multiply;
    uint16_t lifetime;
    
    int8_t target_dx;
    int8_t target_dy;
    int8_t target_str;
    int8_t pain_dx;
    int8_t pain_dy;
    int8_t pain_str;
    uint8_t strength;
    uint8_t state;
    int8_t vx;
    int8_t vy;
    uint8_t move;
    
    uint8_t genome[GENOME_SIZE];
    uint8_t gp;
    uint32_t genome_hash;
    uint16_t mutate_chance;
    uint32_t pacifism_treshold;
    
    int16_t min_x, min_y;
    int16_t max_x, max_y;
    
    uint8_t life_wave_timer;
} Organism;

extern uint16_t grid_width;
extern uint16_t grid_height;
extern Organism population[MAX_ORGANISMS + 1];
extern uint16_t order[MAX_ORGANISMS];

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
    GOTO_0,
    SKIP,
    CHECK_OTHER,
    CHECK_TARGET_DX,
    CHECK_TARGET_DY,
    CHECK_PAIN_DX,
    CHECK_PAIN_DY,
    CHECK_MAT,
    CHECK_MULT,
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
    OP_COUNT
} OpCode;

void Grid_Init(uint16_t w, uint16_t h);
void Grid_Quit();
void Grid_Reset(uint16_t value);
Cell* Grid_Get(int16_t x, int16_t y);
void Grid_Set(int16_t x, int16_t y, uint16_t id);
void Grid_Set_Food(uint16_t x, uint16_t y);
void Grid_Update();
void Grid_Signal(int16_t x, int16_t y, int8_t vx, int8_t vy, int8_t strength);
void Grid_Life_Wave(int16_t x, int16_t y, uint16_t strength);
void Grid_Add_Cooldown(int16_t x, int16_t y, int8_t cd);

uint16_t Organism_Init(int16_t x, int16_t y);
void Genome_Init(uint16_t id);
void Genome_Hash(uint16_t id);
void Genome_Copy(uint16_t id1, uint16_t id2, uint8_t mutate);
void Mutate_Swap_Blocks(uint16_t id);
void Best_Genome_Spread();
void Organism_Quit(uint16_t id);
uint16_t Is_Membrane(int16_t x, int16_t y);
uint16_t Id_Count(int16_t x, int16_t y, uint16_t id);
uint8_t Expand(int16_t x, int16_t y, int8_t strength);
uint16_t Most_Common_Neighbor(int16_t x, int16_t y);
void Repopulate();
void Order_Shuffle();

#endif