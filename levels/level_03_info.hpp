#pragma once

static constexpr phys_box LEVEL_03_BOXES[15] PROGMEM =
{
    { { 48, 16, 112 }, { -28, -4, 52 }, 0, 0 },
    { { 112, 16, 48 }, { 12, -4, 36 }, 0, 0 },
    { { 48, 16, 176 }, { 28, -4, -20 }, 0, 0 },
    { { 48, 16, 16 }, { 28, -4, -76 }, 0, 0 },
    { { 16, 16, 16 }, { 20, -4, -68 }, 0, 0 },
    { { 16, 16, 16 }, { 36, -4, -68 }, 0, 0 },
    { { 48, 32, 0 }, { 28, 0, -80 }, 0, 0 },
    { { 68, 32, 0 }, { -28, 0, 36 }, 32, 0 },
    { { 68, 32, 0 }, { 28, 0, 36 }, 32, 0 },
    { { 48, 0, 29 }, { 28, 4, 6 }, 0, 24 },
    { { 48, 0, 29 }, { 28, 4, -14 }, 0, -24 },
    { { 48, 32, 16 }, { 28, 0, -4 }, 0, 0 },
    { { 48, 16, 16 }, { 28, 0, -44 }, 0, 0 },
    { { 48, 0, 18 }, { 28, 2, -36 }, 0, 19 },
    { { 48, 0, 18 }, { 28, 2, -52 }, 0, -19 },
};
static constexpr dvec3 LEVEL_03_BALL_POS = { -1792, 128, 4352 };
static constexpr dvec3 LEVEL_03_FLAG_POS = { 1792, -256, -4352 };
