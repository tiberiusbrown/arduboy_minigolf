#include "game.hpp"

#ifndef ARDUINO
array<uint8_t, BUF_BYTES> buf;
#endif

#ifndef ARDUINO
void* memcpy_P(void* dst, const void* src, size_t n)
{
    uint8_t* pd = (uint8_t*)dst;
    uint8_t const* ps = (uint8_t const*)src;
    for(size_t i = 0; i < n; ++i)
        *pd++ = pgm_read_byte(ps++);
    return dst;
}
#endif

void clear_buf()
{
    uint8_t* pa = &buf[0];
    uint8_t* pb = pa + 1024;
    while(pa < pb) *pa++ = 0;
}

static void reset_ball()
{
    ball.x = 256 * 16;
    ball.y = 256 * 2;
    ball.z = 256 * -1;

    ball_vel = {};
    ball_vel.z = 256 * 40;

    //ball_vel.x = -256 * 6;
}

void game_setup()
{
    current_level = &LEVELS[0];

    reset_ball();

    yaw = 0;
    pitch = -64;
    cam.x = 256 * 15;
    cam.y = 256 * 20;
    cam.z = 256 * 5;

    cam = { -29, 1760, -515 };
    yaw = 222;
    pitch = -22;
}

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

    for(uint8_t i = 0; i < 4; ++i)
        physics_step();

    if(ball.y < 256 * -10)
        reset_ball();

    render_scene();
}
