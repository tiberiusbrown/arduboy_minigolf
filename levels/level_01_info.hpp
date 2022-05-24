#pragma once

static constexpr phys_box LEVEL_01_BOXES[12] PROGMEM =
{
    { { 768, 256, 2304 }, { 768, -256, 1536 }, 0, 0 },
    { { 1536, 256, 768 }, { -1536, -256, 0 }, 0, 0 },
    { { 768, 256, 768 }, { -2304, -256, -1536 }, 0, 0 },
    { { 256, 256, 256 }, { -2816, -256, -2560 }, 0, 0 },
    { { 256, 256, 256 }, { -1792, -256, -2560 }, 0, 0 },
    { { 768, 256, 256 }, { -2304, -256, -3072 }, 0, 0 },
    { { 768, 256, 256 }, { -2304, 0, -3584 }, 0, 0 },
    { { 512, 256, 512 }, { -2304, -768, -2560 }, 0, 0 },
    { { 1086, 256, 181 }, { -2432, 0, 128 }, 224, 0 },
    { { 1086, 256, 181 }, { 896, 0, -128 }, 224, 0 },
    { { 512, 512, 256 }, { -768, 256, 1024 }, 0, 0 },
    { { 512, 512, 256 }, { -768, 256, -1024 }, 0, 0 },
};
static constexpr dvec3 LEVEL_01_BALL_POS = { 768, 128, 3072 };
static constexpr dvec3 LEVEL_01_FLAG_POS = { -2304, -256, -2560 };
