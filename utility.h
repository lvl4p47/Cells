#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdlib.h>
#include <time.h>

#define RED_BG   "\x1b[41m"
#define GREEN_BG "\x1b[42m"
#define RESET    "\x1b[0m"

static inline int16_t mod(int16_t x, uint16_t m) {
    int16_t r = x % m;
    return r >= 0 ? r : r + m;
}
int min(int a, int b);
int max(int a, int b);
static inline int8_t sign(int16_t x) {
    return (x > 0) - (x < 0);
}
float power(int a, int n);
static inline uint16_t fast_root(uint16_t x)
{
    int min = 1, max = x;
    int ans = (min + max) / 2;
    int sqr = ans * ans;
    int step = 0;
    
    while(abs(sqr - x) > 1 && step < 100)
    {
        if(sqr < x) min = ans;
        if(sqr > x) max = ans;
        
        ans = (min + max) / 2;
        sqr = ans * ans;
        step++;
    }

    return ans;
}
static inline uint32_t lg(uint32_t a, uint32_t pow)
{
    int ans = 1;
    int step = 0;
    while(ans < a && step < 1000)
    {
        ans *= pow;
        step++;
    }
    
    return step;
}

#endif