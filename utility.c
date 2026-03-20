#include "utility.h"

int min(int a, int b)
{
    if(a < b) return a;
    return b;
}

int max(int a, int b)
{
    if(a > b) return a;
    return b;
}

float power(int a, int n)
{
    float ans = 1;
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            ans *= a;
        }
    }
    if(n < 0)
    {
        for(int i = 0; i < -n; i++)
        {
            ans /= a;
        }
    }
    return ans;
}



int lg(int a, int pow)
{
    int ans = 0;
    int step = 0;
    while(ans < a && step < 1000)
    {
        ans *= pow;
        step++;
    }
    
    return ans;
}