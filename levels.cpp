#include "game.hpp"

static constexpr phys_box make_box(
    double sx, double sy, double sz,
    double x, double y, double z,
    int yaw = 0, int pitch = 0)
{
    return {
        int16_t(sx * 256),
        int16_t(sy * 256),
        int16_t(sz * 256),
        int16_t( x * 256),
        int16_t( y * 256),
        int16_t( z * 256),
        uint8_t(yaw), int8_t(pitch)
    };
}

static constexpr double SQRT_2_OVER_2 = 0.70710678118;

static constexpr phys_box make_box_45(
    double sx, double sy, double sz,
    double x, double y, double z,
    int yaw)
{
    return {
        int16_t(sx * 256 * SQRT_2_OVER_2),
        int16_t(sy * 256),
        int16_t(sz * 256 * SQRT_2_OVER_2),
        int16_t(x * 256),
        int16_t(y * 256),
        int16_t(z * 256),
        uint8_t(yaw), 0
    };
}

#include "levels/level_00.hpp"
#include "levels/level_01.hpp"
#include "levels/level_02.hpp"

#include "levels/level_00_info.hpp"
#include "levels/level_01_info.hpp"
#include "levels/level_02_info.hpp"

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

level_info const LEVELS[3] PROGMEM =
{
    LEVEL_GEOM(00),
    LEVEL_GEOM(01),
    LEVEL_GEOM(02),
};
