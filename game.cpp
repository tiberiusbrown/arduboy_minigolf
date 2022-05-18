#include "game.hpp"

static void reset_ball()
{
    ball.x = 256 * -7;
    ball.y = 256 * 3;
    ball.z = 256 * -7;

    ball_vel_ang = {};
    ball_vel = {};
    ball_vel.x = 256 * 5;

#if 0
    ball = { 3840, 128, 256 };

    ball_vel_ang = {};
    ball_vel = {};
    ball_vel.z = 256 * -25;
#endif
}

void game_setup()
{
    current_level = &LEVELS[0];

    reset_ball();

    cam = { -1133, 1880, 376 };
    yaw = 53760;
    pitch = 4880;
}

void move_forward(int16_t amount)
{
    int16_t sinA = fmuls16(amount, fsin16(yaw));
    int16_t cosA = fmuls16(amount, fcos16(yaw));
    cam.x += sinA;
    cam.z -= cosA;
}

void move_right(int16_t amount)
{
    int16_t sinA = fmuls16(amount, fsin16(yaw));
    int16_t cosA = fmuls16(amount, fcos16(yaw));
    cam.x += cosA;
    cam.z += sinA;
}
void move_up(int16_t amount)
{
    cam.y += amount;
}

void look_up(int16_t amount)
{
    pitch -= amount;
}
void look_right(int16_t amount)
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
            if(btns & BTN_UP   ) look_up(256);
            if(btns & BTN_DOWN ) look_up(-256);
            if(btns & BTN_LEFT ) look_right(-256);
            if(btns & BTN_RIGHT) look_right(256);

            pitch = tclamp<int16_t>(pitch, -64 * 256, 64 * 256);
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

    physics_step();

    if(ball.y < 256 * -20)
        reset_ball();

    camera_follow_ball();

    render_scene();
}
