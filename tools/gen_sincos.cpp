#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>

int main()
{
    for(int i = 0; i < 64; ++i)
    {
        printf("%5d,%c", (int)floor(sin(M_PI / 128 * i) * 32767.5), i % 8 < 7 ? ' ' : '\n');
    }
    return 0;
}
