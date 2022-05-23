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

uint8_t shots[18];

static constexpr uint8_t PARS[18] PROGMEM =
{
    3, 3, 3,
};

static void reset_ball()
{
    memcpy_P(&ball, &current_level->ball_pos, sizeof(ball));
    ball_vel = {};
    ball_vel_ang = {};
}

void set_level(uint8_t index)
{
    current_level_index = index;
    current_level = &LEVELS[index];
    reset_ball();
    nframe = 0;
    yaw_level = 0;
    state = st::LEVEL;
}

void game_setup()
{
    for(auto& s : shots) s = 0;
    set_level(0);
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
            shots[current_level_index] += 1;
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

        draw_graphic(INFO_BAR, 7, 0, 1, 76);

        {
            uint8_t n = current_level_index + 1;
            if(n >= 10)
            {
                set_number(1, 7, 18);
                set_number(n - 10, 7, 22);
            }
            else
                set_number(n, 7, 22);
        }
        {
            uint8_t n = shots[current_level_index] + 1;
            uint8_t t = 0;
            while(n >= 10)
                n -= 10, ++t;
            if(t != 0)
                set_number(t, 7, 66);
            set_number(n, 7, 70);
        }
        set_number(pgm_read_byte(&PARS[current_level_index]), 7, 42);
    }
}
