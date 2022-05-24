#pragma once

static constexpr phys_box LEVEL_03_BOXES[16] PROGMEM =
{
    { { 768, 256, 1024 }, { -1792, -256, 4096 }, 0, 0 },
    { { 2560, 256, 768 }, { 0, -256, 2304 }, 0, 0 },
    { { 768, 256, 2816 }, { 1792, -256, -1280 }, 0, 0 },
    { { 768, 256, 256 }, { 1792, -256, -4864 }, 0, 0 },
    { { 256, 256, 256 }, { 2304, -256, -4352 }, 0, 0 },
    { { 256, 256, 256 }, { 1280, -256, -4352 }, 0, 0 },
    { { 512, 256, 512 }, { 1792, -768, -4352 }, 0, 0 },
    { { 768, 512, 256 }, { 1792, 0, -5376 }, 0, 0 },
    { { 768, 256, 256 }, { 1792, 256, -256 }, 0, 0 },
    { { 768, 256, 256 }, { 1792, 0, -2816 }, 0, 0 },
    { { 768, 256, 462 }, { 1792, 41, 256 }, 0, 24 },
    { { 768, 256, 462 }, { 1792, 41, -768 }, 0, -24 },
    { { 768, 256, 287 }, { 1792, -102, -2419 }, 0, 19 },
    { { 768, 256, 287 }, { 1792, -102, -3213 }, 0, -19 },
    { { 256, 256, 1088 }, { -1971, 256, 2125 }, 224, 0 },
    { { 256, 256, 1088 }, { 1971, 256, 2483 }, 224, 0 },
};
static constexpr dvec3 LEVEL_03_BALL_POS = { -1792, 128, 4352 };
static constexpr dvec3 LEVEL_03_FLAG_POS = { 1792, -256, -4352 };