#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdlib.h>
#include <time.h>

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
int lg(int a, int pow);

#endif