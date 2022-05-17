#include <stdio.h>
#include <cmath>

#include "../game.hpp"
uint8_t poll_btns() { return 0; }

int main()
{
    float worst_fdiff = 0;
    uint16_t worst_x = 0;
    for(int i = 256; i < 65536; ++i)
    {
        uint16_t x = uint16_t(i);
        uint16_t y = inv16(x);

        int act_y = (1 << 24) / x;
        uint32_t diff = std::abs(y - act_y);
        float fdiff = float(diff) / x;
        if(fdiff > worst_fdiff)
            worst_fdiff = fdiff, worst_x = x;
    }

    uint16_t x = worst_x;
    uint16_t y = inv16(x);
    int act_y = (1 << 24) / x;
    uint32_t diff = std::abs(y - act_y);
    float fdiff = float(diff) / x;
    printf("x     = %d\n", (int)x);
    printf("y     = %d\n", (int)y);
    printf("act y = %d\n", int((1 << 24) / x));
    printf("diff  = %d\n", (int)diff);
    printf("fdiff = %f\n", fdiff);

    return 0;
}
