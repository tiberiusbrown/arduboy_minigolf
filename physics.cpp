#include "game.hpp"

dvec3 ball;

// velocity, scaled 256x unit: range is (-0.5, +0.5) units per step
// range is OK since the ball should never be moving 0.5 units per step anyway
dvec3 ball_vel;

static constexpr int16_t MAX_VEL = (64 << 8);

// gravity acceleration: y vel units per step
static constexpr int16_t GRAVITY = 256;

// ball restitution as a fraction of 256
static constexpr uint8_t RESTITUTION = 128;

void physics_step()
{
    // TODO: check collision and perform restitution here


    ball.x += int8_t(uint16_t(ball_vel.x + 128) >> 8);
    ball.y += int8_t(uint16_t(ball_vel.y + 128) >> 8);
    ball.z += int8_t(uint16_t(ball_vel.z + 128) >> 8);

    ball_vel.y -= GRAVITY;

    ball_vel.x = tclamp<int16_t>(ball_vel.x, -MAX_VEL, MAX_VEL);
    ball_vel.y = tclamp<int16_t>(ball_vel.y, -MAX_VEL, MAX_VEL);
    ball_vel.z = tclamp<int16_t>(ball_vel.z, -MAX_VEL, MAX_VEL);
}
