#include "game.hpp"

// alternative yaw for non camera uses
// (like aiming, tracking ball velocity)
uint16_t yaw_aim;

uint8_t power_aim;
static constexpr uint8_t MIN_POWER = 4;
static constexpr uint8_t MAX_POWER = 128;

static dvec3 prev_ball;

st state;
static uint16_t nframe;
static uint16_t yaw_level;

static uint8_t prev_btns;

static void reset_ball()
{
    memcpy_P(&ball, &current_level->ball_pos, sizeof(ball));
    ball_vel = {};
    ball_vel_ang = {};
}

void game_setup()
{
    current_level = &LEVELS[2];

    reset_ball();

    cam = { -1133, 1880, 376 };
    yaw = 53760;
    pitch = 4880;

    state = st::LEVEL;
    nframe = 0;
    yaw_level = 0;
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
    uint8_t btns = poll_btns();
    uint8_t pressed = btns & ~prev_btns;
    prev_btns = btns;

    if(state == st::LEVEL)
    {
        update_camera_look_at_fastangle(
            { 0, 0, 0 }, yaw_level, 6000, 256 * 25, 64, 64);
        yaw_level += 256;
        if(++nframe >= 256)
            state = st::AIM;
        if(pressed & BTN_B)
            state = st::AIM;
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

        if(pressed & BTN_B)
        {
            yaw_level = yaw_aim;
            nframe = 0;
            state = st::LEVEL;
        }
    }
    else if(state == st::ROLLING)
    {
        if(physics_step())
        {
            yaw_aim = yaw_to_flag();
            state = st::AIM;
        }
        else if(ball.y < (256 * -20))
        {
            ball = prev_ball;
            ball_vel = {};
            ball_vel_ang = {};
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
