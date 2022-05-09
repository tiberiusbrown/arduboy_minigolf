#include "game.hpp"

static constexpr int8_t SINE_QUAD[64] PROGMEM =
{
      0,   3,   6,   9,  12,  15,  18,  21,
     24,  27,  30,  34,  37,  39,  42,  45,
     48,  51,  54,  57,  60,  62,  65,  68,
     70,  73,  75,  78,  80,  83,  85,  87,
     90,  92,  94,  96,  98, 100, 102, 104,
    106, 107, 109, 110, 112, 113, 115, 116,
    117, 118, 120, 121, 122, 122, 123, 124,
    125, 125, 126, 126, 126, 127, 127, 127,
};

int8_t fsin(uint8_t angle)
{
    uint8_t i = angle & 0x3f;
    if(angle & 0x40) i = 63 - i;
    int8_t r = (int8_t)pgm_read_byte(&SINE_QUAD[i]);
    if(angle & 0x80) r = -r;
    return r;
}

int8_t fcos(uint8_t angle)
{
    angle += 64;
    uint8_t i = angle & 0x3f;
    if(angle & 0x40) i = 63 - i;
    int8_t r = (int8_t)pgm_read_byte(&SINE_QUAD[i]);
    if(angle & 0x80) r = -r;
    return r;
}

static constexpr int16_t SINE_QUAD_16[64] PROGMEM =
{
        0,   804,  1607,  2410,  3211,  4011,  4807,  5601,
     6392,  7179,  7961,  8739,  9511, 10278, 11039, 11792,
    12539, 13278, 14009, 14732, 15446, 16151, 16845, 17530,
    18204, 18867, 19519, 20159, 20787, 21402, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
    27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
    32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
};

static int16_t fsin16_helper(uint8_t angle)
{
    uint8_t i = angle & 0x3f;
    if(angle & 0x40) i = 63 - i;
    int16_t r = (int16_t)pgm_read_word(&SINE_QUAD_16[i]);
    if(angle & 0x80) r = -r;
    return r;
}

int16_t fsin16(uint16_t angle)
{
    uint8_t angle_reduced = uint8_t(angle >> 8);
    uint8_t f = uint8_t(angle);
    int16_t v0 = fsin16_helper(angle_reduced + 0);
    int16_t v1 = fsin16_helper(angle_reduced + 1);
    return v0 + int32_t(v1 - v0) * f / 256;
}

int16_t fcos16(uint16_t angle)
{
    return fsin16(uint16_t(angle + (64 << 8)));
}
