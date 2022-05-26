#include "game.hpp"

#include "levels/level_00.hpp"
#include "levels/level_01.hpp"
#include "levels/level_02.hpp"
#include "levels/level_03.hpp"
#include "levels/level_04.hpp"
#include "levels/level_05.hpp"

#include "levels/level_00_info.hpp"
#include "levels/level_01_info.hpp"
#include "levels/level_02_info.hpp"
#include "levels/level_03_info.hpp"
#include "levels/level_04_info.hpp"
#include "levels/level_05_info.hpp"

uint8_t leveli;
level_info const* current_level = 0;

#define LEVEL_GEOM(n__) { \
    /* verts     */ LEVEL_##n__##_VERTS, \
    /* faces     */ LEVEL_##n__##_FACES, \
    /* boxes     */ LEVEL_##n__##_BOXES, \
    /* num_verts */ sizeof(LEVEL_##n__##_VERTS) / 3, \
    /* num_faces */ sizeof(LEVEL_##n__##_FACES) / 4, \
    /* num_boxes */ sizeof(LEVEL_##n__##_BOXES) / sizeof(phys_box), \
    /* ball_pos  */ LEVEL_##n__##_BALL_POS, \
    /* flag_pos  */ LEVEL_##n__##_FLAG_POS, \
    }

level_info const LEVELS[NUM_LEVELS] PROGMEM =
{
    LEVEL_GEOM(00),
    LEVEL_GEOM(01),
    LEVEL_GEOM(02),
    LEVEL_GEOM(03),
    LEVEL_GEOM(04),
    LEVEL_GEOM(05),
};
