#include "game.hpp"

static constexpr int8_t SINE_QUAD[65] PROGMEM =
{
      0,   3,   6,   9,  12,  15,  18,  21,
     24,  27,  30,  34,  37,  39,  42,  45,
     48,  51,  54,  57,  60,  62,  65,  68,
     70,  73,  75,  78,  80,  83,  85,  87,
     90,  92,  94,  96,  98, 100, 102, 104,
    106, 107, 109, 110, 112, 113, 115, 116,
    117, 118, 120, 121, 122, 122, 123, 124,
    125, 125, 126, 126, 126, 127, 127, 127, 127
};

int8_t fsin(uint8_t angle)
{
    uint8_t i = angle & 0x3f;
    if(angle & 0x40) i = 64 - i;
    int8_t r = (int8_t)pgm_read_byte(&SINE_QUAD[i]);
    if(angle & 0x80) r = -r;
    return r;
}

int8_t fcos(uint8_t angle)
{
    return fsin(angle + 0x40);
}

static constexpr uint8_t SINE_QUAD_DIFF[65] PROGMEM =
{
      0,   9,  18,  27,  36,  45,  54,  63,
     71,  80,  88,  97, 105, 113, 120, 128,
    135, 142, 149, 156, 162, 168, 174, 179,
    184, 189, 193, 197, 201, 204, 207, 210,
    212, 213, 214, 215, 215, 215, 214, 213,
    211, 209, 206, 202, 199, 194, 189, 184,
    178, 171, 164, 156, 147, 138, 129, 119,
    108,  96,  84,  72,  59,  45,  30,  15,
      0,
};

static FORCEINLINE int16_t fsin16_helper(uint8_t angle)
{
    uint8_t i = angle & 0x3f;
    if(angle & 0x40) i = 64 - i;
    uint16_t x = i * 16;
    uint16_t r = x + pgm_read_byte(&SINE_QUAD_DIFF[i]);
    //r *= 32;
    if(angle & 0x80) r = uint16_t(-r);
    return (int16_t)r;
}

int16_t fsin16(uint16_t angle)
{
    uint8_t angle_hi = uint8_t(angle >> 8);
    uint8_t angle_lo = uint8_t(angle);
    int16_t f0 = fsin16_helper(angle_hi);
    int16_t f1 = fsin16_helper(angle_hi + 1);
    uint16_t t = uint16_t(u24(s24(f1 - f0) * angle_lo) >> 8);
    return f0 + t;
}

int16_t fcos16(uint16_t angle)
{
    return fsin16(angle + 0x4000);
}
