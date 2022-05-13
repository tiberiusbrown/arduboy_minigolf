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

void game_setup()
{
    ball.x = 256 * 15;
    ball.y = 256 * 5;
    ball.z = 256 * -1;

    yaw = 0;
    pitch = 0;
    cam.x = 256 * 15;
    cam.y = 2 * 256;
    cam.z = 256 * -12;
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

    physics_step();

    current_level = &LEVELS[0];
    render_scene();
}
