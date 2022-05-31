#pragma once

static constexpr int8_t LEVEL_16_VERTS[159] PROGMEM =
{
     -40,   +6,  -40,  -40,   +6,  -20,  -32,   +6,  -31,  -33,   +6,  -32,
     -32,   +6,  -29,  -33,   +6,  -28,  -35,   +6,  -32,  -36,   +6,  -31,
     -35,   +6,  -28,  -36,   +6,  -29,  -20,   +0,  +40,  -32,   +6,  -40,
     -32,   +6,  -20,  -20,   +0,  +30,   +0,   +0,  +40,  -28,   +6,  -20,
     -28,   +6,  -40,   +0,   +0,  +30,  -20,   +0,  -20,  -20,   +0,  -40,
     -40,  +10,  -40,  -40,  +10,  -20,   +0,   +0,  +10,  +20,   +0,  +30,
      +0,   +6,   +0,  +20,   +0,  +10,  +40,   +0,  +10,  +20,   -6,   +0,
     +40,   -6,   +0,  -20,   +4,  +30,  +20,   +6,   +0,   +0,   +0,  -40,
      +0,   +6,  -20,  +20,   +6,  -20,  +20,   -6,  -20,  +10,   -6,   +0,
     +10,   -6,  -20,   +0,   +0,   +0,   +0,   +0,  -20,   +0,   +6,  -40,
     -20,   +4,  -20,  -20,   +4,  -40,   +0,   +4,  +10,  +20,   +4,  +30,
      +0,  +10,   +0,  +40,   +4,  +10,  +40,   -2,   +0,   +0,   +4,  -40,
      +0,  +10,  -20,  +20,  +10,  -20,  +20,   -2,  -20,   +0,   +4,   +0,
      +0,  +10,  -40, 
};

static constexpr uint8_t LEVEL_16_FACES[153] PROGMEM =
{
      1,  20,   0,  24,  48,  32,  31,  41,  19,   1,  21,  20,
     24,  44,  48,  31,  47,  41,  26,  43,  23,  34,  46,  28,
     13,  42,  22,  37,  40,  18,  39,  49,  33,  26,  45,  43,
     34,  50,  46,  13,  29,  42,  37,  51,  40,  39,  52,  49,
     15,  19,  16,  22,  30,  25,  25,  28,  26,  35,  38,  36,
     15,  18,  19,  22,  24,  30,  25,  27,  28,  35,  37,  38,
      7,   9,   1,  12,  16,  11,  17,  10,  13,  17,  13,  22,
     22,  23,  17,  23,  25,  26,  18,  31,  19,  27,  36,  34,
     24,  33,  30,  28,  27,  34,  38,  37,  18,  33,  32,  39,
      5,   4,  12,   0,  11,   6,  11,   2,   3,  11,   3,   6,
      5,  12,   8,  12,   1,   8,   0,   6,   7,   9,   8,   1,
      0,   7,   1,  12,  15,  16,  17,  14,  10,  22,  25,  23,
     18,  38,  31,  27,  35,  36,  24,  32,  33, 
};

#define LEVEL_16_NUM_FACES { 0, 6, 10, 8, 27, }