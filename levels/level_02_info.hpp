#pragma once

static constexpr phys_box LEVEL_02_BOXES[14] PROGMEM =
{
    { { 768, 256, 1792 }, { 3840, -256, 0 }, 0, 0 },
    { { 1664, 256, 768 }, { 1408, -256, -1024 }, 0, 0 },
    { { 1055, 512, 181 }, { 3968, 0, -1152 }, 224, 0 },
    { { 384, 512, 384 }, { 1408, 0, 128 }, 0, 0 },
    { { 384, 512, 384 }, { 1408, 0, -2176 }, 0, 0 },
    { { 768, 576, 576 }, { -1024, -256, -1024 }, 64, 19 },
    { { 1408, 256, 768 }, { -2688, 256, -1024 }, 0, 0 },
    { { 768, 256, 512 }, { -3328, 256, 256 }, 0, 0 },
    { { 1086, 512, 256 }, { -3509, 512, -1205 }, 32, 0 },
    { { 256, 256, 512 }, { -2816, 256, 1280 }, 0, 0 },
    { { 256, 256, 256 }, { -3328, 256, 1536 }, 0, 0 },
    { { 256, 256, 512 }, { -3840, 256, 1280 }, 0, 0 },
    { { 512, 256, 512 }, { -3328, -256, 1024 }, 0, 0 },
    { { 768, 512, 256 }, { -3328, 512, 2048 }, 0, 0 },
};
static constexpr dvec3 LEVEL_02_BALL_POS = { 3840, 128, 1024 };
static constexpr dvec3 LEVEL_02_FLAG_POS = { -3328, 256, 1024 };