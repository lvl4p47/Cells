#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdlib.h>
#include <time.h>

int mod(int a, int t);
int min(int a, int b);
int max(int a, int b);
int sign(int a);
float power(int a, int n);
int fast_root(int val);
int lg(int a, int pow);

#endif