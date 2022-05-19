#include "game.hpp"

// alternative yaw for non camera uses
// (like aiming, tracking ball velocity)
uint16_t yaw2;

st state;

static void reset_ball()
{
    memcpy_P(&ball, &current_level->ball_pos, sizeof(ball));
    ball_vel = {};
}

void game_setup()
{
    current_level = &LEVELS[0];

    reset_ball();

    cam = { -1133, 1880, 376 };
    yaw = 53760;
    pitch = 4880;

    state = st::AIM;
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
#if 0
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
#endif

    if(state == st::LEVEL)
    {

    }
    else if(state == st::AIM)
    {
        dvec3 above_ball = ball;
        above_ball.y += (256 * 2);
        int16_t pitch = 4096;
        uint16_t dist = 256 * 6;
        update_camera_look_at_fastangle(
            above_ball, yaw2, pitch, dist, 64, 64);

        uint8_t btns = poll_btns();
        uint8_t amount = (btns & BTN_B) ? 16 : 255;
        if(btns & BTN_LEFT ) yaw2 -= amount;
        if(btns & BTN_RIGHT) yaw2 += amount;

        if(btns & BTN_A)
        {
            uint16_t power2 = 256 * 128; // this is max
            int16_t ys = fsin16(yaw2);
            int16_t yc = -fcos16(yaw2);
            ball_vel.x = int16_t(uint32_t(int32_t(ys) * power2) >> 16);
            ball_vel.z = int16_t(uint32_t(int32_t(yc) * power2) >> 16);
            state = st::ROLLING;
        }
    }
    else if(state == st::ROLLING)
    {
        if(physics_step())
            state = st::AIM;
        else if(ball.y < (256 * -20))
        {
            reset_ball();
            state = st::AIM;
        }
        update_camera_follow_ball(256 * 12, 64, 16);
    }

    render_scene();
}
