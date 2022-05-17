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
    125, 125, 126, 126, 126, 127, 127, 127, 127,
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

int16_t fsin16(uint16_t angle)
{
    uint8_t angle_hi = uint8_t(angle >> 8);
    int16_t s0 = int16_t(uint16_t(fsin(angle_hi    )) << 8);
    int16_t s1 = int16_t(uint16_t(fsin(angle_hi + 1)) << 8);
    uint8_t f1 = uint8_t(angle);
    uint8_t f0 = 255 - f1;
    int16_t t = int16_t(u24(s24(s0) * f0 + s24(s1) * f1) >> 8);
    return t;
}

int16_t fcos16(uint16_t angle)
{
    return fsin16(angle + 0x4000);
}
