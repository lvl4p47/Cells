#include "utility.h"

int mod(int a, int t)
{
    int ans;
    ans = a % t;
	ans += ans < 0 ? t : 0;
    return ans;
}

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

int sign(int a)
{
    if(a > 0) return 1;
    if(a < 0) return -1;
    return 0;
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

int fast_root(int val)
{
    int min = 1, max = val;
    int ans = (min + max) / 2;
    int sqr = ans * ans;
    int step = 0;
    
    while(abs(sqr - val) > 1 && step < 100)
    {
        if(sqr < val) min = ans;
        if(sqr > val) max = ans;
        
        ans = (min + max) / 2;
        sqr = ans * ans;
        step++;
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