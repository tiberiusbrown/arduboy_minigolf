#pragma once

static constexpr int8_t LEVEL_13_VERTS[153] PROGMEM =
{
     -40,  -10,  +30,  -20,  -10,  +30,  -31,  -10,  +20,  -32,  -10,  +21,
     -29,  -10,  +20,  -28,  -10,  +21,  -32,  -10,  +23,  -31,  -10,  +24,
     -28,  -10,  +23,  -29,  -10,  +24,  +40,   -2,  +20,  -40,  -10,  +20,
     -20,  -10,  +20,  +40,   -2,   +0,  +20,   -2,   +0,  +20,   -2,  +20,
     -40,   -6,  +30,  -20,   -6,  +30,   +6,  -10,  +20,   +6,  -10,   +8,
      -6,  -10,   +8,   -6,  -10,  +20,  -40,  -10,   +0,  -20,  -10,   +0,
     -40,   -6,  +20,  -40,   -6,   +0,  -20,   -6,  +20,  -20,   -6,   +0,
     -40,   -2,  -20,  -20,   -2,  -20,   +0,   +6,  -20,  -20,   -2,  -40,
      +0,   +6,  -40,  +20,   +6,  -20,  +20,   +6,  -40,  +40,   +6,  -20,
     -34,  -10,   +8,  -26,  -10,   +8,  -34,   -6,   +8,  -26,   -6,   +8,
     +10,   +6,  -34,  +10,   +6,  -26,  +10,  +10,  -34,  +10,  +10,  -26,
     -40,   +2,  -20,  -20,   +2,  -40,  +20,  +10,  -40,  +40,  +10,  -20,
      +0,  +10,  -20,  +20,  +10,  -20,   +0,  +10,  -40, 
};

static constexpr uint8_t LEVEL_13_FACES[132] PROGMEM =
{
      1,  16,   0,  36,  39,  37,  30,  49,  33,  34,  50,  32,
      1,  17,  16,  36,  38,  39,  30,  48,  49,  34,  46,  50,
     22,  24,  11,  12,  27,  23,  40,  43,  41,  35,  46,  34,
     31,  44,  28,  22,  25,  24,  12,  26,  27,  40,  42,  43,
     35,  47,  46,  31,  45,  44,  23,  28,  22,  29,  32,  31,
     14,  35,  13,  23,  29,  28,  29,  30,  32,  14,  33,  35,
      9,   0,   7,  14,  10,  15,  20,  18,  21,  12,  22,  11,
     28,  29,  31,  30,  34,  32,  34,  33,  35,   5,   4,  12,
     12,   1,   8,   1,   0,   9,  11,   2,   3,  11,   3,   6,
      8,   5,  12,   9,   8,   1,   0,  11,   6,   0,   6,   7,
     14,  13,  10,  20,  19,  18,  12,  23,  22,  30,  33,  34,

};

#define LEVEL_13_NUM_FACES { 0, 8, 10, 6, 20, }