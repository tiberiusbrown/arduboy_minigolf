#include "game.hpp"

#include "levels/level_00.hpp"

level_info const* current_level = 0;

#define LEVEL_GEOM(n__) { \
    LEVEL_##n__##_VERTS, \
    LEVEL_##n__##_FACES, \
    sizeof(LEVEL_##n__##_VERTS) / 3, \
    sizeof(LEVEL_##n__##_FACES) / 4, \
    }

level_info const LEVELS[1] PROGMEM =
{
    LEVEL_GEOM(00),
};
