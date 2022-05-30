#pragma once

static constexpr int8_t LEVEL_12_VERTS[144] PROGMEM =
{
     -30,   +0,  -40,  -30,   +0,  -20,  -20,   +0,  -31,  -21,   +0,  -32,
     -20,   +0,  -29,  -21,   +0,  -28,  -23,   +0,  -32,  -24,   +0,  -31,
     -23,   +0,  -28,  -24,   +0,  -29,  +10,   +0,  +40,  -20,   +0,  -40,
     -20,   +0,  -20,  +10,   +0,  +28,  -10,   +0,  +28,  -10,   +0,  +40,
     -30,   +4,  -40,  -30,   +4,  -20,  +10,   +0,   +0,  -10,   +0,  +16,
     +10,   +0,  +16,  -10,   +6,  +22,  +10,   +6,  +22,  -10,  +12,  -12,
     -30,  +12,  -12,  -30,  +16,  -12,  +10,   +0,  -20,  -10,  +12,   +8,
     +10,  +12,   +8,  -10,   +0,   +0,  +10,   +0,  -40,  +30,   +0,  -20,
     -10,   +0,  -40,  -10,   +4,  -40,  -10,  +16,   +8,  +10,   +4,  -40,
     +30,   +4,  -20,  +10,   +4,   +0,  -10,  +16,  -12,  +10,  +16,   +8,
     +10,   +4,  -20,  -10,   +4,   +0,  -10,  +12,  +16,  +10,  +12,  +16,
      +0,   +0,  -26,   +0,   +0,  -34,   +0,   +4,  -26,   +0,   +4,  -34,

};

static constexpr uint8_t LEVEL_12_FACES[138] PROGMEM =
{
     19,  22,  20,  32,  16,  33,  30,  36,  31,  23,  39,  28,
     29,  40,  26,  19,  21,  22,  32,   0,  16,  30,  35,  36,
     23,  38,  39,  29,  41,  40,   1,  16,   0,  19,  14,  21,
     13,  20,  22,  27,  25,  24,  31,  37,  18,  44,  47,  45,
      1,  17,  16,  27,  34,  25,  31,  36,  37,  44,  46,  47,
     13,  21,  14,  13,  22,  21,   9,   0,   7,  13,  15,  14,
     18,  19,  29,  28,  27,  23,  18,  29,  26,  23,  27,  24,
     18,  26,  31,  26,  30,  32,  31,  26,  30,  27,  43,  42,
      5,   4,  12,  12,   1,   8,   1,   0,   9,  11,   2,   3,
     11,   3,   6,   8,   5,  12,   9,   8,   1,   0,  11,   6,
      0,   6,   7,  13,  10,  15,  18,  20,  19,  32,  11,  12,
     12,  26,  32,  27,  28,  43, 
};

#define LEVEL_12_NUM_FACES { 0, 10, 10, 2, 24, }