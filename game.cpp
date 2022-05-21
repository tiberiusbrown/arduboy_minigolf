#include "game.hpp"

// alternative yaw for non camera uses
// (like aiming, tracking ball velocity)
uint16_t yaw_aim;

uint8_t power_aim;
static constexpr uint8_t MIN_POWER = 4;
static constexpr uint8_t MAX_POWER = 128;

static dvec3 prev_ball;

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
    int16_t sinA = mul_f15_s16(amount, fsin16(yaw));
    int16_t cosA = mul_f15_s16(amount, fcos16(yaw));
    cam.x += sinA;
    cam.z -= cosA;
}

void move_right(int16_t amount)
{
    int16_t sinA = mul_f15_s16(amount, fsin16(yaw));
    int16_t cosA = mul_f15_s16(amount, fcos16(yaw));
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
            above_ball, yaw_aim, pitch, dist, 64, 64);

        uint8_t btns = poll_btns();
        if(btns & BTN_LEFT ) yaw_aim -= 128;
        if(btns & BTN_RIGHT) yaw_aim += 128;

        if(btns & BTN_UP  ) power_aim += 4;
        if(btns & BTN_DOWN) power_aim -= 4;
        power_aim = tclamp(power_aim, MIN_POWER, MAX_POWER);

        if(btns & BTN_A)
        {
            int16_t ys = fsin16(yaw_aim);
            int16_t yc = -fcos16(yaw_aim);
            prev_ball = ball;
            ball_vel.x = mul_f8_s16(ys, power_aim);
            ball_vel.z = mul_f8_s16(yc, power_aim);
            state = st::ROLLING;
        }
    }
    else if(state == st::ROLLING)
    {
        if(physics_step())
            state = st::AIM;
        else if(ball.y < (256 * -20))
        {
            ball = prev_ball;
            ball_vel = {};
            state = st::AIM;
        }
        update_camera_follow_ball(256 * 12, 64, 16);
    }

    render_scene();
    
    if(state == st::AIM)
    {
        static constexpr uint8_t OFFX = 46;
        static constexpr uint8_t OFFY = 24;
        for(uint8_t y = OFFY + 1; y <= OFFY + 34; ++y)
        {
            set_pixel(OFFX + 0, y);
            set_pixel(OFFX + 6, y);
            for(uint8_t x = OFFX + 1; x <= OFFX + 5; ++x)
                clear_pixel(x, y);
        }
        for(uint8_t x = OFFX + 1; x <= OFFX + 5; ++x)
        {
            set_pixel(x, OFFY + 0);
            set_pixel(x, OFFY + 35);
        }
        for(uint8_t i = 0, y = OFFY + 33; i < power_aim; i += 4, --y)
        {
            for(uint8_t x = OFFX + 2; x <= OFFX + 4; ++x)
                set_pixel(x, y);
        }
    }
}
