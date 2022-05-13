#include "game.hpp"

#ifndef ARDUINO
array<uint8_t, BUF_BYTES> buf;
#endif

void clear_buf()
{
    uint8_t* pa = &buf[0];
    uint8_t* pb = pa + 1024;
    while(pa < pb) *pa++ = 0;
}

#if 1
#include "levels/level_00.hpp"
static int8_t const* const VERTS = LEVEL_00_VERTS;
static constexpr size_t NUM_VERTS = sizeof(LEVEL_00_VERTS) / 3;
static uint8_t const* const FACES = LEVEL_00_FACES;
static constexpr size_t NUM_FACES = sizeof(LEVEL_00_FACES) / 4;
//static constexpr size_t NUM_FACES = 4;
#elif 1
static constexpr int8_t VERTS[] PROGMEM =
{
    -1, 0, 8,
    1, 0, 8,
    0, 0, -8,
};
static constexpr size_t NUM_VERTS = 3;
static constexpr uint8_t FACES[] PROGMEM =
{
    0, 1, 2, 4,
};
static constexpr size_t NUM_FACES = 1;

#else
static constexpr int8_t VERTS[] PROGMEM =
{
    -1, -1, +1,
    -1, +1, +1,
    -1, -1, -1,
    -1, +1, -1,
    +1, -1, +1,
    +1, +1, +1,
    +1, -1, -1,
    +1, +1, -1,
};
static constexpr size_t NUM_VERTS = 8;

static constexpr uint8_t FACES[] PROGMEM =
{
    2-1, 3-1, 1-1, 1,
    4-1, 7-1, 3-1, 3,
    8-1, 5-1, 7-1, 1,
    6-1, 1-1, 5-1, 3,
    7-1, 1-1, 3-1, 2,
    4-1, 6-1, 8-1, 2,
    2-1, 4-1, 3-1, 1,
    4-1, 8-1, 7-1, 3,
    8-1, 6-1, 5-1, 1,
    6-1, 2-1, 1-1, 3,
    7-1, 5-1, 1-1, 2,
    4-1, 2-1, 6-1, 2,
};
static constexpr size_t NUM_FACES = 12;
#endif

void game_loop()
{
    {
        uint8_t btns = poll_btns();

        if(btns & BTN_A)
        {
            // look
            if(btns & BTN_UP   ) pitch += 1;
            if(btns & BTN_DOWN ) pitch -= 1;
            if(btns & BTN_LEFT ) yaw   += 1;
            if(btns & BTN_RIGHT) yaw   -= 1;

            pitch = tclamp<int8_t>(int8_t(pitch), -64, 64);
        }
        else if(btns & BTN_B)
        {
            // rise and fall
            if(btns & BTN_UP  ) cam.y += 24;
            if(btns & BTN_DOWN) cam.y -= 24;
        }
        else
        {
            // move and strafe
            int8_t sinA = fsin(yaw) / 4;
            int8_t cosA = fcos(yaw) / 4;
            if(btns & BTN_UP   ) cam.x -= sinA, cam.z += cosA;
            if(btns & BTN_DOWN ) cam.x += sinA, cam.z -= cosA;
            if(btns & BTN_LEFT ) cam.x -= cosA, cam.z -= sinA;
            if(btns & BTN_RIGHT) cam.x += cosA, cam.z += sinA;
        }

    }

    ball.x = 256 * 15;
    ball.y = 256 * 0.5;
    ball.z = 256 * -1;

    render_scene(VERTS, NUM_VERTS, FACES, NUM_FACES);
}
