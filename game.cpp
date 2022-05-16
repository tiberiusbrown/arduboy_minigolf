#include "game.hpp"

static void reset_ball()
{
    ball.x = 256 * 12;
    ball.y = 256 * 1;
    ball.z = 256 * -7;

    ball_vel = {};
    ball_vel.x = 256 * -40;

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
    cam.z = 256 * -5;

    cam = { 256 * -5, 1048, 449 };
    yaw = 0;
    pitch = 21;
}

void move_forward(int16_t amount)
{
    int16_t sinA = int16_t((s24(amount) * fsin(yaw)) >> 7);
    int16_t cosA = int16_t((s24(amount) * fcos(yaw)) >> 7);
    cam.x += sinA;
    cam.z -= cosA;
}

void move_right(int16_t amount)
{
    int16_t sinA = int16_t((s24(amount) * fsin(yaw)) >> 7);
    int16_t cosA = int16_t((s24(amount) * fcos(yaw)) >> 7);
    cam.x += cosA;
    cam.z += sinA;
}
void move_up(int16_t amount)
{
    cam.y += amount;
}

void look_up(int8_t amount)
{
    pitch -= amount;
}
void look_right(int8_t amount)
{
    yaw += amount;
}

void game_loop()
{
    {
        uint8_t btns = poll_btns();

        if(btns & BTN_A)
        {
            // look
            if(btns & BTN_UP   ) look_up(1);
            if(btns & BTN_DOWN ) look_up(-1);
            if(btns & BTN_LEFT ) look_right(-1);
            if(btns & BTN_RIGHT) look_right(1);

            pitch = tclamp<int8_t>(int8_t(pitch), -64, 64);
        }
        else if(btns & BTN_B)
        {
            // rise and fall
            if(btns & BTN_UP  ) move_up(64);
            if(btns & BTN_DOWN) move_up(-64);
        }
        else
        {
            // move and strafe
            int8_t sinA = fsin(yaw) / 4;
            int8_t cosA = fcos(yaw) / 4;
            if(btns & BTN_UP   ) move_forward(64);
            if(btns & BTN_DOWN ) move_forward(-64);
            if(btns & BTN_LEFT ) move_right(-64);
            if(btns & BTN_RIGHT) move_right(64);
        }

    }

    for(uint8_t i = 0; i < 4; ++i)
        physics_step();

    if(ball.y < 256 * -10)
        reset_ball();

    render_scene();
}
