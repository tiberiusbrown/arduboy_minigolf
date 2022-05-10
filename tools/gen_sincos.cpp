#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>

int main()
{
    for(int i = 0; i < 65; ++i)
    {
        double x = M_PI / 128 * i;
        double x2 = double(i) / 64;
        printf("%3d,%c", (int)floor((sin(x) - x2) * 1024), i % 8 < 7 ? ' ' : '\n');
    }

    printf("\n\n");

    for(int i = 0; i < 255; ++i)
    {
        int d = int(1.0 / (i + 1) * 65535.5);
        printf("%5d,%c", d, i % 8 < 7 ? ' ' : '\n');
    }

    return 0;
}
